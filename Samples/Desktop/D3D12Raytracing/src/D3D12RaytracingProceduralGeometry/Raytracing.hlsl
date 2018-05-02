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

// ****------------- Shader table indexing and layouts -----------******************
//
// Shader table indexing:
//  o Miss shader table index =  MissShaderIndex            ~ from shader: TraceRay()
//
//  o Hit group shader table index = 
//      RayContributionToHitGroupIndex                      ~ from shader: TraceRay()  
//      + MultiplierForGeometryContributionToHitGroupIndex  ~ from shader: TraceRay()
//          * GeometryContributionToHitGroupIndex +         ~ system generated index of geometry in BLAS 
//      + InstanceContributionToHitGroupIndex               ~ from BLAS instance desc
//
//  --------------------------------------------------------------------
// Shader table layout & indexing in this sample: 
//
// Miss shader table layout:
//   [0] : Miss shader record for a color ray
//   [1] : Miss shader record for a shadow ray
//
// o Therefore MissShaderIndex is 0 for regular rays, and 1 for shadow rays
//
//
// Hit group shader table layout:
//   The sample uses two BLAS, first one for a triangle and second one for 
//   AABB geometry, therefore the hit group shader table is stored as follows:
//      [0 - ...]: Triangle hit groups shader records
//      [# triangle hit groups * 2 hit groups per geometry - ...]: AABB hit group shader records.
//   Each geometry category (Triangle/AABB) stores two hit group shader records 
//   for each geometry ID. 
//      [0] : Hit group shader record for geometry ID 0 for a color ray
//      [1] : Hit group shader record for geometry ID 0 for a shadow ray
//   This sample has 1 triangle geometry and  
//   IntersectionShaderType::TotalPrimitiveCount() AABB geometries. Each geometry
//   
// o Geometry IDs are system generated for each geometry within a BLAS. 
//   This maps to GeometryDesc order passed in by the app.
// o Given two hit groups (color, shadow ray) per geometry ID, 
//   MultiplierForGeometryContributionToHitGroupIndex is 2. This is same 
//   for both color and shadow rays.
// o RayContributionToHitGroupIndex is set to 0 and 1, for color and shadow 
//   rays respectively, since both both hit group shader record for a shadow 
//   ray is stored right after the color ray for each geometry in the shader table.
// * InstanceContributionToHitGroupIndex is set to 0 and 1 
//
// ************************************************************************/ 

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL



#define HLSL
#include "RaytracingHlslCompat.h"
#include "ProceduralPrimitivesLibrary.h"
#include "RaytracingShaderHelper.h"



//
// Shader resources bound via root signatures.
//  g_* - bound via a global root signature
//  l_* - bound via a local root signature
RaytracingAccelerationStructure g_scene : register(t0, space0);
RWTexture2D<float4> g_renderTarget : register(u0);
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

// Triangle resources
ByteAddressBuffer g_indices : register(t1, space0);
StructuredBuffer<Vertex> g_vertices : register(t2, space0);

// Procedural geometry resources
StructuredBuffer<PrimitiveInstancePerFrameBuffer> g_AABBPrimitiveAttributes : register(t3, space0);
ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1);
ConstantBuffer<PrimitiveInstanceConstantBuffer> l_aabbCB: register(b2);


//
// Utility functions
//

// Diffuse lighting calculation.
float CalculateDiffuseCoefficient(in float3 hitPosition, in float3 incidentLightRay, in float3 normal)
{
    float fNDotL = max(0.0f, dot(-incidentLightRay, normal));
    return fNDotL;
}

// Phong lighting specular component
float4 CalculateSpecularCoefficient(in float3 hitPosition, in float3 incidentLightRay, in float3 normal, in float specularPower)
{
    float3 reflectedLightRay = reflect(incidentLightRay, normal);
    return pow(saturate(dot(reflectedLightRay, -WorldRayDirection())), specularPower);
}


// Phong lighting model = ambient + diffuse + specular components.
float4 CalculatePhongLighting(float3 normal, bool isInShadow)
{
    float3 hitPosition = HitWorldPosition();
    float4 albedo = l_materialCB.albedo;
    float3 lightPosition = g_sceneCB.lightPosition.xyz;
    float shadowFactor = isInShadow ? 0.25 : 1.0;
    float3 incidentLightRay = normalize(hitPosition - lightPosition);

    // Diffuse component.
    float4 lightDiffuseColor = g_sceneCB.lightDiffuseColor;
    float Kd = CalculateDiffuseCoefficient(hitPosition, incidentLightRay, normal);
    float4 diffuseColor = 0.8 * shadowFactor * Kd * lightDiffuseColor * albedo;

    // Specular component.
    float4 specularColor = float4(0, 0, 0, 0);
    if (!isInShadow)
    {
        float4 lightSpecularColor = float4(1, 1, 1, 1);
        float specularPower = 50;
        float4 Ks = CalculateSpecularCoefficient(hitPosition, incidentLightRay, normal, specularPower);
        specularColor = Ks * lightSpecularColor;
    }

    // Ambient component.
    float4 ambientColor = g_sceneCB.lightAmbientColor;

    return ambientColor + diffuseColor + specularColor;
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
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000.0;

    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };



    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, /* RayFlags */
        ~0,/* InstanceInclusionMask*/
        0, /* RayContributionToHitGroupIndex */
        2, /* MultiplierForGeometryContributionToHitGroupIndex */
        0, /* MissShaderIndex */
        rayDesc, rayPayload);

    return rayPayload.color;
}

// Trace a shadow ray and return true if it hits any geometry.
bool TraceShadowRayAndReportIfHit(in float3 hitPosition, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = hitPosition;
    ray.Direction = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    // Set TMin to a zero value to avoid aliasing artifcats along contact areas.
    // Note: make sure to enable back-face culling so as to avoid surface face fighting.
    ray.TMin = 0;
    ray.TMax = 10000.0;

    // Initialize shadow ray payload.
    // Set the initial value to true since closest hit shaders are skipped. 
    // Shadow miss shader, if called, will set it to false.
    ShadowRayPayload shadowPayload = { true };

    TraceRay(g_scene,
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
    g_renderTarget[DispatchRaysIndex()] = color;
}

//
// Closest hit shaders.
// 

[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up three 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex, g_indices);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 triangleNormal = g_vertices[indices[0]].normal;

    // Trace a reflection ray.
    // Note it is recommended to limit live values across TraceRay calls. 
    // Therefore HitWorldPosition() is recalculated every time instead.
    Ray reflectionRay = { HitWorldPosition(), reflect(WorldRayDirection(), triangleNormal) };
    float4 reflectionColor = TraceRegularRay(reflectionRay, rayPayload.recursionDepth);

    // Trace a shadow ray.
    bool shadowRayHit = TraceShadowRayAndReportIfHit(HitWorldPosition(), rayPayload.recursionDepth);
    
    // Phong lighting color.
    float4 phongColor = CalculatePhongLighting(triangleNormal, shadowRayHit);

    // Reflected component.
    float4 reflectance = float4(1, 1, 1, 1) - l_materialCB.albedo;
    float4 reflectedColor = reflectance * reflectionColor;

    rayPayload.color = phongColor + reflectedColor;
}

[shader("closesthit")]
void MyClosestHitShader_AABB(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
    // Trace a shadow ray.
    bool shadowRayHit = TraceShadowRayAndReportIfHit(HitWorldPosition(), rayPayload.recursionDepth); 

    rayPayload.color = CalculatePhongLighting(attr.normal, shadowRayHit);
}


//
// Miss shaders.
//

[shader("miss")]
void MyMissShader(inout RayPayload rayPayload)
{
    float4 backgroundColor = float4(0.05f, 0.3f, 0.5f, 1.0f);
    rayPayload.color = backgroundColor;
}

[shader("miss")]
void MyMissShader_ShadowRay(inout ShadowRayPayload rayPayload)
{
    rayPayload.hit = false;
}

//
// Intersection shaders.
//

// Get ray in AABB's local space.
Ray GetRayInAABBPrimitiveLocalSpace()
{
    PrimitiveInstancePerFrameBuffer attr = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
    
    // Retrieve a ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    Ray ray;
    ray.origin = mul(float4(ObjectRayOrigin(), 1), attr.bottomLevelASToLocalSpace).xyz;
    ray.direction = mul(ObjectRayDirection(), (float3x3) attr.bottomLevelASToLocalSpace);
    return ray;
}

[shader("intersection")]
void MyIntersectionShader_AnalyticPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    AnalyticPrimitive::Enum primitiveType = (AnalyticPrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld(), attr.normal));

        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    VolumetricPrimitive::Enum primitiveType = (VolumetricPrimitive::Enum) l_aabbCB.primitiveType;
    
    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayVolumetricGeometryIntersectionTest(localRay, primitiveType, thit, attr, g_sceneCB.elapsedTime))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld(), attr.normal));

        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_SignedDistancePrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    SignedDistancePrimitive::Enum primitiveType = (SignedDistancePrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RaySignedDistancePrimitiveTest(localRay, primitiveType, thit, attr, l_materialCB.stepScale))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld(), attr.normal));
        
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

#endif // RAYTRACING_HLSL