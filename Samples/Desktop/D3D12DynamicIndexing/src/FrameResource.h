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

#include "DXSampleHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class FrameResource
{
private:
    void SetCityPositions(FLOAT intervalX, FLOAT intervalZ);

public:
    struct SceneConstantBuffer
    {
        XMFLOAT4X4 mvp;        // Model-view-projection (MVP) matrix.
        FLOAT padding[48];
    };

    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_bundle;
    ComPtr<ID3D12Resource> m_cbvUploadHeap;
    SceneConstantBuffer* m_pConstantBuffers;
    UINT64 m_fenceValue;

    std::vector<XMFLOAT4X4> m_modelMatrices;
    UINT m_cityRowCount;
    UINT m_cityColumnCount;
    UINT m_cityMaterialCount;

    FrameResource(ID3D12Device* pDevice, UINT cityRowCount, UINT cityColumnCount, UINT cityMaterialCount, float citySpacingInterval);
    ~FrameResource();

    void InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pPso,
        UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
        ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature);

    void PopulateCommandList(ID3D12GraphicsCommandList* pCommandList,
        UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
        ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature);

    void XM_CALLCONV UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection);
};
