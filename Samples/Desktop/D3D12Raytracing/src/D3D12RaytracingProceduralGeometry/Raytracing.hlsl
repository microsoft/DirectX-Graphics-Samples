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
#include "ProceduralPrimitivesLibrary.h"
#include "RaytracingShaderHelper.h"

// ToDo:
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

// Diffuse lighting calculation.
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_sceneCB.lightPosition - hitPosition);
    float fNDotL = max(0.0f, dot(pixelToLight, normal));
    return g_sceneCB.lightDiffuseColor * fNDotL;
}

//
// TraceRay wrappers for regular and shadow rays.
//

// Trace a regular ray into the scene and return a shaded color.
float4 TraceRegularRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // ToDo revise
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    // For shadow ray this will be extremely small to avoid aliasing at contact areas.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000.0;

    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };
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
        rayDesc, rayPayload);

    return rayPayload.color;
}

// Trace a shadow ray and return true if it hits any geometry.
bool TraceShadowRayAndReportIfHit(in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
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


//
// Ray gen shader.
// 

[shader("raygeneration")]
void MyRaygenShader()
{
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    Ray ray = GenerateCameraRay(DispatchRaysIndex(), g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);

    // Cast a ray into the scene and retrieve a shaded color.
    UINT currentRecursionDepth = 0;
    float4 color = TraceRegularRay(ray, currentRecursionDepth);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex()] = color;
}

//
// Closest hit shaders.
// 

[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up 3 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex, Indices);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 triangleNormal = Vertices[indices[0]].normal;

    // Trace a reflection ray.
    float3 hitPosition = HitWorldPosition();
    Ray reflectionRay = { hitPosition, reflect(normalize(WorldRayDirection()), triangleNormal) };
    float4 reflectionColor = TraceRegularRay(reflectionRay, rayPayload.recursionDepth);

    // Trace a shadow ray. 
    bool shadowRayHit = TraceShadowRayAndReportIfHit(rayPayload.recursionDepth);
    float shadowFactor = shadowRayHit ? 0.1 : 1.0;

    // Calculate lighting.
    float4 diffuseColor = shadowFactor * g_materialCB.albedo * CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 reflectance = float4(1, 1, 1, 1) - g_materialCB.albedo;
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor + reflectance * reflectionColor;

    rayPayload.color = color;
}

[shader("closesthit")]
void MyClosestHitShader_AABB(inout RayPayload rayPayload : SV_RayPayload, in ProceduralPrimitiveAttributes attr : SV_IntersectionAttributes)
{
    float3 hitPosition = HitWorldPosition();
    
    // Trace a shadow ray. 
    // ToDo fixup shadow ray for metaballs - threshold.
    bool shadowRayHit = TraceShadowRayAndReportIfHit(rayPayload.recursionDepth);
    float shadowFactor = shadowRayHit ? 0.1 : 1.0;

    float3 normal = attr.normal;
    float4 albedo = g_materialCB.albedo;
    float4 diffuseColor = shadowFactor * albedo * CalculateDiffuseLighting(hitPosition, normal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    rayPayload.color = color; ;// float4(normalize(float3(attr.normal.x, 0, attr.normal.z)), 1);// 
}


//
// Miss shaders.
//


[shader("miss")]
void MyMissShader(inout RayPayload rayPayload : SV_RayPayload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    rayPayload.color = background;
}


[shader("miss")]
void MyMissShader_ShadowRay(inout ShadowRayPayload rayPayload : SV_RayPayload)
{
    rayPayload.hit = false;
}

//
// Intersection shaders.
//

// Get ray in AABB's local space.
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
void MyIntersectionShader_AnalyticPrimitive()
{
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    AnalyticPrimitive::Enum primitiveType = (AnalyticPrimitive::Enum) lrs_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    {
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    float totalTime = g_sceneCB.totalTime;
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    VolumetricPrimitive::Enum primitiveType = (VolumetricPrimitive::Enum) lrs_aabbCB.primitiveType;
    
    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayVolumetricGeometryIntersectionTest(localRay, primitiveType, thit, attr, totalTime))
    {
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_SignedDistancePrimitive()
{
    AABBPrimitiveAttributes inAttr;
    Ray localRay = GetRayInAABBPrimitiveLocalSpace(inAttr);
    SignedDistancePrimitive::Enum primitiveType = (SignedDistancePrimitive::Enum) lrs_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RaySignedDistancePrimitiveTest(localRay, primitiveType, thit, attr, g_materialCB.stepScale))
    {
        float3 position = localRay.origin + thit * localRay.direction;
        AABBPrimitiveAttributes aabbAttribute = g_AABBPrimitiveAttributes[lrs_aabbCB.geometryIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = mul((float3x3) ObjectToWorld(), attr.normal);
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

#endif // RAYTRACING_HLSL