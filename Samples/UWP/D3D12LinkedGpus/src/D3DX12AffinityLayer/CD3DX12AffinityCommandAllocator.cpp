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

#include "d3dx12affinity.h"

CD3DX12AffinityCommandAllocator::CD3DX12AffinityCommandAllocator(CD3DX12AffinityDevice* device, ID3D12CommandAllocator** commandAllocators, UINT Count, bool UseDeviceActiveMaskOnReset)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(commandAllocators), Count)
    , mUseDeviceActiveMaskOnReset(UseDeviceActiveMaskOnReset)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mCommandAllocators[i] = commandAllocators[i];
        }
        else
        {
            mCommandAllocators[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"CommandAllocator";
#endif

#if !ALWAYS_RESET_ALL_COMMAND_LISTS
    if (UseDeviceActiveMaskOnReset)
    {
        SetAffinity(1 << GetActiveNodeIndex());
    }
#endif
}

HRESULT CD3DX12AffinityCommandAllocator::Reset(void)
{
#if !ALWAYS_RESET_ALL_COMMAND_LISTS
    if (mUseDeviceActiveMaskOnReset)
    {
        SetAffinity(1 << GetActiveNodeIndex());
    }
#endif

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12CommandAllocator* Allocator = mCommandAllocators[i];
            HRESULT const hr = Allocator->Reset();

            if (S_OK != hr)
            {
                return hr;
            }
        }
    }

    return S_OK;
}

ID3D12CommandAllocator* CD3DX12AffinityCommandAllocator::GetChildObject(UINT AffinityIndex)
{
    return mCommandAllocators[AffinityIndex];
}
