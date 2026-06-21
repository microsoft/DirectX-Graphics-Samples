struct Material
{
    uint albedoTexIndex;
    uint metallicRoughnessTexIndex;
    uint emissiveTexIndex;
    uint occlusionTexIndex;
    uint normalTexIndex;
    float roughnessFactor;
    float metallicFactor;
    float occlusionStrength;
    float ambientOcclusionFactor;
    float emissiveScale;
    uint flags;
};

static const uint MaterialFlagUnlit = 1u << 0;
static const uint MaterialFlagHasNormalTexture = 1u << 1;
