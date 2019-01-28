//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12SM6WaveIntrinsics.h"
#include "UILayer.h"
#include "wave_vs.hlsl.h"
#include "wave_ps.hlsl.h"
#include "magnify_vs.hlsl.h"
#include "magnify_ps.hlsl.h"


// Note that Windows 10 Creator Update SDK is required for enabling Shader Model 6 feature.
static HRESULT EnableExperimentalShaderModels() {
    static const GUID D3D12ExperimentalShaderModelsID = { /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
        0x76f5573e,
        0xf13a,
        0x40f5,
        { 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
    };

    return D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModelsID, nullptr, nullptr);
}

D3D12SM6WaveIntrinsics::D3D12SM6WaveIntrinsics(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_pCbSrvDataBegin(nullptr),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_fenceValues{},
    m_rtvDescriptorSize(0),
    m_cbSrvDescriptorSize(0),
    m_constantBufferData{},
    m_mousePosition{ width*0.5f, height*0.5f },
    m_mouseLeftButtonDown(false),
    m_rendermode{ 1 }
{
    ThrowIfFailed(DXGIDeclareAdapterRemovalSupport());
}

void D3D12SM6WaveIntrinsics::OnInit()
{
    LoadPipeline();
    LoadAssets();
}


void D3D12SM6WaveIntrinsics::CreateDevice(const ComPtr<IDXGIFactory4>& factory)
{
    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_d3d12Device)
        ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_d3d12Device)
        ));
    }
}


// Load the rendering pipeline dependencies.
void D3D12SM6WaveIntrinsics::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    // Create DXGIFactory.
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    // Create device.
    CreateDevice(factory);

    // Query the level of support of Shader Model.
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModelSupport = { D3D_SHADER_MODEL_6_0 };
    ThrowIfFailed(m_d3d12Device->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_SHADER_MODEL, &shaderModelSupport, sizeof(shaderModelSupport)));
    // Query the level of support of Wave Intrinsics.
    ThrowIfFailed(m_d3d12Device->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &m_WaveIntrinsicsSupport, sizeof(m_WaveIntrinsicsSupport)));

    // If the device doesn't support SM6 or Wave Intrinsics, try enabling the experimental feature for Shader Model 6 and creating the device again.
    if (shaderModelSupport.HighestShaderModel != D3D_SHADER_MODEL_6_0 || m_WaveIntrinsicsSupport.WaveOps != TRUE)
    {
        m_d3d12Device.Reset();
        ThrowIfFailed(EnableExperimentalShaderModels());
        CreateDevice(factory);

        // Query the level of support of Shader Model.
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModelSupport = { D3D_SHADER_MODEL_6_0 };
        ThrowIfFailed(m_d3d12Device->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_SHADER_MODEL, &shaderModelSupport, sizeof(shaderModelSupport)));
        // Query the level of support of Wave Intrinsics.
        ThrowIfFailed(m_d3d12Device->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &m_WaveIntrinsicsSupport, sizeof(m_WaveIntrinsicsSupport)));

        // If the device still doesn't support SM6 or Wave Intrinsics after enabling the experimental feature, you could set up your application to use the highest supported shader model.
        // For simplicity we just exit the application here. 
        if (shaderModelSupport.HighestShaderModel != D3D_SHADER_MODEL_6_0 || m_WaveIntrinsicsSupport.WaveOps != TRUE)
        {
            exit(-1);
        }
    }    
    
    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    NAME_D3D12_OBJECT(m_commandQueue);

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;

    ThrowIfFailed(factory->CreateSwapChainForCoreWindow(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
        &swapChainDesc,
        nullptr,
        &swapChain
    ));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount + 2;    // swap chain back buffers + 1 intermediate UI render buffer + 1 intermediate scene buffer
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a constant buffer view and shader resource view descriptor heap.
        // Flags indicate that this descriptor heap can be bound to the pipeline 
        // and that descriptors contained in it can be referenced by a root table.
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 3;  // 1 constant buffer and 2 SRV. 
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbSrvHeap)));

        m_cbSrvDescriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Create a command allocator for each back buffer in the swapchain.
    for (UINT n = 0; n < FrameCount; ++n)
    {
        ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
    }
}

// Load the sample assets.
void D3D12SM6WaveIntrinsics::LoadAssets()
{
    // Create root signatures.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];

        // Root signature for render pass1.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_d3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_renderPass1RootSignature)));
    }

    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        CD3DX12_ROOT_PARAMETER1 rootParameters[2];

        if (FAILED(m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }
        // Root signature for render pass2
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_d3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_renderPass2RootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

        // Define the vertex input layout for render pass 1. 
        D3D12_INPUT_ELEMENT_DESC renderPass1InputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC renderPass1PSODesc = {};
        renderPass1PSODesc.InputLayout = { renderPass1InputElementDescs, _countof(renderPass1InputElementDescs) };
        renderPass1PSODesc.pRootSignature = m_renderPass1RootSignature.Get();
        renderPass1PSODesc.VS = { g_Wave_VS, sizeof(g_Wave_VS) };
        renderPass1PSODesc.PS = { g_Wave_PS, sizeof(g_Wave_PS) };
        renderPass1PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        renderPass1PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        renderPass1PSODesc.DepthStencilState.DepthEnable = FALSE;
        renderPass1PSODesc.DepthStencilState.StencilEnable = FALSE;
        renderPass1PSODesc.SampleMask = UINT_MAX;
        renderPass1PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        renderPass1PSODesc.NumRenderTargets = 1;
        renderPass1PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        renderPass1PSODesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_d3d12Device->CreateGraphicsPipelineState(&renderPass1PSODesc, IID_PPV_ARGS(&m_renderPass1PSO)));
        NAME_D3D12_OBJECT(m_renderPass1PSO);

        // Define the vertex input layout for render pass 2. 
        D3D12_INPUT_ELEMENT_DESC renderPass2InputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC renderPass2PSODesc = {};
        renderPass2PSODesc.InputLayout = { renderPass2InputElementDescs, _countof(renderPass2InputElementDescs) };
        renderPass2PSODesc.pRootSignature = m_renderPass2RootSignature.Get();
        renderPass2PSODesc.VS = { g_Magnify_VS, sizeof(g_Magnify_VS) };
        renderPass2PSODesc.PS = { g_Magnify_PS, sizeof(g_Magnify_PS) };
        renderPass2PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        renderPass2PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        renderPass2PSODesc.DepthStencilState.DepthEnable = FALSE;
        renderPass2PSODesc.DepthStencilState.StencilEnable = FALSE;
        renderPass2PSODesc.SampleMask = UINT_MAX;
        renderPass2PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        renderPass2PSODesc.NumRenderTargets = 1;
        renderPass2PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        renderPass2PSODesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_d3d12Device->CreateGraphicsPipelineState(&renderPass2PSODesc, IID_PPV_ARGS(&m_renderPass2PSO)));
        NAME_D3D12_OBJECT(m_renderPass2PSO);
    }

    // Create the command list.
    ThrowIfFailed(m_d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    NAME_D3D12_OBJECT(m_commandList);

    // Create a constant buffer.
    {
        ThrowIfFailed(m_d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer)));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbHandle(m_cbSrvHeap->GetCPUDescriptorHandleForHeapStart());
        m_d3d12Device->CreateConstantBufferView(&cbvDesc, cbHandle);

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbSrvDataBegin)));
        memcpy(m_pCbSrvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    }

    LoadSizeDependentResources();

    // Close the command list and execute it to begin the vertex buffer copy into
    // the default heap.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_d3d12Device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_frameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForGpu();
    }
}


void D3D12SM6WaveIntrinsics::LoadSizeDependentResources()
{
    // Create the vertex buffer for render pass 1.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.5f , 0.0f },{ 0.8f, 0.8f, 0.0f, 1.0f } },
            { { 0.5f, -0.5f , 0.0f },{ 0.0f, 0.8f, 0.8f, 1.0f } },
            { { -0.5f, -0.5f , 0.0f },{ 0.8f, 0.0f, 0.8f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        ThrowIfFailed(m_d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_renderPass1VertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_renderPass1VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_renderPass1VertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_renderPass1VertexBufferView.BufferLocation = m_renderPass1VertexBuffer->GetGPUVirtualAddress();
        m_renderPass1VertexBufferView.StrideInBytes = sizeof(Vertex);
        m_renderPass1VertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Create the vertex buffer for render pass 2.
    {
        // Define the geometry for a rectangle.
        Vertex2 triangleVertices[] =
        {
            { { -1.0f*m_aspectRatio, -1.0f , 0.0f },{ 0.0f, 1.0f } },
            { { -1.0f*m_aspectRatio, 1.0f , 0.0f },{ 0.0f, 0.0f } },
            { { 1.0f*m_aspectRatio, 1.0f , 0.0f },{ 1.0f, 0.0f } },

            { { -1.0f*m_aspectRatio, -1.0f , 0.0f },{ 0.0f, 1.0f } },
            { { 1.0f*m_aspectRatio, 1.0f , 0.0f },{ 1.0f, 0.0f } },
            { { 1.0f*m_aspectRatio, -1.0f , 0.0f },{ 1.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        ThrowIfFailed(m_d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_renderPass2VertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_renderPass2VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_renderPass2VertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_renderPass2VertexBufferView.BufferLocation = m_renderPass2VertexBuffer->GetGPUVirtualAddress();
        m_renderPass2VertexBufferView.StrideInBytes = sizeof(Vertex2);
        m_renderPass2VertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV and a command allocator for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderPass2RenderTargets[n])));
            m_d3d12Device->CreateRenderTargetView(m_renderPass2RenderTargets[n].Get(), nullptr, rtvHandle);
            NAME_D3D12_OBJECT_INDEXED(m_renderPass2RenderTargets, n);

            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }

        // Create texture resources for render pass1. The first render pass will render the visualizations of wave intrinsics to an intermediate texture. 
        // In render pass 2, it will blend UI layer and this intermediate texture and then use a magifier effect on the blended texture.
        {
            // Create the texture.
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.MipLevels = 1;
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.Width = m_width;
            textureDesc.Height = m_height;
            textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            textureDesc.DepthOrArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            const float ccolor[4] = { 0, 0, 0, 0 };
            CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R8G8B8A8_UNORM, ccolor);

            m_d3d12Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                &clearValue,
                IID_PPV_ARGS(&m_renderPass1RenderTargets));
            NAME_D3D12_OBJECT(m_renderPass1RenderTargets);

            // Create RTV for the texture
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
            rtvHandle.Offset(2, m_rtvDescriptorSize); // First two are referencing to swapchain back buffers. 
            m_d3d12Device->CreateRenderTargetView(m_renderPass1RenderTargets.Get(), nullptr, rtvHandle);

            // Create SRV for the texture
            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_cbSrvHeap->GetCPUDescriptorHandleForHeapStart());
            srvHandle.Offset(1, m_cbSrvDescriptorSize); // First one is for constant buffer.
            m_d3d12Device->CreateShaderResourceView(m_renderPass1RenderTargets.Get(), nullptr, srvHandle);
        }

        // Create the UI render target and an RTV for it.
        {
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.MipLevels = 1;
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.Width = m_width;
            textureDesc.Height = m_height;
            textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            textureDesc.DepthOrArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            const float ccolor[4] = { 0, 0, 0, 0 };
            CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R8G8B8A8_UNORM, ccolor);

            ThrowIfFailed(m_d3d12Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                &clearValue,
                IID_PPV_ARGS(&m_uiRenderTarget))
            );
            NAME_D3D12_OBJECT(m_uiRenderTarget);

            // Create RTV for UI layer texture
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
            rtvHandle.Offset(3, m_rtvDescriptorSize); // First three are referencing to renderPass1 and renderPass2 buffers. 
            m_d3d12Device->CreateRenderTargetView(m_uiRenderTarget.Get(), nullptr, rtvHandle);

            // Create SRV for UI layer texture
            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_cbSrvHeap->GetCPUDescriptorHandleForHeapStart());
            srvHandle.Offset(2, m_cbSrvDescriptorSize); // First one is for constant buffer. Senond one is for a texture in renderPass1
            m_d3d12Device->CreateShaderResourceView(m_uiRenderTarget.Get(), nullptr, srvHandle);

            if (!m_uiLayer)
            {
                m_uiLayer = std::make_shared<UILayer>(this);
            }
            else
            {
                m_uiLayer->Resize();
            }
        }

        m_viewport.Width = static_cast<float>(m_width);
        m_viewport.Height = static_cast<float>(m_height);

        m_scissorRect.left = 0;
        m_scissorRect.top = 0;
        m_scissorRect.right = static_cast<LONG>(m_width);
        m_scissorRect.bottom = static_cast<LONG>(m_height);
    }
}


// Update frame-based values.
void D3D12SM6WaveIntrinsics::OnUpdate()
{
    static float time = 0;
    m_constantBufferData.orthProjMatrix = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f*m_aspectRatio, 2.0f, 0.0f, 1.0f));  // Transpose from row-major to col-major, which by default is used in HLSL.
    m_constantBufferData.renderingMode = m_rendermode;
    m_constantBufferData.laneSize = m_WaveIntrinsicsSupport.WaveLaneCountMin;
    m_constantBufferData.time = time;
    m_constantBufferData.mousePosition.x = m_mousePosition[0];
    m_constantBufferData.mousePosition.y = m_mousePosition[1];
    m_constantBufferData.resolution.x = static_cast<float>(m_width);
    m_constantBufferData.resolution.y = static_cast<float>(m_height);
    memcpy(m_pCbSrvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    //time += 0.1f;

    m_uiLayer->UpdateLabels(m_rendermode);
}

// Render the scene.
void D3D12SM6WaveIntrinsics::OnRender()
{
    try
    {
        RenderUI();
        // Record all the commands we need to render the scene into the command list.
        RenderScene();

        // Present the frame.
        ThrowIfFailed(m_swapChain->Present(1, 0));

        MoveToNextFrame();
    }
    catch (HrException& e)
    {
        if (e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET)
        {
            RestoreD3DResources();
        }
        else
        {
            throw;
        }
    }
}

// Release sample's D3D objects.
void D3D12SM6WaveIntrinsics::ReleaseD3DResources()
{
    m_fence.Reset();
    m_renderPass1RenderTargets.Reset();
    m_uiRenderTarget.Reset();
    ResetComPtrArray(&m_renderPass2RenderTargets);
    m_commandQueue.Reset();
    m_swapChain.Reset();
    m_d3d12Device.Reset();
}

// Tears down D3D resources and reinitializes them.
void D3D12SM6WaveIntrinsics::RestoreD3DResources()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        WaitForGpu();
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    ReleaseD3DResources();
    OnInit();
}

void D3D12SM6WaveIntrinsics::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}

void D3D12SM6WaveIntrinsics::RenderScene()
{
    // Render Pass 1. Render the scene (triangle) to an intermediate texture.
    // Render Pass 2. Compose the intermediate texture from render pass1 and UI layer together.

    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_renderPass1PSO.Get()));

    // Render Pass 1: Render the scene (triangle) to an intermediate texture.
    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_renderPass1RootSignature.Get());
    ID3D12DescriptorHeap* ppHeaps[] = { m_cbSrvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_commandList->SetGraphicsRootDescriptorTable(0, m_cbSrvHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Set up render target
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderPass1RenderTargets.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
    CD3DX12_CPU_DESCRIPTOR_HANDLE renderPass1RtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &renderPass1RtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0, 0, 0, 0 };
    m_commandList->ClearRenderTargetView(renderPass1RtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_renderPass1VertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderPass1RenderTargets.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Render Pass 2: Merge UI layer and the intermediate texture from render pass 1 together.
    m_commandList->SetPipelineState(m_renderPass2PSO.Get());
    m_commandList->SetGraphicsRootSignature(m_renderPass2RootSignature.Get());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuCbvDescriptorHandle(m_cbSrvHeap->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvDescriptorHandle(m_cbSrvHeap->GetGPUDescriptorHandleForHeapStart());
    gpuSrvDescriptorHandle.Offset(1, m_cbSrvDescriptorSize);
    m_commandList->SetGraphicsRootDescriptorTable(0, gpuCbvDescriptorHandle);
    m_commandList->SetGraphicsRootDescriptorTable(1, gpuSrvDescriptorHandle);
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Set up render target
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderPass2RenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    CD3DX12_CPU_DESCRIPTOR_HANDLE pass2RtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &pass2RtvHandle, FALSE, nullptr);

    // Record commands.
    m_commandList->ClearRenderTargetView(pass2RtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_renderPass2VertexBufferView);
    m_commandList->DrawInstanced(6, 2, 0, 0);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderPass2RenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Transit the ui texture back to render target from pixel shader resource.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_uiRenderTarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

    ThrowIfFailed(m_commandList->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}


void D3D12SM6WaveIntrinsics::RenderUI()
{
    m_uiLayer->Render();
}


// Wait for pending GPU work to complete.
void D3D12SM6WaveIntrinsics::WaitForGpu()
{
    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12SM6WaveIntrinsics::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void D3D12SM6WaveIntrinsics::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        m_rendermode = key - 0x30; // 0x30 = '0'
        break;
    default:
        break;
    }
}

void D3D12SM6WaveIntrinsics::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    UNREFERENCED_PARAMETER(minimized);
    UpdateForSizeChange(width, height);

    if (!m_swapChain)
    {
        return;
    }

    // Flush all current GPU commands.
    WaitForGpu();

    // Release the resources holding references to the swap chain (requirement of
    // IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
    // current fence value.
    m_renderPass1RenderTargets.Reset();
    m_uiRenderTarget.Reset();
    for (UINT n = 0; n < FrameCount; n++)
    {
        m_renderPass2RenderTargets[n].Reset();
        m_fenceValues[n] = m_fenceValues[m_frameIndex];
    }
    m_uiLayer->ReleaseResources();


    // Resize the swap chain to the desired dimensions.
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    m_swapChain->GetDesc1(&desc);
    ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.Format, desc.Flags));

    // Reset the frame index to the current back buffer index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    LoadSizeDependentResources();
}

void D3D12SM6WaveIntrinsics::OnMouseMove(UINT x, UINT y)
{
    // Only update the zoom-in area while mouse left button is pressed.
    if(m_mouseLeftButtonDown)
    {
        m_mousePosition[0] = static_cast<float>(x);
        m_mousePosition[1] = static_cast<float>(y);        
    }
}

void D3D12SM6WaveIntrinsics::OnLeftButtonDown(UINT /*x*/, UINT /*y*/)
{
    m_mouseLeftButtonDown = true;
}

void D3D12SM6WaveIntrinsics::OnLeftButtonUp(UINT /*x*/, UINT /*y*/)
{
    m_mouseLeftButtonDown = false;
}