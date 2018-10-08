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
    class ConstructAABBPass
    {
    public:
        ConstructAABBPass(ID3D12Device *pDevice, UINT nodeMask);

        void ConstructAABB(ID3D12GraphicsCommandList *pCommandList,
            SceneType sceneType,
            D3D12_GPU_VIRTUAL_ADDRESS outputVH,
            D3D12_GPU_VIRTUAL_ADDRESS scratchBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS childNodesProcessedCountBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS outputAABBParentBuffer,
            D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap,
            const bool prepareUpdate,
            const bool performUpdate,
            UINT numElements);
    private:
        enum RootParameterSlot
        {
            OutputBVHRootUAVParam = 0,
            ScratchUAVParam,
            HierarchyUAVParam,
            AABBParentBufferParam,
            ChildNodesProcessedCountBufferParam,
            InputRootConstants,
            GlobalDescriptorHeap,
            NumRootParameters,
        };

        enum Level
        {
            Bottom = 0,
            Top,
            NumLevels
        };

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pPrepareForComputeAABBs[Level::NumLevels];
        CComPtr<ID3D12PipelineState> m_pComputeAABBs[Level::NumLevels];
    };
}
