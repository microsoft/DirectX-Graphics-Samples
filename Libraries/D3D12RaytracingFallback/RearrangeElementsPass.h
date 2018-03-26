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
    class RearrangeElementsPass
    {
    public:
        RearrangeElementsPass(ID3D12Device *pDevice, UINT nodeMask);

        void Rearrange(
            ID3D12GraphicsCommandList *pCommandList,
            SceneType sceneType,
            D3D12_GPU_VIRTUAL_ADDRESS inputBuffer,
            UINT numElements,
            D3D12_GPU_VIRTUAL_ADDRESS indexBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS outputBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS inputMetadataBuffer = 0,
            D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer = 0
        );

    private:
        enum RootParameterSlot
        {
            InputElements= 0,
            IndexBuffer,
            NumTrianglesConstant,
            OutputElements,
            OutputMetadata,
            InputMetadata,
            NumParameters
        };

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pRearrangeTrianglesPSO;
        CComPtr<ID3D12PipelineState> m_pRearrangeBVHsPSO;
    };

}
