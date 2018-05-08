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
#include "CalculateSceneAABBBindings.h"
#include "CompiledShaders/CalculateSceneAABBFromAABBs.h"
#include "CompiledShaders/CalculateSceneAABBFromPrimitives.h"
#include "CompiledShaders/CalculateSceneAABBFromBVHs.h"

namespace FallbackLayer
{
    SceneAABBCalculator::SceneAABBCalculator(ID3D12Device *pDevice, UINT nodeMask)
    {
        CD3DX12_ROOT_PARAMETER1 parameters[RootParameterSlot::NumParameters];
        parameters[InputBuffer].InitAsUnorderedAccessView(SceneAABBCalculatorInputBufferRegister);
        parameters[InputConstants].InitAsConstants(SizeOfInUint32(SceneAABBCalculatorConstants), SceneAABBCalculatorConstantsRegister);
        parameters[InputAABBBuffer].InitAsUnorderedAccessView(SceneAABBCalculatorAABBBuffer);
        parameters[OutputBuffer].InitAsUnorderedAccessView(SceneAABBCalculatorOutputBuffer);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pCalculateSceneAABBFromAABBs), &m_pCalculateSceneAABBFromAABBs);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pCalculateSceneAABBFromPrimitives), &m_pCalculateSceneAABBFromPrimitives);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pCalculateSceneAABBFromBVHs), &m_pCalculateSceneAABBFromBVHs);
    }


    void SceneAABBCalculator::CalculateSceneAABB(ID3D12GraphicsCommandList *pCommandList, SceneType sceneType, D3D12_GPU_VIRTUAL_ADDRESS inputBuffer, UINT numElements, D3D12_GPU_VIRTUAL_ADDRESS scratchBuffer, D3D12_GPU_VIRTUAL_ADDRESS outputAABB)
    {
        if (numElements == 0) return;

        pCommandList->SetComputeRootSignature(m_pRootSignature);
        switch (sceneType)
        {
        case SceneType::Triangles:
            pCommandList->SetPipelineState(m_pCalculateSceneAABBFromPrimitives);
            break;
        case SceneType::BottomLevelBVHs:
            pCommandList->SetPipelineState(m_pCalculateSceneAABBFromBVHs);
            break;
        default:
            assert(false);
        }

        SceneAABBCalculatorConstants constants = {};
        pCommandList->SetComputeRootUnorderedAccessView(InputBuffer, inputBuffer);

        D3D12_GPU_VIRTUAL_ADDRESS scratchBuffers[2];
        scratchBuffers[0] = scratchBuffer;
        scratchBuffers[1] = scratchBuffer + GetNumAABBsOutputFromPass(numElements) * sizeof(AABB);

        UINT outputScratchBufferIndex = 0;
        UINT inputScratchBufferIndex = 1;

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
        UINT threadsNeeded;
        UINT elementsToProcess = numElements;
        for (; elementsToProcess > 1; elementsToProcess = threadsNeeded)
        {
            threadsNeeded = GetNumAABBsOutputFromPass(elementsToProcess);

            const bool bFirstPass = (elementsToProcess == numElements);
            const bool bLastPass = (threadsNeeded == 1);

            constants.NumberOfElements = elementsToProcess;
            pCommandList->SetComputeRoot32BitConstants(InputConstants, SizeOfInUint32(constants), &constants, 0);
            pCommandList->SetComputeRootUnorderedAccessView(OutputBuffer, bLastPass ? outputAABB : scratchBuffers[outputScratchBufferIndex]);
            if (!bFirstPass)
            {
                pCommandList->SetComputeRootUnorderedAccessView(InputAABBBuffer, scratchBuffers[inputScratchBufferIndex]);
            }
            const UINT dispatchWidth = DivideAndRoundUp<UINT>(threadsNeeded, THREAD_GROUP_1D_WIDTH);
            pCommandList->Dispatch(dispatchWidth, 1, 1);
            pCommandList->ResourceBarrier(1, &uavBarrier);

            std::swap(outputScratchBufferIndex, inputScratchBufferIndex);
            if (bFirstPass)
            {
                pCommandList->SetPipelineState(m_pCalculateSceneAABBFromAABBs);
            }
        }
    }

    UINT SceneAABBCalculator::GetNumAABBsOutputFromPass(UINT numElements)
    {
        return  DivideAndRoundUp<UINT>(numElements, ElementsSummedPerThread);
    }


    UINT SceneAABBCalculator::ScratchBufferSizeNeeded(UINT numTriangles)
    {
        if (numTriangles == 0) return 0;

        // Data is ping-ponged between the upper and lower part of the scratch buffer so
        // only need to sum the first 2 biggest passes
        auto numberOfAABBsOutput = GetNumAABBsOutputFromPass(numTriangles);
        if (numberOfAABBsOutput > 1)
        {
            numberOfAABBsOutput += GetNumAABBsOutputFromPass(numberOfAABBsOutput);
        }
        return numberOfAABBsOutput * sizeof(AABB);
    }
}
