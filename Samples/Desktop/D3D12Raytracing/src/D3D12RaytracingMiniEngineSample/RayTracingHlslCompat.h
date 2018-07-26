#ifndef RAYTRACING_HLSL_COMPAT_H_INCLUDED
#define RAYTRACING_HLSL_COMPAT_H_INCLUDED

#ifndef HLSL
#include "HlslCompat.h"
#endif


struct RayTraceMeshInfo
{
    uint  m_indexOffsetBytes;
    uint  m_uvAttributeOffsetBytes;
    uint  m_normalAttributeOffsetBytes;
    uint  m_tangentAttributeOffsetBytes;
    uint  m_bitangentAttributeOffsetBytes;
    uint  m_positionAttributeOffsetBytes;
    uint  m_attributeStrideBytes;
    uint  m_materialInstanceId;
};

#endif //RAYTRACING_USER_HLSL_COMPAT_H_INCLUDED
