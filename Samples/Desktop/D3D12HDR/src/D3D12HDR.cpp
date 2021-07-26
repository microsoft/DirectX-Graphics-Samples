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
#include "D3D12HDR.h"
#include "UILayer.h"
#include <dxgidebug.h>

// Precompiled shaders.
#include "gradientVS.hlsl.h"
#include "gradientPS.hlsl.h"
#include "paletteVS.hlsl.h"
#include "palettePS.hlsl.h"
#include "presentVS.hlsl.h"
#include "presentPS.hlsl.h"

const float D3D12HDR::ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float D3D12HDR::HDRMetaDataPool[4][4] =
{
    // MaxOutputNits, MinOutputNits, MaxCLL, MaxFALL
    // These values are made up for testing. You need to figure out those numbers for your app.
    { 1000.0f, 0.001f, 2000.0f, 500.0f },
    { 500.0f, 0.001f, 2000.0f, 500.0f },
    { 500.0f, 0.100f, 500.0f, 100.0f },
    { 2000.0f, 1.000f, 2000.0f, 1000.0f }
};

D3D12HDR::D3D12HDR(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(),
    m_scissorRect(0, 0, width, height),
    m_swapChainFormats{ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT },
    m_currentSwapChainColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
    m_currentSwapChainBitDepth(_8),
    m_swapChainFormatChanged(false),
    m_intermediateRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT),
    m_rtvDescriptorSize(0),
    m_srvDescriptorSize(0),
    m_dxgiFactoryFlags(0),
    m_rootConstants{},
    m_updateVertexBuffer(true),
    m_fenceValues{},
    m_windowVisible(true),
    m_windowedMode(true)
{
    // Alias the root constants so that we can easily set them as either floats or UINTs.
    m_rootConstantsF = reinterpret_cast<float*>(m_rootConstants);
}

void D3D12HDR::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12HDR::LoadPipeline()
{
#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    NAME_D3D12_OBJECT(m_commandQueue);

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = m_swapChainFormats[m_currentSwapChainBitDepth];
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    // It is recommended to always use the tearing flag when it is available.
    swapChainDesc.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    if (m_tearingSupport)
    {
        // When tearing support is enabled we will handle ALT+Enter key presses in the
        // window message loop rather than let DXGI handle it by calling SetFullscreenState.
        m_dxgiFactory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER);
    }

    ThrowIfFailed(swapChain.As(&m_swapChain));
    
    // Check display HDR support and initialize ST.2084 support to match the display's support.
    CheckDisplayHDRSupport();
    m_enableST2084 = m_hdrSupport;
    EnsureSwapChainColorSpace(m_currentSwapChainBitDepth, m_enableST2084);
    SetHDRMetaData(HDRMetaDataPool[m_hdrMetaDataPoolIdx][0], HDRMetaDataPool[m_hdrMetaDataPoolIdx][1], HDRMetaDataPool[m_hdrMetaDataPoolIdx][2], HDRMetaDataPool[m_hdrMetaDataPoolIdx][3]);

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount + 2;    // A descriptor for each frame + 2 intermediate render targets.
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // Describe and create a shader resource view (SRV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 2;                    // A descriptor for each of the 2 intermediate render targets.
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Create a command allocator for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
    }
}

// Load the sample assets.
void D3D12HDR::LoadAssets()
{
    // Create a root signature containing root constants for brightness information
    // and the desired output curve as well as a SRV descriptor table pointing to the
    // intermediate render targets.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

        CD3DX12_ROOT_PARAMETER rootParameters[2];
        rootParameters[0].InitAsConstants(4, 0);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[0]);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state objects for the different views and render target formats
    // as well as the intermediate blend step.
    {
        // Create the pipeline state for the scene geometry.

        D3D12_INPUT_ELEMENT_DESC gradientElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // Describe and create the graphics pipeline state objects (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { gradientElementDescs, _countof(gradientElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_gradientVS, sizeof(g_gradientVS));
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_gradientPS, sizeof(g_gradientPS));
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = m_intermediateRenderTargetFormat;
        psoDesc.SampleDesc.Count = 1;

        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[GradientPSO])));

        // Create pipeline state for the color space triangles.

        D3D12_INPUT_ELEMENT_DESC colorElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        psoDesc.InputLayout = { colorElementDescs, _countof(colorElementDescs) };
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_paletteVS, sizeof(g_paletteVS));
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_palettePS, sizeof(g_palettePS));
        psoDesc.RTVFormats[0] = m_intermediateRenderTargetFormat;

        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[PalettePSO])));

        // Create pipeline states for the final blend step.
        // There will be one for each swap chain format the sample supports.

        D3D12_INPUT_ELEMENT_DESC quadElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        psoDesc.InputLayout = { quadElementDescs, _countof(quadElementDescs) };
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_presentVS, sizeof(g_presentVS));
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_presentPS, sizeof(g_presentPS));
        psoDesc.RTVFormats[0] = m_swapChainFormats[_8];
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[Present8bitPSO])));

        psoDesc.RTVFormats[0] = m_swapChainFormats[_10];
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[Present10bitPSO])));

        psoDesc.RTVFormats[0] = m_swapChainFormats[_16];
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[Present16bitPSO])));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    {
        // Create geometry for the different sections of the render target.
        GradientVertex gradientVertices[] =
        {
            // Upper strip. SDR Gradient from [0,1].

            { { -1.0f, 0.45f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
            { { -1.0f, 0.55f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
            { { 0.0f, 0.45f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
            { { 0.0f, 0.55f, 0.0f }, { 1.0f, 1.0f, 1.0f } },

            // Lower strip. HDR Gradient from [0,9]. Perceptually, 9.0 is about 3 times as bright as 1.0. (See gradientPS.hlsl.)

            { { -1.0f, -0.55f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
            { { -1.0f, -0.45f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
            { { 0.0f, -0.55f, 0.0f }, { 3.0f, 3.0f, 3.0f } },
            { { 0.0f, -0.45f, 0.0f }, { 3.0f, 3.0f, 3.0f } },
        };

        // The vertices for the color space triangles are dependent on the size of the
        // render target and will not be loaded at this time. We'll leave a gap in the
        // buffer that will be filled in later.
        const UINT triangleVerticesSize = sizeof(TrianglesVertex) * TrianglesVertexCount;
        m_updateVertexBuffer = true;

        PresentVertex presentVertices[] =
        {
            // 1 triangle that fills the entire render target.

            { { -1.0f, -3.0f, 0.0f }, { 0.0f, 2.0f } },    // Bottom left
            { { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },    // Top left
            { { 3.0f, 1.0f, 0.0f }, { 2.0f, 0.0f } },    // Top right
        };

        const UINT vertexBufferSize = sizeof(gradientVertices) + triangleVerticesSize + sizeof(presentVertices);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBufferUpload)));

        // Copy data to the intermediate upload heap. It will be uploaded to the
        // DEFAULT buffer when the color space triangle vertices are updated.
        UINT8* mappedUploadHeap = nullptr;
        ThrowIfFailed(m_vertexBufferUpload->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&mappedUploadHeap)));

        memcpy(mappedUploadHeap, gradientVertices, sizeof(gradientVertices));
        mappedUploadHeap += sizeof(gradientVertices) + triangleVerticesSize;
        memcpy(mappedUploadHeap, presentVertices, sizeof(presentVertices));

        m_vertexBufferUpload->Unmap(0, &CD3DX12_RANGE(0, 0));

        // Initialize the vertex buffer views.
        m_gradientVertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_gradientVertexBufferView.StrideInBytes = sizeof(GradientVertex);
        m_gradientVertexBufferView.SizeInBytes = sizeof(gradientVertices);

        m_trianglesVertexBufferView.BufferLocation = m_gradientVertexBufferView.BufferLocation + m_gradientVertexBufferView.SizeInBytes;
        m_trianglesVertexBufferView.StrideInBytes = sizeof(TrianglesVertex);
        m_trianglesVertexBufferView.SizeInBytes = triangleVerticesSize;

        m_presentVertexBufferView.BufferLocation = m_trianglesVertexBufferView.BufferLocation + m_trianglesVertexBufferView.SizeInBytes;
        m_presentVertexBufferView.StrideInBytes = sizeof(PresentVertex);
        m_presentVertexBufferView.SizeInBytes = sizeof(presentVertices);
    }

    LoadSizeDependentResources();

    // Close the command list and execute it to begin the vertex buffer copy into
    // the default heap.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
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

// Load resources that are dependent on the size of the main window.
void D3D12HDR::LoadSizeDependentResources()
{
    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);
        }

        // Create the intermediate render target and an RTV for it.
        D3D12_RESOURCE_DESC renderTargetDesc = m_renderTargets[0]->GetDesc();
        renderTargetDesc.Format = m_intermediateRenderTargetFormat;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = m_intermediateRenderTargetFormat;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &renderTargetDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_intermediateRenderTarget)));

        NAME_D3D12_OBJECT(m_intermediateRenderTarget);

        m_device->CreateRenderTargetView(m_intermediateRenderTarget.Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
        m_device->CreateShaderResourceView(m_intermediateRenderTarget.Get(), nullptr, srvHandle);
        srvHandle.Offset(1, m_srvDescriptorSize);

        // Create the UI render target and an RTV for it.
        renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        clearValue.Format = renderTargetDesc.Format;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &renderTargetDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_UIRenderTarget)));

        NAME_D3D12_OBJECT(m_UIRenderTarget);

        m_device->CreateRenderTargetView(m_UIRenderTarget.Get(), nullptr, rtvHandle);
        m_device->CreateShaderResourceView(m_UIRenderTarget.Get(), nullptr, srvHandle);
    }

    m_viewport.Width = static_cast<float>(m_width);
    m_viewport.Height = static_cast<float>(m_height);

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = static_cast<LONG>(m_width);
    m_scissorRect.bottom = static_cast<LONG>(m_height);

    // Update the color space triangle vertices when the command list is back in
    // the recording state.
    m_updateVertexBuffer = true;

    if (m_enableUI)
    {
        if (!m_uiLayer)
        {
            m_uiLayer = std::make_shared<UILayer>(this);
        }
        else
        {
            m_uiLayer->Resize();
        }
    }
}

// Transform the triangle coordinates so that they remain centered and at the right scale.
// (mapping a [0,0,1,1] square into a [0,0,1,1] rectangle)
inline XMFLOAT3 D3D12HDR::TransformVertex(XMFLOAT2 point, XMFLOAT2 offset)
{
    auto scale = XMFLOAT2(min(1.0f, 1.0f / m_aspectRatio), min(1.0f, m_aspectRatio));
    auto margin = XMFLOAT2(0.5f * (1.0f - scale.x), 0.5f * (1.0f - scale.y));
    auto v = XMVectorMultiplyAdd(
        XMLoadFloat2(&point),
        XMLoadFloat2(&scale),
        XMVectorAdd(XMLoadFloat2(&margin), XMLoadFloat2(&offset)));

    XMFLOAT3 result;
    XMStoreFloat3(&result, v);
    return result;
}

// Recalculate vertex positions for the color space triangles and then update the vertex buffer.
// The scene's command list must be in the recording state when this method is called.
void D3D12HDR::UpdateVertexBuffer()
{
    const XMFLOAT2 primaries709[] =
    {
        { 0.64f, 0.33f },
        { 0.30f, 0.60f },
        { 0.15f, 0.06f },
        { 0.3127f, 0.3290f }
    };
    const XMFLOAT2 primaries2020[] =
    {
        { 0.708f, 0.292f },
        { 0.170f, 0.797f },
        { 0.131f, 0.046f },
        { 0.3127f, 0.3290f }
    };
    const XMFLOAT2 offset1 = { 0.2f, 0.0f };
    const XMFLOAT2 offset2 = { 0.2f, -1.0f };
    const XMFLOAT3 triangle709[] =
    {
        TransformVertex(primaries709[0], offset1),    // R
        TransformVertex(primaries709[1], offset1),    // G
        TransformVertex(primaries709[2], offset1),    // B
        TransformVertex(primaries709[3], offset1)    // White
    };
    const XMFLOAT3 triangle2020[] =
    {
        TransformVertex(primaries2020[0], offset2),    // R
        TransformVertex(primaries2020[1], offset2),    // G
        TransformVertex(primaries2020[2], offset2),    // B
        TransformVertex(primaries2020[3], offset2)    // White
    };

    TrianglesVertex triangleVertices[] =
    {
        // Upper triangles.  Rec 709 primaries.

        { triangle709[2], primaries709[2] },
        { triangle709[1], primaries709[1] },
        { triangle709[3], primaries709[3] },

        { triangle709[1], primaries709[1] },
        { triangle709[0], primaries709[0] },
        { triangle709[3], primaries709[3] },

        { triangle709[0], primaries709[0] },
        { triangle709[2], primaries709[2] },
        { triangle709[3], primaries709[3] },

        // Lower triangles. Rec 2020 primaries.

        { triangle2020[2], primaries2020[2] },
        { triangle2020[1], primaries2020[1] },
        { triangle2020[3], primaries2020[3] },

        { triangle2020[1], primaries2020[1] },
        { triangle2020[0], primaries2020[0] },
        { triangle2020[3], primaries2020[3] },

        { triangle2020[0], primaries2020[0] },
        { triangle2020[2], primaries2020[2] },
        { triangle2020[3], primaries2020[3] },
    };

    // Copy data to the intermediate upload heap and then schedule a copy
    // from the upload heap to the vertex buffer.
    UINT8* mappedUploadHeap = nullptr;
    ThrowIfFailed(m_vertexBufferUpload->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&mappedUploadHeap)));

    mappedUploadHeap += m_gradientVertexBufferView.SizeInBytes;
    memcpy(mappedUploadHeap, triangleVertices, sizeof(triangleVertices));

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST));
    m_commandList->CopyBufferRegion(m_vertexBuffer.Get(), 0, m_vertexBufferUpload.Get(), 0, m_vertexBuffer->GetDesc().Width);
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

// Update frame-based values.
void D3D12HDR::OnUpdate()
{
    if (m_enableUI)
    {
        m_uiLayer->UpdateLabels();
    }
}

// Render the scene.
void D3D12HDR::OnRender()
{
    if (m_windowVisible)
    {
        if (m_enableUI)
        {
            PIXBeginEvent(m_commandQueue.Get(), 0, L"Render UI");
            m_uiLayer->Render();
            PIXEndEvent(m_commandQueue.Get());
        }

        PIXBeginEvent(m_commandQueue.Get(), 0, L"Render Scene");
        RenderScene();
        PIXEndEvent(m_commandQueue.Get());

        // Present the frame.
        ThrowIfFailed(m_swapChain->Present(1, 0));

        MoveToNextFrame();
    }
}

// Fill the command list with all the render commands and dependent state and
// submit it to the command queue.
void D3D12HDR::RenderScene()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineStates[GradientPSO].Get()));

    if (m_updateVertexBuffer)
    {
        UpdateVertexBuffer();
        m_updateVertexBuffer = false;
    }

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Bind the root constants and the SRV table to the pipeline.
    m_rootConstantsF[ReferenceWhiteNits] = m_referenceWhiteNits;

    m_commandList->SetGraphicsRoot32BitConstants(0, RootConstantsCount, m_rootConstants, 0);
    m_commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    // Draw the scene into the intermediate render target.
    {
        PIXBeginEvent(m_commandList.Get(), 0, L"Draw scene content");

        CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtv(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSize);
        m_commandList->OMSetRenderTargets(1, &intermediateRtv, FALSE, nullptr);

        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_commandList->ClearRenderTargetView(intermediateRtv, clearColor, 0, nullptr);

        m_commandList->IASetVertexBuffers(0, 1, &m_gradientVertexBufferView);
        PIXBeginEvent(m_commandList.Get(), 0, L"Standard Gradient");
        m_commandList->DrawInstanced(4, 1, 0, 0);
        PIXEndEvent(m_commandList.Get());

        PIXBeginEvent(m_commandList.Get(), 0, L"Bright Gradient");
        m_commandList->DrawInstanced(4, 1, 4, 0);
        PIXEndEvent(m_commandList.Get());

        m_commandList->SetPipelineState(m_pipelineStates[PalettePSO].Get());
        m_commandList->IASetVertexBuffers(0, 1, &m_trianglesVertexBufferView);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        PIXBeginEvent(m_commandList.Get(), 0, L"Rec709 Triangles");
        m_commandList->DrawInstanced(3, 1, 0, 0);
        m_commandList->DrawInstanced(3, 1, 3, 0);
        m_commandList->DrawInstanced(3, 1, 6, 0);
        PIXEndEvent(m_commandList.Get());

        m_commandList->SetPipelineState(m_pipelineStates[PalettePSO].Get());
        PIXBeginEvent(m_commandList.Get(), 0, L"Rec2020 Triangles");
        m_commandList->DrawInstanced(3, 1, 9, 0);
        m_commandList->DrawInstanced(3, 1, 12, 0);
        m_commandList->DrawInstanced(3, 1, 15, 0);
        PIXEndEvent(m_commandList.Get());

        PIXEndEvent(m_commandList.Get());

        if (!m_enableUI)
        {
            intermediateRtv.Offset(1, m_rtvDescriptorSize);
            m_commandList->OMSetRenderTargets(1, &intermediateRtv, FALSE, nullptr);
            m_commandList->ClearRenderTargetView(intermediateRtv, clearColor, 0, nullptr);
        }
    }

    // Indicate that the intermediates will be used as SRVs in the pixel shader
    // and the back buffer will be used as a render target.
    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(m_UIRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
    };

    // Process the intermediate and draw into the swap chain render target.
    {
        PIXBeginEvent(m_commandList.Get(), 0, L"Apply HDR");

        // If the UI is enabled, the 11on12 layer will do the state transition for us.
        UINT barrierCount = m_enableUI ? 2 : _countof(barriers);
        m_commandList->ResourceBarrier(barrierCount, barriers);
        m_commandList->SetPipelineState(m_pipelineStates[Present8bitPSO + m_currentSwapChainBitDepth].Get());

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        m_commandList->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);

        m_commandList->IASetVertexBuffers(0, 1, &m_presentVertexBufferView);
        m_commandList->DrawInstanced(3, 1, 0, 0);

        PIXEndEvent(m_commandList.Get());
    }

    // Indicate that the intermediates will be used as render targets and the swap chain
    // back buffer will be used for presentation.
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_commandList->ResourceBarrier(_countof(barriers), barriers);

    ThrowIfFailed(m_commandList->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}


void D3D12HDR::OnWindowMoved(int xPos, int yPos)
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    if (!m_swapChain)
    {
        return;
    }

    // An app could be moved from a HDR monitor to a SDR monitor or vice versa. After the app got moved, we can verify the monitor HDR support again here.
    CheckDisplayHDRSupport();
}


void D3D12HDR::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    // Update the width, height, and aspect ratio member variables.
    UpdateForSizeChange(width, height);

    // Update the size of swapchain buffers.
    UpdateSwapChainBuffer(width, height, GetBackBufferFormat());

    m_windowVisible = !minimized;
}



void D3D12HDR::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    if (!m_tearingSupport)
    {
        // Fullscreen state should always be false before exiting the app.
        ThrowIfFailed(m_swapChain->SetFullscreenState(FALSE, nullptr));
    }
    CloseHandle(m_fenceEvent);
}

void D3D12HDR::OnKeyDown(UINT8 key)
{
    switch (key)
    {
        // Instrument the Space Bar to toggle between fullscreen states.
        // The window message loop callback will receive a WM_SIZE message once the
        // window is in the fullscreen state. At that point, the IDXGISwapChain should
        // be resized to match the new window size.
        //
        // NOTE: ALT+Enter will perform a similar operation; the code below is not
        // required to enable that key combination.
        case VK_SPACE:
        {
            if (m_tearingSupport)
            {
                Win32Application::ToggleFullscreenWindow();
            }
            else
            {
                BOOL fullscreenState;
                ThrowIfFailed(m_swapChain->GetFullscreenState(&fullscreenState, nullptr));
                if (FAILED(m_swapChain->SetFullscreenState(!fullscreenState, nullptr)))
                {
                    // Transitions to fullscreen mode can fail when running apps over
                    // terminal services or for some other unexpected reason.  Consider
                    // notifying the user in some way when this happens.
                    OutputDebugString(L"Fullscreen transition failed");
                    assert(false);
                }
            }
            break;
        }

        case VK_PRIOR:    // Page Up
        {
            m_currentSwapChainBitDepth = static_cast<SwapChainBitDepth>((m_currentSwapChainBitDepth - 1 + SwapChainBitDepthCount) % SwapChainBitDepthCount);
            DXGI_FORMAT newFormat = m_swapChainFormats[m_currentSwapChainBitDepth];
            UpdateSwapChainBuffer(m_width, m_height, newFormat);
            SetHDRMetaData(HDRMetaDataPool[m_hdrMetaDataPoolIdx][0], HDRMetaDataPool[m_hdrMetaDataPoolIdx][1], HDRMetaDataPool[m_hdrMetaDataPoolIdx][2], HDRMetaDataPool[m_hdrMetaDataPoolIdx][3]);
            break;
        }

        case VK_NEXT:    // Page Down
        {
            m_currentSwapChainBitDepth = static_cast<SwapChainBitDepth>((m_currentSwapChainBitDepth + 1) % SwapChainBitDepthCount);
            DXGI_FORMAT newFormat = m_swapChainFormats[m_currentSwapChainBitDepth];
            UpdateSwapChainBuffer(m_width, m_height, newFormat);
            SetHDRMetaData(HDRMetaDataPool[m_hdrMetaDataPoolIdx][0], HDRMetaDataPool[m_hdrMetaDataPoolIdx][1], HDRMetaDataPool[m_hdrMetaDataPoolIdx][2], HDRMetaDataPool[m_hdrMetaDataPoolIdx][3]);
            break;
        }

        case 'H':
        {
            m_enableST2084 = !m_enableST2084;
            if (m_currentSwapChainBitDepth == _10)
            {
                EnsureSwapChainColorSpace(m_currentSwapChainBitDepth, m_enableST2084);
                SetHDRMetaData(HDRMetaDataPool[m_hdrMetaDataPoolIdx][0], HDRMetaDataPool[m_hdrMetaDataPoolIdx][1], HDRMetaDataPool[m_hdrMetaDataPoolIdx][2], HDRMetaDataPool[m_hdrMetaDataPoolIdx][3]);
            }

            break;
        }

        case 'U':
        {
            m_enableUI = !m_enableUI;
            if (m_enableUI)
            {
                m_uiLayer = std::make_shared<UILayer>(this);
            }
            else
            {
                m_uiLayer.reset();
            }

            break;
        }

        case 'M':
        {
            // Switch meta data value for testing. TV should adjust the content based on the metadata we sent.
            m_hdrMetaDataPoolIdx = (m_hdrMetaDataPoolIdx + 1) % 4;
            SetHDRMetaData(HDRMetaDataPool[m_hdrMetaDataPoolIdx][0], HDRMetaDataPool[m_hdrMetaDataPoolIdx][1], HDRMetaDataPool[m_hdrMetaDataPoolIdx][2], HDRMetaDataPool[m_hdrMetaDataPoolIdx][3]);
            break;
        }
    }
}

// Wait for pending GPU work to complete.
void D3D12HDR::WaitForGpu()
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
void D3D12HDR::MoveToNextFrame()
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


// DirectX supports two combinations of swapchain pixel formats and colorspaces for HDR content.
// Option 1: FP16 + DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709
// Option 2: R10G10B10A2 + DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
// Calling this function to ensure the correct color space for the different pixel formats.
void D3D12HDR::EnsureSwapChainColorSpace(SwapChainBitDepth swapChainBitDepth, bool enableST2084)
{
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    switch (swapChainBitDepth)
    {
    case _8:
        m_rootConstants[DisplayCurve] = sRGB;
        break;

    case _10:
        colorSpace = enableST2084 ? DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
        m_rootConstants[DisplayCurve] = enableST2084 ? ST2084 : sRGB;
        break;

    case _16:
        colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
        m_rootConstants[DisplayCurve] = None;
        break;
    }

    if (m_currentSwapChainColorSpace != colorSpace)
    {
        UINT colorSpaceSupport = 0;
        if (SUCCEEDED(m_swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
            ((colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) == DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
        {
            ThrowIfFailed(m_swapChain->SetColorSpace1(colorSpace));
            m_currentSwapChainColorSpace = colorSpace;
        }
    }
}

// Set HDR meta data for output display to master the content and the luminance values of the content.
// An app should estimate and set appropriate metadata based on its contents.
// For demo purpose, we simply made up a few set of metadata for you to experience the effect of appling meta data.
// Please see details in https://msdn.microsoft.com/en-us/library/windows/desktop/mt732700(v=vs.85).aspx.
void D3D12HDR::SetHDRMetaData(float MaxOutputNits /*=1000.0f*/, float MinOutputNits /*=0.001f*/, float MaxCLL /*=2000.0f*/, float MaxFALL /*=500.0f*/)
{
    if (!m_swapChain)
    {
        return;
    }

    // Clean the hdr metadata if the display doesn't support HDR
    if (!m_hdrSupport)
    {
        ThrowIfFailed(m_swapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_NONE, 0, nullptr));
        return;
    }

    static const DisplayChromaticities DisplayChromaticityList[] =
    {
        { 0.64000f, 0.33000f, 0.30000f, 0.60000f, 0.15000f, 0.06000f, 0.31270f, 0.32900f }, // Display Gamut Rec709 
        { 0.70800f, 0.29200f, 0.17000f, 0.79700f, 0.13100f, 0.04600f, 0.31270f, 0.32900f }, // Display Gamut Rec2020
    };

    // Select the chromaticity based on HDR format of the DWM.
    int selectedChroma = 0;
    if (m_currentSwapChainBitDepth == _16 && m_currentSwapChainColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709)
    {
        selectedChroma = 0;
    }
    else if (m_currentSwapChainBitDepth == _10 && m_currentSwapChainColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
    {
        selectedChroma = 1;
    }
    else
    {
        // Reset the metadata since this is not a supported HDR format.
        ThrowIfFailed(m_swapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_NONE, 0, nullptr));
        return;
    }

    // Set HDR meta data
    const DisplayChromaticities& Chroma = DisplayChromaticityList[selectedChroma];
    DXGI_HDR_METADATA_HDR10 HDR10MetaData = {};
    HDR10MetaData.RedPrimary[0] = static_cast<UINT16>(Chroma.RedX * 50000.0f);
    HDR10MetaData.RedPrimary[1] = static_cast<UINT16>(Chroma.RedY * 50000.0f);
    HDR10MetaData.GreenPrimary[0] = static_cast<UINT16>(Chroma.GreenX * 50000.0f);
    HDR10MetaData.GreenPrimary[1] = static_cast<UINT16>(Chroma.GreenY * 50000.0f);
    HDR10MetaData.BluePrimary[0] = static_cast<UINT16>(Chroma.BlueX * 50000.0f);
    HDR10MetaData.BluePrimary[1] = static_cast<UINT16>(Chroma.BlueY * 50000.0f);
    HDR10MetaData.WhitePoint[0] = static_cast<UINT16>(Chroma.WhiteX * 50000.0f);
    HDR10MetaData.WhitePoint[1] = static_cast<UINT16>(Chroma.WhiteY * 50000.0f);
    HDR10MetaData.MaxMasteringLuminance = static_cast<UINT>(MaxOutputNits * 10000.0f);
    HDR10MetaData.MinMasteringLuminance = static_cast<UINT>(MinOutputNits * 10000.0f);
    HDR10MetaData.MaxContentLightLevel = static_cast<UINT16>(MaxCLL);
    HDR10MetaData.MaxFrameAverageLightLevel = static_cast<UINT16>(MaxFALL);
    ThrowIfFailed(m_swapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &HDR10MetaData));
}


void D3D12HDR::UpdateSwapChainBuffer(UINT width, UINT height, DXGI_FORMAT format)
{
    if (!m_swapChain)
    {
        return;
    }

    // Flush all current GPU commands.
    WaitForGpu();

    // Release the resources holding references to the swap chain (requirement of
    // IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
    // current fence value.
    for (UINT n = 0; n < FrameCount; n++)
    {
        m_renderTargets[n].Reset();
        m_fenceValues[n] = m_fenceValues[m_frameIndex];
    }
    if (m_enableUI)
    {
        m_uiLayer->ReleaseResources();
    }

    // Resize the swap chain to the desired dimensions.
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    m_swapChain->GetDesc1(&desc);
    ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, format, desc.Flags));

    EnsureSwapChainColorSpace(m_currentSwapChainBitDepth, m_enableST2084);

    // Reset the frame index to the current back buffer index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Update the width, height, and aspect ratio member variables.
    UpdateForSizeChange(width, height);

    LoadSizeDependentResources();
}


// To detect HDR support, we will need to check the color space in the primary DXGI output associated with the app at
// this point in time (using window/display intersection). 

// Compute the overlay area of two rectangles, A and B.
// (ax1, ay1) = left-top coordinates of A; (ax2, ay2) = right-bottom coordinates of A
// (bx1, by1) = left-top coordinates of B; (bx2, by2) = right-bottom coordinates of B
inline int ComputeIntersectionArea(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{
    return max(0, min(ax2, bx2) - max(ax1, bx1)) * max(0, min(ay2, by2) - max(ay1, by1));
}

void D3D12HDR::CheckDisplayHDRSupport()
{
    // If the display's advanced color state has changed (e.g. HDR display plug/unplug, or OS HDR setting on/off), 
    // then this app's DXGI factory is invalidated and must be created anew in order to retrieve up-to-date display information. 
    if (m_dxgiFactory->IsCurrent() == false)
    {
        ThrowIfFailed(
            CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory))
        );
    }

    // First, the method must determine the app's current display. 
    // We don't recommend using IDXGISwapChain::GetContainingOutput method to do that because of two reasons:
    //    1. Swap chains created with CreateSwapChainForComposition do not support this method.
    //    2. Swap chains will return a stale dxgi output once DXGIFactory::IsCurrent() is false. In addition, 
    //       we don't recommend re-creating swapchain to resolve the stale dxgi output because it will cause a short 
    //       period of black screen.
    // Instead, we suggest enumerating through the bounds of all dxgi outputs and determine which one has the greatest 
    // intersection with the app window bounds. Then, use the DXGI output found in previous step to determine if the 
    // app is on a HDR capable display. 

    // Retrieve the current default adapter.
    ComPtr<IDXGIAdapter1> dxgiAdapter;
    ThrowIfFailed(m_dxgiFactory->EnumAdapters1(0, &dxgiAdapter));

    // Iterate through the DXGI outputs associated with the DXGI adapter,
    // and find the output whose bounds have the greatest overlap with the
    // app window (i.e. the output for which the intersection area is the
    // greatest).

    UINT i = 0;
    ComPtr<IDXGIOutput> currentOutput;
    ComPtr<IDXGIOutput> bestOutput;
    float bestIntersectArea = -1;

    while (dxgiAdapter->EnumOutputs(i, &currentOutput) != DXGI_ERROR_NOT_FOUND)
    {
        // Get the retangle bounds of the app window
        int ax1 = m_windowBounds.left;
        int ay1 = m_windowBounds.top;
        int ax2 = m_windowBounds.right;
        int ay2 = m_windowBounds.bottom;

        // Get the rectangle bounds of current output
        DXGI_OUTPUT_DESC desc;
        ThrowIfFailed(currentOutput->GetDesc(&desc));
        RECT r = desc.DesktopCoordinates;
        int bx1 = r.left;
        int by1 = r.top;
        int bx2 = r.right;
        int by2 = r.bottom;

        // Compute the intersection
        int intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
        if (intersectArea > bestIntersectArea)
        {
            bestOutput = currentOutput;
            bestIntersectArea = static_cast<float>(intersectArea);
        }

        i++;
    }

    // Having determined the output (display) upon which the app is primarily being 
    // rendered, retrieve the HDR capabilities of that display by checking the color space.
    ComPtr<IDXGIOutput6> output6;
    ThrowIfFailed(bestOutput.As(&output6));

    DXGI_OUTPUT_DESC1 desc1;
    ThrowIfFailed(output6->GetDesc1(&desc1));

    m_hdrSupport = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
}

void D3D12HDR::OnDisplayChanged()
{
    // Changing display setting or plugging/unplugging HDR displays could raise a WM_DISPLAYCHANGE message (win32) or OnDisplayContentsInvalidated event(uwp). 
    // Re-check the HDR support while receiving them.
    CheckDisplayHDRSupport();
}
