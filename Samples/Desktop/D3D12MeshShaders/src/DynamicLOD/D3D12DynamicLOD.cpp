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
#include "D3D12DynamicLOD.h"

namespace
{
    // Limit our dispatch threadgroup count to 65536 for indexing simplicity.
    const uint32_t c_maxGroupDispatchCount = 65536u;

    // Calculates the size required for constant buffer alignment
    template <typename T>
    T GetAlignedSize(T size)
    {
        const T alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        const T alignedSize = (size + alignment - 1) & ~(alignment - 1);
        return alignedSize;
    }

    // An integer version of ceil(value / divisor)
    template <typename T, typename U>
    T DivRoundUp(T num, U denom)
    {
        return (num + denom - 1) / denom;
    }

    // Resource view offsets into the descriptor heap
    // The mesh & meshlet SRVs are laid out as they are referenced in shader code
    enum DescriptorHeapIndex
    {
        SRV_MeshInfoLODs          = 0,
        SRV_VertexLODs            = SRV_MeshInfoLODs + MAX_LOD_LEVELS,
        SRV_MeshletLODs           = SRV_VertexLODs + MAX_LOD_LEVELS,
        SRV_UniqueVertexIndexLODs = SRV_MeshletLODs + MAX_LOD_LEVELS,
        SRV_PrimitiveIndexLODs    = SRV_UniqueVertexIndexLODs + MAX_LOD_LEVELS,
        SRV_Count                 = SRV_PrimitiveIndexLODs + MAX_LOD_LEVELS,
    };
}

const float D3D12DynamicLOD::c_fovy = XM_PI / 3.0f;

const wchar_t* D3D12DynamicLOD::c_lodFilenames[] =
{
    L"..\\Assets\\Dragon_LOD0.bin",
    L"..\\Assets\\Dragon_LOD1.bin",
    L"..\\Assets\\Dragon_LOD2.bin",
    L"..\\Assets\\Dragon_LOD3.bin",
    L"..\\Assets\\Dragon_LOD4.bin",
    L"..\\Assets\\Dragon_LOD5.bin",
};

const wchar_t* D3D12DynamicLOD::c_ampShaderFilename = L"MeshletAS.cso";
const wchar_t* D3D12DynamicLOD::c_meshShaderFilename = L"MeshletMS.cso";
const wchar_t* D3D12DynamicLOD::c_pixelShaderFilename = L"MeshletPS.cso";

D3D12DynamicLOD::D3D12DynamicLOD(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name)
    , m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
    , m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
    , m_rtvDescriptorSize(0)
    , m_dsvDescriptorSize(0)
    , m_srvDescriptorSize(0)
    , m_frameIndex(0)
    , m_frameCounter(0)
    , m_fenceEvent{}
    , m_fenceValues{}
    , m_constantData(nullptr)
    , m_instanceData(nullptr)
    , m_renderMode(RenderMode::LOD)
    , m_instanceLevel(0)
    , m_instanceCount(1)
    , m_updateInstances(true)
{ }

void D3D12DynamicLOD::OnInit()
{
    m_camera.Init({ 0, 75, 150 });
    m_camera.SetMoveSpeed(150.0f);

    LoadPipeline();
    LoadAssets();
    RegenerateInstances();
}

// Load the rendering pipeline dependencies.
void D3D12DynamicLOD::LoadPipeline()
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

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
    {
        OutputDebugStringA("ERROR: Shader Model 6.5 is not supported\n");
        throw std::exception("Shader Model 6.5 is not supported");
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features)))
        || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
    {
        OutputDebugStringA("ERROR: Mesh Shaders aren't supported!\n");
        throw std::exception("Mesh Shaders aren't supported!");
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

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
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // Describe and create a shader resourc e view (SRV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = SRV_Count;
        srvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV and a command allocator for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
        }
    }

    // Create the depth stencil view.
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        const CD3DX12_RESOURCE_DESC depthStencilTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &depthStencilHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilTextureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_depthStencil)
        ));

        m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create the constant buffer.
    {
        const UINT64 constantBufferSize = sizeof(Constants) * FrameCount;

        const CD3DX12_HEAP_PROPERTIES constantBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &constantBufferHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer)
        ));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_constantData)));
    }
}

// Load the sample assets.
void D3D12DynamicLOD::LoadAssets()
{
    // Create the pipeline state, which includes compiling and loading shaders.
    {
        // Create the command list.
        ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        ThrowIfFailed(m_commandList->Close());

        struct
        {
            byte* data;
            uint32_t size;
        } ampShader, meshShader, pixelShader;

        ReadDataFromFile(GetAssetFullPath(c_ampShaderFilename).c_str(), &ampShader.data, &ampShader.size);
        ReadDataFromFile(GetAssetFullPath(c_meshShaderFilename).c_str(), &meshShader.data, &meshShader.size);
        ReadDataFromFile(GetAssetFullPath(c_pixelShaderFilename).c_str(), &pixelShader.data, &pixelShader.size);

        // Pull root signature from the precompiled mesh shader.
        ThrowIfFailed(m_device->CreateRootSignature(0, meshShader.data, meshShader.size, IID_PPV_ARGS(&m_rootSignature)));

        D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature    = m_rootSignature.Get();
        psoDesc.AS                = { ampShader.data, ampShader.size };
        psoDesc.MS                = { meshShader.data, meshShader.size };
        psoDesc.PS                = { pixelShader.data, pixelShader.size };
        psoDesc.NumRenderTargets  = 1;
        psoDesc.RTVFormats[0]     = m_renderTargets[0]->GetDesc().Format;
        psoDesc.DSVFormat         = m_depthStencil->GetDesc().Format;
        psoDesc.RasterizerState   = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);      // CW front; cull back
        psoDesc.BlendState        = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                // Opaque
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Less-equal depth test w/ writes; no stencil
        psoDesc.SampleMask        = UINT_MAX;
        psoDesc.SampleDesc        = DefaultSampleDesc();

        auto meshStreamDesc = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

        // Point to our populated stream desc
        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
        streamDesc.SizeInBytes                   = sizeof(meshStreamDesc);
        streamDesc.pPipelineStateSubobjectStream = &meshStreamDesc;

        ThrowIfFailed(m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    m_lods.resize(_countof(c_lodFilenames));
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_lods.size()); ++i)
    {
        auto& lod = m_lods[i];

        // Load and upload model resources to the GPU
        // Just use the D3D12_COMMAND_LIST_TYPE_DIRECT queue since it's a one-and-done operation. 
        // For per-frame uploads consider using the D3D12_COMMAND_LIST_TYPE_COPY command queue.
        lod.LoadFromFile(c_lodFilenames[i]);
        lod.UploadGpuResources(m_device.Get(), m_commandQueue.Get(), m_commandAllocators[m_frameIndex].Get(), m_commandList.Get());

#ifdef _DEBUG
        // Mesh shader file expects a certain vertex layout; assert our mesh conforms to that layout.
        const D3D12_INPUT_ELEMENT_DESC c_elementDescs[2] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        };

        assert(lod.GetMesh(0).LayoutDesc.NumElements == 2);

        for (uint32_t i = 0; i < _countof(c_elementDescs); ++i)
            assert(std::memcmp(&lod.GetMesh(0).LayoutElems[i], &c_elementDescs[i], sizeof(D3D12_INPUT_ELEMENT_DESC)) == 0);
#endif
    }

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    auto OffsetHandle = [=](uint32_t index) { return D3D12_CPU_DESCRIPTOR_HANDLE{ srvHandle.ptr + SIZE_T(index) * m_srvDescriptorSize }; };

    // Populate descriptor table with arrays of SRVs for each LOD
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_lods.size()); ++i)
    {
        auto& m = m_lods[i].GetMesh(0);

        // Mesh Info Buffers
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m.MeshInfoResource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes    = GetAlignedSize<uint32_t>(sizeof(MeshInfo));
        m_device->CreateConstantBufferView(&cbvDesc, OffsetHandle(SRV_MeshInfoLODs + i));

        // Populate common shader resource view desc with shared settings.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                  = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement     = 0;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        // Vertices
        srvDesc.Buffer.StructureByteStride = m.VertexStrides[0]; // We assume we'll only use the first vertex buffer
        srvDesc.Buffer.NumElements         = m.VertexCount;
        m_device->CreateShaderResourceView(m.VertexResources[0].Get(), &srvDesc, OffsetHandle(SRV_VertexLODs + i));

        // Meshlets
        srvDesc.Buffer.StructureByteStride = sizeof(Meshlet);
        srvDesc.Buffer.NumElements         = static_cast<uint32_t>(m.Meshlets.size());
        m_device->CreateShaderResourceView(m.MeshletResource.Get(), &srvDesc, OffsetHandle(SRV_MeshletLODs + i));

        // Primitive Indices
        srvDesc.Buffer.StructureByteStride = sizeof(uint32_t);
        srvDesc.Buffer.NumElements         = m.IndexCount / 3;
        m_device->CreateShaderResourceView(m.PrimitiveIndexResource.Get(), &srvDesc, OffsetHandle(SRV_PrimitiveIndexLODs + i));

        // Unique Vertex Indices
        srvDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Buffer.StructureByteStride = 0;
        srvDesc.Buffer.NumElements         = DivRoundUp(static_cast<uint32_t>(m.UniqueVertexIndices.size()), 4);
        srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
        m_device->CreateShaderResourceView(m.UniqueVertexIndexResource.Get(), &srvDesc, OffsetHandle(SRV_UniqueVertexIndexLODs + i));
    }

    // Null-out remaining LOD slots in the descriptor table.
    for (uint32_t i = static_cast<uint32_t>(m_lods.size()); i < MAX_LOD_LEVELS; ++i)
    {
        m_device->CreateConstantBufferView(nullptr, OffsetHandle(SRV_MeshInfoLODs + i));

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        srvDesc.Buffer.StructureByteStride = 24;
        m_device->CreateShaderResourceView(nullptr, &srvDesc, OffsetHandle(SRV_VertexLODs + i));

        srvDesc.Buffer.StructureByteStride = sizeof(Meshlet);
        m_device->CreateShaderResourceView(nullptr, &srvDesc, OffsetHandle(SRV_MeshletLODs + i));

        srvDesc.Buffer.StructureByteStride = sizeof(uint32_t);
        m_device->CreateShaderResourceView(nullptr, &srvDesc, OffsetHandle(SRV_PrimitiveIndexLODs + i));

        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Buffer.StructureByteStride = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        m_device->CreateShaderResourceView(nullptr, &srvDesc, OffsetHandle(SRV_UniqueVertexIndexLODs + i));
    }
    
    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
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

// Update frame-based values.
void D3D12DynamicLOD::OnUpdate()
{
    m_timer.Tick(NULL);

    if (m_frameCounter++ % 30 == 0)
    {
        // Update window text with FPS value.
        wchar_t fps[64];
        swprintf_s(fps, L"%ufps", m_timer.GetFramesPerSecond());
        SetCustomWindowText(fps);
    }

    m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()));

    XMMATRIX view = m_camera.GetViewMatrix();
    XMMATRIX proj = m_camera.GetProjectionMatrix(c_fovy, m_aspectRatio, 1.0f, 1e4f);
    XMMATRIX viewProj = view * proj;

    XMVECTOR scale, rot, viewPos;
    XMMatrixDecompose(&scale, &rot, &viewPos, view);

    XMMATRIX vp = XMMatrixTranspose(viewProj);
    XMVECTOR planes[6] =
    {
        XMPlaneNormalize(XMVectorAdd(vp.r[3], vp.r[0])),      // Left
        XMPlaneNormalize(XMVectorSubtract(vp.r[3], vp.r[0])), // Right
        XMPlaneNormalize(XMVectorAdd(vp.r[3], vp.r[1])),      // Bottom
        XMPlaneNormalize(XMVectorSubtract(vp.r[3], vp.r[1])), // Top
        XMPlaneNormalize(vp.r[2]),                            // Near
        XMPlaneNormalize(XMVectorSubtract(vp.r[3], vp.r[2])), // Far
    };
    
    Constants& constants = m_constantData[m_frameIndex];
    XMStoreFloat4x4(&constants.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&constants.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat3(&constants.ViewPosition, XMVector3Transform(g_XMZero, XMMatrixInverse(nullptr, view)));

    for (uint32_t i = 0; i < 6; ++i)
    {
        XMStoreFloat4(&constants.Planes[i], planes[i]);
    }

    constants.RenderMode       = static_cast<uint32_t>(m_renderMode);
    constants.LODCount         = static_cast<uint32_t>(m_lods.size());
    constants.RecipTanHalfFovy = 1.0f / tanf(c_fovy * 0.5f);
}

// Render the scene.
void D3D12DynamicLOD::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    MoveToNextFrame();
}

void D3D12DynamicLOD::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}

void D3D12DynamicLOD::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case VK_OEM_PLUS:
        ++m_instanceLevel;
        RegenerateInstances();
        break;

    case VK_OEM_MINUS:
        if (m_instanceLevel != 0)
        {
            --m_instanceLevel;
            RegenerateInstances();
        }
        break;

    case VK_SPACE:
        m_renderMode = RenderMode((uint32_t(m_renderMode) + 1) % uint32_t(RenderMode::Count));
        break;
    }

    m_camera.OnKeyDown(key);
}

void D3D12DynamicLOD::OnKeyUp(UINT8 key)
{
    m_camera.OnKeyUp(key);
}

void D3D12DynamicLOD::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

    // Only upload instance data if we've had a change
    if (m_updateInstances)
    {
        const auto toCopyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_instanceBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandList->ResourceBarrier(1, &toCopyBarrier);
        m_commandList->CopyResource(m_instanceBuffer.Get(), m_instanceUpload.Get());
        const auto toGenericBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_instanceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_commandList->ResourceBarrier(1, &toGenericBarrier);

        m_updateInstances = false;
    }

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(1, heaps);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    const auto toRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &toRenderTargetBarrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + sizeof(Constants) * m_frameIndex);

    m_commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->SetGraphicsRootShaderResourceView(3, m_instanceBuffer->GetGPUVirtualAddress());

    uint32_t dispatchCount = DivRoundUp(m_instanceCount, c_maxGroupDispatchCount);

    for (uint32_t i = 0; i < dispatchCount; ++i)
    {
        uint32_t offset = dispatchCount * i;
        uint32_t count = min(m_instanceCount - offset, c_maxGroupDispatchCount);

        m_commandList->SetGraphicsRoot32BitConstant(1, offset, 0);
        m_commandList->SetGraphicsRoot32BitConstant(1, count, 1);

        m_commandList->DispatchMesh(count, 1, 1);
    }

    // Indicate that the back buffer will now be used to present.
    const auto toPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &toPresentBarrier);

    ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void D3D12DynamicLOD::WaitForGpu()
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
void D3D12DynamicLOD::MoveToNextFrame()
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

void D3D12DynamicLOD::RegenerateInstances()
{
    m_updateInstances = true;

    const float radius = m_lods[0].GetBoundingSphere().Radius;
    const float padding = 0.5f;
    const float spacing = (1.0f + padding) * radius;

    // Create the instances in a growing cube volume
    const uint32_t width = m_instanceLevel * 2 + 1;
    const float extents = spacing * m_instanceLevel;

    m_instanceCount = width * width * width;

    const UINT64 instanceBufferSize = GetAlignedSize(m_instanceCount * sizeof(Instance));

    // Only recreate instance-sized buffers if necessary.
    if (!m_instanceBuffer || m_instanceBuffer->GetDesc().Width < instanceBufferSize)
    {
        WaitForGpu();

        const CD3DX12_HEAP_PROPERTIES instanceBufferDefaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        const CD3DX12_RESOURCE_DESC instanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

        // Create/re-create the instance buffer
        ThrowIfFailed(m_device->CreateCommittedResource(
            &instanceBufferDefaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &instanceBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_instanceBuffer)
        ));

        const CD3DX12_HEAP_PROPERTIES instanceBufferUploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);

        // Create/re-create the instance buffer
        ThrowIfFailed(m_device->CreateCommittedResource(
            &instanceBufferUploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &instanceBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_instanceUpload)
        ));

        m_instanceUpload->Map(0, nullptr, reinterpret_cast<void**>(&m_instanceData));
    }
    
    // Regenerate the instances in our scene.
    for (uint32_t i = 0; i < m_instanceCount; ++i)
    {
        XMVECTOR index = XMVectorSet(float(i % width), float((i / width) % width), float(i / (width * width)), 0);
        XMVECTOR location = index * spacing - XMVectorReplicate(extents);
        location = XMVectorSetW(location, radius);

        XMMATRIX world = XMMatrixTranslationFromVector(location);

        auto& inst = m_instanceData[i];
        XMStoreFloat4x4(&inst.World, XMMatrixTranspose(world));
        XMStoreFloat4x4(&inst.WorldInvTranspose, XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixTranspose(world))));
        XMStoreFloat4(&inst.BoundingSphere, location);
    }
}
