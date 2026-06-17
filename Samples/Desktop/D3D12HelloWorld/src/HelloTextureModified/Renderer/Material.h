#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

static constexpr UINT kMaterialCount = 256;
static constexpr UINT kMaterialFlagUnlit = 1u << 0;
static constexpr UINT kMaterialFlagHasNormalTexture = 1u << 1;

struct Material
{
    UINT albedoTexIndex;
    UINT metallicRoughnessTexIndex;
    UINT emissiveTexIndex;
    UINT occlusionTexIndex;
    UINT normalTexIndex;
    float roughnessFactor;
    float metallicFactor;
    float occlusionStrength;
    float ambientOcclusionFactor;
    float emissiveScale;
    UINT flags;
};

} // namespace Engine
