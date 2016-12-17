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

CD3DX12AffinityDeviceChild::CD3DX12AffinityDeviceChild(CD3DX12AffinityDevice* device, ID3D12DeviceChild** deviceChilds, UINT Count)
    : CD3DX12AffinityObject(device, reinterpret_cast<IUnknown**>(deviceChilds), Count)
{
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"DeviceChild";
#endif
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityDeviceChild::GetDevice(
    REFIID riid,
    void** ppvDevice)
{
    CD3DX12AffinityDevice* pDevice = GetParentDevice();
    pDevice->AddRef();
    *ppvDevice = (void*)pDevice;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityDeviceChild::GetDevice(
    CD3DX12AffinityDevice** ppDevice)
{
    *ppDevice = GetParentDevice();
    (*ppDevice)->AddRef();
    return S_OK;
}

bool CD3DX12AffinityDeviceChild::IsD3D()
{
    return true;
}
