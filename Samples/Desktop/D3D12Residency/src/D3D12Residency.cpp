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
#include "D3D12Residency.h"

D3D12Residency::D3D12Residency(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0),
    m_totalAllocations(0),
    m_textureIndex(0),
    m_cancel(false),
    m_loadedTextureCount(0)
{
}

void D3D12Residency::OnInit()
{
    LoadPipeline();
    LoadAssets();
    LoadTexturesAsync();
}

// Load the rendering pipeline dependencies.
void D3D12Residency::LoadPipeline()
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

        ThrowIfFailed(warpAdapter.As(&m_adapter));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));

        ThrowIfFailed(hardwareAdapter.As(&m_adapter));
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
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // Describe and create a shader resource view (SRV) heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = NumTextures;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void D3D12Residency::LoadAssets()
{
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    ComPtr<ID3D12Resource> vertexBufferUpload;

    // Create the vertex buffer.
    {
        // Define the geometry for a quad.
        Vertex quadVertices[] =
        {
            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
            { { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
        };

        const UINT vertexBufferSize = sizeof(quadVertices);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBufferUpload)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the vertex buffer.
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = reinterpret_cast<UINT8*>(quadVertices);
        vertexData.RowPitch = vertexBufferSize;
        vertexData.SlicePitch = vertexData.RowPitch;

        UpdateSubresources<1>(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        FlushGpu();
    }
}

// Create and upload textures asynchronously. When textures are uploaded, they may
// then be used for rendering.
void D3D12Residency::LoadTexturesAsync()
{
    // For this sample we will only track the residency of the textures.
    // Other resources could also be tracked by adding them to the ResidencySets in the
    // managed command lists.
    m_residencyManager.Initialize(m_device.Get(), 0, m_adapter.Get(), MaxResidencyLatency);
    m_loadedTextures.resize(NumTextures);

    // Initialize the queue of incomplete textures that will be loaded asynchronously.
    for (UINT n = 0; n < NumTextures; n++)
    {
        auto pTexture = std::make_shared<ManagedTexture>();
        pTexture->index = n;
        m_incompleteTextures.push(pTexture);
    }

    // Async method that pulls textures out of the m_incompleteTextures queue,
    // initializes them, and then moves them into the m_loadedTextures vector.
    const auto& CreateAndUpload = [this]()
    {
        // Create a copy queue and associated resources to facilitate uploading textures.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

        ComPtr<ID3D12CommandQueue> copyQueue;
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        std::shared_ptr<D3DX12Residency::ResidencySet> residencySet(m_residencyManager.CreateResidencySet());
        ComPtr<ID3D12Fence> fence;
        UINT64 fenceValue = 1;
        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&copyQueue)));
        ThrowIfFailed(m_device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&commandAllocator)));
        ThrowIfFailed(m_device->CreateCommandList(0, queueDesc.Type, commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&commandList)));
        ThrowIfFailed(commandList->Close());
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        // Allocate the CPU-side memory backing the texture.
        const UINT textureSize = TextureWidth * TextureHeight * TexturePixelSize;
        std::vector<UINT8> textureData(textureSize);
        UINT8* pData = textureData.data();

        // Describe the texture.
        D3D12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, TextureWidth, TextureHeight, 1, 1);
        D3D12_RESOURCE_ALLOCATION_INFO info = m_device->GetResourceAllocationInfo(0, 1, &textureDesc);

        // Create a GPU upload buffer.
        ComPtr<ID3D12Resource> textureUploadHeap;
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(info.SizeInBytes),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)));

        auto pTexture = GetNextIncompleteTexture();

        // Create and upload the next texture.
        while (pTexture && !m_cancel)
        {
            // Create the backing resource for the texture and initialize the ManagedTexture.
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&pTexture->texture)));

            SetNameIndexed(pTexture->texture.Get(), L"texture", pTexture->index);

            pTexture->trackingHandle.Initialize(pTexture->texture.Get(), info.SizeInBytes);
            pTexture->size = info.SizeInBytes;
            m_totalAllocations += info.SizeInBytes;

            // Turn on residency tracking for this texture.
            m_residencyManager.BeginTrackingObject(&pTexture->trackingHandle);

            // Describe and create a SRV for the texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = textureDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), pTexture->index, m_srvDescriptorSize);
            m_device->CreateShaderResourceView(pTexture->texture.Get(), &srvDesc, cpuHandle);

            // Reclaim upload resources for initializing the current texture.
            ThrowIfFailed(commandAllocator->Reset());
            ThrowIfFailed(commandList->Reset(commandAllocator.Get(), m_pipelineState.Get()));
            residencySet->Open();

            // Upload the texture data.
            {
                XMFLOAT3 hsv(static_cast<float>(pTexture->index) / (m_width >> 1), 0.75f, 0.85f);
                XMFLOAT3 rgb;
                XMStoreFloat3(&rgb, XMColorHSVToRGB(XMLoadFloat3(&hsv)));
                UINT8 r = static_cast<UINT8>(rgb.x * 255.0f);
                UINT8 g = static_cast<UINT8>(rgb.y * 255.0f);
                UINT8 b = static_cast<UINT8>(rgb.z * 255.0f);
                UINT8 a = 0xff;

                for (UINT n = 0; n < textureSize; n += TexturePixelSize)
                {
                    pData[n] = r;
                    pData[n + 1] = g;
                    pData[n + 2] = b;
                    pData[n + 3] = a;
                }

                // Copy data to the intermediate upload heap and then schedule a copy
                // from the upload heap to the texture resource.
                D3D12_SUBRESOURCE_DATA textureData = {};
                textureData.pData = pData;
                textureData.RowPitch = TextureWidth * TexturePixelSize;
                textureData.SlicePitch = textureData.RowPitch * TextureHeight;

                UpdateSubresources<1>(commandList.Get(), pTexture->texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
            }

            // Add this resource to the set of resources the command list needs resident.
            residencySet->Insert(&pTexture->trackingHandle);

            ThrowIfFailed(commandList->Close());
            ThrowIfFailed(residencySet->Close());

            ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
            D3DX12Residency::ResidencySet* ppResidencySets[] = { residencySet.get() };

            // Schedule the upload and wait for it to complete.
            m_residencyManager.ExecuteCommandLists(copyQueue.Get(), ppCommandLists, ppResidencySets, _countof(ppCommandLists));

            copyQueue->Signal(fence.Get(), fenceValue);
            ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
            WaitForSingleObject(fenceEvent, INFINITE);
            fenceValue++;

            StoreCompletedTexture(pTexture);
            pTexture = GetNextIncompleteTexture();
        }

        CloseHandle(fenceEvent);
    };

    const UINT numThreads = MaxResidencyLatency + 1;
    m_threads.resize(numThreads);

    for (UINT n = 0; n < numThreads; n++)
    {
        m_threads[n] = std::async(std::launch::async, CreateAndUpload);
    }
}

// Update frame-based values.
void D3D12Residency::OnUpdate()
{
}

// Render the scene.
void D3D12Residency::OnRender()
{
    std::shared_ptr<ManagedCommandList> pManagedCommandList;

    if (m_commandListPool.empty() || m_commandListPool.front()->fenceValue > m_fence->GetCompletedValue())
    {
        pManagedCommandList = std::make_shared<ManagedCommandList>();
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pManagedCommandList->commandAllocator)));
        ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pManagedCommandList->commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&pManagedCommandList->commandList)));
        pManagedCommandList->residencySet = std::shared_ptr<D3DX12Residency::ResidencySet>(m_residencyManager.CreateResidencySet());
    }
    else
    {
        pManagedCommandList = m_commandListPool.front();
        m_commandListPool.pop();

        ThrowIfFailed(pManagedCommandList->commandAllocator->Reset());
        ThrowIfFailed(pManagedCommandList->commandList->Reset(pManagedCommandList->commandAllocator.Get(), m_pipelineState.Get()));
    }

    m_commandListPool.push(pManagedCommandList);

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList(pManagedCommandList);

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { pManagedCommandList->commandList.Get() };
    D3DX12Residency::ResidencySet* ppSets[] = { pManagedCommandList->residencySet.get() };

    m_residencyManager.ExecuteCommandLists(m_commandQueue.Get(), ppCommandLists, ppSets, 1);

    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    pManagedCommandList->fenceValue = fence;
}

void D3D12Residency::OnDestroy()
{
    // Cancel all of the texture loading threads and wait for them to exit.
    m_cancel = true;
    for (UINT n = 0; n < m_threads.size(); n++)
    {
        m_threads[n].wait();
    }

    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up.
    FlushGpu();

    CloseHandle(m_fenceEvent);
    m_residencyManager.Destroy();
}

void D3D12Residency::PopulateCommandList(std::shared_ptr<ManagedCommandList> pManagedCommandList)
{
    DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};
    ThrowIfFailed(m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo));

    UINT64 unused, EvictedMemoryInBytes;
    m_residencyManager.QueryResidencyStats(unused, EvictedMemoryInBytes);
    WCHAR message[200];
    swprintf_s(message, L"Total Allocated: %llu MB | Budget: %llu MB | Using: %llu MB | Evicted: %llu MB", m_totalAllocations >> 20, memoryInfo.Budget >> 20, memoryInfo.CurrentUsage >> 20, EvictedMemoryInBytes >> 20);
    this->SetCustomWindowText(message);

    auto commandList = pManagedCommandList->commandList;
    ThrowIfFailed(pManagedCommandList->residencySet->Open());

    // Set necessary state.
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    commandList->RSSetViewports(1, &m_viewport);
    commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // Draw each texture in a thin strip.
    const UINT texturesPerRow = m_width >> 1;
    D3D12_VIEWPORT movingViewport = m_viewport;
    movingViewport.Width = 2.0f;
    movingViewport.Height = floorf(m_height / ceilf(NumTextures / static_cast<float>(texturesPerRow)));

    // Only reference as much memory in this command list as the budget allows to
    // be resident at any given time.
    UINT64 memoryToUse = UINT64 (float(memoryInfo.Budget) * 0.95f);
    UINT textureCount = LoadedTextureCount();
    if (textureCount > 0)
    {
        UINT textureIndex = m_textureIndex;
        UINT64 sizeUsed = 0;

        // Draw as many textures as we have available up until we reach our memory
        // usage budget for the command list.
        do
        {
            auto pTexture = m_loadedTextures[textureIndex];
            if (pTexture)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), pTexture->index, m_srvDescriptorSize);
                commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

                // Indicate that this texture was used in the current command list.
                pManagedCommandList->residencySet->Insert(&pTexture->trackingHandle);
                sizeUsed += pTexture->size;

                // Position this quad on the render target.
                UINT x = textureIndex % texturesPerRow;
                UINT y = textureIndex / texturesPerRow;
                movingViewport.TopLeftX = x * movingViewport.Width;
                movingViewport.TopLeftY = y * movingViewport.Height;
                commandList->RSSetViewports(1, &movingViewport);
                commandList->DrawInstanced(4, 1, 0, 0);
            }

            textureIndex = (textureIndex + 1) % textureCount;

        } while (sizeUsed < memoryToUse && textureIndex != m_textureIndex);

        // Once we have more textures loaded that we have budget to draw at a time,
        // move the window of textures drawn.
        if (textureIndex != m_textureIndex)
        {
            m_textureIndex = (m_textureIndex + 1) % textureCount;
        }
    }

    // Indicate that the back buffer will now be used to present.
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(commandList->Close());
    ThrowIfFailed(pManagedCommandList->residencySet->Close());
}

void D3D12Residency::FlushGpu()
{
    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the fence has been completed.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}
