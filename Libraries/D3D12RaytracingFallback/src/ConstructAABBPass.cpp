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
#include "ConstructAABBBindings.h"
#include "CompiledShaders/TopLevelPrepareForComputeAABBs.h"
#include "CompiledShaders/TopLevelComputeAABBs.h"
#include "CompiledShaders/BottomLevelComputeAABBs.h"
#include "CompiledShaders/BottomLevelPrepareForComputeAABBs.h"

namespace FallbackLayer
{
    ConstructAABBPass::ConstructAABBPass(ID3D12Device *pDevice, UINT nodeMask)
    {
        D3D12_DESCRIPTOR_RANGE1 globalDescriptorHeapRange = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, (UINT)-1, GlobalDescriptorHeapRegister, GlobalDescriptorHeapRegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, 0);
        CD3DX12_ROOT_PARAMETER1 rootParameters[NumRootParameters];
        rootParameters[OutputBVHRootUAVParam].InitAsUnorderedAccessView(OutputBVHRegister);
        rootParameters[ScratchUAVParam].InitAsUnorderedAccessView(ScratchBufferRegister);
        rootParameters[HierarchyUAVParam].InitAsUnorderedAccessView(HierarchyBufferRegister);
        rootParameters[AABBParentBufferParam].InitAsUnorderedAccessView(AABBParentBufferRegister);
        rootParameters[ChildNodesProcessedCountBufferParam].InitAsUnorderedAccessView(ChildNodesProcessedBufferRegister);
        rootParameters[InputRootConstants].InitAsConstants(SizeOfInUint32(InputConstants), InputConstantsRegister);
        rootParameters[GlobalDescriptorHeap].InitAsDescriptorTable(1, &globalDescriptorHeapRange);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(rootParameters), rootParameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTopLevelComputeAABBs), &m_pComputeAABBs[Level::Top]);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTopLevelPrepareForComputeAABBs), &m_pPrepareForComputeAABBs[Level::Top]);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pBottomLevelComputeAABBs), &m_pComputeAABBs[Level::Bottom]);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pBottomLevelPrepareForComputeAABBs), &m_pPrepareForComputeAABBs[Level::Bottom]);
    }

    void ConstructAABBPass::ConstructAABB(ID3D12GraphicsCommandList *pCommandList,
        SceneType sceneType,
        D3D12_GPU_VIRTUAL_ADDRESS outputVH,
        D3D12_GPU_VIRTUAL_ADDRESS scratchBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS childNodesProcessedCountBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputAABBParentBuffer,
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap,
        const bool prepareUpdate,
        const bool performUpdate,
        UINT numElements)
    {
        bool isEmptyAccelerationStructure = numElements == 0;
        Level level = (sceneType == SceneType::Triangles) ? Level::Bottom : Level::Top;

        InputConstants constants = {};
        constants.NumberOfElements = numElements;
        constants.UpdateFlags = ((UINT) prepareUpdate) | (performUpdate << 1);

        pCommandList->SetComputeRootSignature(m_pRootSignature);
        pCommandList->SetComputeRoot32BitConstants(InputRootConstants, SizeOfInUint32(InputConstants), &constants, 0);
        pCommandList->SetComputeRootUnorderedAccessView(OutputBVHRootUAVParam, outputVH);
        if (!isEmptyAccelerationStructure)
        {
            pCommandList->SetComputeRootUnorderedAccessView(ScratchUAVParam, scratchBuffer);
            pCommandList->SetComputeRootUnorderedAccessView(ChildNodesProcessedCountBufferParam, childNodesProcessedCountBuffer);
            pCommandList->SetComputeRootUnorderedAccessView(HierarchyUAVParam, hierarchyBuffer);
        }

        if (prepareUpdate || performUpdate)
        {
            pCommandList->SetComputeRootUnorderedAccessView(AABBParentBufferParam, outputAABBParentBuffer);
        }

        if (level == Top)
        {
            pCommandList->SetComputeRootDescriptorTable(GlobalDescriptorHeap, globalDescriptorHeap);
        }

        // Only given the GPU VA not the resource itself so need to resort to doing an overarching UAV barrier
        const UINT dispatchWidth = isEmptyAccelerationStructure ? 1 : DivideAndRoundUp<UINT>(numElements, THREAD_GROUP_1D_WIDTH);
        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);

        pCommandList->SetPipelineState(m_pPrepareForComputeAABBs[level]);
        pCommandList->Dispatch(dispatchWidth, 1, 1);
        pCommandList->ResourceBarrier(1, &uavBarrier);

        if (isEmptyAccelerationStructure) return;

        // Build the AABBs from the bottom-up
        pCommandList->SetPipelineState(m_pComputeAABBs[level]);
        pCommandList->Dispatch(dispatchWidth, 1, 1);
        pCommandList->ResourceBarrier(1, &uavBarrier);
    }

}
