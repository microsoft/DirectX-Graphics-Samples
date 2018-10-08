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
#pragma once
namespace FallbackLayer
{
    class SceneAABBCalculator
    {
    public:
        SceneAABBCalculator(ID3D12Device *pDevice, UINT nodeMask);
        void CalculateSceneAABB(ID3D12GraphicsCommandList *pCommandList, SceneType sceneType, D3D12_GPU_VIRTUAL_ADDRESS inputBuffer, UINT numElements, D3D12_GPU_VIRTUAL_ADDRESS scratchBuffer, D3D12_GPU_VIRTUAL_ADDRESS outputAABB);
        static UINT ScratchBufferSizeNeeded(UINT numElements);

    private:
        enum RootParameterSlot
        {
            InputBuffer = 0,
            InputConstants,
            InputAABBBuffer,
            OutputBuffer,
            NumParameters
        };

        static UINT GetNumAABBsOutputFromPass(UINT numElements);

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pCalculateSceneAABBFromPrimitives;
        CComPtr<ID3D12PipelineState> m_pCalculateSceneAABBFromBVHs;
        CComPtr<ID3D12PipelineState> m_pCalculateSceneAABBFromAABBs;
    };
}
