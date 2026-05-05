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
#include "D3D12HelloTexture.h"

#include <windows.h>
#include <cstdarg>
#include <cstdio>

#include "MyDx12Utils.h"

#include <pix3.h>

void DebugPrint(const char* fmt, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buf, fmt, args);
    va_end(args);

    OutputDebugStringA(buf);
}
#define DBG_PRINT(fmt, ...) \
 DebugPrint("[%s:%d] " fmt, __FILE__, __LINE__, __VA_ARGS__)


#include <random>
int rand_0_255() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 0xFF);
    return dist(gen);
}

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 618; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

D3D12HelloTexture::D3D12HelloTexture(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0),
	m_descriptorSize(0)
{
}

void D3D12HelloTexture::OnInit()
{
    m_prevTime = std::chrono::steady_clock::now();
    LoadPipeline();
    LoadAssets();
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
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    
        D3D12_RESOURCE_DESC depthDesc = {};
        depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthDesc.Width = m_width;
        depthDesc.Height = m_height;
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

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_depthStencil)
        ));

        m_device->CreateDepthStencilView(
            m_depthStencil.Get(),
            nullptr,
            m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
		);

    }

	// create command allocators.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[n].commandAllocator)));
    }
}

// Load the sample assets.
void D3D12HelloTexture::LoadAssets()
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

		// t0 - t(TextureCount-1) : Texture SRVs: space 0 : 0 - (kTextureCount-1)
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV[1];
        rangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, kTextureCount, 0/*base*/, 0/*space*/, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        // t0 : SRV structured buffer: space1 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV2[1];
        rangesSRV2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0/*base*/, 1/*space*/, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        // t0 : SRV structured buffer: space2 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV3[1];
        rangesSRV3[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0/*base*/, 2/*space*/, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_DESCRIPTOR_RANGE1 rangesCVB[1];
        rangesCVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[4];
		rootParameters[0].InitAsDescriptorTable(1, &rangesSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); //Texture SRVs
		rootParameters[1].InitAsDescriptorTable(1, &rangesSRV2[0], D3D12_SHADER_VISIBILITY_ALL);  //Structured buffer SRV (Instance data)
		rootParameters[2].InitAsDescriptorTable(1, &rangesSRV3[0], D3D12_SHADER_VISIBILITY_ALL);  //Structured buffer SRV (Material data)
        rootParameters[3].InitAsDescriptorTable(1, &rangesCVB[0], D3D12_SHADER_VISIBILITY_VERTEX);

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
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        UINT8* pVertexShaderData = nullptr;
        UINT8* pPixelShaderData = nullptr;
        UINT vertexShaderDataLength = 0;
        UINT pixelShaderDataLength = 0;

        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_VSMain.cso").c_str(), &pVertexShaderData, &vertexShaderDataLength));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_PSMain.cso").c_str(), &pPixelShaderData, &pixelShaderDataLength));

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
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShaderData, vertexShaderDataLength);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShaderData, pixelShaderDataLength);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
        m_frameResources[m_frameIndex].commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    {
        float _aspectRatio = 1.0; //m_aspectRatio
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * _aspectRatio, 0.0f }, { 0.5f, 0.0f } },
            { { 0.25f, -0.25f * _aspectRatio, 0.0f }, { 1.0f, 1.0f } },
            { { -0.25f, -0.25f * _aspectRatio, 0.0f }, { 0.0f, 1.0f } }
        };

        constexpr float s = 0.5f;
        constexpr float u = 1.f;
        Vertex cubeVertices[] =
        {
            // front
            {{-s,-s,-s},{0,u}}, {{-s,s,-s},{0,0}}, {{s,s,-s},{u,0}},
            {{-s,-s,-s},{0,u}}, {{s,s,-s},{u,0}}, {{s,-s,-s},{u,u}},

            // back
            {{-s,-s,s},{u,u}}, {{s,s,s},{0,0}}, {{-s,s,s},{u,0}},
            {{-s,-s,s},{u,u}}, {{s,-s,s},{0,u}}, {{s,s,s},{0,0}},

            // left
            {{-s,-s,s},{0,u}}, {{-s,s,s},{0,0}}, {{-s,s,-s},{u,0}},
            {{-s,-s,s},{0,u}}, {{-s,s,-s},{u,0}}, {{-s,-s,-s},{u,u}},

            // right
            {{s,-s,-s},{0,u}}, {{s,s,-s},{0,0}}, {{s,s,s},{u,0}},
            {{s,-s,-s},{0,u}}, {{s,s,s},{u,0}}, {{s,-s,s},{u,u}},

            // top
            {{-s,s,-s},{0,u}}, {{-s,s,s},{0,0}}, {{s,s,s},{u,0}},
            {{-s,s,-s},{0,u}}, {{s,s,s},{u,0}}, {{s,s,-s},{u,u}},

            // bottom
            {{-s,-s,s},{u,0}}, {{-s,-s,-s},{u,u}}, {{s,-s,-s},{0,u}},
            {{-s,-s,s},{u,0}}, {{s,-s,-s},{0,u}}, {{s,-s,s},{0,0}},
        };

        const UINT vertexBufferSize = sizeof(cubeVertices);

        m_vertexCountPerInstance = kCubeVertexCount;


        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
		MyDx12Util::CreateUploadBuffer(m_device, vertexBufferSize, m_vertexBuffer);

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
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

        for (int i = 0; i < kTextureCount; i++) {
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_texture[i])));


            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture[i].Get(), 0, 1);

            // Create the GPU upload buffer.
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&textureUploadHeap[i])));
        }

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
		
        // CPUにはTextureTypesだけTextureをつくる
        std::vector<std::vector<UINT8>> texture(kTextureTypes);
        for (int i = 0; i < kTextureTypes; i++) {
            texture[i] = GenerateTextureData();
        }

        for (int i = 0; i < kTextureCount; i++){
            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = &texture[i% kTextureTypes][0];
            textureData.RowPitch = kTextureWidth * kTexturePixelSize;
            textureData.SlicePitch = textureData.RowPitch * kTextureHeight;

            UpdateSubresources(m_commandList.Get(), m_texture[i].Get(), textureUploadHeap[i].Get(), 0, 0, 1, &textureData);
            m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture[i].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

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
		float x_trans = calculateOffsetX(i);

        //CPU only
        m_instanceDataForCPU.emplace_back(
            XMFLOAT3(x_trans, 0.0f, 0.0f), 
            XMFLOAT3(0.0f, 0.0f, 0.0f)
        );
        
        //CPU and GPU
        InstanceData d;
        d.materialId = i;
		XMMATRIX trans = XMMatrixTranslation(x_trans, 0.0f, 0.0f);
        XMStoreFloat4x4(&d.world, XMMatrixTranspose(trans));
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

        D3D12_SHADER_RESOURCE_VIEW_DESC  srvDesc = {};

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kInstanceCount;
        srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);

        UINT index = m_nextFreeIndex++;
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(index, m_descriptorSize);
        m_device->CreateShaderResourceView(m_frameResources[n].instanceBuffer.Get(), &srvDesc, handle);

        m_frameResources[n].instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_frameResources[n].pSrvDataBegin));
		memcpy(m_frameResources[n].pSrvDataBegin, m_instanceData.data(), instanceBufferSize);
        m_frameResources[n].instanceBuffer->Unmap(0, nullptr);
    }

	// Create SRV for material buffer (StructuredBuffer)
    {
		const UINT materialBufferSize = sizeof(Material) * kInstanceCount;

        MyDx12Util::CreateUploadBuffer(m_device, materialBufferSize, m_materialBuffer);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = kInstanceCount;
		srvDesc.Buffer.StructureByteStride = sizeof(Material);

        UINT index = m_nextFreeIndex++;
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(index, m_descriptorSize);

		m_device->CreateShaderResourceView(m_materialBuffer.Get(), &srvDesc, handle);
        m_materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pMaterialDataBegin));
        memcpy(pMaterialDataBegin, m_materialData.data(), materialBufferSize);
		m_materialBuffer->Unmap(0, nullptr);
    }

	m_camerasForCPU.clear();
    {
        m_camerasForCPU.emplace_back(
			XMFLOAT3(0.0f, 0.0f, -5.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			60.0f,
            m_aspectRatio,
            0.1f,
			10000.0f
		);
        XMStoreFloat4x4(&m_constantBufferData.viewProjection, m_camerasForCPU[0].viewProjection);
    }

	// Create the constant buffer.
    {
        const UINT constantBufferSize = sizeof(ConstantBuffer);    // CB size is required to be 256-byte aligned.

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer)));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;


        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart());
        // index番目に移動
        UINT index = m_nextFreeIndex++;
        handle.Offset(index, m_descriptorSize);
        m_device->CreateConstantBufferView(&cbvDesc, handle);

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
        memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    }


    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
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

UINT D3D12HelloTexture::AllocateTextureSRV(ID3D12Resource* texture)
{
    UINT index = m_nextFreeIndex++; 
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart());
    handle.Offset(index, m_descriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {}; 
    srvDesc.Format = texture->GetDesc().Format; 
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; 
    srvDesc.Texture2D.MipLevels = 1; 
    m_device->CreateShaderResourceView(texture, &srvDesc, handle); 
    
    return index; // ← これがGPUで使うID
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> D3D12HelloTexture::GenerateTextureData()
{
    const UINT rowPitch = kTextureWidth * kTexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = kTextureWidth >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * kTextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

	UINT8 R = rand_0_255();
	UINT8 G = rand_0_255();
	UINT8 B = rand_0_255();

	//DBG_PRINT("R=%d G=%d B=%d\n", R, G, B);

    for (UINT n = 0; n < textureSize; n += kTexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n + 0] = 0x00;    // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n + 0] = R;       // R
            pData[n + 1] = G;       // G
            pData[n + 2] = B;       // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}




// Update frame-based values.
void D3D12HelloTexture::OnUpdate()
{
    PIXBeginEvent(0, L"OnUpdate");

    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_prevTime).count();
    static float accumTime = 0.f;
    m_prevTime = now;

	// InstanceBufferのmaterialのtextureIdを1秒ごと切り替える
    accumTime += deltaTime;
	if (accumTime > 1.0f) {
        for (int i = 0; i < kInstanceCount; i++) {
            m_instanceData[i].materialId = (m_instanceData[i].materialId + 1) % kMaterialCount;
        }
        accumTime = 0.f;
    }

	// InstanceBufferのオフセットを毎フレーム更新する
#if 1
    for (int i = 0; i < kInstanceCount; i++) {
		
        m_instanceDataForCPU[i].pos.x += kTranslationSpeed;
		if (m_instanceDataForCPU[i].pos.x > kOffsetBounds) {
            m_instanceDataForCPU[i].pos.x = -kOffsetBounds;
	    }
        m_instanceDataForCPU[i].rot.x += kRotationSpeed;
        if (m_instanceDataForCPU[i].rot.x >= 2.0 * kPI) {
            m_instanceDataForCPU[i].rot.x = 0.f;
        }
        m_instanceDataForCPU[i].rot.y += kRotationSpeed;
        if (m_instanceDataForCPU[i].rot.y >= 2.0 * kPI) {
            m_instanceDataForCPU[i].rot.y = 0.f;
        }
        m_instanceDataForCPU[i].rot.z += kRotationSpeed;
        if (m_instanceDataForCPU[i].rot.z >= 2.0 * kPI) {
            m_instanceDataForCPU[i].rot.z = 0.f;
        }

        XMMATRIX transMat = XMMatrixTranslation(
            m_instanceDataForCPU[i].pos.x, m_instanceDataForCPU[i].pos.y, m_instanceDataForCPU[i].pos.z
        );

        XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(
            m_instanceDataForCPU[i].rot.x, m_instanceDataForCPU[i].rot.y, m_instanceDataForCPU[i].rot.z
        );

		//XMMATRIX rotMat = XMMatrixRotationZ(m_instanceDataForCPU[i].rot.z);
		XMMATRIX worldMat = rotMat * transMat;
        
        //XMStoreFloat4x4(&m_instanceData[i].world, worldMat);
        XMStoreFloat4x4(&m_instanceData[i].world, XMMatrixTranspose(worldMat));
	}
    m_frameResources[m_frameIndex].instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_frameResources[m_frameIndex].pSrvDataBegin));
    memcpy(m_frameResources[m_frameIndex].pSrvDataBegin, m_instanceData.data(), sizeof(InstanceData) * kInstanceCount);
    m_frameResources[m_frameIndex].instanceBuffer->Unmap(0, nullptr);
#endif

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

    m_camerasForCPU[0].updateAllMatrix();
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(m_camerasForCPU[0].viewProjection));
	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

// Render the scene.
void D3D12HelloTexture::OnRender()
{
    PIXBeginEvent(0, L"OnRender");

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    MoveToNextFrame();

    PIXEndEvent();
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

    ID3D12DescriptorHeap* ppHeaps[] = { m_heap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// texture SRV is at descriptor 0 - (TextureCount-1)
    m_commandList->SetGraphicsRootDescriptorTable(0, m_heap->GetGPUDescriptorHandleForHeapStart() );

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

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);


    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    
	UINT instanceCount = kInstanceCount;

	PIXBeginEvent(m_commandList.Get(), 0, L"DrawInstanced");

    m_commandList->DrawInstanced(
        m_vertexCountPerInstance,
        kInstanceCount, 
        0, 
        0
    );

    PIXEndEvent(m_commandList.Get());

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());

    PIXEndEvent();
}

// Wait for pending GPU work to complete.
void D3D12HelloTexture::WaitForGpu()
{
    PIXBeginEvent(3, L"MoveToNextFrame");

    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_frameResources[m_frameIndex].fenceValue));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_frameResources[m_frameIndex].fenceValue, m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_frameResources[m_frameIndex].fenceValue++;

	PIXEndEvent();
}

// Prepare to render the next frame.
void D3D12HelloTexture::MoveToNextFrame()
{
    PIXBeginEvent(2, L"MoveToNextFrame");

    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_frameResources[m_frameIndex].fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
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
}

