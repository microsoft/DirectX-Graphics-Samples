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
#include "CompiledShaders/TreeletReorder.h"
#include "CompiledShaders/ClearBuffer.h"
#include "TreeletReorderBindings.h"

namespace FallbackLayer
{
    TreeletReorder::TreeletReorder(ID3D12Device *pDevice, UINT nodeMask)
    {
        D3D12_DESCRIPTOR_RANGE1 globalDescriptorHeapRange = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, (UINT)-1, GlobalDescriptorHeapRegister, GlobalDescriptorHeapRegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, 0);

        CD3DX12_ROOT_PARAMETER1 parameters[RootParameterSlot::NumParameters];
        parameters[HierarchyBufferSlot].InitAsUnorderedAccessView(HierarchyBufferRegister);
        parameters[TriangleCountBufferSlot].InitAsUnorderedAccessView(NumTrianglesBufferRegister);
        parameters[AABBBufferSlot].InitAsUnorderedAccessView(AABBBufferRegister);
        parameters[InputElementSlot].InitAsUnorderedAccessView(ElementBufferRegister);
        parameters[GlobalDescriptorHeap].InitAsDescriptorTable(1, &globalDescriptorHeapRange);
        parameters[ConstantsSlot].InitAsConstants(SizeOfInUint32(InputConstants), ConstantsRegister);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);
        
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTreeletReorder), &m_pPSO);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pClearBuffer), &m_pClearBufferPSO);
    }

    void TreeletReorder::Optimize(
        ID3D12GraphicsCommandList *pCommandList,
        UINT numElements,
        D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS triangleCountBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS aabbBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS inputElementBuffer,
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlag)
    {
        if (numElements == 0) return;

        InputConstants constants;
        constants.NumberOfElements = numElements;
        constants.MinTrianglesPerTreelet = 7;

        pCommandList->SetComputeRootSignature(m_pRootSignature);
        pCommandList->SetComputeRootUnorderedAccessView(HierarchyBufferSlot, hierarchyBuffer);
        pCommandList->SetComputeRootUnorderedAccessView(TriangleCountBufferSlot, triangleCountBuffer);
        pCommandList->SetComputeRootUnorderedAccessView(AABBBufferSlot, aabbBuffer);
        pCommandList->SetComputeRootUnorderedAccessView(InputElementSlot, inputElementBuffer);
        if (globalDescriptorHeap.ptr)
        {
            pCommandList->SetComputeRootDescriptorTable(GlobalDescriptorHeap, globalDescriptorHeap);
        }

        bool bPrioritizeTrace = buildFlag & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        bool bPrioritizeBuild = buildFlag & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
        UINT numOptimizationPasses = bPrioritizeTrace ? 3 : bPrioritizeBuild  ? 0 : 1;

        for (UINT i = 0; i < numOptimizationPasses; i++)
        {
            pCommandList->SetComputeRoot32BitConstants(ConstantsSlot, SizeOfInUint32(InputConstants), &constants, 0);
            const UINT dispatchWidth = DivideAndRoundUp<UINT>(numElements, THREAD_GROUP_1D_WIDTH);

            pCommandList->SetPipelineState(m_pClearBufferPSO);
            pCommandList->Dispatch(dispatchWidth, 1, 1);
            auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
            pCommandList->ResourceBarrier(1, &uavBarrier);

            pCommandList->SetPipelineState(m_pPSO);
            pCommandList->Dispatch(dispatchWidth, 1, 1);
            pCommandList->ResourceBarrier(1, &uavBarrier);

            constants.MinTrianglesPerTreelet *= 2;
        }
    }

    UINT TreeletReorder::RequiredSizeForAABBBuffer(UINT numElements)
    {
        return (numElements + (numElements - 1)) * sizeof(AABB);
    }

}
