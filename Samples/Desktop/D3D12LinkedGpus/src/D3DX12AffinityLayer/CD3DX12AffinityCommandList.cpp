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
#include "Utils.h"

CD3DX12AffinityCommandList::CD3DX12AffinityCommandList(CD3DX12AffinityDevice* device, ID3D12CommandList** commandLists, UINT Count)
    : CD3DX12AffinityDeviceChild(device, reinterpret_cast<ID3D12DeviceChild**>(commandLists), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mCommandLists[i] = commandLists[i];
        }
        else
        {
            mCommandLists[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"CommandList";
#endif
}

D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE CD3DX12AffinityCommandList::GetType(UINT AffinityIndex)
{
    return mCommandLists[AffinityIndex]->GetType();
}

BOOL STDMETHODCALLTYPE CD3DX12AffinityCommandList::AssertResourceState(
    CD3DX12AffinityResource* pResource,
    UINT Subresource,
    UINT State)
{
    UINT i = GetActiveNodeIndex();
    ID3D12CommandList* CommandList = mCommandLists[i];
    ID3D12DebugCommandList* DebugCommandList = nullptr;

    HRESULT const hr = CommandList->QueryInterface(IID_PPV_ARGS(&DebugCommandList));
    if (S_OK != hr)
    {
        return false;
    }

    if (false == DebugCommandList->AssertResourceState(pResource->mResources[i], Subresource, State))
    {
        return false;
    }

    return true;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityCommandList::SetFeatureMask(
    D3D12_DEBUG_FEATURE Mask)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12CommandList* CommandList = mCommandLists[i];
            ID3D12DebugCommandList* DebugCommandList = nullptr;

            HRESULT hr = CommandList->QueryInterface(IID_PPV_ARGS(&DebugCommandList));
            if (S_OK != hr)
            {
                return hr;
            }

            hr = DebugCommandList->SetFeatureMask(Mask);

            if (S_OK != hr)
            {
                return hr;
            }
        }
    }

    return S_OK;
}

ID3D12CommandList* CD3DX12AffinityCommandList::GetChildObject(UINT AffinityIndex)
{
    return mCommandLists[AffinityIndex];
}

bool CD3DX12AffinityCommandList::IsReady()
{
    return mPendingBroadcast == 0;
}

void CD3DX12AffinityCommandList::RegisterPendingBroadcast()
{
    std::lock_guard<std::mutex> lock(mMutexPendingBroadcast);
    mPendingBroadcast++;
}

void CD3DX12AffinityCommandList::UnRegisterPendingBroadcast()
{
    std::lock_guard<std::mutex> lock(mMutexPendingBroadcast);
    mPendingBroadcast--;
}
