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
#include "LoadInstancesBindings.h"
#include "CompiledShaders/TopLevelLoadAABBsFromArrayOfPointers.h"
#include "CompiledShaders/TopLevelLoadAABBsFromArrayOfInstances.h"

namespace FallbackLayer
{
    LoadInstancesPass::LoadInstancesPass(ID3D12Device *pDevice, UINT nodeMask)
    {
        D3D12_DESCRIPTOR_RANGE1 globalDescriptorHeapRange[2];
        globalDescriptorHeapRange[0] = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, (UINT)-1, DescriptorHeapBufferRegister, DescriptorHeapBufferRegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, 0);
        globalDescriptorHeapRange[1] = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)-1, DescriptorHeapSRVBufferRegister, DescriptorHeapSRVBufferRegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, 0);

        CD3DX12_ROOT_PARAMETER1 parameters[RootParameterSlot::NumRootParameters];
        parameters[OutputBVHRootUAV].InitAsUnorderedAccessView(OutputBVHRegister);
        parameters[InstanceDescsSRV].InitAsShaderResourceView(InstanceDescsRegister);
        parameters[GlobalDescriptorHeap].InitAsDescriptorTable(ARRAYSIZE(globalDescriptorHeapRange), globalDescriptorHeapRange);
        parameters[CachedSortBuffer].InitAsUnorderedAccessView(CachedSortBufferRegister);
        parameters[Constants].InitAsConstants(SizeOfInUint32(LoadInstancesConstants), LoadInstancesConstantsRegister);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTopLevelLoadAABBsFromArrayOfPointers), &m_pLoadAABBsFromArrayOfPointersPSO);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pTopLevelLoadAABBsFromArrayOfInstances), &m_pLoadAABBsFromArrayOfInstancesPSO);
    }

    void LoadInstancesPass::LoadInstances(ID3D12GraphicsCommandList *pCommandList, 
        D3D12_GPU_VIRTUAL_ADDRESS outputBVH, 
        D3D12_GPU_VIRTUAL_ADDRESS instancesDesc, 
        D3D12_ELEMENTS_LAYOUT instanceDescLayout, 
        UINT numElements, 
        D3D12_GPU_DESCRIPTOR_HANDLE descriptorHeapBase,
        D3D12_GPU_VIRTUAL_ADDRESS cachedSortBuffer)
    {
        if (numElements == 0) return;

        const bool performUpdate = cachedSortBuffer != 0;

        pCommandList->SetComputeRootSignature(m_pRootSignature);
        ID3D12PipelineState *pLoadAABBPSO = nullptr;
        switch (instanceDescLayout)
        {
        case D3D12_ELEMENTS_LAYOUT_ARRAY:
            pLoadAABBPSO = m_pLoadAABBsFromArrayOfInstancesPSO;
            break;
        case D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS:
            pLoadAABBPSO = m_pLoadAABBsFromArrayOfPointersPSO;
            break;
        default:
            ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_ELEMENTS_LAYOUT provided");
        }
        pCommandList->SetPipelineState(pLoadAABBPSO);

        LoadInstancesConstants constants = { numElements, (UINT) performUpdate };
        pCommandList->SetComputeRoot32BitConstants(Constants, SizeOfInUint32(LoadInstancesConstants), &constants, 0);
        pCommandList->SetComputeRootDescriptorTable(GlobalDescriptorHeap, descriptorHeapBase);
        pCommandList->SetComputeRootShaderResourceView(InstanceDescsSRV, instancesDesc);
        pCommandList->SetComputeRootUnorderedAccessView(OutputBVHRootUAV, outputBVH);

        if (performUpdate)
        {
            pCommandList->SetComputeRootUnorderedAccessView(CachedSortBuffer, cachedSortBuffer);
        }

        const UINT dispatchWidth = DivideAndRoundUp<UINT>(numElements, THREAD_GROUP_1D_WIDTH);
        pCommandList->Dispatch(dispatchWidth, 1, 1);

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
        pCommandList->ResourceBarrier(1, &uavBarrier);
    }


}

