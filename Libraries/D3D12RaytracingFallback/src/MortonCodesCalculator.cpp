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
#include "CalculateMortonCodesBindings.h"
#include "CompiledShaders/CalculateMortonCodesForPrimitives.h"
#include "CompiledShaders/CalculateMortonCodesForAABBs.h"

namespace FallbackLayer
{
    MortonCodesCalculator::MortonCodesCalculator(ID3D12Device *pDevice, UINT nodeMask)
    {
        CD3DX12_ROOT_PARAMETER1 parameters[RootParameterSlot::NumParameters];
        parameters[InputElementsList].InitAsUnorderedAccessView(MortonCodeCalculatorInputBufferRegister);
        parameters[InputConstants].InitAsConstants(SizeOfInUint32(MortonCodeCalculatorConstants), MortonCodeCalculatorConstantsRegister);
        parameters[SceneAABB].InitAsUnorderedAccessView(MortonCodeCalculatorSceneAABBRegister);
        parameters[OutputIndices].InitAsUnorderedAccessView(MortonCodeCalculatorCalculatorOutputIndices);
        parameters[OutputMortonCodes].InitAsUnorderedAccessView(MortonCodeCalculatorCalculatorOutputMortonCodes);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pCalculateMortonCodesForPrimitives), &m_pCalcuateMortonCodesForPrimitivesPSO);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pCalculateMortonCodesForAABBs), &m_pCalcuateMortonCodesForAABBsPSO);
    }


    void MortonCodesCalculator::CalculateMortonCodes(ID3D12GraphicsCommandList *pCommandList, SceneType sceneType, D3D12_GPU_VIRTUAL_ADDRESS elementsBuffer, UINT numElements, D3D12_GPU_VIRTUAL_ADDRESS sceneAABB, D3D12_GPU_VIRTUAL_ADDRESS outputIndices, D3D12_GPU_VIRTUAL_ADDRESS outputMortonCodes)
    {
        if (numElements == 0) return;

        pCommandList->SetComputeRootSignature(m_pRootSignature);
        switch (sceneType)
        {
        case SceneType::Triangles:
            pCommandList->SetPipelineState(m_pCalcuateMortonCodesForPrimitivesPSO);
            break;
        case SceneType::BottomLevelBVHs:
            pCommandList->SetPipelineState(m_pCalcuateMortonCodesForAABBsPSO);
            break;
        default:
            assert(false);
        }

        MortonCodeCalculatorConstants constants{ numElements };

        pCommandList->SetComputeRootUnorderedAccessView(InputElementsList, elementsBuffer);
        pCommandList->SetComputeRootUnorderedAccessView(OutputIndices, outputIndices);
        pCommandList->SetComputeRootUnorderedAccessView(OutputMortonCodes, outputMortonCodes);
        pCommandList->SetComputeRootUnorderedAccessView(SceneAABB, sceneAABB);
        pCommandList->SetComputeRoot32BitConstants(InputConstants, SizeOfInUint32(constants), &constants, 0);

        const UINT dispatchWidth = DivideAndRoundUp<UINT>(numElements, THREAD_GROUP_1D_WIDTH);
        pCommandList->Dispatch(dispatchWidth, 1, 1);

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
        pCommandList->ResourceBarrier(1, &uavBarrier);
    }

}
