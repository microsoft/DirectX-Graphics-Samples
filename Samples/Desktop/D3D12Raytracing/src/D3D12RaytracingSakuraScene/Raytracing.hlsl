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
// Constants for cube counts

#define NUM_CUBES 882
#define NUM_TRUNKS 441
#define NUM_LEAVES 441

#include "RaytracingHlslCompat.h"

using namespace dx;
RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
    
ByteAddressBuffer IndicesCube : register(t1, space0);
StructuredBuffer<Vertex> VerticesCube : register(t2, space0);
    
ByteAddressBuffer IndicesTrunk : register(t3, space0);
StructuredBuffer<Vertex> VerticesTrunk : register(t4, space0);
    
ByteAddressBuffer IndicesLeaves : register(t5, space0);
StructuredBuffer<Vertex> VerticesLeaves : register(t6, space0);
    
ByteAddressBuffer IndicesBush : register(t7, space0);
StructuredBuffer<Vertex> VerticesBush : register(t8, space0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<ObjectConstantBuffer> g_cubeCB : register(b1);
    
Texture2D<float4> TrunkTexture : register(t9, space0);
SamplerState TrunkSampler : register(s0);
    
Texture2D<float4> SakuraTexture : register(t10, space0);
SamplerState SakuraSampler : register(s1);
    
Texture2D<float4> BushTexture : register(t11, space0);
SamplerState BushSampler : register(s2);

    
// Load three 16 bit indices from a byte addressed buffer. 
uint3 Load3x16BitIndices(uint offsetBytes, ByteAddressBuffer baf)
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
    const uint2 four16BitIndices = baf.Load2(dwordAlignedOffset);
 
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
    
typedef BuiltInTriangleIntersectionAttributes MyAttributes;
    
    
// Struct defines the payload used during ray tracing 
    struct [raypayload] RayPayload
{
    float4 color : write(caller, closesthit, miss) : read(caller);
    uint recursionDepth : write(caller) : read(closesthit);
    uint reflectHint : write(caller) : read(closesthit, caller);
    float currentIOR : write(caller, closesthit) : read(caller); // NEW
    bool insideDielectric : write(caller, closesthit) : read(caller); // NEW
};

   

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

    
// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
    attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
    attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

    
// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics (float2 version).
float2 HitAttribute(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
    attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
    attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

    
// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(g_sceneCB.projectionToWorld, float4(screenPos, 0, 1));

    world.xyz /= world.w;
    origin = g_sceneCB.cameraPosition.xyz;
    direction = normalize(world.xyz - origin);
}
    

[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload payload =
    {
        float4(0, 0, 0, 0),
        0,
        0,
        1.0f, // currentIOR (air)
        false // insideDielectric
    };


    // If toggled 'S', enable SER  
    if (g_sceneCB.enableSER == 1)
    {
        HitObject hit = HitObject::TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
        uint materialID = hit.LoadLocalRootTableConstant(16);
        bool isGround = (materialID == 0 || materialID == 2);
            
        // If material is the ground, sample the texture to decide if it is reflective or not.
        if (isGround)
        {
            // Estimate hit point on ground plane (y = 0)
            float t = -origin.y / rayDir.y;
            float3 estimatedHit = origin + t * rayDir;

            // Generate procedural UVs from XZ
            float2 uv = frac(estimatedHit.xz * 0.5); 
            float4 texColor = TrunkTexture.SampleLevel(TrunkSampler, uv, 0);

            if (texColor.r < 0.05 && texColor.g < 0.05 && texColor.b < 0.05)
            {
               payload.reflectHint = 1;
            }
        }
        
        // If material is transparent cube, set reflectHint to 1
        if (materialID == 1)
        {
            payload.reflectHint = 1;
        }
            
        uint numHintBits = 1;
        if (g_sceneCB.enableSortByHit == 1)
        {
            dx::MaybeReorderThread(hit);
        }
        else if (g_sceneCB.enableSortByMaterial == 1)
        {
            uint sortKey = payload.reflectHint;
            dx::MaybeReorderThread(payload.reflectHint, numHintBits);
        }
        else if (g_sceneCB.enableSortByBoth == 1)
        {
            uint sortKey = payload.reflectHint;
            dx::MaybeReorderThread(hit, payload.reflectHint, numHintBits);
        }
        HitObject::Invoke(hit, payload);
    }
    else
    {
        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
        //uint hint = payload.reflectHint;
    }
        
    float4 color = payload.color;

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = color;
}

    
// Fresnel reflectance - schlick approximation.
float3 FresnelReflectanceSchlick(in float3 I, in float3 N, in float3 f0)
{
    float cosi = saturate(dot(-I, N));
    return f0 + (1 - f0) * pow(1 - cosi, 5);
}
    
    
struct Ray
{
    float3 Origin;
    float3 Direction;
};
    
    
// Trace a radiance ray into the scene and returns a shaded color.
float4 TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= 8)
    {
        return float4(0, 0, 0, 0);
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.Origin;
    rayDesc.Direction = ray.Direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000;
    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1, 0, 1.0f, false };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, rayDesc, rayPayload);

    return rayPayload.color;
}

    
// Star Nest by Pablo Roman Andrioli
[shader("closesthit")]
void FloorClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesCube.Load3(offset);

    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;

    // Procedural UVs
    float2 baseUV = frac(hitPosition.xz * 0.5);
    sampled.rgb = TrunkTexture.SampleLevel(TrunkSampler, baseUV, 0).rgb;
    
    // Interpolate normals
    float3 vertexNormals[3] =
    {
        VerticesCube[indices.x].normal,
        VerticesCube[indices.y].normal,
        VerticesCube[indices.z].normal
    };
    triangleNormal = HitAttribute(vertexNormals, attr);

    float3 baseColor = albedo * sampled.rgb;
    float3 finalColor;

    // Trace reflection ray if surface is dark
    if (all(sampled.rgb < 0.05) && payload.recursionDepth < 8)
    {
        Ray reflectionRay;
        reflectionRay.Origin = hitPosition + triangleNormal * 0.001f;
        reflectionRay.Direction = reflect(WorldRayDirection(), triangleNormal);

        float4 reflectionColor = TraceRadianceRay(reflectionRay, payload.recursionDepth);
        float3 fresnel = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, baseColor);
        float3 refColor = lerp(baseColor, reflectionColor.rgb, fresnel) * 1.7f;

        // Volumetric fractal clouds effect
        const int FractalSampleSteps = 260;
        const int FractalIterations = 20;
        const float SampleSpacing = 0.15;
        const float FractalOffset = 0.53;

        float fractalDensity = 0;
        float fadeFactor = 1.0;

        for (int s = 0; s < FractalSampleSteps; ++s)
        {
            float3 p = hitPosition + triangleNormal * (s * SampleSpacing);
            p = abs(fmod(p, 2.0) - 1.0);

            float pa = 0;
            float a = 0;
            for (int i = 0; i < FractalIterations; ++i)
            {
                float invLen2 = 1.0 / dot(p, p);
                p = abs(p) * invLen2 - FractalOffset;
                float lenP = length(p);
                a += abs(lenP - pa);
                pa = lenP;
            }

            fractalDensity += a * fadeFactor;
            fadeFactor *= 0.9;
        }

        // Normalize & smooth fractal density
        fractalDensity = saturate(fractalDensity / (FractalSampleSteps * FractalIterations * 0.12));
        float d = smoothstep(0.1, 0.9, fractalDensity);

        // Cloud color ramp & light glow
        float3 blue = float3(0.4, 0.6, 1.0);  // Soft blue
        float3 pink = float3(1.0, 0.6, 0.8);  // Soft pink
        float3 cloud = lerp(blue, pink, d);

        float3 L = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
        float lightD = saturate(dot(triangleNormal, L));
        cloud *= lerp(0.8 + 0.2 * lightD, 4.0, d); // lightD ranges [0.8,1.0]

        // Composite reflection with clouds
        finalColor = lerp(refColor, cloud, d * 0.7);
    }
    else
    {
        finalColor = baseColor * 1.7f;
    }

    payload.color = float4(finalColor, g_cubeCB.albedo.w);
}
    
    
[shader("closesthit")]
void TrunkClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesLeaves.Load3(offset);

    // Albedo is defined per shape or material
    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;
        
    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3];
    vertexNormals[0] = VerticesTrunk[indices.x].normal;
    vertexNormals[1] = VerticesTrunk[indices.y].normal;
    vertexNormals[2] = VerticesTrunk[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);
            
    float3 vertexTexCoords[3];
    vertexTexCoords[0] = VerticesTrunk[indices.x].uv;
    vertexTexCoords[1] = VerticesTrunk[indices.y].uv;
    vertexTexCoords[2] = VerticesTrunk[indices.z].uv;
    float2 interpolatedTexCoord = HitAttribute(vertexTexCoords, attr).xy;
    sampled.rgb = TrunkTexture.SampleLevel(TrunkSampler, interpolatedTexCoord, 0).rgb;
        
    float3 baseColor = albedo * sampled.rgb;
    float3 finalColor;

    finalColor = albedo * sampled.rgb * 1.3f; 
    payload.color = float4(finalColor, g_cubeCB.albedo.w);
}
    
    
[shader("closesthit")]
void LeavesClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesLeaves.Load3(offset);

    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;

    float3 vertexNormals[3];
    vertexNormals[0] = VerticesCube[indices.x].normal;
    vertexNormals[1] = VerticesCube[indices.y].normal;
    vertexNormals[2] = VerticesCube[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);

    // Procedural UVs (since cube may not have real UVs)
    float2 baseUV = frac(hitPosition.xz * 0.5);
    sampled.rgb = TrunkTexture.SampleLevel(TrunkSampler, baseUV, 0).rgb;
    float3 baseColor = albedo * sampled.rgb;

    // Volumetric absorption using Beer-Lambert law
    float3 absorptionCoeff = float3(0.2, 0.1, 0.05); // tweak per material
    float distance = RayTCurrent();
    float3 transmittance = exp(-absorptionCoeff * distance);
    baseColor *= transmittance;

    // Determine if we are entering or exiting the dielectric
    float3 incident = WorldRayDirection();
    bool entering = dot(incident, triangleNormal) < 0;
    float3 N = entering ? triangleNormal : -triangleNormal;

    float iorOutside = payload.insideDielectric ? 1.5f : 1.0f;
    float iorInside = payload.insideDielectric ? 1.0f : 1.5f;
    float eta = iorOutside / iorInside;

    float3 refractedDir = refract(incident, N, eta);
    bool validRefraction = length(refractedDir) > 0.001f;

    if (validRefraction && payload.recursionDepth < 8)
    {
        Ray refractedRay;
        refractedRay.Origin = hitPosition + refractedDir * 0.001f;
        refractedRay.Direction = refractedDir;

        RayPayload refractedPayload = payload;
        refractedPayload.recursionDepth += 1;
        refractedPayload.insideDielectric = !payload.insideDielectric;

        float4 refractedColor = TraceRadianceRay(refractedRay, refractedPayload.recursionDepth);
        payload.color.rgb += refractedColor.rgb * baseColor;
    }

    // Also compute direct lighting
    float3 lightDir = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    float NdotL = saturate(dot(triangleNormal, lightDir));
    float3 finalColor = baseColor * g_sceneCB.lightDiffuseColor.rgb * NdotL;
    payload.color = float4(finalColor, g_cubeCB.albedo.w);
}
    
    
[shader("closesthit")]
void LeavesLightClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesLeaves.Load3(offset);

    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;

    float3 vertexNormals[3];
    vertexNormals[0] = VerticesLeaves[indices.x].normal;
    vertexNormals[1] = VerticesLeaves[indices.y].normal;
    vertexNormals[2] = VerticesLeaves[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);
            
    float3 vertexTexCoords[3];
    vertexTexCoords[0] = VerticesLeaves[indices.x].uv;
    vertexTexCoords[1] = VerticesLeaves[indices.y].uv;
    vertexTexCoords[2] = VerticesLeaves[indices.z].uv;
    float2 interpolatedTexCoord = HitAttribute(vertexTexCoords, attr).xy;
    
    float3 finalColor;
    float3 baseColor = albedo * sampled.rgb;
    float3 lightDir = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    float NdotL = saturate(dot(triangleNormal, lightDir));


    finalColor = albedo * sampled.rgb * g_sceneCB.lightDiffuseColor.rgb * NdotL;
    payload.color = float4(finalColor, g_cubeCB.albedo.w);
}

    
[shader("closesthit")]
void LeavesDarkClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesLeaves.Load3(offset);

    float3 albedo = g_cubeCB.albedo.rgb;
    float3 pastelPurple = float3(0.8, 0.7, 0.9); 
    float3 pastelPink = float3(1.0, 0.8, 0.9);
    float3 darkPink = float3(0.85, 0.4, 0.6); 

    // Simple hash to pick one of the three colors
    uint hash = PrimitiveIndex() % 3;
    float3 sampled = (hash == 0) ? pastelPurple :
                    (hash == 1) ? pastelPink :
                                darkPink;

    float3 triangleNormal;
    float3 vertexNormals[3];
    vertexNormals[0] = VerticesLeaves[indices.x].normal;
    vertexNormals[1] = VerticesLeaves[indices.y].normal;
    vertexNormals[2] = VerticesLeaves[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);

    float3 vertexTexCoords[3];
    vertexTexCoords[0] = VerticesLeaves[indices.x].uv;
    vertexTexCoords[1] = VerticesLeaves[indices.y].uv;
    vertexTexCoords[2] = VerticesLeaves[indices.z].uv;
    float2 interpolatedTexCoord = HitAttribute(vertexTexCoords, attr).xy;

    float3 finalColor;
    float3 baseColor = albedo * sampled.rgb;
    float3 lightDir = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    float NdotL = saturate(dot(triangleNormal, lightDir));

    finalColor = albedo * sampled.rgb * g_sceneCB.lightDiffuseColor.rgb * NdotL;
    payload.color = float4(finalColor, g_cubeCB.albedo.w);
}

    
[shader("closesthit")]
void LeavesExtraDarkClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesLeaves.Load3(offset);

    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;

    float3 vertexNormals[3];
    vertexNormals[0] = VerticesLeaves[indices.x].normal;
    vertexNormals[1] = VerticesLeaves[indices.y].normal;
    vertexNormals[2] = VerticesLeaves[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);
            
    float3 vertexTexCoords[3];
    vertexTexCoords[0] = VerticesLeaves[indices.x].uv;
    vertexTexCoords[1] = VerticesLeaves[indices.y].uv;
    vertexTexCoords[2] = VerticesLeaves[indices.z].uv;
    float2 interpolatedTexCoord = HitAttribute(vertexTexCoords, attr).xy;
    float3 finalColor;
    float3 baseColor = albedo * sampled.rgb;
    float3 lightDir = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    float NdotL = saturate(dot(triangleNormal, lightDir));

    finalColor = albedo * sampled.rgb * g_sceneCB.lightDiffuseColor.rgb * NdotL;
}

    
[shader("closesthit")]
void BushClosestHitShader(inout RayPayload payload, in MyAttributes attr)
    {
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesBush.Load3(offset);

    // Albedo is defined per shape or material
    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3];
    vertexNormals[0] = VerticesBush[indices.x].normal;
    vertexNormals[1] = VerticesBush[indices.y].normal;
    vertexNormals[2] = VerticesBush[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);

    float3 vertexTexCoords[3];
    vertexTexCoords[0] = VerticesBush[indices.x].uv;
    vertexTexCoords[1] = VerticesBush[indices.y].uv;
    vertexTexCoords[2] = VerticesBush[indices.z].uv;
    float2 interpolatedTexCoord = HitAttribute(vertexTexCoords, attr).xy;

    // Sample the texture with the modified coordinates
    sampled.rgb = BushTexture.SampleLevel(BushSampler, interpolatedTexCoord, 0).rgb;
        
    float3 baseColor = albedo * sampled.rgb;
    float3 lightDir = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    float NdotL = saturate(dot(triangleNormal, lightDir));
    float3 finalColor = baseColor * g_sceneCB.lightDiffuseColor.rgb * NdotL;
    payload.color = float4(finalColor, g_cubeCB.albedo.w);
}


[shader("closesthit")]
void TCubeClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    uint offset = baseIndex * 4;
    uint3 indices = IndicesCube.Load3(offset);

    // Albedo is defined per shape or material
    float3 albedo = g_cubeCB.albedo.rgb;
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);
    float3 triangleNormal;

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3];
    vertexNormals[0] = VerticesTrunk[indices.x].normal;
    vertexNormals[1] = VerticesTrunk[indices.y].normal;
    vertexNormals[2] = VerticesTrunk[indices.z].normal;
    triangleNormal = HitAttribute(vertexNormals, attr);
        
    // Trace a reflection ray
    Ray reflectionRay = { hitPosition + triangleNormal * 0.5f, reflect(WorldRayDirection(), triangleNormal) };
    float4 reflectionColor = TraceRadianceRay(reflectionRay, payload.recursionDepth);

    float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, g_cubeCB.albedo.xyz);
    float4 reflectedColor = 0.5 * float4(fresnelR, 1) * reflectionColor;
    payload.color = reflectedColor;
}
        
  
[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float4 background = float4(1.0000f, 0.9216f, 0.9373f, 1.0f);
    payload.color = background;
}

#endif // RAYTRACING_HLSL