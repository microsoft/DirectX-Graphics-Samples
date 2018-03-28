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

    IAccelerationStructureBuilder &AccelerationStructureBuilderFactory::GetAccelerationStructureBuilder()
    {
        const BuilderType builderType = DetermineBestBuilder();
        return GetBuilder(builderType);
    }

    AccelerationStructureBuilderFactory::BuilderType AccelerationStructureBuilderFactory::DetermineBestBuilder()
    {
        return GpuBvh2BuilderType;
    }

    IAccelerationStructureBuilder &AccelerationStructureBuilderFactory::GetBuilder(BuilderType builderType)
    {
        assert(builderType < NumBuilders);
        if (!m_spBuilders[builderType])
        {
            m_spBuilders[builderType] = std::unique_ptr<IAccelerationStructureBuilder>(CreateBuilder(builderType));
        }
        return *m_spBuilders[builderType];
    }


    IAccelerationStructureBuilder *AccelerationStructureBuilderFactory::CreateBuilder(BuilderType type)
    {
        switch (type)
        {
        case GpuBvh2BuilderType:
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS1 waveData;
            ThrowInternalFailure(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &waveData, sizeof(waveData)));
            return new GpuBvh2Builder(m_pDevice, waveData.TotalLaneCount, m_nodeMask);
        }
        default:
            return nullptr;
        }
    }
}
