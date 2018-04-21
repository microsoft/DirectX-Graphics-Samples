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

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"
#if !DISABLE_CODE
#include "ProceduralPrimitivesLibrary.h"
#endif
#include "RaytracingShaderHelper.h"

// ToDo:
// - handle RayFlags in intersection tests
// - specify traceRay args in a shared header
// - ReportHit will return to Intersection shader if RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH  is not specified. If that's the case handle it.
// 

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);
StructuredBuffer<AABBPrimitiveAttributes> g_AABBPrimitiveAttributes : register(t3, space0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<MaterialConstantBuffer> g_materialCB : register(b1);
ConstantBuffer<AABBConstantBuffer> lrs_aabbCB: register(b2);          // from local root signature

enum ProceduralPrimitives
{
    Box = 0,
    Spheres,
    Sphere,
    Count
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

#if DISABLE_CODE

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
    ray.TMin = 0;
    ray.TMax = 10000.0;
    RayPayload rayPayload = { float4(0, 0, 0, 0), 1 };
    TraceRay(
        Scene, // Raytracing acceleration structure
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, /* RayFlags */
        ~0, /* InstanceInclusionMask*/
        0, /* RayContributionToHitGroupIndex */
        2, /* MultiplierForGeometryContributionToHitGroupIndex */
        0, /* MissShaderIndex */
        ray, rayPayload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex()] = rayPayload.color;
}

[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
    rayPayload.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
}

[shader("miss")]
void MyMissShader(inout RayPayload rayPayload : SV_RayPayload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    rayPayload.color = background;
}

#else
// ToDo is pixelToLight correct?
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
    ray.TMin = 0;
    ray.TMax = 10000.0;
    RayPayload rayPayload = { float4(0, 0, 0, 0), 1 };
    TraceRay(
        Scene, // Raytracing acceleration structure
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, /* RayFlags */
        ~0, /* InstanceInclusionMask*/
         0, /* RayContributionToHitGroupIndex */
         2, /* MultiplierForGeometryContributionToHitGroupIndex */
         0, /* MissShaderIndex */
        ray, rayPayload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex()] = rayPayload.color;
}

#if DISABLE_CODE
#else
// Get ray in AABB's local space
Ray GetRayInAABBPrimitiveLocalSpace(out AABBPrimitiveAttributes attr)
{
    // Should PrimitiveIndex be passed as arg?
    // ToDo improve desc
    // Retrieve ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    attr = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
    Ray ray;
    ray.origin = mul(float4(ObjectRayOrigin(), 1), attr.bottomLevelASToLocalSpace).xyz;
    ray.direction = mul(ObjectRayDirection(), (float3x3) attr.bottomLevelASToLocalSpace);
    return ray;
}

[shader("intersection")]
void MyIntersectionShader_Sphere()
{
    ProceduralPrimitiveAttributes attr;
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RaySphereIntersectionTest(localRay, thit, attr))
    {
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);       
    }
}

[shader("intersection")]
void MyIntersectionShader_AABB()
{
    ProceduralPrimitiveAttributes attr;
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RayAABBIntersectionTest(localRay, thit, attr))
    {
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_Spheres()
{
    ProceduralPrimitiveAttributes attr;
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RaySpheresIntersectionTest(localRay, thit, attr))
    {
        // ToDo move this to tests?
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}
#endif

#if ENABLE_NEW_CODE
[shader("intersection")]
void MyIntersectionShader_Metaballs()
{
    ProceduralPrimitiveAttributes attr;
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RayMetaballsIntersectionTest(localRay, thit, attr))
    {
        // ToDo move this to tests?
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}


[shader("intersection")]
void MyIntersectionShader_AnalyticPrimitive()
{
    ProceduralPrimitiveAttributes attr;
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RayAnalyticGeometryIntersectionTest(localRay, (AnalyticPrimitive::Enum) lrs_aabbCB.primitiveType, thit, attr))
    {
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    ProceduralPrimitiveAttributes attr;
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RayVolumetricGeometryIntersectionTest(localRay, (VolumetricPrimitive::Enum) lrs_aabbCB.primitiveType, thit, attr))
    {
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_SignedDistancePrimitive()
{
    float thit;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    if (RaySignedDistancePrimitiveTest(localRay, (SignedDistancePrimitive::Enum) lrs_aabbCB.primitiveType, thit))
    {
        ProceduralPrimitiveAttributes attr;
        float3 position = localRay.origin + thit * localRay.direction;
        attr.normal = sdCalculateNormal(position, (SignedDistancePrimitive::Enum) lrs_aabbCB.primitiveType);
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);

        // ReportHit will reject any tHits outside a valid thit range: <RayTMin(), RayTCurrent()>.
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}
#endif
#if !DISABLE_CODE
// Trace a shadow ray and return true if it hits any geometry.
bool TraceShadowRayAndReportIfHit(in RayPayload rayPayload : SV_RayPayload)
{
    if (rayPayload.recursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = HitWorldPosition();
    ray.Direction = normalize(g_sceneCB.lightPosition - HitWorldPosition());
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    // For shadow ray this will be extremely small to avoid aliasing at contact areas.
    ray.TMin = 0;
    ray.TMax = 10000.0;
    // Set to true, since closest hit shaders are skipped. 
    // Shadow Miss shader, if called, will set it to false.
    ShadowRayPayload shadowPayload = { true };
    // ToDo use hit/miss indices from a header
    // ToDo place ShadowHitGroup right after Closest hitgroup?
    // ToDo review hit group indexing
    // ToDo - improve wording, reformat: Offset by 1 as AABB  BLAS offsets by 1 => 2
    TraceRay(Scene,
        // ToDo explain
        /* RayFlags */
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES
        | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders

        ~0,/* InstanceInclusionMask*/
        1, /* RayContributionToHitGroupIndex */
        2, /* MultiplierForGeometryContributionToHitGroupIndex */
        1, /* MissShaderIndex */
        ray, shadowPayload);

    return shadowPayload.hit;
}

[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
    float3 hitPosition = HitWorldPosition();

    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up 3 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex, Indices);

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

    // Trace a reflection ray. 
    // ToDo move into a function
    // ToDo is this a live value?
    float4 reflectionColor = float4(0,0,0,0);
    if (rayPayload.recursionDepth < MAX_RAY_RECURSION_DEPTH)
    {
        // Set the ray's extents.
        RayDesc ray;
        ray.Origin = hitPosition;
        ray.Direction = reflect(normalize(WorldRayDirection()), float3(0, 1, 0));// normalize(triangleNormal));
        // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
        // TMin should be kept small to prevent missing geometry at close contact areas.
        // For shadow ray this will be extremely small to avoid aliasing at contact areas.
        ray.TMin = 0;
        ray.TMax = 10000.0;
        RayPayload reflectionRayPayload = { float4(0, 0, 0, 0), rayPayload.recursionDepth + 1 };
        // ToDo use hit/miss indices from a header
        // ToDo place ShadowHitGroup right after Closest hitgroup?
        // ToDo review hit group indexing
        // ToDo - improve wording, reformat: Offset by 1 as AABB  BLAS offsets by 1 => 2
        TraceRay(Scene,
            RAY_FLAG_CULL_BACK_FACING_TRIANGLES, /* RayFlags */
            ~0,/* InstanceInclusionMask*/
            0, /* RayContributionToHitGroupIndex */
            2, /* MultiplierForGeometryContributionToHitGroupIndex */
            0, /* MissShaderIndex */
            ray, reflectionRayPayload);
        reflectionColor = reflectionRayPayload.color;
    }

    // Trace a shadow ray. 
    bool shadowRayHit = TraceShadowRayAndReportIfHit(rayPayload);
    float shadowFactor = shadowRayHit ? 0.1 : 1.0;

    float4 diffuseColor = shadowFactor * g_materialCB.albedo * CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor + (float4(1,1,1,1) - g_materialCB.albedo) * reflectionColor;

    rayPayload.color = color;
}

[shader("closesthit")]
void MyClosestHitShader_AABB(inout RayPayload rayPayload : SV_RayPayload, in ProceduralPrimitiveAttributes attr : SV_IntersectionAttributes)
{
    float3 hitPosition = HitWorldPosition();

    // Trace a shadow ray. 
    bool shadowRayHit = TraceShadowRayAndReportIfHit(rayPayload);
    float shadowFactor = shadowRayHit ? 0.1 : 1.0;

    float3 triangleNormal = attr.normal;
    float4 albedo = g_materialCB.albedo;
    float4 diffuseColor = shadowFactor * albedo * CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    rayPayload.color = color;
}

[shader("miss")]
void MyMissShader(inout RayPayload rayPayload : SV_RayPayload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    rayPayload.color = background;
}

[shader("closesthit")]
void MyClosestHitShader_ShadowRayTriangle(inout ShadowRayPayload rayPayload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
    rayPayload.hit = true;
}

[shader("closesthit")]
void MyClosestHitShader_ShadowRayAABB(inout ShadowRayPayload rayPayload : SV_RayPayload, in ProceduralPrimitiveAttributes attr : SV_IntersectionAttributes)
{
    rayPayload.hit = true;
}


[shader("miss")]
void MyMissShader_Shadow(inout ShadowRayPayload rayPayload : SV_RayPayload)
{
    rayPayload.hit = false;
}
#endif
#endif
#endif // RAYTRACING_HLSL