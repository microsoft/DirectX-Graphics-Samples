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
    class LoadPrimitivesPass
    {
    public:
        LoadPrimitivesPass(ID3D12Device *pDevice, UINT nodeMask);
        void LoadPrimitives(ID3D12GraphicsCommandList *pCommandList,
            const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &buildDesc,
            UINT totalTriangleCount,
            D3D12_GPU_VIRTUAL_ADDRESS outputTriangleBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS cachedSortBuffer);
    private:
        enum RootParameterSlot
        {
            OutputBuffer = 0,
            OutputMetadataBuffer,
            IndexBufferSRV,
            ElementBufferSRV,
            InputRootConstants,
            TransformsBuffer,
            CachedSortBuffer,
            NumRootParameters,
        };

        enum IndexBufferType
        {
            NoIndexBuffer,
            Index16Bit,
            Index32Bit,
            NumIndexBufferTyes
        };

        static IndexBufferType GetIndexBufferType(DXGI_FORMAT format);

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12PipelineState> m_pLoadTrianglesPSOs[NumIndexBufferTyes];
        CComPtr<ID3D12PipelineState> m_pLoadProceduralGeometryPSO;
    };
}
