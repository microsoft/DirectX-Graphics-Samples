#pragma once

#if defined(__cplusplus)
typedef unsigned int uint;
#endif

enum ConfigFlags
{
    SHOW_AHS = 0x1,
};

enum Descriptors
{
    RENDER_TARGET,
    TLAS,
    FONT,

    MODEL_TEXTURES_START,
    MODEL_TEXTURES_END = MODEL_TEXTURES_START + 16,

    POSITION_BUFFER,
    NORMAL_BUFFER,
    TEXCOORD_BUFFER,

    POSITION_INDEX_BUFFER,
    NORMAL_INDEX_BUFFER,
    TEXCOORD_INDEX_BUFFER,

    GEOMETRY_INFO_BUFFER,

    SCENE_CBV_0,    // Double buffered
    SCENE_CBV_1,    // Double buffered

    COUNT
};

struct GeometryInfo
{
    uint primitiveOffset;
    uint diffuseTextureIndex;
    uint alphaTextureIndex;
};