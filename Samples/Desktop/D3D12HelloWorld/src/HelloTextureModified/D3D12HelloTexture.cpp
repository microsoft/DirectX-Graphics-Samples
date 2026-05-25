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

#include "GltfLoader.h"

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
        rtvHeapDesc.NumDescriptors = kRTVDescriptorCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a heap for SRV/CBV/UAV
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = kMainHeapDescriptorCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
        // Create a descriptor allocator to manage the descriptors in the heap.
        m_descriptorHeapAllocator.Init(m_device.Get(), m_heap.Get());

        D3D12_DESCRIPTOR_HEAP_DESC imguiHeapDesc = {};
        imguiHeapDesc.NumDescriptors = kHeapDescriptorCount;
        imguiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        imguiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&imguiHeapDesc, IID_PPV_ARGS(&m_imguiHeap)));
        m_ImGuiDescriptorHeapAllocator.Init(m_device.Get(), m_imguiHeap.Get());

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

std::array<GltfVertex, D3D12HelloTexture::kCubeVertexCount> D3D12HelloTexture::CreateCubeVertices() const
{

    constexpr float s = kCubeScale;
    constexpr float u = 1.f;

    return {{
        // front
        {{-s, -s, -s}, {0, u}, {0.0f, 0.0f, -1.0f}},
        {{-s, s, -s}, {0, 0}, {0.0f, 0.0f, -1.0f}},
        {{s, s, -s}, {u, 0}, {0.0f, 0.0f, -1.0f}},
        {{-s, -s, -s}, {0, u}, {0.0f, 0.0f, -1.0f}},
        {{s, s, -s}, {u, 0}, {0.0f, 0.0f, -1.0f}},
        {{s, -s, -s}, {u, u}, {0.0f, 0.0f, -1.0f}},

        // back
        {{-s, -s, s}, {u, u}, {0.0f, 0.0f, 1.0f}},
        {{s, s, s}, {0, 0}, {0.0f, 0.0f, 1.0f}},
        {{-s, s, s}, {u, 0}, {0.0f, 0.0f, 1.0f}},
        {{-s, -s, s}, {u, u}, {0.0f, 0.0f, 1.0f}},
        {{s, -s, s}, {0, u}, {0.0f, 0.0f, 1.0f}},
        {{s, s, s}, {0, 0}, {0.0f, 0.0f, 1.0f}},

        // left
        {{-s, -s, s}, {0, u}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, s}, {0, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, -s}, {u, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, -s, s}, {0, u}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, -s}, {u, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, -s, -s}, {u, u}, {-1.0f, 0.0f, 0.0f}},

        // right
        {{s, -s, -s}, {0, u}, {1.0f, 0.0f, 0.0f}},
        {{s, s, -s}, {0, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {1.0f, 0.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, -s, s}, {u, u}, {1.0f, 0.0f, 0.0f}},

        // top
        {{-s, s, -s}, {0, u}, {0.0f, 1.0f, 0.0f}},
        {{-s, s, s}, {0, 0}, {0.0f, 1.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {0.0f, 1.0f, 0.0f}},
        {{-s, s, -s}, {0, u}, {0.0f, 1.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {0.0f, 1.0f, 0.0f}},
        {{s, s, -s}, {u, u}, {0.0f, 1.0f, 0.0f}},

        // bottom
        {{-s, -s, s}, {u, 0}, {0.0f, -1.0f, 0.0f}},
        {{-s, -s, -s}, {u, u}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {0.0f, -1.0f, 0.0f}},
        {{-s, -s, s}, {u, 0}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, s}, {0, 0}, {0.0f, -1.0f, 0.0f}},
    }};
}

DescriptorHeapHandle D3D12HelloTexture::CreateTextureFromRGBA8(const UINT8 *pixels, UINT width, UINT height,
                                                               ComPtr<ID3D12Resource> &texture,
                                                               ComPtr<ID3D12Resource> &uploadHeap)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    // Create the GPU resource for the texture.
    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                    nullptr, IID_PPV_ARGS(&texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(
        m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadHeap)));

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = pixels;
    textureData.RowPitch = width * kTexturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * kTextureHeight;

    UpdateSubresources(m_commandList.Get(), texture.Get(), uploadHeap.Get(), 0, 0, 1, &textureData);

    m_commandList->ResourceBarrier(1,
                                   &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    return AllocateTextureSRV(texture.Get());
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

        // t0 - t3 : GBuffer SRVs, t4 : depth SRV, space 3
        CD3DX12_DESCRIPTOR_RANGE1 rangesGBufferSRV[1];
        rangesGBufferSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBuffer::kCount + 1, 0 /*base*/, 3 /*space*/,
                                 D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        CD3DX12_DESCRIPTOR_RANGE1 rangesCVB[1];
        rangesCVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_DESCRIPTOR_RANGE1 rangesLightCBV[1];
        rangesLightCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[7];
        rootParameters[0].InitAsDescriptorTable(1, &rangesSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // Texture SRVs
        rootParameters[1].InitAsDescriptorTable(1, &rangesSRV2[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Instance data)
        rootParameters[2].InitAsDescriptorTable(1, &rangesSRV3[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Material data)
        rootParameters[3].InitAsDescriptorTable(1, &rangesCVB[0], D3D12_SHADER_VISIBILITY_ALL); // Camera constants
        rootParameters[4].InitAsDescriptorTable(1, &rangesGBufferSRV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer SRVs
        rootParameters[5].InitAsDescriptorTable(1, &rangesLightCBV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL);    // Light constants
        rootParameters[6].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer debug target

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
#if 1
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
#else
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
#endif
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

        UINT8 *pGBufferVS = nullptr;
        UINT8 *pGBufferPS = nullptr;
        UINT gbufferVSSize = 0;
        UINT gbufferPSSize = 0;
        UINT8 *pGBufferDebugVS = nullptr;
        UINT8 *pGBufferDebugPS = nullptr;
        UINT gbufferDebugVSSize = 0;
        UINT gbufferDebugPSSize = 0;

        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_GBuffer_VSMain.cso").c_str(), &pGBufferVS, &gbufferVSSize));
        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_GBuffer_PSMain.cso").c_str(), &pGBufferPS, &gbufferPSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_GBufferDebug_VSMain.cso").c_str(), &pGBufferDebugVS,
                                       &gbufferDebugVSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_GBufferDebug_PSMain.cso").c_str(), &pGBufferDebugPS,
                                       &gbufferDebugPSSize));

        UINT8 *pLightPassVS = nullptr;
        UINT8 *pLightPassPS = nullptr;
        UINT lightPassVSSize = 0;
        UINT lightPassPSSize = 0;

        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPass_VSMain.cso").c_str(), &pLightPassVS,
                                       &lightPassVSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPass_PSMain.cso").c_str(), &pLightPassPS,
                                       &lightPassPSSize));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

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
        // GBuffer PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferPSODesc = MyDx12Util::CreateGBufferPSODesc(
            psoDesc, pGBufferVS, gbufferVSSize, pGBufferPS, gbufferPSSize, m_gbuffer.formats, GBuffer::kCount);
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&gbufferPSODesc, IID_PPV_ARGS(&m_gbufferPSO)));

        //
        // LightPass PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC lightPassPSODesc = MyDx12Util::CreateFullscreenPassPSODesc(
            psoDesc, pLightPassVS, lightPassVSSize, pLightPassPS, lightPassPSSize, DXGI_FORMAT_R8G8B8A8_UNORM);
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&lightPassPSODesc, IID_PPV_ARGS(&m_lightPassPSO)));

        //
        // GBuffer Debug PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferDebugPSODesc =
            MyDx12Util::CreateFullscreenPassPSODesc(psoDesc, pGBufferDebugVS, gbufferDebugVSSize, pGBufferDebugPS,
                                                    gbufferDebugPSSize, DXGI_FORMAT_R8G8B8A8_UNORM);
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&gbufferDebugPSODesc, IID_PPV_ARGS(&m_gbufferDebugPSO)));

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

    //
    CreateGBuffer();

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              m_frameResources[m_frameIndex].commandAllocator.Get(),
                                              m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    GltfMeshData mesh;

    std::vector<GltfVertex> vertices_;
    UINT vertexBufferSize_;

    if constexpr (kGltfLoadingEnabled)
    {
        bool loaded = LoadGltfMesh("Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", mesh);
        assert(loaded);
    }

    if constexpr (kGltfLoadingEnabled && kGltfMeshDisplay)
    {
        const UINT size = static_cast<UINT>(sizeof(GltfVertex) * mesh.vertices.size());
        m_indexCountPerInstance = static_cast<UINT>(mesh.indices.size());
        m_vertexCountPerInstance = static_cast<UINT>(mesh.vertices.size());
        vertices_ = mesh.vertices;
        vertexBufferSize_ = size;
    }
    else
    {
        auto a = CreateCubeVertices();
        vertices_ = std::vector<GltfVertex>(a.begin(), a.end());
        m_vertexCountPerInstance = static_cast<UINT>(vertices_.size());
        vertexBufferSize_ = static_cast<UINT>(sizeof(GltfVertex) * vertices_.size());
    }

    const std::vector<GltfVertex> vertices = vertices_;
    const UINT vertexBufferSize = vertexBufferSize_;

    // Note: using upload heaps to transfer static data like vert buffers is not
    // recommended. Every time the GPU needs it, the upload heap will be marshalled
    // over. Please read up on Default Heap usage. An upload heap is used here for
    // code simplicity and because there are very few verts to actually transfer.
    MyDx12Util::CreateUploadBuffer(m_device, vertexBufferSize, m_vertexBuffer);

    // Copy the triangle data to the vertex buffer.
    UINT8 *pVertexDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertices.data(), vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(GltfVertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    if constexpr (kGltfLoadingEnabled)
    {
        const UINT indexBufferSize = static_cast<UINT>(mesh.indices.size() * sizeof(uint32_t));

        MyDx12Util::CreateUploadBuffer(m_device, indexBufferSize, m_indexBuffer);

        UINT8 *pIndexDataBegin = nullptr;
        ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, mesh.indices.data(), indexBufferSize);
        m_indexBuffer->Unmap(0, nullptr);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.

    std::vector<ComPtr<ID3D12Resource>> textureUploadHeap;

    if constexpr (kGltfLoadingEnabled)
    {
        // randomな色のチェッカーボードテクスチャをkTextureCount毎生成
        textureUploadHeap.resize(kTextureCount);

        // glTFファイルからテクスチャを読み込む
        m_texture.resize(kTextureCount);

        std::vector<std::vector<UINT8>> texture(kTextureTypes);

        m_gltfTextureCount = static_cast<UINT>(mesh.textures.size());

        DBG_PRINT("m_gltfTextureCount = %d\n", m_gltfTextureCount);

        for (size_t i = 0; i < kTextureCount; i++)
        {
            bool useGltfTex = i < mesh.textures.size();
            UINT8 *pixels = nullptr;
            UINT width, height;

            if (useGltfTex)
            {
                // Gltfファイルのテクスチャがあれば使う。
                const auto &tex = mesh.textures[i];
                pixels = (UINT8 *)tex.pixels.data();
                width = tex.width;
                height = tex.height;
                DBG_PRINT("[%d] gltfTexture :width %d height %d\n", i, tex.width, tex.height);
            }
            else
            {
                // Gltfファイルのテクスチャが足りなければチェッカーボードテクスチャを使う。
                texture[i] = GenerateCheckerboardTextureData();
                pixels = &texture[i % kTextureTypes][0];
                width = kTextureWidth;
                height = kTextureHeight;
                DBG_PRINT("[%d] CheckerBoardTexture :width %d height %d\n", i, kTextureWidth, kTextureHeight);
            }

            DescriptorHeapHandle srv =
                CreateTextureFromRGBA8(pixels, width, height, m_texture[i], textureUploadHeap[i]);
            if (i == 0)
            {
                m_textureTableStart = srv;
            }
            m_texIndex[i] = srv.Index - m_textureTableStart.Index;
            DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
        }
    }
    else
    {
        // randomな色のチェッカーボードテクスチャをkTextureCount毎生成
        textureUploadHeap.resize(kTextureCount);

        m_texture.resize(kTextureCount);

        // CPUメモリ状にテクスチャを作る

        std::vector<std::vector<UINT8>> texture(kTextureTypes);
        for (int i = 0; i < kTextureTypes; i++)
        {
            texture[i] = GenerateCheckerboardTextureData();

            DescriptorHeapHandle srv = CreateTextureFromRGBA8(&texture[i % kTextureTypes][0], kTextureWidth,
                                                              kTextureHeight, m_texture[i], textureUploadHeap[i]);
            if (i == 0)
            {
                m_textureTableStart = srv;
            }
            m_texIndex[i] = srv.Index - m_textureTableStart.Index;
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

        if constexpr (kGltfLoadingEnabled)
        {
            // glTFのマテリアル数に応じてmaterialIdを割り当てる。足りない分は0番のマテリアルを使う。
            const UINT gltfMaterialCount = static_cast<UINT>(mesh.materials.size());
            d.materialId = gltfMaterialCount > 0 ? i % gltfMaterialCount : 0;
        }
        else
        {
            d.materialId = i % kMaterialCount;
        }

        XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMStoreFloat4x4(&d.world, XMMatrixTranspose(transMat));
        d.prevWorld = d.world;
        m_instanceData.push_back(d);
    }

    // Generate the material data.
    m_materialData.clear();
    const auto resolveTextureIndex = [this](int gltfTextureIndex, UINT fallbackIndex) -> UINT
    {
        if (gltfTextureIndex >= 0 && gltfTextureIndex < static_cast<int>(kTextureCount))
        {
            return m_texIndex[gltfTextureIndex];
        }
        return fallbackIndex;
    };

    for (int i = 0; i < kMaterialCount; i++)
    {
        const UINT fallbackTexIndex = m_texIndex[i % kTextureCount];
        Material m = {};
        m.albedoTexIndex = fallbackTexIndex;
        m.metallicRoughnessTexIndex = fallbackTexIndex;
        m.emissiveTexIndex = fallbackTexIndex;
        m.occlusionTexIndex = fallbackTexIndex;
        m.normalTexIndex = fallbackTexIndex;
        m.roughnessFactor = 0.2f + 0.6f * static_cast<float>(i % 16) / 15.0f;
        m.metallicFactor = (i % 8 == 0) ? 1.0f : 0.0f;
        m.occlusionStrength = 1.0f;
        m.flags = 0;

        if constexpr (kGltfLoadingEnabled)
        {
            if (i < static_cast<int>(mesh.materials.size()))
            {
                const auto &gltfMaterial = mesh.materials[i];
                m.albedoTexIndex = resolveTextureIndex(gltfMaterial.albedoTexIndex, fallbackTexIndex);
                m.metallicRoughnessTexIndex =
                    resolveTextureIndex(gltfMaterial.metallicRoughnessTexIndex, fallbackTexIndex);
                m.emissiveTexIndex = resolveTextureIndex(gltfMaterial.emissiveTexIndex, fallbackTexIndex);
                m.occlusionTexIndex = resolveTextureIndex(gltfMaterial.occlusionTexIndex, fallbackTexIndex);
                m.normalTexIndex = resolveTextureIndex(gltfMaterial.normalTexIndex, fallbackTexIndex);
                m.roughnessFactor = gltfMaterial.roughnessFactor;
                m.metallicFactor = gltfMaterial.metallicFactor;
                m.occlusionStrength = gltfMaterial.occlusionStrength;
            }
        }

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

        m_frameResources[n].instanceBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_device->CreateShaderResourceView(m_frameResources[n].instanceBuffer.Get(), &srvDesc,
                                           m_frameResources[n].instanceBufferSrv.cpu);

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

        m_materialBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_device->CreateShaderResourceView(m_materialBuffer.Get(), &srvDesc, m_materialBufferSrv.cpu);
        Material *pMaterialDataBegin = nullptr;
        m_materialBuffer->Map(0, nullptr, reinterpret_cast<void **>(&pMaterialDataBegin));
        memcpy(pMaterialDataBegin, m_materialData.data(), materialBufferSize);
        m_materialBuffer->Unmap(0, nullptr);
    }

    m_camerasForCPU.clear();
    {
        m_camerasForCPU.emplace_back(XMFLOAT3(0.0f, 0.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 60.0f, m_aspectRatio,
                                     0.1f, 10000.0f);
        XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(m_camerasForCPU[0].viewProjection));
        m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
        XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                        XMMatrixTranspose(XMMatrixInverse(nullptr, m_camerasForCPU[0].viewProjection)));
    }

    // Create the per-frame constant buffers.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        CreateConstantBuffer(m_frameResources[n].cameraCB, &m_constantBufferData, sizeof(m_constantBufferData));
        CreateConstantBuffer(m_frameResources[n].lightCB, &m_lightingConstantsData, sizeof(m_lightingConstantsData));
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
    g_allocator = &m_ImGuiDescriptorHeapAllocator;

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
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate
    // more)
    init_info.SrvDescriptorHeap = m_imguiHeap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle)
    { g_allocator->Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    { g_allocator->Free(&cpu_handle, &gpu_handle); };
    ImGui_ImplDX12_Init(&init_info);
#endif
}

void D3D12HelloTexture::CreateConstantBuffer(ConstantBufferResource &constantBuffer, const void *initialData,
                                             UINT sizeInBytes)
{
    assert(sizeInBytes % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0);

    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                    D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
                                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                    IID_PPV_ARGS(&constantBuffer.buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constantBuffer.buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeInBytes;

    constantBuffer.cbv = m_descriptorHeapAllocator.AllocWithHandle();
    m_device->CreateConstantBufferView(&cbvDesc, constantBuffer.cbv.cpu);

    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(constantBuffer.buffer->Map(0, &readRange, reinterpret_cast<void **>(&constantBuffer.mappedData)));
    memcpy(constantBuffer.mappedData, initialData, sizeInBytes);
}

DescriptorHeapHandle D3D12HelloTexture::AllocateTextureSRV(ID3D12Resource *texture)
{
    DescriptorHeapHandle handle = m_descriptorHeapAllocator.AllocWithHandle();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(texture, &srvDesc, handle.cpu);

    return handle;
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

void D3D12HelloTexture::CreateGBuffer()
{
    CreateGBufferResources();
    CreateGBufferRTVs();
    CreateGBufferSRVs();
}

void D3D12HelloTexture::CreateGBufferResources()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        m_gbuffer.resources[i].Reset();

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = m_width;
        desc.Height = m_height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = m_gbuffer.formats[i];
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET, &m_gbuffer.clearValues[i], IID_PPV_ARGS(&m_gbuffer.resources[i])));
    }
}

void D3D12HelloTexture::CreateGBufferRTVs()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        m_gbuffer.rtvIndex[i] = kGBufferRTVBaseIndex + i;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_gbuffer.rtvIndex[i],
                                                m_rtvDescriptorSize);
        m_device->CreateRenderTargetView(m_gbuffer.resources[i].Get(), nullptr, rtvHandle);
    }
}

void D3D12HelloTexture::CreateGBufferSRVs()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        if (m_gbuffer.srvHandles[i].Index == UINT_MAX)
        {
            m_gbuffer.srvHandles[i] = m_descriptorHeapAllocator.AllocWithHandle();
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = m_gbuffer.formats[i];
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;

        m_device->CreateShaderResourceView(m_gbuffer.resources[i].Get(), &srvDesc, m_gbuffer.srvHandles[i].cpu);
    }

    if (m_depthStencilSrv.Index == UINT_MAX)
    {
        m_depthStencilSrv = m_descriptorHeapAllocator.AllocWithHandle();
    }
    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[GBuffer::Albedo].Index + GBuffer::kCount);
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
    r.desc.Format = DXGI_FORMAT_R32_TYPELESS;
    r.desc.SampleDesc.Count = 1;
    r.desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    r.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    r.clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    r.clearValue.DepthStencil.Depth = 1.0f;
    r.clearValue.DepthStencil.Stencil = 0;

    r.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    m_transientResources[kDepthStencilResourceName] = std::move(r);
}

void D3D12HelloTexture::CreateDepthStencilDescriptors()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[GBuffer::Albedo].Index + GBuffer::kCount);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_depthStencil.Get(), &srvDesc, m_depthStencilSrv.cpu);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetBackBufferRtv() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetDepthDsv() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetGBufferRTV(UINT index) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(m_gbuffer.rtvIndex[index], m_rtvDescriptorSize);
    return h;
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
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
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
    CreateDepthStencilDescriptors();
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
                if constexpr (kGltfLoadingEnabled)
                {
                    m_instanceData[i].materialId = 0; // base texture only
                    //  glTFのテクスチャ数に合わせて切り替える
                    // m_instanceData[i].materialId = (m_instanceData[i].materialId + 1) % m_gltfTextureCount;
                }
                else
                {
                    // チェッカーボードテクスチャ(kTextureCount)に合わせて切り替える
                    m_instanceData[i].materialId = (m_instanceData[i].materialId + 1) % kTextureCount;
                }
            }
            accumTime = 0.f;
        }

        // InstanceBufferのオフセットを毎フレーム更新する
        for (int i = 0; i < kInstanceCount; i++)
        {
            m_instanceData[i].prevWorld = m_instanceData[i].world;
            bool resetMotionVector = false;

#if 1 // cube array auto-translation
            m_instanceDataForCPU[i].pos.x += kTranslationSpeed;
#endif
            if (m_instanceDataForCPU[i].pos.x > kOffsetBounds)
            {
                m_instanceDataForCPU[i].pos.x = -kOffsetBounds;
                resetMotionVector = true;
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
            if (resetMotionVector)
            {
                m_instanceData[i].prevWorld = m_instanceData[i].world;
            }
        }
    }
    else
    {
        for (int i = 0; i < kInstanceCount; i++)
        {
            m_instanceData[i].prevWorld = m_instanceData[i].world;
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

    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
    m_camerasForCPU[0].updateAllMatrix();
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(m_camerasForCPU[0].viewProjection));
    XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                    XMMatrixTranspose(XMMatrixInverse(nullptr, m_camerasForCPU[0].viewProjection)));
    memcpy(m_frameResources[m_frameIndex].cameraCB.mappedData, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

void D3D12HelloTexture::UpdateImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug");

    ImGui::Text("Hello ImGui");
    ImGui::Text("FrameIndex: %d", m_frameIndex);
    ImGui::SliderInt("Max Visible Cubes", &m_maxVisibleCubeCount, 0, static_cast<int>(kInstanceCount));
    m_maxVisibleCubeCount = std::clamp(m_maxVisibleCubeCount, 0, static_cast<int>(kInstanceCount));
    ImGui::SliderFloat("Camera FovH", &m_camerasForCPU[0].fov, 20.f, 150.f);
    ImGui::ColorEdit4("BackBuffer Clear", m_backBufferClearColor.data());
    ImGui::SliderFloat3("Light Direction", &m_lightingConstantsData.lightDirection.x, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", &m_lightingConstantsData.lightColor.x);
    ImGui::SliderFloat("Ambient", &m_lightingConstantsData.ambientIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &m_lightingConstantsData.diffuseIntensity, 0.0f, 4.0f);
    m_lightingConstantsData.backgroundColor = {m_backBufferClearColor[0], m_backBufferClearColor[1],
                                               m_backBufferClearColor[2], m_backBufferClearColor[3]};
    memcpy(m_frameResources[m_frameIndex].lightCB.mappedData, &m_lightingConstantsData,
           sizeof(m_lightingConstantsData));

    int renderViewMode = static_cast<int>(m_renderViewMode);
    ImGui::Text("Render View");
    ImGui::RadioButton("LightPass", &renderViewMode, static_cast<int>(RenderViewMode::LightPass));
    ImGui::RadioButton("Albedo", &renderViewMode, static_cast<int>(RenderViewMode::GBufferAlbedo));
    ImGui::SameLine();
    ImGui::RadioButton("Normal", &renderViewMode, static_cast<int>(RenderViewMode::GBufferNormal));
    ImGui::SameLine();
    ImGui::RadioButton("Material", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMaterial));
    //    ImGui::SameLine();
    ImGui::RadioButton("MotionVector", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMotionVector));
    ImGui::SameLine();
    ImGui::RadioButton("PBRParams", &renderViewMode, static_cast<int>(RenderViewMode::GBufferPBRParams));
    ImGui::SameLine();
    ImGui::RadioButton("Depth", &renderViewMode, static_cast<int>(RenderViewMode::Depth));
    m_renderViewMode = static_cast<RenderViewMode>(renderViewMode);

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

UINT D3D12HelloTexture::GetVisibleCubeCount() const { return static_cast<UINT>(m_maxVisibleCubeCount); }

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
    CreateGBuffer();

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
            {}, {{GetBackBufferRtv()}, GetDepthDsv(), m_backBufferClearColor},
            [this](const RenderPass &pass) { RecordClear(pass.renderTargets); });
    AddPass(L"Depth PrePass", {},
            MakeResourceUsageMap({{kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE}}),
            {
                {RootParam_InstanceSrv, m_frameResources[m_frameIndex].instanceBufferSrv},
                {RootParam_ConstantBuffer, m_frameResources[m_frameIndex].cameraCB.cbv},
            },
            {{}, GetDepthDsv()}, [this](const RenderPass &) { RecordDepthPrePass(); });
    AddPass(
        L"GBufferPass",
        MakeResourceUsageMap({{kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE}}),
        MakeResourceUsageMap({{kGBufferResourceNames[GBuffer::Albedo], m_gbuffer.resources[GBuffer::Albedo].Get(),
                               D3D12_RESOURCE_STATE_RENDER_TARGET},
                              {kGBufferResourceNames[GBuffer::Normal], m_gbuffer.resources[GBuffer::Normal].Get(),
                               D3D12_RESOURCE_STATE_RENDER_TARGET},
                              {kGBufferResourceNames[GBuffer::Material], m_gbuffer.resources[GBuffer::Material].Get(),
                               D3D12_RESOURCE_STATE_RENDER_TARGET},
                              {kGBufferResourceNames[GBuffer::MotionVector],
                               m_gbuffer.resources[GBuffer::MotionVector].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET},
                              {kGBufferResourceNames[GBuffer::PBRParams], m_gbuffer.resources[GBuffer::PBRParams].Get(),
                               D3D12_RESOURCE_STATE_RENDER_TARGET}}),
        {{RootParam_TextureTable, m_textureTableStart},
         {RootParam_InstanceSrv, m_frameResources[m_frameIndex].instanceBufferSrv},
         {RootParam_MaterialSrv, m_materialBufferSrv},
         {RootParam_ConstantBuffer, m_frameResources[m_frameIndex].cameraCB.cbv}},
        {{GetGBufferRTV(GBuffer::Albedo), GetGBufferRTV(GBuffer::Normal), GetGBufferRTV(GBuffer::Material),
          GetGBufferRTV(GBuffer::MotionVector), GetGBufferRTV(GBuffer::PBRParams)},
         GetDepthDsv()},
        [this](const RenderPass &pass) { RecordGBufferPass(pass.renderTargets); });
#if 0
    AddPass(L"MainPass",
            MakeResourceUsageMap({{kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE}}),
            MakeResourceUsageMap(
                {{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET}}),
            {{RootParam_TextureTable, m_textureTableStart},
             {RootParam_InstanceSrv, m_frameResources[m_frameIndex].instanceBufferSrv},
             {RootParam_MaterialSrv, m_materialBufferSrv},
             {RootParam_ConstantBuffer, m_frameResources[m_frameIndex].cameraCB.cbv}},
            {{GetBackBufferRtv()}, GetDepthDsv()}, [this](const RenderPass &) { RecordMainPass(); });
#endif
    AddPass(L"LightPass", MakeGBufferReadUsageMap(),
            MakeResourceUsageMap(
                {{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET}}),
            {{RootParam_GBufferSrvBase, m_gbuffer.srvHandles[GBuffer::Albedo]},
             {RootParam_MaterialSrv, m_materialBufferSrv},
             {RootParam_ConstantBuffer, m_frameResources[m_frameIndex].cameraCB.cbv},
             {RootParam_LightConstants, m_frameResources[m_frameIndex].lightCB.cbv}},
            {{GetBackBufferRtv()}, std::nullopt}, [this](const RenderPass &) { RecordLightPass(); });

    if (IsGBufferDebugView())
    {
        AddPass(L"GBufferDebugPass", MakeGBufferReadUsageMap(),
                MakeResourceUsageMap({{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(),
                                       D3D12_RESOURCE_STATE_RENDER_TARGET}}),
                MakeGBufferSrvBindings(), {{GetBackBufferRtv()}, std::nullopt},
                [this](const RenderPass &) { RecordGBufferDebugPass(); });
    }

    AddPass(L"ImGui", {},
            MakeResourceUsageMap(
                {{kBackBufferResourceName, m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET}}),
            {}, {{GetBackBufferRtv()}, std::nullopt}, [this](const RenderPass &) { RecordImGuiPass(); });
}

void D3D12HelloTexture::AddPass(const wchar_t *name, ResourceUsageMap reads, ResourceUsageMap writes,
                                std::vector<PassDescriptorBinding> descriptorBindings,
                                PassRenderTargetBinding renderTargets, std::function<void(const RenderPass &)> execute)
{
    m_renderPasses.push_back({name, std::move(reads), std::move(writes), std::move(descriptorBindings),
                              std::move(renderTargets), std::move(execute)});
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

auto D3D12HelloTexture::MakeGBufferReadUsageMap() const -> ResourceUsageMap
{
    return MakeResourceUsageMap(
        {{kGBufferResourceNames[GBuffer::Albedo], m_gbuffer.resources[GBuffer::Albedo].Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::Normal], m_gbuffer.resources[GBuffer::Normal].Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::Material], m_gbuffer.resources[GBuffer::Material].Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::MotionVector], m_gbuffer.resources[GBuffer::MotionVector].Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::PBRParams], m_gbuffer.resources[GBuffer::PBRParams].Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}});
}

auto D3D12HelloTexture::MakeGBufferSrvBindings() const -> std::vector<PassDescriptorBinding>
{
    return {{RootParam_GBufferSrvBase, m_gbuffer.srvHandles[GBuffer::Albedo]}};
}

bool D3D12HelloTexture::IsGBufferDebugView() const { return m_renderViewMode != RenderViewMode::LightPass; }

UINT D3D12HelloTexture::GetGBufferDebugTarget() const
{
    assert(IsGBufferDebugView());
    return static_cast<UINT>(m_renderViewMode) - static_cast<UINT>(RenderViewMode::GBufferAlbedo);
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

void D3D12HelloTexture::BindPassDescriptors(const RenderPass &pass)
{
    for (const auto &binding : pass.descriptorBindings)
    {
        m_commandList->SetGraphicsRootDescriptorTable(binding.rootParameterIndex, binding.handle.gpu);
    }
}

void D3D12HelloTexture::BindPassRenderTargets(const RenderPass &pass)
{
    const D3D12_CPU_DESCRIPTOR_HANDLE *rtvs =
        pass.renderTargets.rtvs.empty() ? nullptr : pass.renderTargets.rtvs.data();
    const D3D12_CPU_DESCRIPTOR_HANDLE *dsv = pass.renderTargets.dsv ? &pass.renderTargets.dsv.value() : nullptr;

    m_commandList->OMSetRenderTargets(static_cast<UINT>(pass.renderTargets.rtvs.size()), rtvs, FALSE, dsv);
}

void D3D12HelloTexture::ExecutePasses()
{
    for (int passIndex = 0; passIndex < static_cast<int>(m_renderPasses.size()); ++passIndex)
    {
        CreateResourcesForPass(passIndex);

        const RenderPass &pass = m_renderPasses[passIndex];
        TransitionPassResources(pass);
        BindPassRenderTargets(pass);
        BindPassDescriptors(pass);
        pass.execute(pass);

        ReleaseResourcesAfterPass(passIndex);
    }
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
            CreateDepthStencilDescriptors();
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
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        SetResourceState(kGBufferResourceNames[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
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

    ID3D12Resource *resource = usage.resource;
    if (resource == nullptr && m_transientResources.contains(usage.name))
    {
        resource = m_transientResources.at(usage.name).resource.Get();
    }

    assert(resource != nullptr && "Cannot transition a null resource.");
    if (resource == nullptr)
    {
        DBG_PRINT("Resource %s is null. Skip transition.\n", usage.name.c_str());
        return;
    }

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, currentState, usage.state));
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

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    m_gpuWorkMeter.StartGpu(m_commandList.Get(), m_frameResources[m_frameIndex].gpuWorkMeterCheckPoints);
}

void D3D12HelloTexture::RecordClear(const PassRenderTargetBinding &renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"ClearPrepass");
    assert(!renderTargets.rtvs.empty());
    assert(renderTargets.dsv.has_value());
    assert(renderTargets.clearColor.has_value());

    for (auto rtv : renderTargets.rtvs)
    {
        m_commandList->ClearRenderTargetView(rtv, renderTargets.clearColor->data(), 0, nullptr);
    }
    m_commandList->ClearDepthStencilView(renderTargets.dsv.value(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    PIXEndEvent(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Clear");
}

void D3D12HelloTexture::DrawInstanceWrapper(UINT vertexOrIndexCount, UINT instanceCount)
{
    if (kGltfLoadingEnabled && kGltfMeshDisplay)
    {
        m_commandList->IASetIndexBuffer(&m_indexBufferView);
        m_commandList->DrawIndexedInstanced(vertexOrIndexCount, instanceCount, 0, 0, 0);
    }
    else
    {
        m_commandList->DrawInstanced(vertexOrIndexCount, instanceCount, 0, 0);
    }
}

//
// Depth Pre-pass
//
void D3D12HelloTexture::RecordDepthPrePass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"DepthPrepass");
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->SetPipelineState(m_depthPrePassPSO.Get());
    DrawInstanceWrapper(m_vertexCountPerInstance, GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Depth Prepass");
}

void D3D12HelloTexture::RecordGBufferPass(const PassRenderTargetBinding &renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"GBufferPass");

    for (UINT i = 0; i < static_cast<UINT>(renderTargets.rtvs.size()); ++i)
    {
        m_commandList->ClearRenderTargetView(renderTargets.rtvs[i], m_gbuffer.clearValues[i].Color, 0, nullptr);
    }

    m_commandList->SetPipelineState(m_gbufferPSO.Get());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(m_vertexCountPerInstance, GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Pass");
}

void D3D12HelloTexture::RecordGBufferDebugPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"GBufferDebugPass");

    const UINT debugTarget = GetGBufferDebugTarget();
    m_commandList->SetGraphicsRoot32BitConstants(RootParam_GBufferDebugConstants, 1, &debugTarget, 0);
    m_commandList->SetPipelineState(m_gbufferDebugPSO.Get());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Debug Pass");
}

void D3D12HelloTexture::RecordLightPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"LightPass");

    m_commandList->SetPipelineState(m_lightPassPSO.Get());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Lighting Pass");
}

//
// Main Pass
//
void D3D12HelloTexture::RecordMainPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"MainPass");
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->SetPipelineState(m_pipelineState.Get());
    DrawInstanceWrapper(m_vertexCountPerInstance, GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Main Pass");
}

void D3D12HelloTexture::RecordImGuiPass()
{
#if IMGUI_IMPL > 0
    {
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        ID3D12DescriptorHeap *imguiHeaps[] = {m_imguiHeap.Get()};

        m_commandList->SetDescriptorHeaps(1, imguiHeaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
    }
#endif
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ImGUI");
}

void D3D12HelloTexture::EndFrame()
{
    m_gpuWorkMeter.EndGpu(m_commandList.Get());

    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        TransitionResource(
            {kGBufferResourceNames[i], m_gbuffer.resources[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET});
    }
    TransitionResource({kDepthStencilResourceName, m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE});

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
