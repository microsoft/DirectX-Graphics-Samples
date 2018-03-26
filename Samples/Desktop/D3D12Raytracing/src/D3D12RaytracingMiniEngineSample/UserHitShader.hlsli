#ifndef HLSL
#define HLSL
#endif

#include "RaytracingUserHlslCompat.h"


//
// Extra stuff engines must provide or read vertices/indices in some different way somehow
//

StructuredBuffer<RayTraceMeshInfo> g_meshInfo : register(t0, space1);
ByteAddressBuffer g_indices : register(t1, space1);
ByteAddressBuffer g_attributes : register(t2, space1);

//
// Bits copied from ModelPS
//

Texture2D<float4> g_texDiffuse[32] : register(t3, space1);

SamplerState g_s0 : register(s0);


//
// 16 bits from ByteAddressBuffer. A bit awkward.
//

uint3 Load3x16BitIndices(
    uint offsetBytes)
{
    const uint dwordAlignedOffset = offsetBytes & ~3;

    const uint2 four16BitIndices = g_indices.Load2(dwordAlignedOffset);

    uint3 indices;

    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = four16BitIndices.x >> 16;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else
    {
        indices.x = four16BitIndices.x >> 16;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = four16BitIndices.y >> 16;
    }

    return indices;
}

//
// Experimental hit shader
//

float4 GetDiffuseColor(
        out uint materialInstanceId,
        out float3 v0Normal,
        uint triId,
        float minT,
        float2 bary)
{
    // Work back to mesh
    const uint meshIndex = triId >> 24; // Pathetic. Real games may need 64bpp of this.
    const uint triangleIndex = triId & ((1 << 24) - 1);
    RayTraceMeshInfo info = g_meshInfo[meshIndex];

    // Read triangle indices
    const uint3 ii = Load3x16BitIndices(info.m_indexOffsetBytes + triangleIndex * 3 * 2);

    // Read triangle uv coordinates
    // NOTE: it'd be great to deinterleave these
    const float2 uv0 = asfloat(g_attributes.Load2(info.m_uvAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes));
    const float2 uv1 = asfloat(g_attributes.Load2(info.m_uvAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes));
    const float2 uv2 = asfloat(g_attributes.Load2(info.m_uvAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes));

    // Read normal -- that's optional
    const float3 normal0 = asfloat(g_attributes.Load3(info.m_normalAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes));
    //const float3 normal1 = asfloat(g_attributes.Load3(info.m_normalAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes));
    //const float3 normal2 = asfloat(g_attributes.Load3(info.m_normalAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes));
    v0Normal = normalize(normal0);// + normal1 + normal2);

    // Compute gradients
    // We talked about a bunch of methods...
    //float2 gradX = float2(1.f/1024, 0);
    //float2 gradY = float2(0, 1.f/1024);

    // Read the texture
    const float2 uv = bary.x * uv0 + bary.y * uv1 + saturate(1 - bary.x - bary.y) * uv2;

    //const float4 color = g_texDiffuse.SampleGrad(g_s0, uv, gradX, gradY);
    //const float4 color = g_texDiffuse.SampleLevel(g_s0, uv, 0);
    //const float4 color = float4(uv, 0, 0);
    materialInstanceId = info.m_materialInstanceId;

    //uint width, height, dummy;
    //g_texDiffuse[NonUniformResourceIndex(materialInstanceId)].GetDimensions(0, width, height, dummy);
    const float tAtWhich1x1 = 200;  // Depends on the FOV of the "camera". A surface normal could also help here.
    const float maxDim = 1024;//(float)max(width, height);
    const float grad = minT / (tAtWhich1x1 * maxDim);

    const float4 color = g_texDiffuse[NonUniformResourceIndex(materialInstanceId)].SampleGrad(g_s0, uv, float2(grad, 0), float2(0, grad));

    // Return the value
    return color;
}



