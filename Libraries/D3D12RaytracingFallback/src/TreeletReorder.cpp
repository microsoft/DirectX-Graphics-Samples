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
#include "CompiledShaders/ClearBuffer.h"
#include "CompiledShaders/TreeletReorder.h"
#include "CompiledShaders/TreeletReorderV2.h"
#include "CompiledShaders/TreeletComputeAABBs.h"
#include "CompiledShaders/TreeletReorderWave.h"
#include "CompiledShaders/TreeletReorderThread.h"
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
        parameters[BubbleBufferSlot].InitAsUnorderedAccessView(BubbleBufferRegister);
        parameters[ConstantsSlot].InitAsConstants(SizeOfInUint32(InputConstants), ConstantsRegister);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);
        
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTreeletReorder), &m_pPSO);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTreeletReorderV2), &m_pPSO_OPT);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTreeletReorderWave), &m_pPSO_OPT_PL);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTreeletReorderThread), &m_pPSO_OPT_T);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTreeletComputeAABBs), &m_pComputeAABBsPSO);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pClearBuffer), &m_pClearBufferPSO);
    }

#define FORCE_PL 1

    void TreeletReorder::Optimize(
        ID3D12GraphicsCommandList *pCommandList,
        UINT numElements,
        D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS triangleCountBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS aabbBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS inputElementBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS reorderBubbleBuffer,
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
        if (reorderBubbleBuffer != 0)
        {
            pCommandList->SetComputeRootUnorderedAccessView(BubbleBufferSlot, reorderBubbleBuffer);
        }

        bool bDefault = (buildFlag & (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)) == 0;
        bool bPrioritizeTrace = buildFlag & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        bool bPrioritizeBuild = buildFlag & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

        UINT numOptimizationPasses = 3;

        for (UINT i = 0; i < numOptimizationPasses; i++)
        {
            if (constants.MinTrianglesPerTreelet > numElements)
            {
                break;
            }

            pCommandList->SetComputeRoot32BitConstants(ConstantsSlot, SizeOfInUint32(InputConstants), &constants, 0);
            
            auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
            
            UINT numGroupsForElements = DivideAndRoundUp<UINT>(numElements, THREAD_GROUP_1D_WIDTH);

            pCommandList->SetPipelineState(m_pClearBufferPSO);
            pCommandList->Dispatch(numGroupsForElements, 1, 1);
            pCommandList->ResourceBarrier(1, &uavBarrier);

            if (bDefault)
            {
#if FORCE_PL
                pCommandList->SetPipelineState(m_pComputeAABBsPSO);
                pCommandList->Dispatch(numGroupsForElements, 1, 1);
                pCommandList->ResourceBarrier(1, &uavBarrier);
                
                pCommandList->SetPipelineState(m_pPSO_OPT_T);
                UINT numTreelets = (UINT) std::max(numElements / constants.MinTrianglesPerTreelet, 1u);
                uint threadsX = numTreelets % 65535;
                uint threadsY = ((numTreelets / 65535u) + 1) % 65535u;
                uint threadsZ = (((numTreelets / 65535u) + 1) / 65535u) + 1;   
                pCommandList->Dispatch(threadsX, threadsY, threadsZ);
                pCommandList->ResourceBarrier(1, &uavBarrier);
#else
                pCommandList->SetPipelineState(m_pPSO);
                pCommandList->Dispatch(numGroupsForElements, 1, 1);
                pCommandList->ResourceBarrier(1, &uavBarrier);
#endif
            } 
            else if (bPrioritizeTrace)
            {
                pCommandList->SetPipelineState(m_pComputeAABBsPSO);
                pCommandList->Dispatch(numGroupsForElements, 1, 1);
                pCommandList->ResourceBarrier(1, &uavBarrier);
                
                pCommandList->SetPipelineState(m_pPSO_OPT_PL);
                UINT numTreelets = (UINT) std::max(numElements / constants.MinTrianglesPerTreelet, 1u);
                pCommandList->Dispatch(numTreelets, 1, 1);
                pCommandList->ResourceBarrier(1, &uavBarrier);
            }
            else if (bPrioritizeBuild)
            {
                pCommandList->SetPipelineState(m_pPSO_OPT);
                pCommandList->Dispatch(numGroupsForElements, 1, 1);
                pCommandList->ResourceBarrier(1, &uavBarrier);
            }

            constants.MinTrianglesPerTreelet *= 2;
        }
    }

    UINT TreeletReorder::RequiredSizeForAABBBuffer(UINT numElements)
    {
        return (numElements + (numElements - 1)) * sizeof(AABB);
    }

    UINT TreeletReorder::RequiredSizeForBubbleBuffer(UINT numElements)
    {
        return (numElements / 7) + 1;
    }

}
