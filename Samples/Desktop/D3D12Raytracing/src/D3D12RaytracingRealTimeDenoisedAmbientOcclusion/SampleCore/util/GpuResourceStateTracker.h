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

#include "GpuResource.h"

class GpuResourceStateTracker
{
public:
    void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) { m_commandList = commandList; }
    void Reset();

    void TransitionResource(GpuResource* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
    void BeginResourceTransition(GpuResource* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
    void InsertUAVBarrier(GpuResource* Resource, bool FlushImmediate = false);
    void FlushResourceBarriers();

protected:

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    static const UINT c_MaxNumBarriers = 16;
    D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[c_MaxNumBarriers];
    UINT m_NumBarriersToFlush = 0;
};
