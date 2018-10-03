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
    class ShaderAssociations
    {
    public:
        ID3D12RootSignature *m_pRootSignature;
        D3D12_RAYTRACING_SHADER_CONFIG m_shaderConfig;
        std::wstring m_exportToRename;
        std::wstring m_mangledExport;
        std::wstring m_mangledExportToRename;
    };


    class StateObjectCollection
    {
    public:
        UINT64 m_pipelineStackSize = 0;
        UINT m_maxAttributeSizeInBytes = 0;
        UINT m_nodeMask = 0;
        std::vector<D3D12_DXIL_LIBRARY_DESC> m_dxilLibraries;
        std::vector<std::wstring> m_exportNames;
        std::vector<DxcExportDesc> m_exportDescs;
        std::unordered_map<std::wstring, D3D12_HIT_GROUP_DESC> m_hitGroups;
        ID3D12RootSignature *m_pGlobalRootSignature = nullptr;
        std::unordered_map<std::wstring, ShaderAssociations> m_shaderAssociations;
        D3D12_RAYTRACING_PIPELINE_CONFIG m_config = {};
        D3D12_DXIL_LIBRARY_DESC m_traversalShader = {};

        CDXILLibraryCache m_libraryCache;
        CStateObjectInfo m_stateObjectInfo;

        bool IsUsingAnyHit = false;
        bool IsUsingIntersection = false;
    };

    class RaytracingProgramFactory
    {
    public:
        RaytracingProgramFactory(ID3D12Device *pDevice);
        IRaytracingProgram *GetRaytracingProgram(
            const StateObjectCollection &stateObjectCollection);

    private:
        ID3D12Device *m_pDevice;

        enum ProgramTypes {
            UberShader = 0,
            NumBuilders
        };

        DxilShaderPatcher m_DxilShaderPatcher;

        ProgramTypes DetermineBestProgram(const StateObjectCollection &stateObjectCollection);
        IRaytracingProgram *NewRaytracingProgram(ProgramTypes programTypes, const StateObjectCollection &stateObjectCollection);
        ITraversalShaderBuilder *NewTraversalShaderBuilder(AccelerationStructureLayoutType type);

        const AccelerationStructureLayoutType m_DefaultAccelerationStructureLayoutType = BVH2;
        std::unique_ptr<ITraversalShaderBuilder> m_spTraversalShaderBuilder;
    };
}
