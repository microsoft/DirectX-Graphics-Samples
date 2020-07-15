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
const float ShadowsFogScatteringSquidScene::s_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

ShadowsFogScatteringSquidScene::ShadowsFogScatteringSquidScene(UINT frameCount, DXSample* pSample) :
    m_frameCount(frameCount),
    m_fogDensity(0.04f),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, 0.0f, 0.0f),
    m_scissorRect(0, 0, 0L, 0L),
    m_rtvDescriptorSize(0),
    m_cbvSrvDescriptorSize(0),
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
    XMVECTOR eye = XMVectorSet(0.0f, 17.1954231f, -28.555980f, 1.0f);
    XMVECTOR at = XMVectorSet(0.0f, 8.0f, 0.0f, 0.0f);
    XMVECTOR up = XMVectorSet(0.0f, 0.951865792f, 0.306514263f, 1.0f);
    m_camera.Set(eye, at, up);

    // Create lights.
    {
        for (int i = 0; i < NumLights; i++)
        {
            switch (i)
            {
            case 0: m_lights[0].direction = { 0.0, 0.0f, 1.0f, 0.0f }; break;   // +z
#if NumLights > 1
            case 1: m_lights[1].direction = { 1.0, 0.0f, 0.0f, 0.0f }; break;   // +x
            case 2: m_lights[2].direction = { 0.0, 0.0f, -1.0f, 0.0f }; break;  // -z
            case 3: m_lights[3].direction = { -1.0, 0.0f, 0.0f, 0.0f }; break;  // -x
            case 4: m_lights[4].direction = { 0.0, 1.0f, 0.0f, 0.0f }; break;   // +y
            case 5: m_lights[5].direction = { 0.0, -1.0f, 0.0f, 0.0f }; break;  // -y
#endif
            }
            m_lights[i].position = { 0.0f, 15.0f, -30.0f, 1.0f };
            m_lights[i].falloff = { 120.0f, 1.0f, 0.0f, 1.0f };
            m_lights[i].color = { 0.7f, 0.7f, 0.7f, 1.0f };

            XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
            XMVECTOR at = XMVectorAdd(eye, XMLoadFloat4(&m_lights[i].direction));
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            switch (i)
            {
            case 4: up = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f); break;
            case 5: up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); break;
            }

            m_lightCameras[i].Set(eye, at, up);
        }
    }
}

void ShadowsFogScatteringSquidScene::Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pDirectCommandQueue, ID3D12GraphicsCommandList* pCommandList, UINT frameIndex)
{
    CreateDescriptorHeaps(pDevice);
    CreateRootSignatures(pDevice);
    CreatePipelineStates(pDevice);
    CreatePostprocessPassResources(pDevice);
    CreateSamplers(pDevice);
    CreateFrameResources(pDevice, pDirectCommandQueue);
    CreateCommandLists(pDevice);

    CreateAssetResources(pDevice, pCommandList);

    SetFrameIndex(frameIndex);
}

// Load the rendering pipeline dependencies.
void ShadowsFogScatteringSquidScene::CreateDescriptorHeaps(ID3D12Device* pDevice)
{
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = GetNumRtvDescriptors();
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    NAME_D3D12_OBJECT(m_rtvHeap);

    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = _countof(m_depthDsvs);
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    NAME_D3D12_OBJECT(m_dsvHeap);

    // Describe and create a shader resource view (SRV) and constant 
    // buffer view (CBV) descriptor heap.  
    // Heap layout: 
    // 1) null views, 
    // 2) depth buffer views,
    // 3) object diffuse + normal textures views
    // Note that we use root constant buffer views, so we don't need descriptor 
    // heap space for each frames's constant buffer.
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = GetNumCbvSrvUavDescriptors();
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));
    NAME_D3D12_OBJECT(m_cbvSrvHeap);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 2; // One clamp and one wrap sampler.
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap)));
    NAME_D3D12_OBJECT(m_samplerHeap);

    // Get descriptor sizes for the current device.
    m_cbvSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void ShadowsFogScatteringSquidScene::CreateCommandLists(ID3D12Device* pDevice)
{
    // Temporarily use a frame resource's command allocator to create command lists.
    ID3D12CommandAllocator* pCommandAllocator = m_frameResources[0]->m_commandAllocator.Get();

    for (UINT i = 0; i < CommandListCount; i++)
    {
        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator, nullptr, IID_PPV_ARGS(&m_commandLists[i])));
        ThrowIfFailed(m_commandLists[i]->Close());
        NAME_D3D12_OBJECT_INDEXED(m_commandLists, i);
    }

    for (UINT i = 0; i < NumContexts; i++)
    {
        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator, nullptr, IID_PPV_ARGS(&m_shadowCommandLists[i])));
        ThrowIfFailed(m_shadowCommandLists[i]->Close());
        NAME_D3D12_OBJECT_INDEXED(m_shadowCommandLists, i);

        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator, nullptr, IID_PPV_ARGS(&m_sceneCommandLists[i])));
        ThrowIfFailed(m_sceneCommandLists[i]->Close());
        NAME_D3D12_OBJECT_INDEXED(m_sceneCommandLists, i);
    }

    // Batch up command lists for execution later.
    const UINT batchSize = _countof(m_sceneCommandLists) + _countof(m_shadowCommandLists) + CommandListCount;
    m_batchSubmit[0] = m_commandLists[CommandListPre].Get();
    memcpy(m_batchSubmit + 1, m_shadowCommandLists, _countof(m_shadowCommandLists) * sizeof(ID3D12CommandList*));
    m_batchSubmit[_countof(m_shadowCommandLists) + 1] = m_commandLists[CommandListMid].Get();
    memcpy(m_batchSubmit + _countof(m_shadowCommandLists) + 2, m_sceneCommandLists, _countof(m_sceneCommandLists) * sizeof(ID3D12CommandList*));
    m_batchSubmit[batchSize - 1] = m_commandLists[CommandListPost].Get();
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

    // Create a root signature for the shadow pass.
    {
        CD3DX12_ROOT_PARAMETER1 rootParameters[1]; // Performance tip: Order root parameters from most frequently accessed to least frequently accessed.
        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX); // 1 frequently changed constant buffer.

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | // Performance tip: Limit root signature access when possible.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::ShadowPass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::ShadowPass]);
    }

    // Create a root signature for the scene pass.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[4]; // Performance tip: Order root parameters from most frequently accessed to least frequently accessed.
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL); // 2 frequently changed diffuse + normal textures - starting in register t0.
        rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL); // 1 frequently changed constant buffer.
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL); // 1 infrequently changed shadow texture - starting in register t2.
        rootParameters[3].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL); // 2 static samplers.

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | // Performance tip: Limit root signature access when possible.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::ScenePass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::ScenePass]);
    }

    // Create a root signature for the post-process pass.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL); // 1 depth texture - starting in register t0.
        rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL); // 1 frequently changed constant buffer.
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL); // 1 static sampler.

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

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
        psoDesc.pRootSignature = m_rootSignatures[RootSignature::ScenePass].Get();
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
        psoDesc.pRootSignature = m_rootSignatures[RootSignature::ShadowPass].Get();
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

void ShadowsFogScatteringSquidScene::CreateAssetResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
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

        PIXBeginEvent(pCommandList, 0, L"Copy vertex buffer data to default resource...");

        UpdateSubresources<1>(pCommandList, m_vertexBuffers[VertexBuffer::SceneGeometry].Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

        // Performance tip: You can avoid some resource barriers by relying on resource state promotion and decay.
        // Resources accessed on a copy queue will decay back to the COMMON after ExecuteCommandLists()
        // completes on the GPU. Search online for "D3D12 Implicit State Transitions" for more details. 

        PIXEndEvent(pCommandList);

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

        PIXBeginEvent(pCommandList, 0, L"Copy index buffer data to default resource...");

        UpdateSubresources<1>(pCommandList, m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);

        // Performance tip: You can avoid some resource barriers by relying on resource state promotion and decay.
        // Resources accessed on a copy queue will decay back to the COMMON after ExecuteCommandLists()
        // completes on the GPU. Search online for "D3D12 Implicit State Transitions" for more details. 

        PIXEndEvent(pCommandList);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = SampleAssets::IndexDataSize;
        m_indexBufferView.Format = SampleAssets::StandardIndexFormat;
    }
    
    // Create shader resources.
    {
        // Get a handle to the start of the descriptor heap.
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

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

            for (UINT i = 0; i < NumNullSrvs; i++)
            {
                pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvCpuHandle);
                cbvSrvCpuHandle.Offset(m_cbvSrvDescriptorSize);
                cbvSrvGpuHandle.Offset(m_cbvSrvDescriptorSize);
            }
        }

        // Save the descriptor handles for the depth buffer views.
        for (UINT i = 0; i < _countof(m_depthSrvCpuHandles); i++)
        {
            m_depthSrvCpuHandles[i] = cbvSrvCpuHandle;
            m_depthSrvGpuHandles[i] = cbvSrvGpuHandle;
            cbvSrvCpuHandle.Offset(m_cbvSrvDescriptorSize);
            cbvSrvGpuHandle.Offset(m_cbvSrvDescriptorSize);
        }

        // Create each texture and SRV descriptor.
        PIXBeginEvent(pCommandList, 0, L"Copy diffuse and normal texture data to default resources...");
        for (UINT i = 0; i < _countof(SampleAssets::Textures); i++)
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

                UpdateSubresources(pCommandList, m_textures[i].Get(), m_textureUploads[i].Get(), 0, 0, subresourceCount, &textureData);
                
                // Performance tip: You can avoid some resource barriers by relying on resource state promotion and decay.
                // Resources accessed on a copy queue will decay back to the COMMON after ExecuteCommandLists()
                // completes on the GPU. Search online for "D3D12 Implicit State Transitions" for more details. 
            }

            // Describe and create an SRV.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = tex.Format;
            srvDesc.Texture2D.MipLevels = tex.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            pDevice->CreateShaderResourceView(m_textures[i].Get(), &srvDesc, cbvSrvCpuHandle);
            cbvSrvCpuHandle.Offset(m_cbvSrvDescriptorSize);
            cbvSrvGpuHandle.Offset(m_cbvSrvDescriptorSize);
        }
        PIXEndEvent(pCommandList);
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
    NAME_D3D12_OBJECT(m_vertexBuffers[VertexBuffer::ScreenQuad]);

                                             // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
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

void ShadowsFogScatteringSquidScene::CreateFrameResources(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue)
{
    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_frameResources[i] = make_unique<FrameResource>(pDevice, pCommandQueue);
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

        m_workerFinishedScenePass[i] = CreateEvent(
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
        assert(m_workerFinishedScenePass[i] != NULL);
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
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < m_frameCount; i++)
        {
            m_renderTargets[i] = ppRenderTargets[i];
            pDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvCpuHandle);
            rtvCpuHandle.Offset(1, m_rtvDescriptorSize);
            NAME_D3D12_OBJECT_INDEXED(m_renderTargets, i);
        }
    }

    // Create the depth stencil views (DSVs).
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        const UINT dsvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // Shadow depth resource.
        m_depthDsvs[DepthGenPass::Shadow] = dsvCpuHandle;
        ThrowIfFailed(CreateDepthStencilTexture2D(pDevice, width, height, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, &m_depthTextures[DepthGenPass::Shadow], m_depthDsvs[DepthGenPass::Shadow], m_depthSrvCpuHandles[DepthGenPass::Shadow]));
        NAME_D3D12_OBJECT(m_depthTextures[DepthGenPass::Shadow]);
        
        dsvCpuHandle.Offset(dsvDescriptorSize);
        
        // Scene depth resource.
        m_depthDsvs[DepthGenPass::Scene] = dsvCpuHandle;
        ThrowIfFailed(CreateDepthStencilTexture2D(pDevice, width, height, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, &m_depthTextures[DepthGenPass::Scene], m_depthDsvs[DepthGenPass::Scene], m_depthSrvCpuHandles[DepthGenPass::Scene]));
        NAME_D3D12_OBJECT(m_depthTextures[DepthGenPass::Scene]);
    }
}

// Release sample's D3D objects
void ShadowsFogScatteringSquidScene::ReleaseSizeDependentResources()
{
    m_depthTextures[DepthGenPass::Shadow].Reset();
    m_depthTextures[DepthGenPass::Scene].Reset();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_renderTargets[i].Reset();
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
    // Update camera and lights.
    const float angleChange = 2.0f * static_cast<float>(elapsedTime);

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
            XMVECTOR at = XMVectorSet(0.0f, 15.0f, 0.0f, 0.0f);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            switch (i)
            {
            case 4: up = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f); break;
            case 5: up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); break;
            }
            m_lightCameras[i].Set(eye, at, up);

            m_lightCameras[i].Get3DViewProjMatrices(&m_lights[i].view, &m_lights[i].projection, 90.0f, m_viewport.Width, m_viewport.Height);
        }
    }

    // Update and commmit constant buffers.
    UpdateConstantBuffers();
    CommitConstantBuffers();
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
    EndFrame(setBackbufferReadyForPresent);

    WaitForMultipleObjects(NumContexts, m_workerFinishShadowPass, TRUE, INFINITE);

    pCommandQueue->ExecuteCommandLists(NumContexts + 2, m_batchSubmit); // Submit PRE, MID and shadows.

    WaitForMultipleObjects(NumContexts, m_workerFinishedScenePass, TRUE, INFINITE);

    // Submit remaining command lists.
    pCommandQueue->ExecuteCommandLists(_countof(m_batchSubmit) - NumContexts - 2, m_batchSubmit + NumContexts + 2);
#endif
}

float ShadowsFogScatteringSquidScene::GetScenePassGPUTimeInMs() const
{
    return m_pCurrentFrameResource->m_gpuTimer.GetAverageMS(SceneEnums::Timestamp::ScenePass);
}

float ShadowsFogScatteringSquidScene::GetPostprocessPassGPUTimeInMs() const
{
    return m_pCurrentFrameResource->m_gpuTimer.GetAverageMS(SceneEnums::Timestamp::PostprocessPass);
}

void ShadowsFogScatteringSquidScene::ShadowPass(ID3D12GraphicsCommandList* pCommandList, int threadIndex)
{
    // Set necessary state.
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::ShadowPass].Get());

    pCommandList->SetGraphicsRootConstantBufferView(0, m_pCurrentFrameResource->GetConstantBufferGPUVirtualAddress(RenderPass::Shadow));

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::SceneGeometry]);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->OMSetRenderTargets(0, nullptr, FALSE, &m_depthDsvs[DepthGenPass::Shadow]); // No render target needed for the shadow pass.

    // Draw. Distribute objects over threads by drawing only 1/NumContexts 
    // objects per worker (i.e. every object such that objectnum % 
    // NumContexts == threadIndex).
    for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
    {
        const SampleAssets::DrawParameters& drawArgs = SampleAssets::Draws[j];
        pCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
    }
}

void ShadowsFogScatteringSquidScene::ScenePass(ID3D12GraphicsCommandList* pCommandList, int threadIndex)
{
    // Set necessary state.
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::ScenePass].Get());

    pCommandList->SetGraphicsRootConstantBufferView(1, m_pCurrentFrameResource->GetConstantBufferGPUVirtualAddress(RenderPass::Scene)); // Set scene constant buffer.
    pCommandList->SetGraphicsRootDescriptorTable(2, m_depthSrvGpuHandles[DepthGenPass::Shadow]); // Set the shadow texture as an SRV.
    pCommandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart()); // Set samplers.

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::SceneGeometry]);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->OMSetRenderTargets(1, &GetCurrentBackBufferRtvCpuHandle(), FALSE, &m_depthDsvs[DepthGenPass::Scene]);

    // Draw.
    const D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
    {
        const SampleAssets::DrawParameters& drawArgs = SampleAssets::Draws[j];

        // Set the diffuse and normal textures for the current object.
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, NumNullSrvs + _countof(m_depthSrvCpuHandles) + drawArgs.DiffuseTextureIndex, m_cbvSrvDescriptorSize);
        pCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

        pCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
    }
}

// Apply a postprocess pass with a light scattering effect. 
void ShadowsFogScatteringSquidScene::PostprocessPass(ID3D12GraphicsCommandList* pCommandList)
{
    // Set necessary state.
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::PostprocessPass].Get());
    pCommandList->SetPipelineState(m_pipelineStates[RenderPass::Postprocess].Get());

    DrawInScattering(pCommandList, GetCurrentBackBufferRtvCpuHandle());
}

void ShadowsFogScatteringSquidScene::UpdateConstantBuffers()
{
    // Scale down the world a bit.
    const float worldScale = GetWorldScale();
    const XMMATRIX worldScalingMatrix = XMMatrixScaling(worldScale, worldScale, worldScale);
    ::XMStoreFloat4x4(&m_sceneConstantBuffer.model, worldScalingMatrix);
    ::XMStoreFloat4x4(&m_shadowConstantBuffer.model, worldScalingMatrix);

    m_sceneConstantBuffer.viewport = m_shadowConstantBuffer.viewport = { m_viewport.Width, m_viewport.Height, 0.0f, 0.0f };

    // The scene pass is drawn from the camera.
    m_camera.Get3DViewProjMatrices(&m_sceneConstantBuffer.view, &m_sceneConstantBuffer.projection, 90.0f, m_viewport.Width, m_viewport.Height);

    // The light pass is drawn from the first light.
    m_lightCameras[0].Get3DViewProjMatrices(&m_shadowConstantBuffer.view, &m_shadowConstantBuffer.projection, 90.0f, m_viewport.Width, m_viewport.Height);

    for (int i = 0; i < NumLights; i++)
    {
        memcpy(&m_sceneConstantBuffer.lights[i], &m_lights[i], sizeof(LightState));
        memcpy(&m_shadowConstantBuffer.lights[i], &m_lights[i], sizeof(LightState));
    }

    // The shadow pass won't sample the shadow map, but rather write to it.
    m_shadowConstantBuffer.sampleShadowMap = FALSE;

    // The scene pass samples the shadow map.
    m_sceneConstantBuffer.sampleShadowMap = TRUE;

    m_shadowConstantBuffer.ambientColor = m_sceneConstantBuffer.ambientColor = { 0.1f, 0.2f, 0.3f, 1.0f };

    m_postprocessConstantBuffer.lightPosition = m_lights[0].position;
    m_postprocessConstantBuffer.fogDensity = m_fogDensity;
    XMStoreFloat4(&m_postprocessConstantBuffer.cameraPosition, m_camera.mEye);

    XMMATRIX view = XMMatrixLookAtRH(m_camera.mEye, m_camera.mAt, m_camera.mUp);
    XMVECTOR determinant = XMMatrixDeterminant(view);
    XMMATRIX viewInverse = XMMatrixInverse(&determinant, view);

    const float fovAngleY = 90.0f;
    const float fovRadiansY = fovAngleY * XM_PI / 180.0f;
    XMMATRIX proj = XMMatrixPerspectiveFovRH(fovRadiansY, m_viewport.Width / m_viewport.Height, 0.01f, 125.0f);
    determinant = XMMatrixDeterminant(proj);
    XMMATRIX projInverse = XMMatrixInverse(&determinant, proj);

    const float nearZ1 = 1.0f;
    XMMATRIX projAtNearZ1 = XMMatrixPerspectiveFovRH(fovRadiansY, m_viewport.Width / m_viewport.Height, nearZ1, 125.0f);
    XMMATRIX viewProjAtNearZ1 = view * projAtNearZ1;
    determinant = XMMatrixDeterminant(viewProjAtNearZ1);
    XMMATRIX viewProjInverseAtNearZ1 = XMMatrixInverse(&determinant, viewProjAtNearZ1);

    // Copy over a transposed row-major inverse matrix, since HLSL assumes col-major order.
    XMStoreFloat4x4(&m_postprocessConstantBuffer.viewInverse, XMMatrixTranspose(viewInverse));
    XMStoreFloat4x4(&m_postprocessConstantBuffer.projInverse, XMMatrixTranspose(projInverse));
    XMStoreFloat4x4(&m_postprocessConstantBuffer.viewProjInverseAtNearZ1, XMMatrixTranspose(viewProjInverseAtNearZ1));
}

void ShadowsFogScatteringSquidScene::CommitConstantBuffers()
{
    memcpy(m_pCurrentFrameResource->m_pConstantBuffersWO[RenderPass::Scene], &m_sceneConstantBuffer, sizeof(m_sceneConstantBuffer));
    memcpy(m_pCurrentFrameResource->m_pConstantBuffersWO[RenderPass::Shadow], &m_shadowConstantBuffer, sizeof(m_shadowConstantBuffer));
    memcpy(m_pCurrentFrameResource->m_pConstantBuffersWO[RenderPass::Postprocess], &m_postprocessConstantBuffer, sizeof(m_postprocessConstantBuffer));
}

void ShadowsFogScatteringSquidScene::DrawInScattering(ID3D12GraphicsCommandList* pCommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& renderTargetHandle)
{
    PIXBeginEvent(pCommandList, 0, L"In-Scattering");

    // Set necessary state.
    pCommandList->SetGraphicsRootDescriptorTable(0, m_depthSrvGpuHandles[DepthGenPass::Scene]); // Set scene depth as an SRV.
    pCommandList->SetGraphicsRootConstantBufferView(1, m_pCurrentFrameResource->GetConstantBufferGPUVirtualAddress(RenderPass::Postprocess)); // Set postprocess constant buffer.
    pCommandList->SetGraphicsRootDescriptorTable(2, m_samplerHeap->GetGPUDescriptorHandleForHeapStart()); // Set samplers.

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::ScreenQuad]);
    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->OMSetRenderTargets(1, &renderTargetHandle, FALSE, nullptr); // No depth stencil needed for in-scattering.

    // Draw.
    pCommandList->DrawInstanced(4, 1, 0, 0);

    PIXEndEvent(pCommandList);
}

// Record the CommandListPre command list.
void ShadowsFogScatteringSquidScene::BeginFrame()
{
    m_pCurrentFrameResource->InitFrame();

    ID3D12GraphicsCommandList* pCommandListPre = m_commandLists[CommandListPre].Get();

    // Reset the command list.
    ThrowIfFailed(pCommandListPre->Reset(m_pCurrentFrameResource->m_commandAllocator.Get(), nullptr));

    m_pCurrentFrameResource->BeginFrame(pCommandListPre);

    // Transition back-buffer to a writable state for rendering.
    pCommandListPre->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the render target and depth stencils.
    pCommandListPre->ClearRenderTargetView(GetCurrentBackBufferRtvCpuHandle(), s_clearColor, 0, nullptr);
    pCommandListPre->ClearDepthStencilView(m_depthDsvs[DepthGenPass::Shadow], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCommandListPre->ClearDepthStencilView(m_depthDsvs[DepthGenPass::Scene], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Close the command list.
    ThrowIfFailed(pCommandListPre->Close());
}

// Record the CommandListMid command list.
void ShadowsFogScatteringSquidScene::MidFrame()
{
    ID3D12GraphicsCommandList* pCommandListMid = m_commandLists[CommandListMid].Get();

    // Reset the command list.
    ThrowIfFailed(pCommandListMid->Reset(m_pCurrentFrameResource->m_commandAllocator.Get(), nullptr));

    // Transition our shadow map to a readable state for scene rendering.
    pCommandListMid->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Shadow].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Close the command list.
    ThrowIfFailed(pCommandListMid->Close());
}

// Record the CommandListPost command list.
void ShadowsFogScatteringSquidScene::EndFrame(bool setBackbufferReadyForPresent)
{
    ID3D12GraphicsCommandList* pCommandListPost = m_commandLists[CommandListPost].Get();

    // Reset the command list.
    ThrowIfFailed(pCommandListPost->Reset(m_pCurrentFrameResource->m_commandAllocator.Get(), nullptr));

    // Set descriptor heaps.
    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
    pCommandListPost->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // Transition scene depth to a readable state for post-processing.
    pCommandListPost->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Scene].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    PIXBeginEvent(pCommandListPost, 0, L"Postprocess pass");
    m_pCurrentFrameResource->m_gpuTimer.Start(pCommandListPost, Timestamp::PostprocessPass);
    PostprocessPass(pCommandListPost);
    m_pCurrentFrameResource->m_gpuTimer.Stop(pCommandListPost, Timestamp::PostprocessPass);
    PIXEndEvent(pCommandListPost);

    // Transition depth buffers back to a writable state for the next frame
    // and conditionally indicate that the back buffer will now be used to present.
   // Performance tip: Batch resource barriers into as few API calls as possible to minimize the amount of work the GPU does.
    D3D12_RESOURCE_BARRIER barriers[3];
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Shadow].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Scene].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    pCommandListPost->ResourceBarrier(setBackbufferReadyForPresent ? _countof(barriers) : _countof(barriers) - 1, barriers);

    m_pCurrentFrameResource->EndFrame(pCommandListPost);

    // Close the command list.
    ThrowIfFailed(pCommandListPost->Close());
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

        //
        // Shadow pass
        //
        {
            ID3D12GraphicsCommandList* pShadowCommandList = m_shadowCommandLists[threadIndex].Get();

        // Reset the command list.
        ThrowIfFailed(pShadowCommandList->Reset(m_pCurrentFrameResource->m_contextCommandAllocators[threadIndex].Get(), m_pipelineStates[SceneEnums::RenderPass::Shadow].Get()));
            PIXBeginEvent(pShadowCommandList, 0, L"Worker drawing shadow pass...");

            // Performance tip: Only set descriptor heaps if you need access to them.
            ShadowPass(pShadowCommandList, threadIndex);

            // Close the command list.
        PIXEndEvent(pShadowCommandList);
        ThrowIfFailed(pShadowCommandList->Close());

#if !SINGLETHREADED
            // Tell main thread that we are done with the shadow pass.
        SetEvent(m_workerFinishShadowPass[threadIndex]);
#endif
        }

        //
        // Scene pass
        //
        {
            ID3D12GraphicsCommandList* pSceneCommandList = m_sceneCommandLists[threadIndex].Get();

        // Reset the command list.
            ThrowIfFailed(pSceneCommandList->Reset(m_pCurrentFrameResource->m_contextCommandAllocators[threadIndex].Get(), m_pipelineStates[RenderPass::Scene].Get()));
            PIXBeginEvent(pSceneCommandList, 0, L"Worker drawing scene pass...");

            if (threadIndex == 0)
            {
                m_pCurrentFrameResource->m_gpuTimer.Start(pSceneCommandList, Timestamp::ScenePass);
            }

            // Set descriptor heaps.
            ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
            pSceneCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

            ScenePass(pSceneCommandList, threadIndex);

            if (threadIndex == NumContexts - 1)
        {
                m_pCurrentFrameResource->m_gpuTimer.Stop(pSceneCommandList, Timestamp::ScenePass);
        }

            // Close the command list.
        PIXEndEvent(pSceneCommandList);
        ThrowIfFailed(pSceneCommandList->Close());

#if !SINGLETHREADED
            // Tell main thread that we are done with the scene pass.
            SetEvent(m_workerFinishedScenePass[threadIndex]);
#endif
    }
}
}
