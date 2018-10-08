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
#include "pch.h"

#include "CompiledShaders/GetBVHCompactedSize.h"
PostBuildInfoQuery::PostBuildInfoQuery(ID3D12Device *pDevice, UINT nodeMask)
{
    CD3DX12_ROOT_PARAMETER1 parameters[NumParameters];
    parameters[OutputCount].InitAsUnorderedAccessView(OutputCountRegister);
    parameters[InputConstants].InitAsConstants(SizeOfInUint32(GetBVHCompactedSizeConstants), GetBVHCompactedSizeConstantsRegister);
    for (UINT i = 0; i < NumberOfReadableBVHsPerDispatch; i++)
    {
        parameters[BVHStart + i].InitAsUnorderedAccessView(BVHStartRegister + i);
    }

    auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
    CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

    CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pGetBVHCompactedSize), &m_pPSO);
}

void PostBuildInfoQuery::GetCompactedBVHSizes(
    _In_  ID3D12GraphicsCommandList *pCommandList,
    _In_  D3D12_GPU_VIRTUAL_ADDRESS DestBuffer,
    _In_  UINT NumSourceAccelerationStructures,
    _In_reads_(NumSourceAccelerationStructures) const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData)
{
    pCommandList->SetComputeRootSignature(m_pRootSignature);
    pCommandList->SetPipelineState(m_pPSO);

    UINT NumAccelerationStructuresProcessed = 0;
    D3D12_GPU_VIRTUAL_ADDRESS outputCountAddress = DestBuffer;
    GetBVHCompactedSizeConstants constant;
    while (NumAccelerationStructuresProcessed != NumSourceAccelerationStructures)
    {
        UINT numBVHsProcessedThisDispatch = std::min(NumSourceAccelerationStructures - NumAccelerationStructuresProcessed, (UINT)NumberOfReadableBVHsPerDispatch);
        constant.NumberOfBoundBVHs = numBVHsProcessedThisDispatch;
        pCommandList->SetComputeRootUnorderedAccessView(OutputCount, outputCountAddress);
        pCommandList->SetComputeRoot32BitConstants(InputConstants, SizeOfInUint32(GetBVHCompactedSizeConstants), &constant, 0);
        
        for (UINT i = 0; i < numBVHsProcessedThisDispatch; i++)
        {
            pCommandList->SetComputeRootUnorderedAccessView(BVHStart + i, pSourceAccelerationStructureData[NumAccelerationStructuresProcessed + i]);
        }

        const UINT dispatchWidth = DivideAndRoundUp<UINT>(numBVHsProcessedThisDispatch, THREAD_GROUP_1D_WIDTH);
        pCommandList->Dispatch(dispatchWidth, 1, 1);

        NumAccelerationStructuresProcessed += numBVHsProcessedThisDispatch;
        outputCountAddress += numBVHsProcessedThisDispatch * sizeof(UINT32);
    }
}
