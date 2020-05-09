//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):    James Stanard, Christopher Wallis
//

#define HLSL
#include "ModelViewerRaytracing.h"
#include "RayTracingHlslCompat.h"

cbuffer Material : register(b3)
{
    uint MaterialID;
}

StructuredBuffer<RayTraceMeshInfo> g_meshInfo : register(t1);
ByteAddressBuffer g_indices : register(t2);
ByteAddressBuffer g_attributes : register(t3);
Texture2D<float> texShadow : register(t4);
Texture2D<float> texSSAO : register(t5);
SamplerState      g_s0 : register(s0);
SamplerComparisonState shadowSampler : register(s1);

Texture2D<float4> g_localTexture : register(t6);
Texture2D<float4> g_localNormal : register(t7);

Texture2D<float4>   normals  : register(t13);

uint3 Load3x16BitIndices(
    uint offsetBytes)
{
    const uint dwordAlignedOffset = offsetBytes & ~3;

    const uint2 four16BitIndices = g_indices.Load2(dwordAlignedOffset);

    uint3 indices;

    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

float GetShadow(float3 ShadowCoord)
{
    const float Dilation = 2.0;
    float d1 = Dilation * ShadowTexelSize.x * 0.125;
    float d2 = Dilation * ShadowTexelSize.x * 0.875;
    float d3 = Dilation * ShadowTexelSize.x * 0.625;
    float d4 = Dilation * ShadowTexelSize.x * 0.375;
    float result = (
        2.0 * texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy, ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d2, d1), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d1, -d2), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d2, -d1), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d1, d2), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d4, d3), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d3, -d4), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d4, -d3), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d3, d4), ShadowCoord.z)
        ) / 10.0;
    return result * result;
}

float2 GetUVAttribute(uint byteOffset)
{
    return asfloat(g_attributes.Load2(byteOffset));
}

void AntiAliasSpecular(inout float3 texNormal, inout float gloss)
{
    float normalLenSq = dot(texNormal, texNormal);
    float invNormalLen = rsqrt(normalLenSq);
    texNormal *= invNormalLen;
    gloss = lerp(1, gloss, rcp(invNormalLen));
}

// Apply fresnel to modulate the specular albedo
void FSchlick(inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec)
{
    float fresnel = pow(1.0 - saturate(dot(lightDir, halfVec)), 5.0);
    specular = lerp(specular, 1, fresnel);
    diffuse = lerp(diffuse, 0, fresnel);
}

float3 ApplyLightCommon(
    float3    diffuseColor,    // Diffuse albedo
    float3    specularColor,    // Specular albedo
    float    specularMask,    // Where is it shiny or dingy?
    float    gloss,            // Specular power
    float3    normal,            // World-space normal
    float3    viewDir,        // World-space vector from eye to point
    float3    lightDir,        // World-space vector from point to light
    float3    lightColor        // Radiance of directional light
)
{
    float3 halfVec = normalize(lightDir - viewDir);
    float nDotH = saturate(dot(halfVec, normal));

    FSchlick(specularColor, diffuseColor, lightDir, halfVec);

    float specularFactor = specularMask * pow(nDotH, gloss) * (gloss + 2) / 8;

    float nDotL = saturate(dot(normal, lightDir));

    return nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}

float3 RayPlaneIntersection(float3 planeOrigin, float3 planeNormal, float3 rayOrigin, float3 rayDirection)
{
    float t = dot(-planeNormal, rayOrigin - planeOrigin) / dot(planeNormal, rayDirection);
    return rayOrigin + rayDirection * t;
}

/*
    REF: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
    From "Real-Time Collision Detection" by Christer Ericson
*/
float3 BarycentricCoordinates(float3 pt, float3 v0, float3 v1, float3 v2)
{
    float3 e0 = v1 - v0;
    float3 e1 = v2 - v0;
    float3 e2 = pt - v0;
    float d00 = dot(e0, e0);
    float d01 = dot(e0, e1);
    float d11 = dot(e1, e1);
    float d20 = dot(e2, e0);
    float d21 = dot(e2, e1);
    float denom = 1.0 / (d00 * d11 - d01 * d01);
    float v = (d11 * d20 - d01 * d21) * denom;
    float w = (d00 * d21 - d01 * d20) * denom;
    float u = 1.0 - v - w;
    return float3(u, v, w);
}

[shader("closesthit")]
void Hit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.RayHitT = RayTCurrent();
    if (payload.SkipShading)
    {
        return;
    }

    uint materialID = MaterialID;
    uint triangleID = PrimitiveIndex();

    RayTraceMeshInfo info = g_meshInfo[materialID];

    const uint3 ii = Load3x16BitIndices(info.m_indexOffsetBytes + PrimitiveIndex() * 3 * 2);
    const float2 uv0 = GetUVAttribute(info.m_uvAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes);
    const float2 uv1 = GetUVAttribute(info.m_uvAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes);
    const float2 uv2 = GetUVAttribute(info.m_uvAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes);

    float3 bary = float3(1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    float2 uv = bary.x * uv0 + bary.y * uv1 + bary.z * uv2;

    const float3 normal0 = asfloat(g_attributes.Load3(info.m_normalAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes));
    const float3 normal1 = asfloat(g_attributes.Load3(info.m_normalAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes));
    const float3 normal2 = asfloat(g_attributes.Load3(info.m_normalAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes));
    float3 vsNormal = normalize(normal0 * bary.x + normal1 * bary.y + normal2 * bary.z);
    
    const float3 tangent0 = asfloat(g_attributes.Load3(info.m_tangentAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes));
    const float3 tangent1 = asfloat(g_attributes.Load3(info.m_tangentAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes));
    const float3 tangent2 = asfloat(g_attributes.Load3(info.m_tangentAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes));
    float3 vsTangent = normalize(tangent0 * bary.x + tangent1 * bary.y + tangent2 * bary.z);

    // Reintroduced the bitangent because we aren't storing the handedness of the tangent frame anywhere.  Assuming the space
    // is right-handed causes normal maps to invert for some surfaces.  The Sponza mesh has all three axes of the tangent frame.
    //float3 vsBitangent = normalize(cross(vsNormal, vsTangent)) * (isRightHanded ? 1.0 : -1.0);
    const float3 bitangent0 = asfloat(g_attributes.Load3(info.m_bitangentAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes));
    const float3 bitangent1 = asfloat(g_attributes.Load3(info.m_bitangentAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes));
    const float3 bitangent2 = asfloat(g_attributes.Load3(info.m_bitangentAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes));
    float3 vsBitangent = normalize(bitangent0 * bary.x + bitangent1 * bary.y + bitangent2 * bary.z);

    // TODO: Should just store uv partial derivatives in here rather than loading position and caculating it per pixel
    const float3 p0 = asfloat(g_attributes.Load3(info.m_positionAttributeOffsetBytes + ii.x * info.m_attributeStrideBytes));
    const float3 p1 = asfloat(g_attributes.Load3(info.m_positionAttributeOffsetBytes + ii.y * info.m_attributeStrideBytes));
    const float3 p2 = asfloat(g_attributes.Load3(info.m_positionAttributeOffsetBytes + ii.z * info.m_attributeStrideBytes));

    float3 worldPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    //---------------------------------------------------------------------------------------------
    // Compute partial derivatives of UV coordinates:
    //
    //  1) Construct a plane from the hit triangle
    //  2) Intersect two helper rays with the plane:  one to the right and one down
    //  3) Compute barycentric coordinates of the two hit points
    //  4) Reconstruct the UV coordinates at the hit points
    //  5) Take the difference in UV coordinates as the partial derivatives X and Y

    // Normal for plane
    float3 triangleNormal = normalize(cross(p2 - p0, p1 - p0));

    // Helper rays
    uint2 threadID = DispatchRaysIndex().xy;
    float3 ddxOrigin, ddxDir, ddyOrigin, ddyDir;
    GenerateCameraRay(uint2(threadID.x + 1, threadID.y), ddxOrigin, ddxDir);
    GenerateCameraRay(uint2(threadID.x, threadID.y + 1), ddyOrigin, ddyDir);

    // Intersect helper rays
    float3 xOffsetPoint = RayPlaneIntersection(worldPosition, triangleNormal, ddxOrigin, ddxDir);
    float3 yOffsetPoint = RayPlaneIntersection(worldPosition, triangleNormal, ddyOrigin, ddyDir);

    // Compute barycentrics 
    float3 baryX = BarycentricCoordinates(xOffsetPoint, p0, p1, p2);
    float3 baryY = BarycentricCoordinates(yOffsetPoint, p0, p1, p2);

    // Compute UVs and take the difference
    float3x2 uvMat = float3x2(uv0, uv1, uv2);
    float2 ddxUV = mul(baryX, uvMat) - uv;
    float2 ddyUV = mul(baryY, uvMat) - uv;

    //---------------------------------------------------------------------------------------------

    const float3 diffuseColor = g_localTexture.SampleGrad(g_s0, uv, ddxUV, ddyUV).rgb;
    float3 normal;
    float3 specularAlbedo = float3(0.56, 0.56, 0.56);
    float specularMask = 0;     // TODO: read the texture
    float gloss = 128.0;
    {
        normal = g_localNormal.SampleGrad(g_s0, uv, ddxUV, ddyUV).rgb * 2.0 - 1.0;
        AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(vsTangent, vsBitangent, vsNormal);
        normal = normalize(mul(normal, tbn));
    }
    
    float3 outputColor = AmbientColor * diffuseColor * texSSAO[DispatchRaysIndex().xy];

    float shadow = 1.0;
    if (UseShadowRays)
    {
        float3 shadowDirection = SunDirection;
        float3 shadowOrigin = worldPosition;
        RayDesc rayDesc = { shadowOrigin,
            0.1f,
            shadowDirection,
            FLT_MAX };
        RayPayload shadowPayload;
        shadowPayload.SkipShading = true;
        shadowPayload.RayHitT = FLT_MAX;
        TraceRay(g_accel, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,~0,0,1,0,rayDesc,shadowPayload);
        if (shadowPayload.RayHitT < FLT_MAX)
        {
            shadow = 0.0;
        }
    }
    else
    {
        // TODO: This could be pre-calculated once per vertex if this mul per pixel was a concern
        float4 shadowCoord = mul(ModelToShadow, float4(worldPosition, 1.0f));
        shadow = GetShadow(shadowCoord.xyz);
    }
    
    const float3 viewDir = normalize(-WorldRayDirection());

    outputColor +=  shadow * ApplyLightCommon(
        diffuseColor,
        specularAlbedo,
        specularMask,
        gloss,
        normal,
        viewDir,
        SunDirection,
        SunColor);

    // TODO: Should be passed in via material info
    if (IsReflection)
    {
        float reflectivity = normals[DispatchRaysIndex().xy].w;
        outputColor = g_screenOutput[DispatchRaysIndex().xy].rgb + reflectivity * outputColor;
    }

    g_screenOutput[DispatchRaysIndex().xy] = float4(outputColor, 1.0);
}