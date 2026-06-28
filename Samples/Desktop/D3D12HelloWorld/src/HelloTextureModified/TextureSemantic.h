#pragma once

#include <cstdint>

namespace Engine
{

enum class TextureSemantic : uint8_t
{
    BaseColor,
    MetallicRoughness,
    Normal,
    Occlusion,
    Emissive,

    Count
};

static constexpr uint32_t kTextureSemanticCount = static_cast<uint32_t>(TextureSemantic::Count);

} // namespace Engine
