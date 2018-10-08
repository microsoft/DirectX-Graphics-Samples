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

class PostBuildInfoQuery
{
public:
    PostBuildInfoQuery(ID3D12Device *pDevice, UINT nodeMask);

    void GetCompactedBVHSizes(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestBuffer,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures) const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData);

private:
    enum RootParameterSlot
    {
        OutputCount = 0,
        InputConstants,
        BVHStart,
        NumParameters = BVHStart + NumberOfReadableBVHsPerDispatch
    };

    CComPtr<ID3D12RootSignature> m_pRootSignature;
    CComPtr<ID3D12PipelineState> m_pPSO;
};
