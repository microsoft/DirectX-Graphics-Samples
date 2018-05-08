//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACINGSHADERHELPER_H
#define RAYTRACINGSHADERHELPER_H

#include "RayTracingHlslCompat.h"

#define INFINITY (1.0/0.0)

struct Ray
{
    float3 origin;
    float3 direction;
};

float length_toPow2(float2 p)
{
    return dot(p, p);
}

float length_toPow2(float3 p)
{
    return dot(p, p);
}

// Returns a cycling <0 -> 1 -> 0> animation interpolant 
float CalculateAnimationInterpolant(in float elapsedTime, in float cycleDuration)
{
    float curLinearCycleTime = fmod(elapsedTime, cycleDuration) / cycleDuration;
    curLinearCycleTime = (curLinearCycleTime <= 0.5f) ? 2 * curLinearCycleTime : 1 - 2 * (curLinearCycleTime - 0.5f);
    return smoothstep(0, 1, curLinearCycleTime);
}

void swap(inout float a, inout float b)
{
    float temp = a;
    a = b;
    b = temp;
}

bool IsInRange(in float val, in float min, in float max)
{
    return (val >= min && val <= max);
}

// Load three 16 bit indices from a byte addressed buffer.
uint3 Load3x16BitIndices(uint offsetBytes, ByteAddressBuffer Indices)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since we need to read three 16 bit indices: { 0, 1, 2 } 
    // aligned at a 4 byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = Indices.Load2(dwordAlignedOffset);

    // Aligned: { 0 1 | 2 - } => retrieve first three 16bit indices
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 16bit indices
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], float2 barycentrics)
{
    return vertexAttribute[0] +
        barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline Ray GenerateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 projectionToWorld)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions() * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorld);
    world.xyz /= world.w;

    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(world.xyz - ray.origin);

    return ray;
}

// Test if a hit is culled based on specified RayFlags.
bool IsCulled(in Ray ray, in float3 hitSurfaceNormal)
{
    float rayDirectionNormalDot = dot(ray.direction, hitSurfaceNormal);

    bool isCulled = 
        ((RayFlags() & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (rayDirectionNormalDot > 0))
        ||
        ((RayFlags() & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) && (rayDirectionNormalDot < 0));

    return isCulled; 
}

// Test if a hit is valid based on specified RayFlags and <RayTMin, RayTCurrent> range.
bool IsAValidHit(in Ray ray, in float thit, in float3 hitSurfaceNormal)
{
    return IsInRange(thit, RayTMin(), RayTCurrent()) && !IsCulled(ray, hitSurfaceNormal);
}

// Texture coordinates on a horizontal plane.
float2 TexCoords(in float3 position)
{
    return position.xz;
}

// Calculate ray differentials.
void CalculateRayDifferentials(out float2 ddx_uv, out float2 ddy_uv, in float2 uv, in float3 hitPosition, in float3 surfaceNormal, in float3 cameraPosition, in float4x4 projectionToWorld)
{
    // Compute ray differentials by intersecting the tangent plane to the  surface.
    Ray ddx = GenerateCameraRay(DispatchRaysIndex() + uint2(1, 0), cameraPosition, projectionToWorld);
    Ray ddy = GenerateCameraRay(DispatchRaysIndex() + uint2(0, 1), cameraPosition, projectionToWorld);

    // Compute ray differentials.
    float3 ddx_pos = ddx.origin - ddx.direction * dot(ddx.origin - hitPosition, surfaceNormal) / dot(ddx.direction, surfaceNormal);
    float3 ddy_pos = ddy.origin - ddy.direction * dot(ddy.origin - hitPosition, surfaceNormal) / dot(ddy.direction, surfaceNormal);

    // Calculate texture sampling footprint.
    ddx_uv = TexCoords(ddx_pos) - uv;
    ddy_uv = TexCoords(ddy_pos) - uv;
}

// Forward declaration.
float CheckersTextureBoxFilter(in float2 uv, in float2 dpdx, in float2 dpdy, in UINT ratio);

// Return analytically integrated checkerboard texture (box filter).
float AnalyticalCheckersTexture(in float3 hitPosition, in float3 surfaceNormal, in float3 cameraPosition, in float4x4 projectionToWorld)
{
    float2 ddx_uv;
    float2 ddy_uv;
    float2 uv = TexCoords(hitPosition);

    CalculateRayDifferentials(ddx_uv, ddy_uv, uv, hitPosition, surfaceNormal, cameraPosition, projectionToWorld);
    return CheckersTextureBoxFilter(uv, ddx_uv, ddy_uv, 50);
}

// Fresnel reflectance - schlick approximation.
float3 FresnelReflectanceSchlick(in float3 I, in float3 N, in float3 f0)
{
    float cosi = saturate(dot(-I, N));
    return f0 + (1 - f0)*pow(1 - cosi, 5);
}


#endif // RAYTRACINGSHADERHELPER_H