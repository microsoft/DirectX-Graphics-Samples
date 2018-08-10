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
    class UberShaderRaytracingProgram : public IRaytracingProgram
    {
    public:
        UberShaderRaytracingProgram(ID3D12Device *m_pDevice, DxilShaderPatcher &dxilShaderPatcher, const StateObjectCollection &stateObjectCollection);
        virtual ~UberShaderRaytracingProgram() {}
        virtual void DispatchRays(
            ID3D12GraphicsCommandList *pCommandList, 
            ID3D12DescriptorHeap *pSrvCbvUavDescriptorHeap,
            ID3D12DescriptorHeap *pSamplerDescriptorHeap,
            const std::unordered_map<UINT, WRAPPED_GPU_POINTER> &boundAccelerationStructures,
            const D3D12_DISPATCH_RAYS_DESC &desc);

        virtual ShaderIdentifier *GetShaderIdentifier(LPCWSTR pExportName);
        virtual UINT64 GetShaderStackSize(LPCWSTR pExportName);

        virtual void SetPredispatchCallback(std::function<void(ID3D12GraphicsCommandList *, UINT)> callback)
        {
            m_pPredispatchCallback = callback;
        }
        std::function<void(ID3D12GraphicsCommandList *, UINT)> m_pPredispatchCallback;
    private:
        StateIdentifier GetStateIdentfier(LPCWSTR pExportName);

        DxilShaderPatcher &m_DxilShaderPatcher;
        struct ShaderData
        {
          ShaderIdentifier stateIdentifier;
          UINT stackSize;
        };

        ShaderData *GetShaderData(LPCWSTR pExportName);

        std::unordered_map<std::wstring, ShaderData> m_ExportNameToShaderData;
        CComPtr<ID3D12PipelineState> m_pRayTracePSO;
        UINT m_patchRootSignatureParameterStart;

        UINT m_largestNonRayGenStackSize = 0;
        UINT m_largestRayGenStackSize = 0;

        void DumpReproInformation(UINT maxAttributeSizeInBytes, std::vector<DxilLibraryInfo> librariesInfo, std::vector<LPCWSTR> exportNames);
    };
}
