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
    class ConstructHierarchyPass
    {
    public:
        ConstructHierarchyPass(ID3D12Device *pDevice, UINT nodeMask);

        void ConstructHierarchy(ID3D12GraphicsCommandList *pCommandList,
            SceneType sceneType,
            D3D12_GPU_VIRTUAL_ADDRESS mortonCodeBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
            D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap,
            UINT numElements);
    private:
        enum RootParameterSlot
        {
            HierarchyUAVParam = 0,
            MortonCodesBufferParam,
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
        CComPtr<ID3D12PipelineState> m_pBuildSplits[Level::NumLevels];
    };
}
