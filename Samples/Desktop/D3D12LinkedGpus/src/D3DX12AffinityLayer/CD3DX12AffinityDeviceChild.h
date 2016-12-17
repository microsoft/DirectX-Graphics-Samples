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
#include "CD3DX12AffinityObject.h"

class CD3DX12AffinityDevice;

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityDeviceChild : public CD3DX12AffinityObject
{
public:
    CD3DX12AffinityDeviceChild(CD3DX12AffinityDevice* device, ID3D12DeviceChild* deviceChilds[], UINT Count);
    HRESULT STDMETHODCALLTYPE GetDevice(CD3DX12AffinityDevice** ppDevice);

    HRESULT STDMETHODCALLTYPE GetDevice(
        REFIID riid,
        void** ppvDevice);

protected:
    virtual bool IsD3D();
};
