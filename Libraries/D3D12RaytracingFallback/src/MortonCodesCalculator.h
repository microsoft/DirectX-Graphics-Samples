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
    class MortonCodesCalculator
    {
    public:
        MortonCodesCalculator(ID3D12Device *pDevice, UINT nodeMask);
        void CalculateMortonCodes(ID3D12GraphicsCommandList *pCommandList, SceneType sceneType, D3D12_GPU_VIRTUAL_ADDRESS triangleBuffer, UINT numTriangles, D3D12_GPU_VIRTUAL_ADDRESS sceneAABB, D3D12_GPU_VIRTUAL_ADDRESS outputIndices, D3D12_GPU_VIRTUAL_ADDRESS outputMortonCodes);
    
    private:
        enum RootParameterSlot
        {
            InputElementsList = 0,
            InputConstants,
            SceneAABB,
            OutputMortonCodes,
            OutputIndices,
            NumParameters
        };

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pCalcuateMortonCodesForPrimitivesPSO;
        CComPtr<ID3D12PipelineState> m_pCalcuateMortonCodesForAABBsPSO;
    };
}
