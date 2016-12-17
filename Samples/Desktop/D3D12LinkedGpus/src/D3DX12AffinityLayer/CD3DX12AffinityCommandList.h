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

#include "Utils.h"
#include "CD3DX12AffinityDeviceChild.h"

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityCommandList : public CD3DX12AffinityDeviceChild
{
public:
    CD3DX12AffinityCommandList(CD3DX12AffinityDevice* device, ID3D12CommandList** commandLists, UINT Count);

    D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE GetType(UINT AffinityIndex = 0);

    BOOL STDMETHODCALLTYPE AssertResourceState(
        _In_  CD3DX12AffinityResource* pResource,
        UINT Subresource,
        UINT State);

    HRESULT STDMETHODCALLTYPE SetFeatureMask(
        D3D12_DEBUG_FEATURE Mask);

    ID3D12CommandList* GetChildObject(UINT AffinityIndex);

    bool IsReady();
    void RegisterPendingBroadcast();
    void UnRegisterPendingBroadcast();

private:
    ID3D12CommandList* mCommandLists[D3DX12_MAX_ACTIVE_NODES];
    std::mutex mMutexPendingBroadcast;
    UINT mPendingBroadcast = 0;
};
