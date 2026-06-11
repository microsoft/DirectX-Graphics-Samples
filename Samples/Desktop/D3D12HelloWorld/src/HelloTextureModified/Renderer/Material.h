#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

static constexpr UINT kMaterialCount = 256;

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
