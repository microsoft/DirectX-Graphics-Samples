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
// When a ray hits geometry or needs to call a miss shader, GPU indexes into the application 
// provided shader tables. As per DXR spec, the shader table indexing is defined as follows:
//
//  o Miss shader table index = 
//      MissShaderIndex                                     ~ from shader: TraceRay()
//
//  o Hit group shader table index = 
//      RayContributionToHitGroupIndex                      ~ from shader: TraceRay()  
//      + MultiplierForGeometryContributionToHitGroupIndex  ~ from shader: TraceRay()
//          * GeometryContributionToHitGroupIndex +         ~ system generated index 
//                                                            of geometry in BLAS
//      + InstanceContributionToHitGroupIndex               ~ from BLAS instance desc
//
// 
//  ------ Shader table layout & indexing values used in this sample ---------------
//
// The sample traces radiance and shadow rays. Since the ray types apply different 
// actions in the shaders, they require separate shader records for each geometry,
// so that GPU executes the right shader when a geometry is hit or or miss shader needs
// to be executed.
//
// In addition, this sample uses multiple different geometries. First, there is 
// triangle geometry (the ground plane) and then there is AABB/procedural geometry. 
// AABB geometry requires intersection shaders to be specifiend in the hit 
// groups. This sample demonstrates a use of three different intersection shaders,
// and thus further enumeration of different shader records per each:
//  - MyIntersectionShader_AnalyticPrimitive
//  - MyIntersectionShader_VolumetricPrimitive
//  - MyIntersectionShader_SignedDistancePrimitive
// 
// Last, the sample defines multiple geometries per each intersection shader, 
// all parametrized via attributes provided via local root signatures,
// which in turn each require a different shader record. 
//
// With that said, the sample has following shader table layouts. 
//
// Miss shader table layout
// o There are two miss shader records:
//   [0] : Miss shader record for a radiance ray
//   [1] : Miss shader record for a shadow ray
//
// Hit group shader table layout:
//  o Triangle geometry shader records - single triangular geometry (ground plane)
//  o AABB geometry shader records- multiple AABB geometries ~ IntersectionShaderType::TotalPrimitiveCount()
// Both triangle and ABB geometry require two shader records, per ray type:
//   [0] : Hit group shader record for geometry ID 0 for a radiance ray
//   [1] : Hit group shader record for geometry ID 0 for a shadow ray
//
// The actual shader tables are printed out in the debug output by this sample 
// and its useful as a reference to indexing parameters. Here are first few
// shader records of a hit group for reference in an example below:
// | Shader table - HitGroupShaderTable: 
// | [0] : MyHitGroup_Triangle                           // Triangle geometry in 1st BLAS
// | [1] : MyHitGroup_Triangle_ShadowRay
// | [2] : MyHitGroup_AABB_AnalyticPrimitive             // ~ 1st AABB geometry in 2nd BLAS
// | [3] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
// | [4] : MyHitGroup_AABB_AnalyticPrimitive             // ~ 2nd AABB geometry in 2nd BLAS
// | [5] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
// ...
//
// Given the shader table layout, the shader table indexing is set as follows:
// o MissShaderIndex is set to 0 for radiance rays, and 1 for shadow rays in TraceRay().
// o *GeometryContributionToHitGroupIndex* is a Geometry ID that is system 
//   generated for each geometry within a BLAS. This directly maps to GeometryDesc 
//   order passed in by the app, i.e. {0, 1, 2,...}
// o Given two hit groups (radiance, shadow ray) per geometry ID, 
//   *MultiplierForGeometryContributionToHitGroupIndex* is 2. 
// o *RayContributionToHitGroupIndex* is set to 0 and 1, for radiance and shadow 
//   rays respectively, since they're stored subsequently in a shader table.
// * InstanceContributionToHitGroupIndex is an offset in-between instances.
//   Since triangle BLAS is first, it's set to 0 for triangle BLAS. AABB BLAS 
//   sets it 2 since the BLAS has to skip over the triangle BLAS's shader
//   records, which is 2 since the triangle plane has two shader records 
//   (one for radiance, and one for shadow ray).
//
// Example:
//  A shader calls a TraceRay() for a shadow ray and sets *RayContributionToHitGroupIndex*
//  to 1 as an offset to shadow shader records and *MultiplierForGeometryContributionToHitGroupIndex*
//  to 2 since there are two shader records per geometry. The shadow ray hits a second 
//  AABB geometry in the 2nd BLAS that contains AABB geometries. The shader index will be
//  5 in the table above and is calculated as:
//     1  // ~ RayContributionToHitGroupIndex                   - from TraceRay()                 
//   + 2  // ~ MultiplierForGeometryContributionToHitGroupIndex - from TraceRay() 
//   * 1  // ~ GeometryContributionToHitGroupIndex              - from runtime, 2nd geometry => ID:1
//   + 2  // ~ InstanceContributionToHitGroupIndex              - from BLAS instance desc
//
// ************************************************************************/ 

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"
#include "ProceduralPrimitivesLibrary.h"
#include "RaytracingShaderHelper.h"

//***************************************************************************
//*****------ Shader resources bound via root signatures -------*************
//***************************************************************************

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


//***************************************************************************
//****************------ Utility functions -------***************************
//***************************************************************************

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

//***************************************************************************
//*****------ TraceRay wrappers for radiance and shadow rays. -------*********
//***************************************************************************

// Trace a radiance ray into the scene and returns a shaded color.
float4 TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
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
    rayDesc.TMax = 10000;
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
    ray.TMax = 10000;

    // Initialize shadow ray payload.
    // Set the initial value to true since closest and any hit shaders are skipped. 
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

//***************************************************************************
//********************------ Ray gen shader.. -------************************
//***************************************************************************

[shader("raygeneration")]
void MyRaygenShader()
{
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    Ray ray = GenerateCameraRay(DispatchRaysIndex(), g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);

    // Cast a ray into the scene and retrieve a shaded color.
    UINT currentRecursionDepth = 0;
    float4 color = TraceRadianceRay(ray, currentRecursionDepth);

    // Write the raytraced color to the output texture.
    g_renderTarget[DispatchRaysIndex()] = color;
}

//***************************************************************************
//******************------ Closest hit shaders -------***********************
//***************************************************************************

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
    float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

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

//***************************************************************************
//**********************------ Miss shaders -------**************************
//***************************************************************************

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
// 
//***************************************************************************
//*****************------ Intersection shaders-------************************
//***************************************************************************

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