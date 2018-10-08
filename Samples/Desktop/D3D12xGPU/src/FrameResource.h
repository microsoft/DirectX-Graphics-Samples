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

#pragma once

#include "ShadowsFogScatteringSquidScene.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class FrameResource
{
public:
    ID3D12CommandList * m_batchSubmit[NumContexts * 2 + CommandListCount];   // *2: shadowCommandLists, sceneCommandLists

    ComPtr<ID3D12CommandAllocator> m_commandAllocators[CommandListCount];
    ComPtr<ID3D12GraphicsCommandList> m_commandLists[CommandListCount];

    ComPtr<ID3D12CommandAllocator> m_shadowCommandAllocators[NumContexts];
    ComPtr<ID3D12GraphicsCommandList> m_shadowCommandLists[NumContexts];

    ComPtr<ID3D12CommandAllocator> m_sceneCommandAllocators[NumContexts];
    ComPtr<ID3D12GraphicsCommandList> m_sceneCommandLists[NumContexts];

    ComPtr<ID3D12CommandAllocator> m_postprocessCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_postprocessCommandList;

    ComPtr<ID3D12Resource> m_renderTarget;
    D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetView;
    ComPtr<ID3D12PipelineState> m_pipelineStates[SceneEnums::RenderPass::Count];
    D3D12_GPU_DESCRIPTOR_HANDLE m_nullSrvHandle;                        // Null SRV for out of bounds behavior.

    ComPtr<ID3D12Resource> m_constantBuffers[SceneEnums::RenderPass::Count];
    SceneConstantBuffer* m_pConstantBuffersWO[SceneEnums::RenderPass::Count];        // WRITE-ONLY pointers to the constant buffers
    D3D12_GPU_DESCRIPTOR_HANDLE m_cbvHandles[SceneEnums::RenderPass::Count];
    
    ComPtr<ID3D12Resource> m_depthTextures[SceneEnums::DepthGenPass::Count];
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthDSVs[SceneEnums::DepthGenPass::Count];
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthSRVs[SceneEnums::DepthGenPass::Count];
    D3D12_GPU_DESCRIPTOR_HANDLE m_depthSrvHandles[SceneEnums::DepthGenPass::Count];

    UINT m_frameResourceIndex;

public:
    FrameResource(ID3D12Device* pDevice, ComPtr<ID3D12PipelineState> pPipelineStates[SceneEnums::RenderPass::Count], ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, UINT frameResourceIndex);
    ~FrameResource();

    void LoadSizeDependentResources(ID3D12Device* pDevice, UINT width, UINT height);
    void ReleaseSizeDependentResources();
   
    void ClearDepthStencilViews(ID3D12GraphicsCommandList* pCommandList);
    void Bind(ID3D12GraphicsCommandList* pCommandList, SceneEnums::RenderPass::Value renderPass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle);
    void Init();
    void SwapBarriers();
    void Finish();
    void WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[NumLights], LightState lights[NumLights]);
};


inline HRESULT CreateDepthStencilTexture2D(
    ID3D12Device* pDevice,
    UINT width,
    UINT height,
    DXGI_FORMAT typelessFormat,
    DXGI_FORMAT dsvFormat,
    DXGI_FORMAT srvFormat,
    ID3D12Resource** ppResource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDSVHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuSRVHandle,
    D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_DEPTH_WRITE,
    float initDepthValue = 1.0f,
    UINT8 initStencilValue = 0)
{
    try
    {
        *ppResource = nullptr;

        CD3DX12_RESOURCE_DESC texDesc(
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,
            width,
            height,
            1,
            1,
            typelessFormat,
            1,
            0,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        D3D12_CLEAR_VALUE clearValue;        // Performance tip: Tell the runtime at resource creation the desired clear value.
        clearValue.Format = dsvFormat;
        clearValue.DepthStencil.Depth = initDepthValue;
        clearValue.DepthStencil.Stencil = initStencilValue;

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            initState,
            &clearValue,
            IID_PPV_ARGS(ppResource)));

        ThrowIfFailed((*ppResource)->SetName(L"DepthStencilResource"));

        // Create a depth stencil view (DSV).
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = dsvFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        pDevice->CreateDepthStencilView(*ppResource, &dsvDesc, cpuDSVHandle);

        // Create a shader resource view (SRV).
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = srvFormat;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        pDevice->CreateShaderResourceView(*ppResource, &srvDesc, cpuSRVHandle);
    }
    catch (HrException& e)
    {
        SAFE_RELEASE(*ppResource);
        return e.Error();
    }
    return S_OK;
}

inline HRESULT CreateConstantBuffer(
    ID3D12Device* pDevice,
    UINT size,
    ID3D12Resource** ppResource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuCBVHandle,
    D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_GENERIC_READ)
{
    try
    {
        *ppResource = nullptr;

        const UINT alignedSize = CalculateConstantBufferByteSize(size);
        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(alignedSize),
            initState,
            nullptr,
            IID_PPV_ARGS(ppResource)));

        // Create the constant buffer views: one for the shadow pass and
        // another for the scene pass.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.SizeInBytes = alignedSize;

        // Describe and create the shadow constant buffer view (CBV) and 
        // cache the GPU descriptor handle.
        cbvDesc.BufferLocation = (*ppResource)->GetGPUVirtualAddress();
        pDevice->CreateConstantBufferView(&cbvDesc, cpuCBVHandle);
    }
    catch (HrException& e)
    {
        SAFE_RELEASE(*ppResource);
        return e.Error();
    }
    return S_OK;
}