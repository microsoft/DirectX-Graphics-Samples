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

HRESULT STDMETHODCALLTYPE CD3DX12AffinityPipelineState::GetCachedBlob(
    ID3DBlob** ppBlob,
    UINT AffinityIndex)
{
    return mPipelineStates[AffinityIndex]->GetCachedBlob(ppBlob);
}

CD3DX12AffinityPipelineState::CD3DX12AffinityPipelineState(CD3DX12AffinityDevice* device, ID3D12PipelineState** pipelineStates, UINT Count)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(pipelineStates), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mPipelineStates[i] = pipelineStates[i];
        }
        else
        {
            mPipelineStates[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"PipelineState";
#endif
}
