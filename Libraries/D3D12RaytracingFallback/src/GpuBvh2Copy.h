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

class GpuBvh2Copy
{
public:
    GpuBvh2Copy(ID3D12Device *pDevice, UINT totalLaneCount, UINT nodeMask);

    void CopyRaytracingAccelerationStructure(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData);

private:
    enum RootParameterSlot
    {
        DestBvh = 0,
        SourceBvh,
        Constants,
        NumParameters
    };

    const UINT m_OptimalDispatchWidth;
    CComPtr<ID3D12RootSignature> m_pRootSignature;
    CComPtr<ID3D12PipelineState> m_pPSO;
};
