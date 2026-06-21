#include "stdafx.h"

#include "RayTracingSupport.h"

namespace Engine
{

using Microsoft::WRL::ComPtr;

const wchar_t* RayTracingSupportInfo::TierName() const
{
    switch (tier)
    {
        case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
            return L"NotSupported";
        case D3D12_RAYTRACING_TIER_1_0:
            return L"1.0";
        case D3D12_RAYTRACING_TIER_1_1:
            return L"1.1";
        case D3D12_RAYTRACING_TIER_1_2:
            return L"1.2";
        default:
            return L"Unknown";
    }
}

RayTracingSupportInfo RayTracingSupportInfo::Create(ID3D12Device* device)
{
    RayTracingSupportInfo info;

    if (device == nullptr)
    {
        return info;
    }

    ComPtr<ID3D12Device5> device5;
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(&device5))))
    {
        return info;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    if (SUCCEEDED(device5->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
    {
        if (options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
        {
            info.supported = true;
            info.tier = options5.RaytracingTier;
        }
    }

    return info;
}

} // namespace Engine
