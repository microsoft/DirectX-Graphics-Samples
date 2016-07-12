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
#include "CD3DX12AffinityPageable.h"

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityHeap : public CD3DX12AffinityPageable
{
public:
    CD3DX12AffinityHeap(CD3DX12AffinityDevice* device, ID3D12Heap** heaps, UINT Count);

    D3D12_HEAP_DESC STDMETHODCALLTYPE GetDesc(UINT AffinityIndex = 0);
    ID3D12Heap* GetChildObject(UINT AffinityIndex);

private:
    ID3D12Heap* mHeaps[D3DX12_MAX_ACTIVE_NODES];
};
