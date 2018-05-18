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
#include "pch.h"

HRESULT D3D12CreateRaytracingFallbackDevice(
    _In_ ID3D12Device *pDevice, 
    _In_ DWORD createRaytracingFallbackDeviceFlags, 
    _In_ UINT NodeMask,
    _In_ REFIID riid,
    _COM_Outptr_opt_ void** ppDevice)
{
    if (pDevice == nullptr || riid != __uuidof(ID3D12RaytracingFallbackDevice))
    {
        return E_INVALIDARG;
    }

    CComPtr<ID3D12DeviceRaytracingPrototype> pRaytracingDevice;
    HRESULT hr = pDevice->QueryInterface(&pRaytracingDevice);

    const bool bSupportsNativeRaytracing = SUCCEEDED(hr) && pRaytracingDevice;
    if (!bSupportsNativeRaytracing || ((UINT)createRaytracingFallbackDeviceFlags & (UINT)CreateRaytracingFallbackDeviceFlags::ForceComputeFallback) != 0)
    {
        *ppDevice = new FallbackLayer::RaytracingDevice(pDevice, NodeMask, createRaytracingFallbackDeviceFlags);
    }
    else
    {
        *ppDevice = new NativeRaytracingDevice(pDevice);
    }

    if (!(*ppDevice))
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
