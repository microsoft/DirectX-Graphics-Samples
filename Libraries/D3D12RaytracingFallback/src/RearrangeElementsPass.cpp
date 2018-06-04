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
#include "RearrangeTrianglesBindings.h"
#include "CompiledShaders/RearrangeTriangles.h"
#include "CompiledShaders/RearrangeBVHs.h"

namespace FallbackLayer
{
    RearrangeElementsPass::RearrangeElementsPass(ID3D12Device *pDevice, UINT nodeMask)
    {
        CD3DX12_ROOT_PARAMETER1 parameters[NumParameters];
        parameters[InputElements].InitAsUnorderedAccessView(InputElementBufferRegister);
        parameters[InputMetadata].InitAsUnorderedAccessView(InputMetadataBufferRegister);
        parameters[IndexBuffer].InitAsUnorderedAccessView(IndexBufferRegister);
        parameters[OutputElements].InitAsUnorderedAccessView(OutputElementBufferRegister);
        parameters[OutputMetadata].InitAsUnorderedAccessView(OutputMetadataBufferRegister);
        parameters[OutputIndexBuffer].InitAsUnorderedAccessView(OutputIndexBufferRegister);
        parameters[InputRootConstants].InitAsConstants(SizeOfInUint32(InputConstants), InputConstantsRegister);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pRearrangeTriangles), &m_pRearrangeTrianglesPSO);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pRearrangeBVHs), &m_pRearrangeBVHsPSO);
    }

    void RearrangeElementsPass::Rearrange(
        ID3D12GraphicsCommandList *pCommandList,
        SceneType sceneType,
        UINT numTriangles,
        D3D12_GPU_VIRTUAL_ADDRESS inputElements,
        D3D12_GPU_VIRTUAL_ADDRESS inputMetadataBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputTriangles,
        D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputIndexBuffer)
    {
        if (numTriangles == 0) return;

        bool updatesAllowed = outputIndexBuffer != 0;
        InputConstants constants = {};
        constants.NumberOfTriangles = numTriangles;
        constants.UpdatesAllowed = (UINT) (updatesAllowed);

        pCommandList->SetComputeRootSignature(m_pRootSignature);
        switch (sceneType)
        {
        case SceneType::Triangles:
            pCommandList->SetPipelineState(m_pRearrangeTrianglesPSO);
            break;
        case SceneType::BottomLevelBVHs:
            pCommandList->SetPipelineState(m_pRearrangeBVHsPSO);
            break;
        default:
            assert(false);
        }

        pCommandList->SetComputeRoot32BitConstants(InputRootConstants, SizeOfInUint32(InputConstants), &constants, 0);

        pCommandList->SetComputeRootUnorderedAccessView(InputElements, inputElements);
        pCommandList->SetComputeRootUnorderedAccessView(IndexBuffer, indexBuffer);
        pCommandList->SetComputeRootUnorderedAccessView(OutputElements, outputTriangles);
        if (inputMetadataBuffer)
        {
            pCommandList->SetComputeRootUnorderedAccessView(InputMetadata, inputMetadataBuffer);
            pCommandList->SetComputeRootUnorderedAccessView(OutputMetadata, outputMetadataBuffer);
        }
        if (updatesAllowed)
        {
            pCommandList->SetComputeRootUnorderedAccessView(OutputIndexBuffer, outputIndexBuffer);
        }

        const UINT dispatchWidth = DivideAndRoundUp<UINT>(numTriangles, THREAD_GROUP_1D_WIDTH);
        pCommandList->Dispatch(dispatchWidth, 1, 1);

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
        pCommandList->ResourceBarrier(1, &uavBarrier);
    }

}