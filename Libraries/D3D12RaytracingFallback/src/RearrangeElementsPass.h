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
            UINT numTriangles,
            D3D12_GPU_VIRTUAL_ADDRESS inputElements,
            D3D12_GPU_VIRTUAL_ADDRESS inputMetadataBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS indexBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS outputTriangles,
            D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS outputIndexBuffer
        );

    private:
        enum RootParameterSlot
        {
            InputElements= 0,
            InputMetadata,
            IndexBuffer,
            OutputElements,
            OutputMetadata,
            OutputIndexBuffer,
            InputRootConstants,
            NumParameters
        };

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pRearrangeTrianglesPSO;
        CComPtr<ID3D12PipelineState> m_pRearrangeBVHsPSO;
    };

}