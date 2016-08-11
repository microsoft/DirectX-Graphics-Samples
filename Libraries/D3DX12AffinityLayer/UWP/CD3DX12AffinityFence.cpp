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

UINT64 STDMETHODCALLTYPE CD3DX12AffinityFence::GetCompletedValue(void)
{
    UINT64 Minimum = mFences[0]->GetCompletedValue();
    for (UINT i = 1; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {

            UINT64 CompletedValue = mFences[i]->GetCompletedValue();
            Minimum = min(CompletedValue, Minimum);
        }
    }

    return Minimum;
}

UINT64 CD3DX12AffinityFence::GetCompletedValue(UINT AffinityMask)
{
    UINT64 Minimum = 0 - 1ull;
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            UINT64 CompletedValue = mFences[i]->GetCompletedValue();
            Minimum = min(CompletedValue, Minimum);
        }
    }
    return Minimum;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityFence::SetEventOnCompletion(
    UINT64 Value,
    HANDLE hEvent)
{
    UINT i = GetActiveNodeIndex();
    ID3D12Fence* Fence = mFences[i];
    return Fence->SetEventOnCompletion(Value, hEvent);
}

HRESULT CD3DX12AffinityFence::SetEventOnCompletion(UINT64 Value, HANDLE hEvent, UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            return mFences[i]->SetEventOnCompletion(Value, hEvent);
        }
    }
    //should never get here, mask is incorrect
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityFence::WaitOnFenceCompletion(
    UINT64 Value)
{
    std::vector<HANDLE> Events;

    UINT EventCount = 0;
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12Fence* Fence = mFences[i];
            Events.push_back(0);
            Events[EventCount] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            HRESULT const hr = Fence->SetEventOnCompletion(Value, Events[EventCount]);

            if (hr != S_OK)
            {
                return hr;
            }

            ++EventCount;
        }
    }
    WaitForMultipleObjects((DWORD)EventCount, Events.data(), TRUE, INFINITE);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityFence::Signal(
    UINT64 Value)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12Fence* Fence = mFences[i];

            HRESULT const hr = Fence->Signal(Value);

            if (hr != S_OK)
            {
                return hr;
            }
        }
    }

    return S_OK;
}

HRESULT CD3DX12AffinityFence::Signal(UINT64 Value, UINT AffinityIndex)
{
    return mFences[AffinityIndex]->Signal(Value);
}

CD3DX12AffinityFence::CD3DX12AffinityFence(CD3DX12AffinityDevice* device, ID3D12Fence** fences, UINT Count)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(fences), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mFences[i] = fences[i];
        }
        else
        {
            mFences[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"Fence";
#endif
    mDevice = device;
}

CD3DX12AffinityFence::~CD3DX12AffinityFence()
{
}

ID3D12Fence* CD3DX12AffinityFence::GetChildObject(UINT AffinityIndex)
{
    return mFences[AffinityIndex];
}
