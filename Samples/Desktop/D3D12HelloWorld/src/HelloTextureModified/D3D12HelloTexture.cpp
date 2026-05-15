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

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include "D3D12HelloTexture.h"

#include <cstdarg>
#include <cstdio>
#include <windows.h>

#include "MyDx12Utils.h"

#include <pix3.h>

// ImGui
#define IMGUI_IMPL 1

#include <random>
int rand_0_255()
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 0xFF);
    return dist(gen);
}

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 618;
}
extern "C"
{
    __declspec(dllexport) extern const char *D3D12SDKPath = ".\\D3D12\\";
}

D3D12HelloTexture::D3D12HelloTexture(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_frameIndex(0),
      m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
      m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)), m_rtvDescriptorSize(0),
      m_descriptorSize(0)
{
}

void D3D12HelloTexture::OnInit()
{
    m_prevTime = std::chrono::steady_clock::now();
    LoadPipeline();
    LoadAssets();
    InitImGui();
}

// Load the rendering pipeline dependencies.
void D3D12HelloTexture::LoadPipeline()
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

        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = kFrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = kFrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a heap for SRV/CBV/UAV
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = kHeapDescriptorCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
        // Create a descriptor allocator to manage the descriptors in the heap.
        m_descriptorHeapAllocator.Init(m_device.Get(), m_heap.Get());

        m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    // Create the depth stencil view.
    {
        //        CreateDepthStencil(m_width, m_height);
        CreateDsvHeap();
        RegisterDepthStencil(m_width, m_height);
    }

    // create command allocators.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                       IID_PPV_ARGS(&m_frameResources[n].commandAllocator)));
    }

    //
    m_gpuWorkMeter.Init(m_device.Get(),
                        kGpuWorkMeterQueryCount); // Initialize GPU work meter with a maximum of 100 timestamp queries.
}

// Load the sample assets.
void D3D12HelloTexture::LoadAssets()
{
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned
        // will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // t0 - t(TextureCount-1) : Texture SRVs: space 0 : 0 - (kTextureCount-1)
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV[1];
        rangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, kTextureCount, 0 /*base*/, 0 /*space*/,
                          D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        // t0 : SRV structured buffer: space1 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV2[1];
        rangesSRV2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 1 /*space*/,
                           D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        // t0 : SRV structured buffer: space2 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV3[1];
        rangesSRV3[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 2 /*space*/,
                           D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_DESCRIPTOR_RANGE1 rangesCVB[1];
        rangesCVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[4];
        rootParameters[0].InitAsDescriptorTable(1, &rangesSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // Texture SRVs
        rootParameters[1].InitAsDescriptorTable(1, &rangesSRV2[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Instance data)
        rootParameters[2].InitAsDescriptorTable(1, &rangesSRV3[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Material data)
        rootParameters[3].InitAsDescriptorTable(
            1, &rangesCVB[0], D3D12_SHADER_VISIBILITY_VERTEX); // CBV for vertex shader (Per draw data)

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
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
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                    IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        UINT8 *pVertexShaderData = nullptr;
        UINT8 *pPixelShaderData = nullptr;
        UINT vertexShaderDataLength = 0;
        UINT pixelShaderDataLength = 0;

        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_VSMain.cso").c_str(), &pVertexShaderData,
                                       &vertexShaderDataLength));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_PSMain.cso").c_str(), &pPixelShaderData,
                                       &pixelShaderDataLength));

        UINT8 *pDepthVS = nullptr;
        UINT depthVSSize = 0;

        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_DepthOnlyVS_VSMain.cso").c_str(), &pDepthVS, &depthVSSize));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        D3D12_INPUT_ELEMENT_DESC depthLayout[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        //
        // Main Pass PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShaderData, vertexShaderDataLength);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShaderData, pixelShaderDataLength);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // MainPassではDepthは書かない。
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // EQUALを描画する (LESSは念のため)。
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

        //
        // Depth PrePass PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC depthPSODesc = psoDesc;
        depthPSODesc.InputLayout = {depthLayout, _countof(depthLayout)};
        depthPSODesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
        depthPSODesc.VS = CD3DX12_SHADER_BYTECODE(pDepthVS, depthVSSize);
        depthPSODesc.PS = {};                                                       // Pixel Shaderなし
        depthPSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // Depth書き込みON
        depthPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;          // Color write禁止
        depthPSODesc.NumRenderTargets = 0;                                          // 重要
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&depthPSODesc, IID_PPV_ARGS(&m_depthPrePassPSO)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              m_frameResources[m_frameIndex].commandAllocator.Get(),
                                              m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    {
        float _aspectRatio = 1.0; // m_aspectRatio
        // Define the geometry for a triangle.
        Vertex triangleVertices[] = {{{0.0f, 0.25f * _aspectRatio, 0.0f}, {0.5f, 0.0f}},
                                     {{0.25f, -0.25f * _aspectRatio, 0.0f}, {1.0f, 1.0f}},
                                     {{-0.25f, -0.25f * _aspectRatio, 0.0f}, {0.0f, 1.0f}}};

        constexpr float s = kCubeScale;
        constexpr float u = 1.f;
        Vertex cubeVertices[] = {
            // front
            {{-s, -s, -s}, {0, u}},
            {{-s, s, -s}, {0, 0}},
            {{s, s, -s}, {u, 0}},
            {{-s, -s, -s}, {0, u}},
            {{s, s, -s}, {u, 0}},
            {{s, -s, -s}, {u, u}},

            // back
            {{-s, -s, s}, {u, u}},
            {{s, s, s}, {0, 0}},
            {{-s, s, s}, {u, 0}},
            {{-s, -s, s}, {u, u}},
            {{s, -s, s}, {0, u}},
            {{s, s, s}, {0, 0}},

            // left
            {{-s, -s, s}, {0, u}},
            {{-s, s, s}, {0, 0}},
            {{-s, s, -s}, {u, 0}},
            {{-s, -s, s}, {0, u}},
            {{-s, s, -s}, {u, 0}},
            {{-s, -s, -s}, {u, u}},

            // right
            {{s, -s, -s}, {0, u}},
            {{s, s, -s}, {0, 0}},
            {{s, s, s}, {u, 0}},
            {{s, -s, -s}, {0, u}},
            {{s, s, s}, {u, 0}},
            {{s, -s, s}, {u, u}},

            // top
            {{-s, s, -s}, {0, u}},
            {{-s, s, s}, {0, 0}},
            {{s, s, s}, {u, 0}},
            {{-s, s, -s}, {0, u}},
            {{s, s, s}, {u, 0}},
            {{s, s, -s}, {u, u}},

            // bottom
            {{-s, -s, s}, {u, 0}},
            {{-s, -s, -s}, {u, u}},
            {{s, -s, -s}, {0, u}},
            {{-s, -s, s}, {u, 0}},
            {{s, -s, -s}, {0, u}},
            {{s, -s, s}, {0, 0}},
        };

        const UINT vertexBufferSize = sizeof(cubeVertices);

        m_vertexCountPerInstance = kCubeVertexCount;

        // Note: using upload heaps to transfer static data like vert buffers is not
        // recommended. Every time the GPU needs it, the upload heap will be marshalled
        // over. Please read up on Default Heap usage. An upload heap is used here for
        // code simplicity and because there are very few verts to actually transfer.
        MyDx12Util::CreateUploadBuffer(m_device, vertexBufferSize, m_vertexBuffer);

        // Copy the triangle data to the vertex buffer.
        UINT8 *pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, cubeVertices, sizeof(cubeVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    std::vector<ComPtr<ID3D12Resource>> textureUploadHeap;
    textureUploadHeap.resize(kTextureCount);

    // Create the texture.
    {
        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = kTextureWidth;
        textureDesc.Height = kTextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        m_texture.resize(kTextureCount);

        for (int i = 0; i < kTextureCount; i++)
        {
            // Create the GPU resource for the texture.
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_texture[i])));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture[i].Get(), 0, 1);

            // Create the GPU upload buffer.
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&textureUploadHeap[i])));
        }

        // Copy data to the intermediate upload heap and then schedule a copy
        // from the upload heap to the Texture2D.

        // CPUにはkTextureTypesの数だけTextureをつくる
        std::vector<std::vector<UINT8>> texture(kTextureTypes);
        for (int i = 0; i < kTextureTypes; i++)
        {
            texture[i] = GenerateCheckerboardTextureData(); // randomな色のチェッカーボードテクスチャデータを生成
        }

        for (int i = 0; i < kTextureCount; i++)
        {
            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = &texture[i % kTextureTypes][0];
            textureData.RowPitch = kTextureWidth * kTexturePixelSize;
            textureData.SlicePitch = textureData.RowPitch * kTextureHeight;

            UpdateSubresources(m_commandList.Get(), m_texture[i].Get(), textureUploadHeap[i].Get(), 0, 0, 1,
                               &textureData);
            m_commandList->ResourceBarrier(
                1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture[i].Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

            // Describe and create a SRV for the texture.
            m_texIndex[i] = AllocateTextureSRV(m_texture[i].Get());
            DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
        }
    }

    // Generate the instance data.
    m_instanceData.clear();
    m_instanceDataForCPU.clear();
    for (int i = 0; i < kInstanceCount; i++)
    {
        XMFLOAT3 pos = instanceIdToXYZ(i, GridDim(10, 10, 10));

        // CPU only
        m_instanceDataForCPU.emplace_back(pos, XMFLOAT3(0.0f, 0.0f, 0.0f));

        // CPU and GPU
        InstanceData d;
        d.materialId = i % kMaterialCount;
        XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMStoreFloat4x4(&d.world, XMMatrixTranspose(transMat));
        m_instanceData.push_back(d);
    }

    // Generate the material data.
    m_materialData.clear();
    for (int i = 0; i < kMaterialCount; i++)
    {
        Material m;
        m.textureIndex = m_texIndex[i % kTextureCount];
        m_materialData.push_back(m);
    }

    // Create the instance buffer.
    for (int n = 0; n < kFrameCount; n++)
    {
        const UINT instanceBufferSize = sizeof(InstanceData) * kInstanceCount;

        MyDx12Util::CreateUploadBuffer(m_device, instanceBufferSize, m_frameResources[n].instanceBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kInstanceCount;
        srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        m_descriptorHeapAllocator.Alloc(&cpuHandle);
        m_device->CreateShaderResourceView(m_frameResources[n].instanceBuffer.Get(), &srvDesc, cpuHandle);

        m_frameResources[n].instanceBuffer->Map(0, nullptr,
                                                reinterpret_cast<void **>(&m_frameResources[n].pSrvDataBegin));
        memcpy(m_frameResources[n].pSrvDataBegin, m_instanceData.data(), instanceBufferSize);
        m_frameResources[n].instanceBuffer->Unmap(0, nullptr);
    }

    // Create SRV for material buffer (StructuredBuffer)
    {
        const UINT materialBufferSize = sizeof(Material) * kMaterialCount;

        MyDx12Util::CreateUploadBuffer(m_device, materialBufferSize, m_materialBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaterialCount;
        srvDesc.Buffer.StructureByteStride = sizeof(Material);

        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        m_descriptorHeapAllocator.Alloc(&handle);
        m_device->CreateShaderResourceView(m_materialBuffer.Get(), &srvDesc, handle);
        m_materialBuffer->Map(0, nullptr, reinterpret_cast<void **>(&pMaterialDataBegin));
        memcpy(pMaterialDataBegin, m_materialData.data(), materialBufferSize);
        m_materialBuffer->Unmap(0, nullptr);
    }

    m_camerasForCPU.clear();
    {
        m_camerasForCPU.emplace_back(XMFLOAT3(0.0f, 0.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 60.0f, m_aspectRatio,
                                     0.1f, 10000.0f);
        XMStoreFloat4x4(&m_constantBufferData.viewProjection, m_camerasForCPU[0].viewProjection);
    }

    // Create the CBV for the constant buffer.
    {
        const UINT constantBufferSize = sizeof(ConstantBuffer); // CB size is required to be 256-byte aligned.

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_constantBuffer)));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;

        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        m_descriptorHeapAllocator.Alloc(&handle);
        m_device->CreateConstantBufferView(&cbvDesc, handle);

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void **>(&m_pCbvDataBegin)));
        memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList *ppCommandLists[] = {m_commandList.Get()};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_frameResources[m_frameIndex].fenceValue = 1;

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

static SimpleDescriptorHeapAllocator *g_allocator = nullptr;

void D3D12HelloTexture::InitImGui()
{
#if IMGUI_IMPL > 0
    g_allocator = &m_descriptorHeapAllocator;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(Win32Application::GetHwnd());

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = m_device.Get();
    init_info.CommandQueue = m_commandQueue.Get();
    init_info.NumFramesInFlight = kFrameCount;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
    init_info.SrvDescriptorHeap = m_heap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle)
    { g_allocator->Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    { g_allocator->Free(&cpu_handle, &gpu_handle); };
    ImGui_ImplDX12_Init(&init_info);
#endif
}

UINT D3D12HelloTexture::AllocateTextureSRV(ID3D12Resource *texture)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    UINT index = m_descriptorHeapAllocator.Alloc(&cpuHandle);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(texture, &srvDesc, cpuHandle);

    return index; // ← これがGPUで使うID
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> D3D12HelloTexture::GenerateCheckerboardTextureData()
{
    const UINT rowPitch = kTextureWidth * kTexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;       // The width of a cell in the checkboard texture.
    const UINT cellHeight = kTextureWidth >> 3; // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * kTextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8 *pData = &data[0];

    UINT8 R = rand_0_255();
    UINT8 G = rand_0_255();
    UINT8 B = rand_0_255();

    // DBG_PRINT("R=%d G=%d B=%d\n", R, G, B);

    for (UINT n = 0; n < textureSize; n += kTexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n + 0] = 0x00; // R
            pData[n + 1] = 0x00; // G
            pData[n + 2] = 0x00; // B
            pData[n + 3] = 0xff; // A
        }
        else
        {
            pData[n + 0] = R;    // R
            pData[n + 1] = G;    // G
            pData[n + 2] = B;    // B
            pData[n + 3] = 0xff; // A
        }
    }

    return data;
}

void D3D12HelloTexture::CreateDsvHeap()
{
    if (m_dsvHeap)
        return;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void D3D12HelloTexture::RegisterDepthStencil(UINT width, UINT height)
{
    TransientResource r;

    r.state = TransientResourceState::Initialized;
    r.name = kDepthStencilResourceName;
    r.persistent = true;

    r.desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    r.desc.Width = width;
    r.desc.Height = height;
    r.desc.DepthOrArraySize = 1;
    r.desc.MipLevels = 1;
    r.desc.Format = DXGI_FORMAT_D32_FLOAT;
    r.desc.SampleDesc.Count = 1;
    r.desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    r.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    r.clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    r.clearValue.DepthStencil.Depth = 1.0f;
    r.clearValue.DepthStencil.Stencil = 0;

    r.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    m_transientResources[kDepthStencilResourceName] = std::move(r);
}

void D3D12HelloTexture::CreateDepthStencil(UINT width, UINT height)
{
    // Release if DS exist
    m_depthStencil.Reset();

    // Create Depth Resource
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                    &clearValue, IID_PPV_ARGS(&m_depthStencil)));

    // Create DSV
    m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

// Update frame-based values.
void D3D12HelloTexture::OnUpdate()
{
    PIXBeginEvent(0, L"OnUpdate");

    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_prevTime).count();
    static float accumTime = 0.f;
    m_prevTime = now;

    if (m_isPlaying)
    {

        // InstanceBufferのmaterialのtextureIdを1秒ごと切り替える
        accumTime += deltaTime;
        if (accumTime > 1.0f)
        {
            for (int i = 0; i < kInstanceCount; i++)
            {
                m_instanceData[i].materialId = (m_instanceData[i].materialId + 1) % kMaterialCount;
            }
            accumTime = 0.f;
        }

        // InstanceBufferのオフセットを毎フレーム更新する
        for (int i = 0; i < kInstanceCount; i++)
        {

            m_instanceDataForCPU[i].pos.x += kTranslationSpeed;
            if (m_instanceDataForCPU[i].pos.x > kOffsetBounds)
            {
                m_instanceDataForCPU[i].pos.x = -kOffsetBounds;
            }
            m_instanceDataForCPU[i].rot.x += kRotationSpeed;
            if (m_instanceDataForCPU[i].rot.x >= 2.0 * kPI)
            {
                m_instanceDataForCPU[i].rot.x = 0.f;
            }
            m_instanceDataForCPU[i].rot.y += kRotationSpeed;
            if (m_instanceDataForCPU[i].rot.y >= 2.0 * kPI)
            {
                m_instanceDataForCPU[i].rot.y = 0.f;
            }
            m_instanceDataForCPU[i].rot.z += kRotationSpeed;
            if (m_instanceDataForCPU[i].rot.z >= 2.0 * kPI)
            {
                m_instanceDataForCPU[i].rot.z = 0.f;
            }

            XMMATRIX transMat = XMMatrixTranslation(m_instanceDataForCPU[i].pos.x, m_instanceDataForCPU[i].pos.y,
                                                    m_instanceDataForCPU[i].pos.z);

            XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_instanceDataForCPU[i].rot.x, m_instanceDataForCPU[i].rot.y,
                                                           m_instanceDataForCPU[i].rot.z);

            // XMMATRIX rotMat = XMMatrixRotationZ(m_instanceDataForCPU[i].rot.z);
            XMMATRIX worldMat = rotMat * transMat;

            // XMStoreFloat4x4(&m_instanceData[i].world, worldMat);
            XMStoreFloat4x4(&m_instanceData[i].world, XMMatrixTranspose(worldMat));
        }
    }

    m_frameResources[m_frameIndex].instanceBuffer->Map(
        0, nullptr, reinterpret_cast<void **>(&m_frameResources[m_frameIndex].pSrvDataBegin));
    memcpy(m_frameResources[m_frameIndex].pSrvDataBegin, m_instanceData.data(), sizeof(InstanceData) * kInstanceCount);
    m_frameResources[m_frameIndex].instanceBuffer->Unmap(0, nullptr);

    if (GetForegroundWindow() == Win32Application::GetHwnd())
    {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            m_isPlaying = !m_isPlaying;
            Sleep(200); // スペースキーのトグルが速すぎるのを防止
        }

        if (GetAsyncKeyState('A') & 0x8000)
        {
            m_camerasForCPU[0].pos.x -= kCameraMoveSpeed;
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            m_camerasForCPU[0].pos.x += kCameraMoveSpeed;
        }

        if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.y -= kCameraMoveSpeed;
        }
        if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.y += kCameraMoveSpeed;
        }
        if ((GetAsyncKeyState('W') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.z += kCameraMoveSpeed;
        }
        if ((GetAsyncKeyState('S') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.z -= kCameraMoveSpeed;
        }
    }

    m_camerasForCPU[0].updateAllMatrix();
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(m_camerasForCPU[0].viewProjection));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

void D3D12HelloTexture::UpdateImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug");

    ImGui::Text("Hello ImGui");
    ImGui::Text("FrameIndex: %d", m_frameIndex);
    ImGui::SliderFloat("Camera FovH", &m_camerasForCPU[0].fov, 20.f, 150.f);

    ImGui::Text("CPU Frame: %.2f ms (%.1f FPS)", m_cpuFrameTime, 1000.0f / m_cpuFrameTime);

    {
        auto &gpuCeckPoints = m_frameResources[m_fremeIndexPrevious].gpuWorkMeterCheckPoints;
        size_t gpuCheckPointCount = gpuCeckPoints.size();

        if (gpuCheckPointCount >= 2)
        {

            for (int i = 1; i < gpuCheckPointCount; i++)
            {
                auto &checkPoint = gpuCeckPoints[i];

                if (i < gpuCheckPointCount - 1)
                {
                    float timeFromPrevious = checkPoint.timeStamp - gpuCeckPoints[i - 1].timeStamp;
                    ImGui::Text("GPU[%d] %s: %f ms", i, checkPoint.name.c_str(), timeFromPrevious);
                }
                else
                {
                    ImGui::Text("GPU[%d] Total: %f ms", i, checkPoint.timeStamp);
                }
            }
        }
    }
    ImGui::End();
    ImGui::Render();
}

// Render the scene.
void D3D12HelloTexture::OnRender()
{
    PIXBeginEvent(0, L"OnRender");

    // ImGui frame update
#if IMGUI_IMPL > 0
    UpdateImGui();
#endif

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList.Get()};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    UINT64 submittedFenceValue = MoveToNextFrame();

    // submittedFenceValue = 今回SubmitしたCommandListが完了したことを示すFence値
    MarkPendingTransientResources(submittedFenceValue);

    CollectGarbageTransientResources();

    m_gpuWorkMeter.ReadbackData(m_commandQueue.Get());

    PIXEndEvent();
}

void D3D12HelloTexture::OnWindowSizeChanged(UINT width, UINT height)
{
    m_pendingResize = true;
    m_pendingResizeWidth = width;
    m_pendingResizeHeight = height;
}

void D3D12HelloTexture::OnIdle()
{
    if (m_pendingResize)
    {
        Resize(m_pendingResizeWidth, m_pendingResizeHeight);
        m_pendingResize = false;
    }

    m_workMeter.Start();
    OnUpdate();
    OnRender();
    m_workMeter.End();
    m_cpuFrameTime = m_workMeter.GetCpuFrameTimeMs();
}

void D3D12HelloTexture::Resize(UINT width, UINT height)
{
    DBG_PRINT("D3D12HelloTexture::OnWindowSizeChanged() %d %d\n", width, height);
    m_width = width;
    m_height = height;

    if (width == 0 || height == 0)
    {
        return;
    }

    if (m_device.Get() == nullptr || m_swapChain.Get() == nullptr)
    {
        return;
    }

    FlushGpu();

    // Clear RTV
    for (UINT n = 0; n < kFrameCount; n++)
    {
        m_renderTargets[n].Reset();
    }

    // Resize SwapChain
    m_swapChain->ResizeBuffers(kFrameCount, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    // ★重要
    m_fremeIndexPrevious = m_frameIndex;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Re-create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    m_depthStencil.Reset();
    m_transientResources.erase(kDepthStencilResourceName);
    RegisterDepthStencil(m_width, m_height);

    // Camera
    m_camerasForCPU[0].aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    m_camerasForCPU[0].updateAllMatrix();

    // Screen
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(m_width), static_cast<FLOAT>(m_height),
                                  D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    // Imgui
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
}

void D3D12HelloTexture::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloTexture::PopulateCommandList()
{
    PIXBeginEvent(1, L"PopulateCommandList");

    BeginFrame();
    ResetResourceStates();
    BuildRenderPasses();
    AnalyzeResourceLifetimes();
    // DebugPrintLifetimes();
    ExecutePasses();
    EndFrame();

    PIXEndEvent();
}

void D3D12HelloTexture::BuildRenderPasses()
{
    m_renderPasses.clear();
    AddPass(L"Clear", {},
            MakeResourceUsageMap(
                {{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE}}),
            [this]() { RecordClear(); });
    AddPass(L"Depth PrePass", {},
            MakeResourceUsageMap({{kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE}}),
            [this]() { RecordDepthPrePass(); });
    AddPass(L"MainPass",
            MakeResourceUsageMap({{kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE}}),
            MakeResourceUsageMap(
                {{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET}}),
            [this]() { RecordMainPass(); });
    AddPass(L"ImGui", {},
            MakeResourceUsageMap(
                {{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET}}),
            [this]() { RecordImGuiPass(); });
}

void D3D12HelloTexture::AddPass(const wchar_t *name, ResourceUsageMap reads, ResourceUsageMap writes,
                                std::function<void()> execute)
{
    m_renderPasses.push_back({name, reads, writes, execute});
}

auto D3D12HelloTexture::MakeResourceUsageMap(std::initializer_list<ResourceUsage> usages) const -> ResourceUsageMap
{
    ResourceUsageMap usageMap;
    for (const ResourceUsage &usage : usages)
    {
        usageMap.emplace(usage.name, usage);
    }

    return usageMap;
}

void D3D12HelloTexture::AnalyzeResourceLifetimes()
{
    m_resourceLifetimes.clear();

    for (int passIndex = 0; passIndex < static_cast<int>(m_renderPasses.size()); ++passIndex)
    {
        const auto &pass = m_renderPasses[passIndex];

        for (const auto &[name, usage] : pass.reads)
        {
            auto &lifetime = m_resourceLifetimes[name];
            lifetime.firstPass = (std::min)(lifetime.firstPass, passIndex);
            lifetime.lastPass = (std::max)(lifetime.lastPass, passIndex);
        }

        for (const auto &[name, usage] : pass.writes)
        {
            auto &lifetime = m_resourceLifetimes[name];
            lifetime.firstPass = (std::min)(lifetime.firstPass, passIndex);
            lifetime.lastPass = (std::max)(lifetime.lastPass, passIndex);
        }
    }
}

void D3D12HelloTexture::DebugPrintLifetimes()
{
    DBG_PRINT("Resource Lifetimes:\n");
    for (auto &[name, lt] : m_resourceLifetimes)
    {
        DBG_PRINT("Resource %s: [%d - %d]\n", name.c_str(), lt.firstPass, lt.lastPass);
    }
}

void D3D12HelloTexture::ExecutePasses()
{
#if 0
    for (const RenderPass &pass : m_renderPasses)
    {
        TransitionPassResources(pass);
        pass.execute();
    }
#else
    for (int passIndex = 0; passIndex < static_cast<int>(m_renderPasses.size()); ++passIndex)
    {
        CreateResourcesForPass(passIndex);

        const RenderPass &pass = m_renderPasses[passIndex];
        TransitionPassResources(pass);
        pass.execute();

        ReleaseResourcesAfterPass(passIndex);
    }
#endif
}

void D3D12HelloTexture::CreateResourcesForPass(int passIndex)
{
    for (auto &[name, lt] : m_resourceLifetimes)
    {
        if (lt.firstPass != passIndex)
            continue;

        if (name == kBackBufferResourceName)
            continue;

        if (!m_transientResources.contains(name))
            continue;

        auto &tr = m_transientResources.at(name);
        if (tr.state == TransientResourceState::Uninitialized)
        {
            assert(false && "Transient resource must be registered before use.");
            DBG_PRINT("Resource %s is uninitialized.\n", name.c_str());
            continue;
        }

        if (tr.state == TransientResourceState::Created)
            continue;

        if (tr.state == TransientResourceState::PendingRelease1 || tr.state == TransientResourceState::PendingRelease2)
        {
            tr.retireFenceValue = 0;
            tr.state = TransientResourceState::Created;
            DBG_PRINT("Resource %s reused before release.\n", name.c_str());
            continue;
        }

        if (tr.state != TransientResourceState::Initialized)
            continue;

        ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                        D3D12_HEAP_FLAG_NONE, &tr.desc, tr.initialState, &tr.clearValue,
                                                        IID_PPV_ARGS(&tr.resource)));

        tr.state = TransientResourceState::Created;

        if (name == kDepthStencilResourceName)
        {
            m_depthStencil = tr.resource;
            m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr,
                                             m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        }
        else
        {
            assert(false && "Unsupported resource in CreateResourceForPass()");
        }

        DBG_PRINT("Resource %s created.\n", name.c_str());
    }
}

void D3D12HelloTexture::ReleaseResourcesAfterPass(int passIndex)
{
    for (auto &[name, lt] : m_resourceLifetimes)
    {
        if (lt.lastPass != passIndex)
            continue;

        // lastPassなら解放してよい。
        if (name == kBackBufferResourceName) // BackBufferは決して解放しない
            continue;

        if (!m_transientResources.contains(name))
            continue;

        // TransientResouceを取得できた。
        auto &tr = m_transientResources.at(name);

        if (tr.state == TransientResourceState::Uninitialized)
        {
            assert(false && "Transient resource must be registered before release.");
            DBG_PRINT("Resource %s is uninitialized.\n", name.c_str());
            continue;
        }

        if (tr.persistent)
            continue;

        if (tr.state != TransientResourceState::Created)
            continue;

        // ここではGPU fence値がまだ確定していないので、
        // 「このフレーム終了後に解放候補」とだけ記録する。
        tr.state = TransientResourceState::PendingRelease1;

        DBG_PRINT("Resource %s endOfLife.\n", name.c_str());
    }
}

void D3D12HelloTexture::MarkPendingTransientResources(UINT64 fenceValue)
{
    for (auto &[name, tr] : m_transientResources)
    {
        if (tr.state != TransientResourceState::PendingRelease1)
            continue;

        tr.retireFenceValue = fenceValue;
        tr.state = TransientResourceState::PendingRelease2;

        DBG_PRINT("Resource %s waitFenceValue.\n", name.c_str(), tr.retireFenceValue);
    }
}

void D3D12HelloTexture::CollectGarbageTransientResources()
{
    const UINT64 completed = m_fence->GetCompletedValue();

    for (auto &[name, tr] : m_transientResources)
    {
        if (tr.state != TransientResourceState::PendingRelease2)
            continue;

        if (completed < tr.retireFenceValue)
            continue;

        if (name == kDepthStencilResourceName)
        {
            m_depthStencil.Reset();
        }

        tr.resource.Reset();
        tr.retireFenceValue = 0;
        tr.state = TransientResourceState::Initialized;

        DBG_PRINT("Resource %s released.\n", name.c_str());
    }
}

void D3D12HelloTexture::ResetResourceStates()
{
    m_resourceStates.clear();
    SetResourceState(kBackBufferResourceName, D3D12_RESOURCE_STATE_PRESENT);
    SetResourceState(kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void D3D12HelloTexture::TransitionPassResources(const RenderPass &pass)
{
    for (const auto &read : pass.reads)
    {
        TransitionResource(read.second);
    }

    for (const auto &write : pass.writes)
    {
        TransitionResource(write.second);
    }
}

void D3D12HelloTexture::TransitionResource(const ResourceUsage &usage)
{
    D3D12_RESOURCE_STATES currentState = GetResourceState(usage.name);
    if (currentState == usage.state)
    {
        return;
    }

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(usage.resource, currentState, usage.state));
    SetResourceState(usage.name, usage.state);
}

D3D12_RESOURCE_STATES D3D12HelloTexture::GetResourceState(const std::string &name) const
{
    auto resourceState = m_resourceStates.find(name);
    if (resourceState != m_resourceStates.end())
    {
        return resourceState->second;
    }

    return D3D12_RESOURCE_STATE_COMMON;
}

void D3D12HelloTexture::SetResourceState(const std::string &name, D3D12_RESOURCE_STATES state)
{
    m_resourceStates[name] = state;
}

void D3D12HelloTexture::BeginFrame()
{

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_frameResources[m_frameIndex].commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_frameIndex].commandAllocator.Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap *ppHeaps[] = {m_heap.Get()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // texture SRV is at descriptor 0 - (TextureCount-1)
    m_commandList->SetGraphicsRootDescriptorTable(0, m_heap->GetGPUDescriptorHandleForHeapStart());

    // instance buffer SRV is at descriptor TextureCount
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_heap->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(kTextureCount + m_frameIndex, m_descriptorSize);
    m_commandList->SetGraphicsRootDescriptorTable(1, handle);

    // material buffer SRV is at descriptor TextureCount + FrameCount
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle2(m_heap->GetGPUDescriptorHandleForHeapStart());
    handle2.Offset(kTextureCount + 2, m_descriptorSize);
    m_commandList->SetGraphicsRootDescriptorTable(2, handle2);

    // constant buffer is at descriptor TextureCount + FrameCount + 1
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle3(m_heap->GetGPUDescriptorHandleForHeapStart());
    handle3.Offset(kTextureCount + 3, m_descriptorSize);
    m_commandList->SetGraphicsRootDescriptorTable(3, handle3);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);
#if 0
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
#endif
    m_gpuWorkMeter.StartGpu(m_commandList.Get(), m_frameResources[m_frameIndex].gpuWorkMeterCheckPoints);
}

void D3D12HelloTexture::RecordClear()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Clear");
}

//
// Depth Pre-pass
//
void D3D12HelloTexture::RecordDepthPrePass()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    m_commandList->SetPipelineState(m_depthPrePassPSO.Get());
    m_commandList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

    PIXBeginEvent(m_commandList.Get(), 0, L"DepthPrepass");

    m_commandList->DrawInstanced(m_vertexCountPerInstance, kInstanceCount, 0, 0);

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Depth Prepass");
}

//
// Main Pass
//
void D3D12HelloTexture::RecordMainPass()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    PIXBeginEvent(m_commandList.Get(), 0, L"MainPass");

    m_commandList->SetPipelineState(m_pipelineState.Get());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    m_commandList->DrawInstanced(m_vertexCountPerInstance, kInstanceCount, 0, 0);
    PIXEndEvent(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Main Pass");
}

void D3D12HelloTexture::RecordImGuiPass()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex,
                                            m_rtvDescriptorSize);

#if IMGUI_IMPL > 0
    {
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        ID3D12DescriptorHeap *heaps[] = {m_heap.Get()};
        m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
    }
#endif

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ImGUI");
}

void D3D12HelloTexture::EndFrame()
{
    m_gpuWorkMeter.EndGpu(m_commandList.Get());

    TransitionResource({kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT});

    ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void D3D12HelloTexture::WaitForGpu()
{
    PIXBeginEvent(3, L"WaitForGpu");

    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_frameResources[m_frameIndex].fenceValue));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_frameResources[m_frameIndex].fenceValue, m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_frameResources[m_frameIndex].fenceValue++;

    PIXEndEvent();
}

void D3D12HelloTexture::FlushGpu()
{
    for (UINT n = 0; n < kFrameCount; n++)
    {
        const UINT64 fenceValue = ++m_frameResources[n].fenceValue;

        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValue));

        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));

        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

// Prepare to render the next frame.
UINT64 D3D12HelloTexture::MoveToNextFrame()
{
    PIXBeginEvent(2, L"MoveToNextFrame");

    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_frameResources[m_frameIndex].fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_fremeIndexPrevious = m_frameIndex;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_frameResources[m_frameIndex].fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_frameResources[m_frameIndex].fenceValue, m_fenceEvent));
        PIXBeginEvent(4, L"WaitForSingleObjectEx");
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
        PIXEndEvent();
    }

    // Set the fence value for the next frame.
    m_frameResources[m_frameIndex].fenceValue = currentFenceValue + 1;

    PIXEndEvent();

    return currentFenceValue;
}
