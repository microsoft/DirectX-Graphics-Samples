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

#define HLSL
#include "RaytracingHlslCompat.h"
#include "ProceduralPrimitivesLibrary.h"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);
StructuredBuffer<AABBPrimitiveAttributes> g_AABBPrimitiveAttributes : register(t3, space0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);

// Load three 16 bit indices from a byte addressed buffer.
uint3 Load3x16BitIndices(uint offsetBytes)
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

struct MyAttributes
{
    float2 barycentrics;
    float4 normal;
};

enum ProceduralPrimitives
{
    Box = 0,
    Spheres,
    Sphere,
    Count
};

struct ShadowPayload
{
    bool hit;
};


struct HitData
{
    float4 color;
};

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
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions() * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(float4(screenPos, 0, 1), g_sceneCB.projectionToWorld);

    world.xyz /= world.w;
    origin = g_sceneCB.cameraPosition.xyz;
    direction = normalize(world - origin);
}

// Diffuse lighting calculation.
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_sceneCB.lightPosition - hitPosition);

    // Diffuse contribution.
    float fNDotL = max(0.0f, dot(pixelToLight, normal));

    return g_sceneCB.lightDiffuseColor * fNDotL;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;

    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex(), origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    HitData payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex()] = payload.color;
}

bool IntersectCustomPrimitiveFrontToBack(
    float3 rayOriginObject, float3 rayDirObject,
    float rayTMin, float rayTMax, inout float curT,
    out ProceduralPrimitiveAttributes attr)
{
    AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[PrimitiveIndex()];

    float3 rayOriginLocal = mul(float4(rayOriginObject, 1), aabbAttribute.bottomLevelASToLocalSpace).xyz;
    //float3 rayDirLocal = mul(rayDirObject, (float3x3) aabbAttribute.bottomLevelASToLocalSpace).xyz;
    float3 rayDirLocal = mul(float4(rayDirObject,0), aabbAttribute.bottomLevelASToLocalSpace).xyz;

    switch (PrimitiveIndex() % ProceduralPrimitives::Count)
    {
    case Box:       return intersectBox(rayOriginLocal, rayDirLocal, curT, attr);
    case Sphere:    return intersectSphere(rayOriginLocal, rayDirLocal, curT, attr);
    case Spheres:   return intersectSpheres(rayOriginLocal, rayDirLocal, curT, attr);
    }
    return false;
}

[shader("intersection")]
void IntersectionShader_Box()
{

    float THit = RayTCurrent();
    ProceduralPrimitiveAttributes attr;
    if (IntersectCustomPrimitiveFrontToBack(
        ObjectRayOrigin(), ObjectRayDirection(),
        //WorldRayOrigin(), WorldRayDirection(),
        RayTMin(), RayTCurrent(), THit, attr))
    {
        ReportHit(THit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_AABB()
{
    float THit = RayTCurrent();
    ProceduralPrimitiveAttributes attr;
    if (IntersectCustomPrimitiveFrontToBack(
        ObjectRayOrigin(), ObjectRayDirection(),
        //WorldRayOrigin(), WorldRayDirection(),
        RayTMin(), RayTCurrent(), THit, attr))
    {
        ReportHit(THit, /*hitKind*/ 0, attr);
    }
}

[shader("closesthit")]
void MyClosestHitShader_Triangle(inout HitData payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
    float3 hitPosition = HitWorldPosition();

    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up 3 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3] = {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal
    };

    // Compute the triangle's normal.
    // This is redundant and done for illustration purposes 
    // as all the per-vertex normals are the same and match triangle's normal in this sample. 
    float3 triangleNormal = HitAttribute(vertexNormals, attr.barycentrics);

    // Trace a shadow ray. 
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = hitPosition;
    ray.Direction = normalize(g_sceneCB.lightPosition - hitPosition);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    // For shadow ray this will be extremely small to avoid aliasing at contact areas.
    ray.TMin = 0.000001;
    ray.TMax = 10000.0;
    ShadowPayload shadowPayload;
    // ToDo use hit/miss indices from a header
    TraceRay(Scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 
                2 /* RayContributionToHitGroupIndex*/, 
                0, 
                1 /* MissShaderIndex */, 
                ray, shadowPayload);
    float shadowFactor = shadowPayload.hit ? 0.1 : 1.0;

    float4 diffuseColor = shadowFactor * g_cubeCB.diffuseColor * CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    payload.color = color;
}

[shader("closesthit")]
void MyClosestHitShader_AABB(inout HitData payload : SV_RayPayload, in ProceduralPrimitiveAttributes attr : SV_IntersectionAttributes)
{
    float3 hitPosition = HitWorldPosition();

#if 0 // ToDo doesn't work properly
    // Trace a shadow ray. 
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = hitPosition;
    ray.Direction = normalize(g_sceneCB.lightPosition - hitPosition);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.5;
    ray.TMax = 10000.0;
    ShadowPayload shadowPayload;
    // ToDo use hit/miss indices from a header
    TraceRay(Scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0,
        2 /* RayContributionToHitGroupIndex*/,
        0,
        1 /* MissShaderIndex */,
        ray, shadowPayload);
    float shadowFactor = shadowPayload.hit ? 0.1 : 1.0;
#else
    float shadowFactor = 1.0;
#endif

    float3 triangleNormal = attr.normal;
    float4 albedo = float4(g_AABBPrimitiveAttributes[PrimitiveIndex()].albedo, 1);
    float4 diffuseColor = shadowFactor * albedo * CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    payload.color = color;
}

[shader("miss")]
void MyMissShader(inout HitData payload : SV_RayPayload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    payload.color = background;
}

[shader("closesthit")]
void MyClosestHitShader_ShadowAABB(inout ShadowPayload payload : SV_RayPayload, in ProceduralPrimitiveAttributes attr : SV_IntersectionAttributes)
{
    payload.hit = true;
}

[shader("miss")]
void MyMissShader_Shadow(inout ShadowPayload payload : SV_RayPayload)
{
    payload.hit = false;
}
