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
            D3D12_GPU_VIRTUAL_ADDRESS reorderBubbleBuffer,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlag);

        static UINT RequiredSizeForAABBBuffer(UINT numElements);
        static UINT RequiredSizeForBubbleBuffer(UINT numElements);
    private:
        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pPSO;
        CComPtr<ID3D12PipelineState> m_pPSO_OPT;
        CComPtr<ID3D12PipelineState> m_pPSO_OPT_PL;
        CComPtr<ID3D12PipelineState> m_pPSO_OPT_T;
        CComPtr<ID3D12PipelineState> m_pClearBufferPSO;
        CComPtr<ID3D12PipelineState> m_pComputeAABBsPSO;

        enum RootParameterSlot
        {
            HierarchyBufferSlot = 0,
            TriangleCountBufferSlot,
            AABBBufferSlot,
            InputElementSlot,
            BubbleBufferSlot,
            ConstantsSlot,
            NumParameters
        };
    };
}

