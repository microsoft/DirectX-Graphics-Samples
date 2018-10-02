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
#include "ShadowsFogScatteringSquidScene.h"
#include <dxgidebug.h>
#include "FrameResource.h"
#include "DXSample.h"

using namespace std;
using namespace SceneEnums;

ShadowsFogScatteringSquidScene* ShadowsFogScatteringSquidScene::s_app = nullptr;

ShadowsFogScatteringSquidScene::ShadowsFogScatteringSquidScene(UINT frameCount, DXSample* pSample) :
    m_frameCount(frameCount),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, 0.0f, 0.0f),
    m_scissorRect(0, 0, 0L, 0L),
    m_rtvDescriptorSize(0),
    m_keyboardInput(),
    m_pCurrentFrameResource(nullptr),
    m_pSample(pSample)
{
    s_app = this;

    m_keyboardInput.animate = true;
    m_renderTargets.resize(m_frameCount);
    m_frameResources.resize(m_frameCount);

    LoadContexts();
    InitializeCameraAndLights();
}

ShadowsFogScatteringSquidScene::~ShadowsFogScatteringSquidScene()
{
}

void ShadowsFogScatteringSquidScene::InitializeCameraAndLights()
{
    XMVECTOR eye = { 0.0f, 17.1954231f, -28.555980f, 1.0f };
    XMVECTOR at = { 0.0f, 8.0f, 0.0f, 0.0f };  
    XMVECTOR up = { 0.0f, 0.951865792f, 0.306514263f, 1.0f };
    m_camera.Set(eye, at, up);

    // Create lights.
    {
        for (int i = 0; i < NumLights; i++)
        {
            switch (i)
            {
            case 0: m_lights[0].direction = { 0.0, 0.0f, 1.0f, 0.0f }; break;   // +z
            case 1: m_lights[1].direction = { 1.0, 0.0f, 0.0f, 0.0f }; break;   // +x
            case 2: m_lights[2].direction = { 0.0, 0.0f, -1.0f, 0.0f }; break;  // -z
            case 3: m_lights[3].direction = { -1.0, 0.0f, 0.0f, 0.0f }; break;  // -x
            case 4: m_lights[4].direction = { 0.0, 1.0f, 0.0f, 0.0f }; break;   // +y
            case 5: m_lights[5].direction = { 0.0, -1.0f, 0.0f, 0.0f }; break;  // -y
            }
            m_lights[i].position = { 0.0f, 15.0f, -30.0f, 1.0f };
            m_lights[i].falloff = { 120.0f, 1.0f, 0.0f, 1.0f };
            m_lights[i].color = { 0.7f, 0.7f, 0.7f, 1.0f };

            XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
            XMVECTOR at = XMVectorAdd(eye, XMLoadFloat4(&m_lights[i].direction));
            XMVECTOR up = { 0, 1, 0 };
            switch (i)
            {
            case 4: up = { 0, 0, -1 }; break;
            case 5: up = { 0, 0, 1 }; break;
            }

            m_lightCameras[i].Set(eye, at, up);
        }
    }
}

void ShadowsFogScatteringSquidScene::Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, UINT frameIndex)
{
    CreateDescriptorHeaps(pDevice);
    CreateRootSignatures(pDevice);
    CreatePipelineStates(pDevice);
    CreatePostprocessPassResources(pDevice);
    CreateSamplers(pDevice);
    CreateFrameResources(pDevice);

    // Temporarily use one of the cmdlists for loading data into GPU memory.
    
    ID3D12GraphicsCommandList* pAssetLoadingCmdList = m_frameResources[0]->m_commandLists[0].Get();
    ID3D12CommandAllocator* pAssetLoadingCmdAllocator = m_frameResources[0]->m_commandAllocators[0].Get();

    ThrowIfFailed(pAssetLoadingCmdAllocator->Reset());
    ThrowIfFailed(pAssetLoadingCmdList->Reset(pAssetLoadingCmdAllocator, m_pipelineStates[RenderPass::Scene].Get()));
    CreateAssetResources(pDevice, pAssetLoadingCmdList);
    ThrowIfFailed(pAssetLoadingCmdList->Close());
    ID3D12CommandList* ppCommandLists[] = { pAssetLoadingCmdList };
    pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    SetFrameIndex(frameIndex);
}

// Load the rendering pipeline dependencies.
void ShadowsFogScatteringSquidScene::CreateDescriptorHeaps(ID3D12Device* pDevice)
{
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = m_frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    // Describe and create a depth stencil view (DSV) descriptor heap.
    // Each frame has its own depth stencils (to write shadows onto) 
    // and then there is one for the scene itself.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = m_frameCount * NumDepthBuffers;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    // Describe and create a shader resource view (SRV) and constant 
    // buffer view (CBV) descriptor heap.  
    // Heap layout: 
    // 1) null views, 
    // 2) object diffuse + normal textures views, 
    // 3) per frame views: 2x depth buffers (shadow, scene pass), 3x constant buffers (shadow, scene, postprocess pass)
    const UINT NumNullSrvs = 2;        // Null descriptors are needed for out of bounds behavior reads.
    const UINT cbvCount = m_frameCount * NumConstantBuffers;
    const UINT srvCount = _countof(SampleAssets::Textures) + m_frameCount * NumDepthBuffers;
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = NumNullSrvs + cbvCount + srvCount;
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));
    NAME_D3D12_OBJECT(m_cbvSrvHeap);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 2;        // One clamp and one wrap sampler.
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap)));
    NAME_D3D12_OBJECT(m_samplerHeap);

    // Get descriptor sizeS for the current device.
    m_cbvSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void ShadowsFogScatteringSquidScene::CreateRootSignatures(ID3D12Device* pDevice)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Create a root signature for shadow and scene render pass.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[4]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 2 frequently changed diffuse + normal textures - using registers t1 and t2.
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 1 frequently changed constant buffer.
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);                                                // 1 infrequently changed shadow texture - starting in register t0.
        ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);                                            // 2 static samplers.

        CD3DX12_ROOT_PARAMETER1 rootParameters[4];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::SceneAndShadowPass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::SceneAndShadowPass]);
    }

    // Create a root signature for post-process pass.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
        CD3DX12_ROOT_PARAMETER1 rootParameters[3];

        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);    // depth texture - using register t0.
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::PostprocessPass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::PostprocessPass]);
    }
}

void ShadowsFogScatteringSquidScene::CreatePipelineStates(ID3D12Device* pDevice)
{
    // Create the scene and shadow render pass pipeline state.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

        vertexShader = CompileShader(m_pSample->GetAssetFullPath(L"ShadowsAndScenePass.hlsl").c_str(), nullptr, "VSMain", "vs_5_0");
        pixelShader = CompileShader(m_pSample->GetAssetFullPath(L"ShadowsAndScenePass.hlsl").c_str(), nullptr, "PSMain", "ps_5_0");

        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
        inputLayoutDesc.pInputElementDescs = SampleAssets::StandardVertexDescription;
        inputLayoutDesc.NumElements = _countof(SampleAssets::StandardVertexDescription);

        CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        depthStencilDesc.StencilEnable = FALSE;

        // Describe and create the PSO for rendering the scene.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = inputLayoutDesc;
        psoDesc.pRootSignature = m_rootSignatures[RootSignature::SceneAndShadowPass].Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depthStencilDesc;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;

        ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[RenderPass::Scene])));
        NAME_D3D12_OBJECT(m_pipelineStates[RenderPass::Scene]);

        // Alter the description and create the PSO for rendering
        // the shadow map.  The shadow map does not use a pixel
        // shader or render targets.
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
        psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
        psoDesc.NumRenderTargets = 0;

        ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[RenderPass::Shadow])));
        NAME_D3D12_OBJECT(m_pipelineStates[RenderPass::Shadow]);
    }

    // Create the postprocess pass pipeline state.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

        vertexShader = CompileShader(m_pSample->GetAssetFullPath(L"PostprocessPass.hlsl").c_str(), nullptr, "VSMain", "vs_5_0");
        pixelShader = CompileShader(m_pSample->GetAssetFullPath(L"PostprocessPass.hlsl").c_str(), nullptr, "PSMain", "ps_5_0");


        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignatures[RootSignature::PostprocessPass].Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
        psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_ALPHA;
        psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates[RenderPass::Postprocess])));
    }
}

void ShadowsFogScatteringSquidScene::CreateAssetResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList)
{
    // Load scene assets.
    UINT fileSize = 0;
    UINT8* pAssetData;
    ThrowIfFailed(ReadDataFromFile(m_pSample->GetAssetFullPath(SampleAssets::DataFileName).c_str(), &pAssetData, &fileSize));

    // Create the vertex buffer.
    {
        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffers[VertexBuffer::SceneGeometry])));

        NAME_D3D12_OBJECT(m_vertexBuffers[VertexBuffer::SceneGeometry]);

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBufferUpload)));

        // Copy data to the upload heap and then schedule a copy 
        // from the upload heap to the vertex buffer.
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = pAssetData + SampleAssets::VertexDataOffset;
        vertexData.RowPitch = SampleAssets::VertexDataSize;
        vertexData.SlicePitch = vertexData.RowPitch;

        PIXBeginEvent(pCmdList, 0, L"Copy vertex buffer data to default resource...");

        UpdateSubresources<1>(pCmdList, m_vertexBuffers[VertexBuffer::SceneGeometry].Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
        pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffers[VertexBuffer::SceneGeometry].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        PIXEndEvent(pCmdList);

        // Initialize the vertex buffer view.
        m_vertexBufferViews[VertexBuffer::SceneGeometry].BufferLocation = m_vertexBuffers[VertexBuffer::SceneGeometry]->GetGPUVirtualAddress();
        m_vertexBufferViews[VertexBuffer::SceneGeometry].SizeInBytes = SampleAssets::VertexDataSize;
        m_vertexBufferViews[VertexBuffer::SceneGeometry].StrideInBytes = SampleAssets::StandardVertexStride;
    }

    // Create the index buffer.
    {
        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)));

        NAME_D3D12_OBJECT(m_indexBuffer);

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_indexBufferUpload)));

        // Copy data to the upload heap and then schedule a copy 
        // from the upload heap to the index buffer.
        D3D12_SUBRESOURCE_DATA indexData = {};
        indexData.pData = pAssetData + SampleAssets::IndexDataOffset;
        indexData.RowPitch = SampleAssets::IndexDataSize;
        indexData.SlicePitch = indexData.RowPitch;

        PIXBeginEvent(pCmdList, 0, L"Copy index buffer data to default resource...");

        UpdateSubresources<1>(pCmdList, m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);
        pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

        PIXEndEvent(pCmdList);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = SampleAssets::IndexDataSize;
        m_indexBufferView.Format = SampleAssets::StandardIndexFormat;
    }

    // Create shader resources.
    {
        // Get a handle to the start of the descriptor heap.
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());

        {
            // Describe and create 2 null SRVs. Null descriptors are needed in order 
            // to achieve the effect of an "unbound" resource.
            D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
            nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            nullSrvDesc.Texture2D.MipLevels = 1;
            nullSrvDesc.Texture2D.MostDetailedMip = 0;
            nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

            pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
            cbvSrvHandle.Offset(m_cbvSrvDescriptorSize);

            pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
            cbvSrvHandle.Offset(m_cbvSrvDescriptorSize);
        }

        // Create each texture and SRV descriptor.
        const UINT srvCount = _countof(SampleAssets::Textures);
        PIXBeginEvent(pCmdList, 0, L"Copy diffuse and normal texture data to default resources...");
        for (UINT i = 0; i < srvCount; i++)
        {
            // Describe and create a Texture2D.
            const SampleAssets::TextureResource &tex = SampleAssets::Textures[i];
            CD3DX12_RESOURCE_DESC texDesc(
                D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                0,
                tex.Width,
                tex.Height,
                1,
                static_cast<UINT16>(tex.MipLevels),
                tex.Format,
                1,
                0,
                D3D12_TEXTURE_LAYOUT_UNKNOWN,
                D3D12_RESOURCE_FLAG_NONE);

            ThrowIfFailed(pDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &texDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_textures[i])));

            NAME_D3D12_OBJECT_INDEXED(m_textures, i);

            {
                const UINT subresourceCount = texDesc.DepthOrArraySize * texDesc.MipLevels;
                UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textures[i].Get(), 0, subresourceCount);
                ThrowIfFailed(pDevice->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&m_textureUploads[i])));

                // Copy data to the intermediate upload heap and then schedule a copy
                // from the upload heap to the Texture2D.
                D3D12_SUBRESOURCE_DATA textureData = {};
                textureData.pData = pAssetData + tex.Data->Offset;
                textureData.RowPitch = tex.Data->Pitch;
                textureData.SlicePitch = tex.Data->Size;

                UpdateSubresources(pCmdList, m_textures[i].Get(), m_textureUploads[i].Get(), 0, 0, subresourceCount, &textureData);
                pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textures[i].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
            }

            // Describe and create an SRV.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = tex.Format;
            srvDesc.Texture2D.MipLevels = tex.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            pDevice->CreateShaderResourceView(m_textures[i].Get(), &srvDesc, cbvSrvHandle);

            // Move to the next descriptor slot.
            cbvSrvHandle.Offset(m_cbvSrvDescriptorSize);
        }
        PIXEndEvent(pCmdList);
    }

    free(pAssetData);
}

void ShadowsFogScatteringSquidScene::CreatePostprocessPassResources(ID3D12Device* pDevice)
{
    // Create the vertex buffer.
    // Define the screen space quad geometry.
    Vertex triangleVertices[] =
    {
        { { -1.0f, -1.0f, 0.0f, 1.0f } },
        { { -1.0f, 1.0f, 0.0f, 1.0f } },
        { { 1.0f, -1.0f, 0.0f, 1.0f } },
        { { 1.0f, 1.0f, 0.0f, 1.0f } },
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.
    ThrowIfFailed(pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffers[VertexBuffer::ScreenQuad])));

                                             // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffers[VertexBuffer::ScreenQuad]->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffers[VertexBuffer::ScreenQuad]->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferViews[VertexBuffer::ScreenQuad].BufferLocation = m_vertexBuffers[VertexBuffer::ScreenQuad]->GetGPUVirtualAddress();
    m_vertexBufferViews[VertexBuffer::ScreenQuad].StrideInBytes = sizeof(Vertex);
    m_vertexBufferViews[VertexBuffer::ScreenQuad].SizeInBytes = vertexBufferSize;
}

void ShadowsFogScatteringSquidScene::CreateSamplers(ID3D12Device* pDevice)
{
    // Get the sampler descriptor size for the current device.
    const UINT samplerDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    // Get a handle to the start of the descriptor heap.
    CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(m_samplerHeap->GetCPUDescriptorHandleForHeapStart());

    // Describe and create the point clamping sampler, which is 
    // used for the shadow map.
    D3D12_SAMPLER_DESC clampSamplerDesc = {};
    clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    clampSamplerDesc.MipLODBias = 0.0f;
    clampSamplerDesc.MaxAnisotropy = 1;
    clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    clampSamplerDesc.BorderColor[0] = clampSamplerDesc.BorderColor[1] = clampSamplerDesc.BorderColor[2] = clampSamplerDesc.BorderColor[3] = 0;
    clampSamplerDesc.MinLOD = 0;
    clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    pDevice->CreateSampler(&clampSamplerDesc, samplerHandle);

    // Move the handle to the next slot in the descriptor heap.
    samplerHandle.Offset(samplerDescriptorSize);

    // Describe and create the wrapping sampler, which is used for 
    // sampling diffuse/normal maps.
    D3D12_SAMPLER_DESC wrapSamplerDesc = {};
    wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.MinLOD = 0;
    wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    wrapSamplerDesc.MipLODBias = 0.0f;
    wrapSamplerDesc.MaxAnisotropy = 1;
    wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
    pDevice->CreateSampler(&wrapSamplerDesc, samplerHandle);
}

void ShadowsFogScatteringSquidScene::CreateFrameResources(ID3D12Device* pDevice)
{
    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_frameResources[i] = make_unique<FrameResource>(pDevice, m_pipelineStates, m_dsvHeap.Get(), m_cbvSrvHeap.Get(), i);
    }
}

// Initialize threads and events.
void ShadowsFogScatteringSquidScene::LoadContexts()
{
#if !SINGLETHREADED
    struct threadwrapper
    {
        static unsigned int WINAPI thunk(LPVOID lpParameter)
        {
            ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
            ShadowsFogScatteringSquidScene::Get()->WorkerThread(parameter->threadIndex);
            return 0;
        }
    };

    for (int i = 0; i < NumContexts; i++)
    {
        m_workerBeginRenderFrame[i] = CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL);

        m_workerFinishedRenderFrame[i] = CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL);

        m_workerFinishShadowPass[i] = CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL);

        m_threadParameters[i].threadIndex = i;

        m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
            nullptr,
            0,
            threadwrapper::thunk,
            reinterpret_cast<LPVOID>(&m_threadParameters[i]),
            0,
            nullptr));

        assert(m_workerBeginRenderFrame[i] != NULL);
        assert(m_workerFinishedRenderFrame[i] != NULL);
        assert(m_threadHandles[i] != NULL);

    }
#endif
}

// Load resources that are dependent on the size of the main window.
void ShadowsFogScatteringSquidScene::LoadSizeDependentResources(ID3D12Device* pDevice, ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height)
{
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = static_cast<LONG>(width);
    m_scissorRect.bottom = static_cast<LONG>(height);

    // Create render target views (RTVs).
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_renderTargets[i] = ppRenderTargets[i];
        pDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
        NAME_D3D12_OBJECT_INDEXED(m_renderTargets, i);
    }

    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_frameResources[i]->LoadSizeDependentResources(pDevice, width, height);
        m_frameResources[i]->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
    }
}

// Release sample's D3D objects
void ShadowsFogScatteringSquidScene::ReleaseSizeDependentResources()
{
    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_renderTargets[i].Reset();
        m_frameResources[i]->ReleaseSizeDependentResources();
    }
}

// Release sample's D3D objects
void ShadowsFogScatteringSquidScene::ReleaseD3DObjects()
{
    ResetComPtrArray(&m_renderTargets);
    ResetComPtrArray(&m_rootSignatures);
    ResetComPtrArray(&m_pipelineStates);
    ResetComPtrArray(&m_vertexBuffers);
    ResetComPtrArray(&m_rootSignatures);
    ResetComPtrArray(&m_textures);
    ResetComPtrArray(&m_textureUploads);
    m_indexBuffer.Reset();
    m_indexBufferUpload.Reset();
    m_vertexBufferUpload.Reset();

    m_rtvHeap.Reset();
    m_dsvHeap.Reset();
    m_cbvSrvHeap.Reset();
    m_samplerHeap.Reset();

    ResetUniquePtrArray(&m_frameResources);
    m_pCurrentFrameResource = nullptr;
}

// Caller is expected to enforce frame synchronization and that the GPU is done with frameIndex frame before being set as current frame again.
void ShadowsFogScatteringSquidScene::SetFrameIndex(UINT frameIndex)
{
    m_frameIndex = frameIndex;
    m_pCurrentFrameResource = m_frameResources[m_frameIndex].get();
}

void ShadowsFogScatteringSquidScene::KeyUp(UINT8 key)
{
    switch (key)
    {
    case VK_LEFT:
        m_keyboardInput.leftArrowPressed = false;
        break;
    case VK_RIGHT:
        m_keyboardInput.rightArrowPressed = false;
        break;
    case VK_UP:
        m_keyboardInput.upArrowPressed = false;
        break;
    case VK_DOWN:
        m_keyboardInput.downArrowPressed = false;
        break;
    default:
        break;
    }
}

void ShadowsFogScatteringSquidScene::KeyDown(UINT8 key)
{
    switch (key)
    {
    case VK_LEFT:
        m_keyboardInput.leftArrowPressed = true;
        break;
    case VK_RIGHT:
        m_keyboardInput.rightArrowPressed = true;
        break;
    case VK_UP:
        m_keyboardInput.upArrowPressed = true;
        break;
    case VK_DOWN:
        m_keyboardInput.downArrowPressed = true;
        break;
    case VK_SPACE:
        m_keyboardInput.animate = !m_keyboardInput.animate;
        break;
    default:
        break;
    }
}

// Update frame-based values.
void ShadowsFogScatteringSquidScene::Update(double elapsedTime)
{
    float angleChange = 2.0f * static_cast<float>(elapsedTime);

    if (m_keyboardInput.leftArrowPressed)
        m_camera.RotateAroundYAxis(-angleChange);
    if (m_keyboardInput.rightArrowPressed)
        m_camera.RotateAroundYAxis(angleChange);
    if (m_keyboardInput.upArrowPressed)
        m_camera.RotatePitch(-angleChange);
    if (m_keyboardInput.downArrowPressed)
        m_camera.RotatePitch(angleChange);

    if (m_keyboardInput.animate)
    {
        for (int i = 0; i < NumLights; i++)
        {
            XMStoreFloat4(&m_lights[i].position, XMVector4Transform(XMLoadFloat4(&m_lights[i].position), XMMatrixRotationY(angleChange)));
            XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
            XMVECTOR at = XMVectorSet(0.0f, 15.0f, 0.0f,0.0f);
            XMVECTOR up = { 0, 1, 0 };
            switch (i)
            {
            case 4: up = { 0, 0, -1 }; break;
            case 5: up = { 0, 0, 1 }; break;
            }
            m_lightCameras[i].Set(eye, at, up);

            m_lightCameras[i].Get3DViewProjMatrices(&m_lights[i].view, &m_lights[i].projection, 90.0f, m_viewport.Width, m_viewport.Height);
        }
    }
    m_pCurrentFrameResource->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
}

// Render the scene.
void ShadowsFogScatteringSquidScene::Render(ID3D12CommandQueue* pCommandQueue, bool setBackbufferReadyForPresent)
{
    BeginFrame();
    // Shadow generation and scene render passes
#if SINGLETHREADED
    for (int i = 0; i < NumContexts; i++)
    {
        WorkerThread(i);
    }
    MidFrame();
    EndFrame();
    pCommandQueue->ExecuteCommandLists(_countof(m_pCurrentFrameResource->m_batchSubmit), m_pCurrentFrameResource->m_batchSubmit);
#else
    for (int i = 0; i < NumContexts; i++)
    {
        SetEvent(m_workerBeginRenderFrame[i]); // Tell each worker to start drawing.
    }

    MidFrame();
    EndFrame();

    WaitForMultipleObjects(NumContexts, m_workerFinishShadowPass, TRUE, INFINITE);

    // You can execute command lists on any thread. Depending on the work 
    // load, apps can choose between using ExecuteCommandLists on one thread 
    // vs ExecuteCommandList from multiple threads.
    pCommandQueue->ExecuteCommandLists(NumContexts + 2, m_pCurrentFrameResource->m_batchSubmit); // Submit PRE, MID and shadows.

    WaitForMultipleObjects(NumContexts, m_workerFinishedRenderFrame, TRUE, INFINITE);

    // Submit remaining command lists.
    pCommandQueue->ExecuteCommandLists(_countof(m_pCurrentFrameResource->m_batchSubmit) - NumContexts - 2, m_pCurrentFrameResource->m_batchSubmit + NumContexts + 2);
#endif

    // Postprocess pass
    PostprocessPass(pCommandQueue, setBackbufferReadyForPresent);
}

// Apply a postprocess pass with a light scattering effect. 
void ShadowsFogScatteringSquidScene::PostprocessPass(ID3D12CommandQueue* pCommandQueue, bool setBackbufferReadyForPresent)
{
    PIXBeginEvent(pCommandQueue, 0, L"Postprocess pass");

    ID3D12GraphicsCommandList* pCmdList = m_pCurrentFrameResource->m_postprocessCommandList.Get();
    pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pCurrentFrameResource->m_depthTextures[DepthGenPass::Scene].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Set necessary state.
    pCmdList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::PostprocessPass].Get());
    pCmdList->RSSetViewports(1, &m_viewport);
    pCmdList->RSSetScissorRects(1, &m_scissorRect);

    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
    pCmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_pCurrentFrameResource->Bind(pCmdList, RenderPass::Postprocess, &rtvHandle);
    pCmdList->SetGraphicsRootDescriptorTable(2, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());

    // Record commands.
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    pCmdList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::ScreenQuad]);
    pCmdList->DrawInstanced(4, 1, 0, 0);

    pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pCurrentFrameResource->m_depthTextures[DepthGenPass::Scene].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    if (setBackbufferReadyForPresent)
    {
        // Indicate that the back buffer will now be used to present.
        pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    }

    ThrowIfFailed(pCmdList->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { pCmdList };
    pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    PIXEndEvent(pCommandQueue);
}

// Assemble the CommandListPre command list.
void ShadowsFogScatteringSquidScene::BeginFrame()
{
    m_pCurrentFrameResource->Init();

    ID3D12GraphicsCommandList* pCmdListPre = m_pCurrentFrameResource->m_commandLists[CommandListPre].Get();
    // Indicate that the back buffer will be used as a render target.
    pCmdListPre->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the render target and depth stencil.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    pCmdListPre->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_pCurrentFrameResource->ClearDepthStencilViews(pCmdListPre);

    ThrowIfFailed(pCmdListPre->Close());
}

// Assemble the CommandListMid command list.
void ShadowsFogScatteringSquidScene::MidFrame()
{
    // Transition our shadow map from the shadow pass to readable in the scene pass.
    m_pCurrentFrameResource->SwapBarriers();

    ThrowIfFailed(m_pCurrentFrameResource->m_commandLists[CommandListMid]->Close());
}

// Assemble the CommandListPost command list.
void ShadowsFogScatteringSquidScene::EndFrame()
{
    m_pCurrentFrameResource->Finish();

    ThrowIfFailed(m_pCurrentFrameResource->m_commandLists[CommandListPost]->Close());
}

// Worker thread body. workerIndex is an integer from 0 to NumContexts 
// describing the worker's thread index.
void ShadowsFogScatteringSquidScene::WorkerThread(int threadIndex)
{
    assert(threadIndex >= 0);
    assert(threadIndex < NumContexts);

#if !SINGLETHREADED
    while (threadIndex >= 0 && threadIndex < NumContexts)
    {
        // Wait for main thread to tell us to draw.

        WaitForSingleObject(m_workerBeginRenderFrame[threadIndex], INFINITE);

#endif
        ID3D12GraphicsCommandList* pShadowCommandList = m_pCurrentFrameResource->m_shadowCommandLists[threadIndex].Get();
        ID3D12GraphicsCommandList* pSceneCommandList = m_pCurrentFrameResource->m_sceneCommandLists[threadIndex].Get();

        //
        // Shadow pass
        //

        // Populate the command list.
        SetCommonPipelineState(pShadowCommandList);
        m_pCurrentFrameResource->Bind(pShadowCommandList, RenderPass::Shadow, nullptr);

        // Set null SRVs for the diffuse/normal textures.
        pShadowCommandList->SetGraphicsRootDescriptorTable(0, m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

        // Distribute objects over threads by drawing only 1/NumContexts 
        // objects per worker (i.e. every object such that objectnum % 
        // NumContexts == threadIndex).
        PIXBeginEvent(pShadowCommandList, 0, L"Worker drawing shadow pass...");

        for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
        {
            SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[j];

            pShadowCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
        }

        PIXEndEvent(pShadowCommandList);

        ThrowIfFailed(pShadowCommandList->Close());

#if !SINGLETHREADED
        // Submit shadow pass.
        SetEvent(m_workerFinishShadowPass[threadIndex]);
#endif

        //
        // Scene pass
        // 

        // Populate the command list.  These can only be sent after the shadow 
        // passes for this frame have been submitted.
        SetCommonPipelineState(pSceneCommandList);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        m_pCurrentFrameResource->Bind(pSceneCommandList, RenderPass::Scene, &rtvHandle);

        PIXBeginEvent(pSceneCommandList, 0, L"Worker drawing scene pass...");

        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
        for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
        {
            SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[j];

            // Set the diffuse and normal textures for the current object.
            CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, NumNullSrvs + drawArgs.DiffuseTextureIndex, m_cbvSrvDescriptorSize);
            pSceneCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

            pSceneCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
        }

        PIXEndEvent(pSceneCommandList);
        ThrowIfFailed(pSceneCommandList->Close());

#if !SINGLETHREADED
        // Tell main thread that we are done.
        SetEvent(m_workerFinishedRenderFrame[threadIndex]);
    }
#endif
}

void ShadowsFogScatteringSquidScene::SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList)
{
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::SceneAndShadowPass].Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::SceneGeometry]);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
    pCommandList->OMSetStencilRef(0);

    // Render targets and depth stencil are set elsewhere because the 
    // depth stencil depends on the frame resource being used.

    // Constant buffers are set elsewhere because they depend on the 
    // frame resource being used.

    // SRVs are set elsewhere because they change based on the object 
    // being drawn.
}
