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
namespace FallbackLayer
{
    class TreeletReorder
    {
    public:
        TreeletReorder(ID3D12Device *pDevice, UINT nodeMask);

        void Optimize(
            ID3D12GraphicsCommandList *pCommandList,
            UINT numElements,
            D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS triangleCountBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS aabbBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS inputElementBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS baseTreeletsCountBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS baseTreeletsBuffer,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlag);

        static UINT RequiredSizeForAABBBuffer(UINT numElements);
        static UINT RequiredSizeForBaseTreeletBuffers(UINT numElements);
    private:
        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pClearBuffersPSO;
        CComPtr<ID3D12PipelineState> m_pFindTreeletsPSO;
        CComPtr<ID3D12PipelineState> m_pTreeletReorderPSO;

        enum RootParameterSlot
        {
            HierarchyBufferSlot = 0,
            TriangleCountBufferSlot,
            AABBBufferSlot,
            InputElementSlot,
            BaseTreeletsCountBufferSlot,
            BaseTreeletsIndexBufferSlot,
            ConstantsSlot,
            NumParameters
        };

        static UINT MaxNumTreelets(UINT numElements, UINT minElementsPerTreelet);
    };
}

