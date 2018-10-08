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
    class LoadInstancesPass
    {
    public:
        LoadInstancesPass(ID3D12Device *pDevice, UINT nodeMask);
        void LoadInstances(ID3D12GraphicsCommandList *pCommandList, 
            D3D12_GPU_VIRTUAL_ADDRESS outputBVH, 
            D3D12_GPU_VIRTUAL_ADDRESS instancesDesc, 
            D3D12_ELEMENTS_LAYOUT instanceDescLayout, 
            UINT numElements, 
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorHeapBase,
            D3D12_GPU_VIRTUAL_ADDRESS cachedSortBuffer);
    private:
        enum RootParameterSlot
        {
            OutputBVHRootUAV = 0,
            InstanceDescsSRV,
            GlobalDescriptorHeap,
            Constants,
            CachedSortBuffer,
            NumRootParameters,
        };

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pLoadAABBsFromArrayOfPointersPSO;
        CComPtr<ID3D12PipelineState> m_pLoadAABBsFromArrayOfInstancesPSO;
    };
}
