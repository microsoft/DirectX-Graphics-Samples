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
}
