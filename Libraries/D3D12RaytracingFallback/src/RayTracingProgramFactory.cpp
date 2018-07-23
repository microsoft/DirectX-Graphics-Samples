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

namespace FallbackLayer
{

    ITraversalShaderBuilder *RaytracingProgramFactory::NewTraversalShaderBuilder(AccelerationStructureLayoutType type)
    {
        switch (type)
        {
        case BVH2:
            return new BVHTraversalShaderBuilder(m_DxilShaderPatcher);
        default:
            ThrowInternalFailure(E_INVALIDARG);
            return nullptr;
        }
    }

    IRaytracingProgram *RaytracingProgramFactory::NewRaytracingProgram(ProgramTypes programType, const StateObjectCollection &stateObjectCollection)
    {
        switch (programType)
        {
        case RaytracingProgramFactory::UberShader:
                return new UberShaderRaytracingProgram(m_pDevice, m_DxilShaderPatcher, stateObjectCollection);
            default:
                ThrowInternalFailure(E_INVALIDARG);
                return nullptr;
        }
    }

    IRaytracingProgram *RaytracingProgramFactory::GetRaytracingProgram(
        const StateObjectCollection &stateObjectCollection)
    {
        ProgramTypes programType = DetermineBestProgram(stateObjectCollection);
        
        TraversalShader traversalShader;
        m_spTraversalShaderBuilder->Compile(stateObjectCollection.IsUsingAnyHit || stateObjectCollection.IsUsingIntersection, traversalShader);

        memcpy((void *)&stateObjectCollection.m_traversalShader, &traversalShader.m_TraversalShaderDxilLib, sizeof(stateObjectCollection.m_traversalShader));


        return NewRaytracingProgram(programType, stateObjectCollection);
    }

    RaytracingProgramFactory::RaytracingProgramFactory(ID3D12Device *pDevice) : m_pDevice(pDevice)
    {
        m_spTraversalShaderBuilder.reset(NewTraversalShaderBuilder(m_DefaultAccelerationStructureLayoutType));
    }

    RaytracingProgramFactory::ProgramTypes RaytracingProgramFactory::DetermineBestProgram(
        const StateObjectCollection &stateObjectCollection)
    {
        UNREFERENCED_PARAMETER(stateObjectCollection);
        return UberShader;
    }

    void StateObjectCollection::CombineCollection(const StateObjectCollection &collection)
    {
        if (collection.m_nodeMask)
        {
            if (m_nodeMask) ThrowFailure(E_INVALIDARG, L"Redefinition of nodemask in State Object");
            m_nodeMask = collection.m_nodeMask;
        }

        if (collection.m_pipelineStackSize)
        {
            if (m_pipelineStackSize) ThrowFailure(E_INVALIDARG, L"Redefinition of Pipeline Stack Size in State Object");
            m_pipelineStackSize = collection.m_pipelineStackSize;
        }

        if (collection.m_pGlobalRootSignature)
        {
            if (m_pGlobalRootSignature) ThrowFailure(E_INVALIDARG, L"Redefinition of the root signature in State Object");
            m_pGlobalRootSignature = collection.m_pGlobalRootSignature;
        }

        static const D3D12_RAYTRACING_PIPELINE_CONFIG nullConfig = {};
        if (memcmp(&collection.m_config, &nullConfig, sizeof(nullConfig)) != 0)
        {
            if (memcmp(&m_config, &nullConfig, sizeof(nullConfig)) != 0)
            {
                ThrowFailure(E_INVALIDARG, L"Redefinition of the raytracing pipeline config in State Object");
            }

            memcpy(&m_config, &nullConfig, sizeof(m_config));
        }

        for (auto &library : collection.m_dxilLibraries)
        {
            m_dxilLibraries.push_back(library);
        }

        for (auto &hitGroup : collection.m_hitGroups)
        {
            m_hitGroups[hitGroup.first] = hitGroup.second;
        }

        for (auto &shaderAssociation : collection.m_shaderAssociations)
        {
            m_shaderAssociations[shaderAssociation.first] = shaderAssociation.second;
        }

        if (collection.IsUsingAnyHit)
        {
            IsUsingAnyHit = true;
        }

        if (collection.IsUsingIntersection)
        {
            IsUsingIntersection = true;
        }
        m_maxAttributeSizeInBytes = (UINT)std::max(m_maxAttributeSizeInBytes, collection.m_maxAttributeSizeInBytes);
    }

}
