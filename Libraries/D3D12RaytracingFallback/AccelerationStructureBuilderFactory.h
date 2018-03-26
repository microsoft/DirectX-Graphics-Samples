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
    class AccelerationStructureBuilderFactory
    {
    public:
        AccelerationStructureBuilderFactory(ID3D12Device *pDevice, UINT nodeMask) : m_pDevice(pDevice), m_nodeMask(nodeMask) {}

        IAccelerationStructureBuilder &GetAccelerationStructureBuilder();

    private:
        enum BuilderType {
            GpuBvh2BuilderType = 0,
            NumBuilders
        };

        BuilderType DetermineBestBuilder();
        IAccelerationStructureBuilder &GetBuilder(BuilderType builderType);
        IAccelerationStructureBuilder *CreateBuilder(BuilderType type);

        std::unique_ptr<IAccelerationStructureBuilder> m_spBuilders[NumBuilders];

        UINT m_nodeMask;
        ID3D12Device *m_pDevice; 
    };
}
