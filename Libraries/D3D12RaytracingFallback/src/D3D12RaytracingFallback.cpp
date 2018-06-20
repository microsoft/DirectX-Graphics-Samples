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

    enum DXRSupportType
    {
        OfficialAPI,
        ExperimentalAPI,
        None
    } supportType = None;
    
    bool bForceComputeFallback = ((UINT)createRaytracingFallbackDeviceFlags & (UINT)CreateRaytracingFallbackDeviceFlags::ForceComputeFallback) != 0;
    if (!bForceComputeFallback)
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
        if (SUCCEEDED(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options)))
            && options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
        {
            supportType = OfficialAPI;
        }

        CComPtr<ID3D12DeviceRaytracingPrototype> pRaytracingDevice;
        if (supportType == None && SUCCEEDED(pDevice->QueryInterface(&pRaytracingDevice)) && pRaytracingDevice)
        {
            supportType = ExperimentalAPI;
        }
    }
    
    switch (supportType)
    {
    case OfficialAPI:
        *ppDevice = new NativeRaytracingDevice(pDevice);
        break;
    case ExperimentalAPI:
        *ppDevice = new ExperimentalRaytracingDevice(pDevice);
        break;
    case None:
    default:
        *ppDevice = new FallbackLayer::RaytracingDevice(pDevice, NodeMask, createRaytracingFallbackDeviceFlags);
        break;
    }

    if (!(*ppDevice))
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
