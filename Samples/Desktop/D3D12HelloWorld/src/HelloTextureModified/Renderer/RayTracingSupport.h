#pragma once

#include <d3d12.h>

namespace Engine
{

struct RayTracingSupportInfo
{
    bool IsSupported() const
    {
        return supported;
    }

    D3D12_RAYTRACING_TIER Tier() const
    {
        return tier;
    }

    const wchar_t* TierName() const;

    bool supported = false;
    D3D12_RAYTRACING_TIER tier = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

    static RayTracingSupportInfo Create(ID3D12Device* device);
};

} // namespace Engine
