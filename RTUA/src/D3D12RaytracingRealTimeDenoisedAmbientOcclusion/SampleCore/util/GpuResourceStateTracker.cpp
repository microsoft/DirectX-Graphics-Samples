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

#include "DXSampleHelper.h"
#include "Utility.h"

void GpuResourceStateTracker::Reset()
{ 
    m_commandList.Reset(); 
    m_NumBarriersToFlush = 0; 
}

void GpuResourceStateTracker::TransitionResource(GpuResource* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
    D3D12_RESOURCE_STATES OldState = Resource->m_UsageState;

    if (m_NumBarriersToFlush == c_MaxNumBarriers)
        FlushResourceBarriers();

    if (OldState != NewState)
    {
        D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = Resource->GetResource();
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;

        // Insert UAV barrier on SRV<->UAV transitions.
        if (OldState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS || NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            InsertUAVBarrier(Resource);

        // Check to see if we already started the transition
        if (NewState == Resource->m_TransitioningState)
        {
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            Resource->m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
        }
        else
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        Resource->m_UsageState = NewState;
    }
    else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        InsertUAVBarrier(Resource);

    if (FlushImmediate)
        FlushResourceBarriers();
}

void GpuResourceStateTracker::BeginResourceTransition(GpuResource* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
    if (m_NumBarriersToFlush == c_MaxNumBarriers)
        FlushResourceBarriers();

    // If it's already transitioning, finish that transition
    if (Resource->m_TransitioningState != (D3D12_RESOURCE_STATES)-1)
        TransitionResource(Resource, Resource->m_TransitioningState);

    D3D12_RESOURCE_STATES OldState = Resource->m_UsageState;

    if (OldState != NewState)
    {
        D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = Resource->GetResource();
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;
        BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

        Resource->m_TransitioningState = NewState;
    }

    if (FlushImmediate || m_NumBarriersToFlush == c_MaxNumBarriers)
        FlushResourceBarriers();
}

void GpuResourceStateTracker::InsertUAVBarrier(GpuResource* Resource, bool FlushImmediate)
{
    if (m_NumBarriersToFlush == c_MaxNumBarriers)
        FlushResourceBarriers();

    D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

    BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    BarrierDesc.UAV.pResource = Resource->GetResource();

    if (FlushImmediate)
        FlushResourceBarriers();
}

void GpuResourceStateTracker::FlushResourceBarriers()
{
    if (m_NumBarriersToFlush > 0)
    {
        m_commandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
        m_NumBarriersToFlush = 0;
    }
}