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

CD3DX12AffinityHeap::CD3DX12AffinityHeap(CD3DX12AffinityDevice* device, ID3D12Heap** heaps, UINT Count)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(heaps), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mHeaps[i] = heaps[i];
        }
        else
        {
            mHeaps[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"Heap";
#endif
}

D3D12_HEAP_DESC STDMETHODCALLTYPE CD3DX12AffinityHeap::GetDesc(UINT AffinityIndex)
{
    return mHeaps[AffinityIndex]->GetDesc();
}

ID3D12Heap* CD3DX12AffinityHeap::GetChildObject(UINT AffinityIndex)
{
    return mHeaps[AffinityIndex];
}
