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
#include "GpuBvh2CopyBindings.h"
#include "CompiledShaders/GpuBvh2Copy.h"


GpuBvh2Copy::GpuBvh2Copy(ID3D12Device *pDevice, UINT totalLaneCount, UINT nodeMask) : 
    // TODO: Consider making this use dispatch indirect instead. Due to the ambiguity in how 
    // drivers can handle large thread groups, it's unclear if this is actually an "optimal"
    // dispatch width
    m_OptimalDispatchWidth(DivideAndRoundUp<UINT>(totalLaneCount, GPU_BVH2_COPY_THREAD_GROUP_WIDTH))
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[NumParameters];
    rootParameters[DestBvh].InitAsUnorderedAccessView(DestBvhRegister);
    rootParameters[SourceBvh].InitAsUnorderedAccessView(SourceBvhRegister);
    rootParameters[Constants].InitAsConstants(SizeOfInUint32(DispatchWidthConstant), DispatchWidthConstantsRegister);
    
    auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(rootParameters), rootParameters);
    CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

    CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pGpuBvh2Copy), &m_pPSO);
}

void GpuBvh2Copy::CopyRaytracingAccelerationStructure(
    _In_  ID3D12GraphicsCommandList *pCommandList,
    _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
    _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData)
{
    pCommandList->SetComputeRootSignature(m_pRootSignature);
    pCommandList->SetPipelineState(m_pPSO);
    pCommandList->SetComputeRootUnorderedAccessView(DestBvh, DestAccelerationStructureData);
    pCommandList->SetComputeRootUnorderedAccessView(SourceBvh, SourceAccelerationStructureData);
    pCommandList->SetComputeRoot32BitConstant(Constants, m_OptimalDispatchWidth, 0);
    pCommandList->Dispatch(m_OptimalDispatchWidth, 1, 1);
}
