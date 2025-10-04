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
#include "D3D12ReservedResources.h"

D3D12ReservedResources::D3D12ReservedResources(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0),
    m_tilingSupport(false),
    m_packedMipInfo(),
    m_activeMip(0),
    m_activeMipChanged(true),
    m_fenceValues{}
{
    UINT mipLevels = 0;
    for (UINT w = TextureWidth, h = TextureHeight; w > 0 && h > 0; w >>= 1, h >>= 1)
    {
        mipLevels++;
    }
    m_activeMip = mipLevels - 1;    // Show the least detailed mip first.
    m_mips.resize(mipLevels);
}

void D3D12ReservedResources::OnInit()
{
    LoadPipeline();
    if (m_tilingSupport)
    {
        LoadAssets();
    }
}

// Load the rendering pipeline dependencies.
void D3D12ReservedResources::LoadPipeline()
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
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

    // Check to see which support tier the GPU has for tiled resources.
    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
    m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
    m_tilingSupport = options.TiledResourcesTier != D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED;
    if (!m_tilingSupport)
    {
        MessageBox(
            Win32Application::GetHwnd(),
            L"Reserved resources are not supported by your GPU.\n"
                L"Add '/warp' to the command line to run the sample with WARP.\n"
                L"The sample will now exit.",
            L"Feature unavailable",
            MB_OK);
        PostQuitMessage(0);
        return;
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

        // Describe and create a shader resource view (SRV) heap for the reserved resource.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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
}

// Load the sample assets.
void D3D12ReservedResources::LoadAssets()
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

        // We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
        // is executed on the GPU so we can use the default range behavior:
        // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[2];
        rootParameters[0].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
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
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> error;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &error));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &error));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state objects (PSO).
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
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    ComPtr<ID3D12Resource> vertexBufferUpload;

    // Create the vertex buffer.
    {
        // Create geometry for a quad.
        Vertex quadVertices[] =
        {
            { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f } },    // Bottom left.
            { { -0.25f, 0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f } },    // Top left.
            { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 1.0f, 1.0f } },    // Bottom right.
            { { 0.25f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f } },        // Top right.
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
        m_vertexBufferView.SizeInBytes = sizeof(quadVertices);
    }

    // Create the reserved texture and map the low-resolution mips into it.
    {
        // Describe and create a reserved Texture2D. This resource has no backing texture
        // when it is created. It will be mapped to a physical resource dynamically.
        D3D12_RESOURCE_DESC reservedTextureDesc = {};
        reservedTextureDesc.MipLevels = static_cast<UINT16>(m_mips.size());
        reservedTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        reservedTextureDesc.Width = TextureWidth;
        reservedTextureDesc.Height = TextureHeight;
        reservedTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        reservedTextureDesc.DepthOrArraySize = 1;
        reservedTextureDesc.SampleDesc.Count = 1;
        reservedTextureDesc.SampleDesc.Quality = 0;
        reservedTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        reservedTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;

        ThrowIfFailed(m_device->CreateReservedResource(
            &reservedTextureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_reservedResource)));

        // Describe and create a SRV for the resource.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = reservedTextureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = reservedTextureDesc.MipLevels;
        m_device->CreateShaderResourceView(m_reservedResource.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create an upload heap big enough to fit the largest mip.
        const UINT64 resourceSize = GetRequiredIntermediateSize(m_reservedResource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(resourceSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadHeap)));

        // Get information about the tile layout for the resource.
        //
        // The GetResourceTiling method should always be used rather than manually
        // calculating it since the tile information is dependent on the driver
        // implementation.

        UINT numTiles = 0;
        D3D12_TILE_SHAPE tileShape = {};
        UINT subresourceCount = reservedTextureDesc.MipLevels;
        std::vector<D3D12_SUBRESOURCE_TILING> tilings(subresourceCount);
        m_device->GetResourceTiling(m_reservedResource.Get(), &numTiles, &m_packedMipInfo, &tileShape, &subresourceCount, 0, &tilings[0]);

        UINT heapCount = m_packedMipInfo.NumStandardMips + (m_packedMipInfo.NumPackedMips > 0 ? 1 : 0);
        for (UINT n = 0; n < m_mips.size(); n++)
        {
            if (n < m_packedMipInfo.NumStandardMips)
            {
                m_mips[n].heapIndex = n;
                m_mips[n].packedMip = false;
                m_mips[n].mapped = false;
                m_mips[n].startCoordinate = CD3DX12_TILED_RESOURCE_COORDINATE(0, 0, 0, n);
                m_mips[n].regionSize.Width = tilings[n].WidthInTiles;
                m_mips[n].regionSize.Height = tilings[n].HeightInTiles;
                m_mips[n].regionSize.Depth = tilings[n].DepthInTiles;
                m_mips[n].regionSize.NumTiles = tilings[n].WidthInTiles * tilings[n].HeightInTiles * tilings[n].DepthInTiles;
                m_mips[n].regionSize.UseBox = TRUE;
            }
            else
            {
                // All of the packed mips will go into the last heap.
                m_mips[n].heapIndex = heapCount - 1;
                m_mips[n].packedMip = true;
                m_mips[n].mapped = false;

                // Mark all of the packed mips as having the same start coordinate and size.
                m_mips[n].startCoordinate = CD3DX12_TILED_RESOURCE_COORDINATE(0, 0, 0, heapCount - 1);
                m_mips[n].regionSize.NumTiles = m_packedMipInfo.NumTilesForPackedMips;
                m_mips[n].regionSize.UseBox = FALSE;    // regionSize.Width/Height/Depth will be ignored.
            }
        }

        // Describe and create heaps for the mips in the mip chain to be used by the
        // reserved resource. The packed mips will all be stored in the same heap.
        // 
        // You could also put all of the mips in the same heap, but we do it this way
        // in the sample to illustrate the ability to map tiles from multiple heaps
        // into the same reserved resource.

        m_heaps.resize(heapCount);
        for (UINT n = 0; n < heapCount; n++)
        {
            const UINT heapSize = m_mips[n].regionSize.NumTiles * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

            CD3DX12_HEAP_DESC heapDesc(heapSize, D3D12_HEAP_TYPE_DEFAULT, 0, D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES);
            ThrowIfFailed(m_device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_heaps[n])));
        }

        UpdateTileMapping();

        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_reservedResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

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

// Generate and upload the texture corresponding to the active mip level then
// map it to the reserved resource.
void D3D12ReservedResources::UpdateTileMapping()
{
    UINT firstSubresource = m_mips[m_activeMip].heapIndex;
    UINT subresourceCount = m_mips[m_activeMip].packedMip ? m_packedMipInfo.NumPackedMips : 1;

    // Only update tile mappings if necessary.
    if (!m_mips[firstSubresource].mapped)
    {
        // Generate the texture data for the active mip level.
        // If the mip level corresponds to a packed mip, generate all the packed mips.
        std::vector<UINT8> texture = GenerateTextureData(firstSubresource, subresourceCount);

        // Update the tile mappings on the reserved resource.
        {
            D3D12_TILED_RESOURCE_COORDINATE startCoordinates;
            D3D12_TILE_REGION_SIZE regionSizes;
            D3D12_TILE_RANGE_FLAGS rangeFlags;
            UINT heapRangeStartOffsets;
            UINT rangeTileCounts;

            for (UINT n = 0; n < m_heaps.size(); n++)
            {
                if (!m_mips[n].mapped && n != firstSubresource)
                {
                    // Skip unchanged tile regions.
                    continue;
                }

                startCoordinates = m_mips[n].startCoordinate;
                regionSizes = m_mips[n].regionSize;

                if (n == firstSubresource)
                {
                    // Map the currently active mip.
                    rangeFlags = D3D12_TILE_RANGE_FLAG_NONE;
                    m_mips[n].mapped = true;
                }
                else
                {
                    // Unmap the previously active mip.
                    assert(m_mips[n].mapped);

                    rangeFlags = D3D12_TILE_RANGE_FLAG_NULL;
                    m_mips[n].mapped = false;
                }
                heapRangeStartOffsets = 0;        // In this sample, each heap contains only one tile region.
                rangeTileCounts = m_mips[n].regionSize.NumTiles;

                m_commandQueue->UpdateTileMappings(
                    m_reservedResource.Get(),
                    1,
                    &startCoordinates,
                    &regionSizes,
                    m_heaps[n].Get(),
                    1,
                    &rangeFlags,
                    &heapRangeStartOffsets,
                    &rangeTileCounts,
                    D3D12_TILE_MAPPING_FLAG_NONE
                );
            }
        }

        // Upload the mip(s) to the GPU and copy them to the reserved resource.
        {
            UINT mipOffset = 0;
            std::vector<D3D12_SUBRESOURCE_DATA> data(subresourceCount);
            for (UINT n = 0; n < subresourceCount; n++)
            {
                UINT currentMip = firstSubresource + n;

                data[n].pData = &texture[mipOffset];
                data[n].RowPitch = (TextureWidth >> currentMip) * TexturePixelSizeInBytes;
                data[n].SlicePitch = data[n].RowPitch * (TextureHeight >> currentMip);

                mipOffset += static_cast<UINT>(data[n].SlicePitch);
            }

            UpdateSubresources(m_commandList.Get(), m_reservedResource.Get(), m_uploadHeap.Get(), 0, firstSubresource, subresourceCount, &data[0]);
        }
    }

    m_activeMipChanged = false;

    WCHAR message[100];
    swprintf_s(message, L"Mip Level: %d", m_activeMip);
    SetCustomWindowText(message);
}

// Generate a simple red and white checkerboard texture.
std::vector<UINT8> D3D12ReservedResources::GenerateTextureData(UINT firstMip, UINT mipCount)
{
    // Determine the size of the data required by the mips(s).
    UINT dataSize = (TextureWidth >> firstMip) * (TextureHeight >> firstMip) * TexturePixelSizeInBytes;
    if (mipCount > 1)
    {
        // If generating more than 1 mip, double the size of the texture allocation
        // (you will never need more than this).
        dataSize *= 2;
    }
    std::vector<UINT8> data(dataSize);
    UINT8* pData = &data[0];

    UINT index = 0;
    for (UINT n = 0; n < mipCount; n++)
    {
        const UINT currentMip = firstMip + n;
        const UINT width = TextureWidth >> currentMip;
        const UINT height = TextureHeight >> currentMip;
        const UINT rowPitch = width * TexturePixelSizeInBytes;
        const UINT cellPitch = max(rowPitch >> 3, TexturePixelSizeInBytes);    // The width of a cell in the checkboard texture.
        const UINT cellHeight = max(height >> 3, 1);                        // The height of a cell in the checkerboard texture.
        const UINT textureSize = rowPitch * height;

        for (UINT m = 0; m < textureSize; m += TexturePixelSizeInBytes)
        {
            UINT x = m % rowPitch;
            UINT y = m / rowPitch;
            UINT i = x / cellPitch;
            UINT j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[index++] = 0xff;    // R
                pData[index++] = 0x00;    // G
                pData[index++] = 0x00;    // B
                pData[index++] = 0xff;    // A
            }
            else
            {
                pData[index++] = 0xff;    // R
                pData[index++] = 0xff;    // G
                pData[index++] = 0xff;    // B
                pData[index++] = 0xff;    // A
            }
        }
    }
    return data;
}

// Update frame-based values.
void D3D12ReservedResources::OnUpdate()
{
}

// Render the scene.
void D3D12ReservedResources::OnRender()
{
    if (m_tilingSupport)
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
}

void D3D12ReservedResources::OnDestroy()
{
    if (m_tilingSupport)
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitForGpu();

        CloseHandle(m_fenceEvent);
    }
}

void D3D12ReservedResources::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case VK_LEFT:
    case VK_UP:
        if (m_activeMip != 0)
        {
            m_activeMip--;
            m_activeMipChanged = true;
        }
        break;

    case VK_RIGHT:
    case VK_DOWN:
        if (m_activeMip < m_mips.size() - 1)
        {
            m_activeMip++;
            m_activeMipChanged = true;
        }
        break;
    }
}

// Fill the command list with all the render commands and dependent state.
void D3D12ReservedResources::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

    if (m_activeMipChanged)
    {
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_reservedResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
        UpdateTileMapping();
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_reservedResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->SetGraphicsRoot32BitConstant(0, m_activeMip, 0);
    m_commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(4, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void D3D12ReservedResources::WaitForGpu()
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
void D3D12ReservedResources::MoveToNextFrame()
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
