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

CD3DX12AffinityPageable::CD3DX12AffinityPageable(CD3DX12AffinityDevice* device, ID3D12Pageable** pageables, UINT Count)
    : CD3DX12AffinityDeviceChild(device, reinterpret_cast<ID3D12DeviceChild**>(pageables), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mPageables[i] = pageables[i];
        }
        else
        {
            mPageables[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"Pageable";
#endif
}
