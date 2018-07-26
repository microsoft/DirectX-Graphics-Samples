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
#include "FrameResource.h"
#include "SquidRoom.h"
#include "DXSampleHelper.h"

using namespace SceneEnums;

FrameResource::FrameResource(ID3D12Device* pDevice, ComPtr<ID3D12PipelineState> pPipelineStates[RenderPass::Count], ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, UINT frameIndex) :
    m_frameResourceIndex(frameIndex)
{
    for (UINT i = 0; i < RenderPass::Count; i++)
    {
        m_pipelineStates[i] = pPipelineStates[i];
    }

    for (UINT i = 0; i < CommandListCount; i++)
    {
        ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i].Get(), m_pipelineStates[i].Get(), IID_PPV_ARGS(&m_commandLists[i])));
        NAME_D3D12_OBJECT_INDEXED(m_commandLists, i);

        // Close these command lists; don't record into them for now.
        ThrowIfFailed(m_commandLists[i]->Close());
    }

    for (UINT i = 0; i < NumContexts; i++)
    {
        // Create command list allocators for worker threads. One alloc is 
        // for the shadow pass command list, and one is for the scene pass.
        ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_shadowCommandAllocators[i])));
        ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[i])));

        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_shadowCommandAllocators[i].Get(), m_pipelineStates[RenderPass::Shadow].Get(), IID_PPV_ARGS(&m_shadowCommandLists[i])));
        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_sceneCommandAllocators[i].Get(), m_pipelineStates[RenderPass::Scene].Get(), IID_PPV_ARGS(&m_sceneCommandLists[i])));

        NAME_D3D12_OBJECT_INDEXED(m_shadowCommandLists, i);
        NAME_D3D12_OBJECT_INDEXED(m_sceneCommandLists, i);

        // Close these command lists; don't record into them for now. We will 
        // reset them to a recording state when we start the render loop.
        ThrowIfFailed(m_shadowCommandLists[i]->Close());
        ThrowIfFailed(m_sceneCommandLists[i]->Close());
    }
    ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_postprocessCommandAllocator)));
    ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_postprocessCommandAllocator.Get(), m_pipelineStates[RenderPass::Postprocess].Get(), IID_PPV_ARGS(&m_postprocessCommandList)));
    NAME_D3D12_OBJECT(m_postprocessCommandList);
    ThrowIfFailed(m_postprocessCommandList->Close());

    const UINT textureCount = _countof(SampleAssets::Textures);    // Diffuse + normal textures near the start of the heap.  Ideally, track descriptor heap contents/offsets at a higher level.
    const UINT cbvSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const UINT dsvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(pCbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(pCbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(pDsvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameResourceIndex * NumDepthBuffers, dsvDescriptorSize);
    m_nullSrvHandle = cbvSrvGpuHandle;
    cbvSrvCpuHandle.Offset(NumNullSrvs + textureCount + (m_frameResourceIndex * (NumDepthBuffers + NumConstantBuffers)), cbvSrvDescriptorSize);
    cbvSrvGpuHandle.Offset(NumNullSrvs + textureCount + (m_frameResourceIndex * (NumDepthBuffers + NumConstantBuffers)), cbvSrvDescriptorSize);
   
    // Initialize descript heap handles for depth resources.
    // The resources and views are created later as they are window size dependent.
    {
        // Shadow depth resource.
        m_depthDSVs[DepthGenPass::Shadow] = dsvHandle;
        m_depthSRVs[DepthGenPass::Shadow] = cbvSrvCpuHandle;
        m_depthSrvHandles[DepthGenPass::Shadow] = cbvSrvGpuHandle;
     
        dsvHandle.Offset(dsvDescriptorSize);
        cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
        cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);

        // Scene depth resource.
        m_depthDSVs[DepthGenPass::Scene] = dsvHandle;
        m_depthSRVs[DepthGenPass::Scene] = cbvSrvCpuHandle;
        m_depthSrvHandles[DepthGenPass::Scene] = cbvSrvGpuHandle;
    }
        
    // Create constant buffers.
    {
        // Shadow generation constant buffer.
        cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
        cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);
        ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(SceneConstantBuffer), &m_constantBuffers[RenderPass::Shadow], cbvSrvCpuHandle, D3D12_RESOURCE_STATE_GENERIC_READ));
        m_cbvHandles[RenderPass::Shadow] = cbvSrvGpuHandle;
        NAME_D3D12_OBJECT(m_constantBuffers[RenderPass::Shadow]);
        
        // Scene render constant buffer.
        cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
        cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);
        ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(SceneConstantBuffer), &m_constantBuffers[RenderPass::Scene], cbvSrvCpuHandle, D3D12_RESOURCE_STATE_GENERIC_READ));
        m_cbvHandles[RenderPass::Scene] = cbvSrvGpuHandle;
        NAME_D3D12_OBJECT(m_constantBuffers[RenderPass::Scene]);

        // Postprocess pass constant buffer.
        cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
        cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);
        ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(PostprocessConstantBuffer), &m_constantBuffers[RenderPass::Postprocess], cbvSrvCpuHandle, D3D12_RESOURCE_STATE_GENERIC_READ));
        m_cbvHandles[RenderPass::Postprocess] = cbvSrvGpuHandle;
        NAME_D3D12_OBJECT(m_constantBuffers[RenderPass::Postprocess]);

        // Map the constant buffers and cache their heap pointers.
        // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffers[RenderPass::Shadow]->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffersWO[RenderPass::Shadow])));
        ThrowIfFailed(m_constantBuffers[RenderPass::Scene]->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffersWO[RenderPass::Scene])));
        ThrowIfFailed(m_constantBuffers[RenderPass::Postprocess]->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffersWO[RenderPass::Postprocess])));
        
    }

    // Batch up command lists for execution later.
    const UINT batchSize = _countof(m_sceneCommandLists) + _countof(m_shadowCommandLists) + CommandListCount;
    m_batchSubmit[0] = m_commandLists[CommandListPre].Get();
    memcpy(m_batchSubmit + 1, m_shadowCommandLists, _countof(m_shadowCommandLists) * sizeof(ID3D12CommandList*));
    m_batchSubmit[_countof(m_shadowCommandLists) + 1] = m_commandLists[CommandListMid].Get();
    memcpy(m_batchSubmit + _countof(m_shadowCommandLists) + 2, m_sceneCommandLists, _countof(m_sceneCommandLists) * sizeof(ID3D12CommandList*));
    m_batchSubmit[batchSize - 1] = m_commandLists[CommandListPost].Get();
}

FrameResource::~FrameResource()
{
    for (int i = 0; i < CommandListCount; i++)
    {
        m_commandAllocators[i] = nullptr;
        m_commandLists[i] = nullptr;
    }

    m_constantBuffers[RenderPass::Shadow] = nullptr;
    m_constantBuffers[RenderPass::Scene] = nullptr;

    for (int i = 0; i < NumContexts; i++)
    {
        m_shadowCommandLists[i] = nullptr;
        m_shadowCommandAllocators[i] = nullptr;

        m_sceneCommandLists[i] = nullptr;
        m_sceneCommandAllocators[i] = nullptr;
    }
    m_postprocessCommandAllocator = nullptr;
    m_postprocessCommandList = nullptr;

    m_depthTextures[DepthGenPass::Shadow] = nullptr;
}

void FrameResource::LoadSizeDependentResources(ID3D12Device* pDevice, UINT width, UINT height)
{
    // Create depth stencil resources.

    // Shadow depth resource.
    ThrowIfFailed(CreateDepthStencilTexture2D(pDevice, width, height, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, &m_depthTextures[DepthGenPass::Shadow], m_depthDSVs[DepthGenPass::Shadow], m_depthSRVs[DepthGenPass::Shadow]));
    NAME_D3D12_OBJECT(m_depthTextures[DepthGenPass::Shadow]);

    // Scene depth resource.
    ThrowIfFailed(CreateDepthStencilTexture2D(pDevice, width, height, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, &m_depthTextures[DepthGenPass::Scene], m_depthDSVs[DepthGenPass::Scene], m_depthSRVs[DepthGenPass::Scene]));
    NAME_D3D12_OBJECT(m_depthTextures[DepthGenPass::Scene]);
}

void FrameResource::ReleaseSizeDependentResources()
{
    m_depthTextures[DepthGenPass::Shadow].Reset();
    m_depthTextures[DepthGenPass::Scene].Reset();
}

void FrameResource::ClearDepthStencilViews(ID3D12GraphicsCommandList* pCommandList)
{
    pCommandList->ClearDepthStencilView(m_depthDSVs[DepthGenPass::Shadow], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCommandList->ClearDepthStencilView(m_depthDSVs[DepthGenPass::Scene], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

// Builds and writes constant buffers from scratch to the proper slots for 
// this frame resource.
void FrameResource::WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[NumLights], LightState lights[NumLights])
{
    SceneConstantBuffer sceneConsts = {}; 
    SceneConstantBuffer shadowConsts = {};
    PostprocessConstantBuffer postprocessConsts = {};
    
    // Scale down the world a bit.
    ::XMStoreFloat4x4(&sceneConsts.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));
    ::XMStoreFloat4x4(&shadowConsts.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));

    // The scene pass is drawn from the camera.
    pSceneCamera->Get3DViewProjMatrices(&sceneConsts.view, &sceneConsts.projection, 90.0f, pViewport->Width, pViewport->Height);

    // The light pass is drawn from the first light.
    lightCams[0].Get3DViewProjMatrices(&shadowConsts.view, &shadowConsts.projection, 90.0f, pViewport->Width, pViewport->Height);

    for (int i = 0; i < NumLights; i++)
    {
        memcpy(&sceneConsts.lights[i], &lights[i], sizeof(LightState));
        memcpy(&shadowConsts.lights[i], &lights[i], sizeof(LightState));
    }

    // The shadow pass won't sample the shadow map, but rather write to it.
    shadowConsts.sampleShadowMap = FALSE;

    // The scene pass samples the shadow map.
    sceneConsts.sampleShadowMap = TRUE;

    shadowConsts.ambientColor = sceneConsts.ambientColor = { 0.1f, 0.2f, 0.3f, 1.0f };

    postprocessConsts.lightPosition = lights[0].position;
    XMStoreFloat4(&postprocessConsts.cameraPosition, pSceneCamera->mEye);
    float fovAngleY = 90.0f;
    float fovRadiansY = fovAngleY * XM_PI / 180.0f;
    XMMATRIX view = XMMatrixLookAtRH(pSceneCamera->mEye, pSceneCamera->mAt, pSceneCamera->mUp);
    XMVECTOR determinant = XMMatrixDeterminant(view);
    XMMATRIX viewInverse = XMMatrixInverse(&determinant, view);

    XMMATRIX proj = XMMatrixPerspectiveFovRH(fovRadiansY, pViewport->Width / pViewport->Height, 0.01f, 125.0f);
    determinant = XMMatrixDeterminant(proj);
    XMMATRIX projInverse = XMMatrixInverse(&determinant, proj);

    float nearZ1 = 1.0f;
    XMMATRIX projAtNearZ1 = XMMatrixPerspectiveFovRH(fovRadiansY, pViewport->Width/ pViewport->Height, nearZ1, 125.0f);
    XMMATRIX viewProjAtNearZ1 = view * projAtNearZ1;
    determinant = XMMatrixDeterminant(viewProjAtNearZ1);
    XMMATRIX viewProjInverseAtNearZ1 = XMMatrixInverse(&determinant, viewProjAtNearZ1);

    // Copy over a transposed row-major inverse matrix, since HLSL assumes col-major order.
    XMStoreFloat4x4(&postprocessConsts.viewInverse, XMMatrixTranspose(viewInverse));
    XMStoreFloat4x4(&postprocessConsts.projInverse, XMMatrixTranspose(projInverse));
    XMStoreFloat4x4(&postprocessConsts.viewProjInverseAtNearZ1, XMMatrixTranspose(viewProjInverseAtNearZ1));

    memcpy(m_pConstantBuffersWO[RenderPass::Scene], &sceneConsts, sizeof(sceneConsts));
    memcpy(m_pConstantBuffersWO[RenderPass::Shadow], &shadowConsts, sizeof(shadowConsts));
    memcpy(m_pConstantBuffersWO[RenderPass::Postprocess], &postprocessConsts, sizeof(postprocessConsts));
}

void FrameResource::Init()
{
    // Reset the command allocators and lists for the main thread.
    for (int i = 0; i < CommandListCount; i++)
    {
        ThrowIfFailed(m_commandAllocators[i]->Reset());
        ThrowIfFailed(m_commandLists[i]->Reset(m_commandAllocators[i].Get(), m_pipelineStates[RenderPass::Scene].Get()));
    }
    
    // Reset the worker command allocators and lists.
    for (int i = 0; i < NumContexts; i++)
    {
        ThrowIfFailed(m_shadowCommandAllocators[i]->Reset());
        ThrowIfFailed(m_shadowCommandLists[i]->Reset(m_shadowCommandAllocators[i].Get(), m_pipelineStates[RenderPass::Shadow].Get()));

        ThrowIfFailed(m_sceneCommandAllocators[i]->Reset());
        ThrowIfFailed(m_sceneCommandLists[i]->Reset(m_sceneCommandAllocators[i].Get(), m_pipelineStates[RenderPass::Scene].Get()));
    }

    ThrowIfFailed(m_postprocessCommandAllocator->Reset());
    ThrowIfFailed(m_postprocessCommandList->Reset(m_postprocessCommandAllocator.Get(), m_pipelineStates[RenderPass::Postprocess].Get()));
}

void FrameResource::SwapBarriers()
{
    // Transition the shadow map from writeable to readable.
    m_commandLists[CommandListMid]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Shadow].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}


void FrameResource::Finish()
{
    m_commandLists[CommandListPost]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Shadow].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

// Sets up the descriptor tables for the worker command list to use 
// resources provided by frame resource.
void FrameResource::Bind(ID3D12GraphicsCommandList* pCommandList, RenderPass::Value renderPass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle)
{
    switch (renderPass)
    {
    case RenderPass::Shadow:
    {
        pCommandList->SetGraphicsRootDescriptorTable(1, m_cbvHandles[RenderPass::Shadow]);
        pCommandList->SetGraphicsRootDescriptorTable(2, m_nullSrvHandle);                                // Set a null SRV for the shadow texture.

        pCommandList->OMSetRenderTargets(0, nullptr, FALSE, &m_depthDSVs[DepthGenPass::Shadow]);        // No render target needed for the shadow pass.
        break;
    }
    case RenderPass::Scene:
    {
        pCommandList->SetGraphicsRootDescriptorTable(1, m_cbvHandles[RenderPass::Scene]);
        pCommandList->SetGraphicsRootDescriptorTable(2, m_depthSrvHandles[DepthGenPass::Shadow]);        // Set the shadow texture as an SRV.

        pCommandList->OMSetRenderTargets(1, pRtvHandle, FALSE, &m_depthDSVs[DepthGenPass::Scene]);
        break;
    }
    case RenderPass::Postprocess:
    {
        pCommandList->SetGraphicsRootDescriptorTable(0, m_depthSrvHandles[DepthGenPass::Scene]);
        pCommandList->SetGraphicsRootDescriptorTable(1, m_cbvHandles[RenderPass::Postprocess]);

        pCommandList->OMSetRenderTargets(1, pRtvHandle, FALSE, nullptr);                                // No depth stencil needed for the postprocess pass.
        break;
    }
    }
}
