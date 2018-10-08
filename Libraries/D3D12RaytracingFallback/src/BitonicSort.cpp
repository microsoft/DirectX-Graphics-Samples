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
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//

#include "pch.h"

#include "CompiledShaders/BitonicIndirectArgsCS.h"
#include "CompiledShaders/BitonicPreSortCS.h"
#include "CompiledShaders/BitonicInnerSortCS.h"
#include "CompiledShaders/BitonicOuterSortCS.h"

BitonicSort::BitonicSort(ID3D12Device *pDevice, UINT nodeMask)
{    
    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto indirectArgBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cIndirectArgStride * 22 * 23 / 2, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    ThrowInternalFailure(pDevice->CreateCommittedResource(
        &defaultHeapProperties, 
        D3D12_HEAP_FLAG_NONE, 
        &indirectArgBufferDesc, 
        D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 
        nullptr, 
        IID_PPV_ARGS(&m_pDispatchArgs)));

    CD3DX12_ROOT_PARAMETER1 parameters[NumParameters];
    parameters[ShaderSpecificConstants].InitAsConstants(2, 0);
    parameters[OutputUAV].InitAsUnorderedAccessView(0);
    parameters[IndexBufferUAV].InitAsUnorderedAccessView(1);
    parameters[GenericConstants].InitAsConstants(2, 1);

    auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
    CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

    CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pBitonicIndirectArgsCS),&m_pBitonicIndirectArgsCS);
    CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pBitonicInnerSortCS), &m_pBitonicInnerSortCS);
    CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pBitonicOuterSortCS), &m_pBitonicOuterSortCS);
    CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pBitonicPreSortCS),   &m_pBitonicPreSortCS);
    
    D3D12_INDIRECT_ARGUMENT_DESC indirectArgDesc = {};
    indirectArgDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc;
    commandSignatureDesc.NodeMask = nodeMask;
    commandSignatureDesc.pArgumentDescs = &indirectArgDesc;
    commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
    commandSignatureDesc.NumArgumentDescs = 1;
    pDevice->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&m_pCommandSignature));
}

void BitonicSort::Sort(
    ID3D12GraphicsCommandList *pCommandList,
    D3D12_GPU_VIRTUAL_ADDRESS SortKeyBuffer,
    D3D12_GPU_VIRTUAL_ADDRESS IndexBuffer,
    UINT ElementCount,
    bool IsPartiallyPreSorted,
    bool SortAscending)
{
    if (ElementCount == 0) return;

    const uint32_t AlignedNumElements = AlignPowerOfTwo(ElementCount);
    const uint32_t MaxIterations = Log2(std::max(2048u, AlignedNumElements)) - 10;

    pCommandList->SetComputeRootSignature(m_pRootSignature);

    struct InputConstants
    {
        UINT NullIndex;
        UINT ListCount;
    };
    InputConstants constants { SortAscending ? 0xffffffff : 0, ElementCount };
    pCommandList->SetComputeRoot32BitConstants(GenericConstants, SizeOfInUint32(InputConstants), &constants, 0);
    
    // Generate execute indirect arguments
    pCommandList->SetPipelineState(m_pBitonicIndirectArgsCS);

    auto argToUAVTransition = CD3DX12_RESOURCE_BARRIER::Transition(m_pDispatchArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    pCommandList->ResourceBarrier(1, &argToUAVTransition);

    pCommandList->SetComputeRoot32BitConstant(ShaderSpecificConstants, MaxIterations, 0);
    pCommandList->SetComputeRootUnorderedAccessView(OutputUAV, m_pDispatchArgs->GetGPUVirtualAddress());
    pCommandList->SetComputeRootUnorderedAccessView(IndexBufferUAV, IndexBuffer);
    pCommandList->Dispatch(1, 1, 1);

    // Pre-Sort the buffer up to k = 2048.  This also pads the list with invalid indices
    // that will drift to the end of the sorted list.
    auto argToIndirectArgTransition = CD3DX12_RESOURCE_BARRIER::Transition(m_pDispatchArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    pCommandList->ResourceBarrier(1, &argToIndirectArgTransition);
    pCommandList->SetComputeRootUnorderedAccessView(OutputUAV, SortKeyBuffer);

    auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    if (!IsPartiallyPreSorted)
    {
        pCommandList->SetPipelineState(m_pBitonicPreSortCS);
        pCommandList->ExecuteIndirect(m_pCommandSignature, 1, m_pDispatchArgs, 0, nullptr, 0);
        pCommandList->ResourceBarrier(1, &uavBarrier);
    }

    uint32_t IndirectArgsOffset = cIndirectArgStride;

    // We have already pre-sorted up through k = 2048 when first writing our list, so
    // we continue sorting with k = 4096.  For unnecessarily large values of k, these
    // indirect dispatches will be skipped over with thread counts of 0.

    for (uint32_t k = 4096; k <= AlignedNumElements; k *= 2)
    {
        pCommandList->SetPipelineState(m_pBitonicOuterSortCS);

        for (uint32_t j = k / 2; j >= 2048; j /= 2)
        {
            struct OuterSortConstants
            {
                UINT k;
                UINT j;
            } constants { k, j };

            pCommandList->SetComputeRoot32BitConstants(ShaderSpecificConstants, SizeOfInUint32(OuterSortConstants), &constants, 0);
            pCommandList->ExecuteIndirect(m_pCommandSignature, 1, m_pDispatchArgs, IndirectArgsOffset, nullptr, 0);
            pCommandList->ResourceBarrier(1, &uavBarrier);
            IndirectArgsOffset += cIndirectArgStride;
        }

        pCommandList->SetPipelineState(m_pBitonicInnerSortCS);
        pCommandList->ExecuteIndirect(m_pCommandSignature, 1, m_pDispatchArgs, IndirectArgsOffset, nullptr, 0);
        pCommandList->ResourceBarrier(1, &uavBarrier);
        IndirectArgsOffset += cIndirectArgStride;
    }
}
