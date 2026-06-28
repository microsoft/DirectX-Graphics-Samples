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

inline const char* GetSemanticName(TextureSemantic semantic)
{
    switch (semantic)
    {
        case TextureSemantic::BaseColor:         return "BaseColor";
        case TextureSemantic::MetallicRoughness: return "MetallicRoughness";
        case TextureSemantic::Normal:            return "Normal";
        case TextureSemantic::Occlusion:         return "Occlusion";
        case TextureSemantic::Emissive:          return "Emissive";
        default:                                 return "Unknown";
    }
}

} // namespace Engine
