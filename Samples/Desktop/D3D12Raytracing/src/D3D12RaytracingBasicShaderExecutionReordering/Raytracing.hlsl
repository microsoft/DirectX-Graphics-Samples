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
#define CUBE_INSTANCE_COUNT 5041 
#include "RaytracingHlslCompat.h"

using namespace dx;
RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
    
ByteAddressBuffer IndicesCube : register(t1, space0);
StructuredBuffer<Vertex> VerticesCube : register(t2, space0);
ByteAddressBuffer IndicesComplex : register(t3, space0);
StructuredBuffer<Vertex> VerticesComplex : register(t4, space0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<ObjectConstantBuffer> g_objectCB : register(b1);
    
Texture2D<float4> MaterialTexture : register(t5, space0);
SamplerState TextureSampler : register(s0);

    
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
    };

    // If toggled 'S', enable SER  
    if (g_sceneCB.enableSER == 1)
    {
        HitObject hit = HitObject::TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
        uint materialID = hit.LoadLocalRootTableConstant(16);
        uint numHintBits = 1;
        
        // Reorder threads based on material ID (0 - cube, 1 - complex).
        dx::MaybeReorderThread(materialID, numHintBits);
            
        // Reorder threads based on hitobject and material ID (0 - cube, 1 - complex).
        // dx::MaybeReorderThread(hit, materialID, numHintBits);
            
        HitObject::Invoke(hit, payload);
    }
    else
    {
        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    }
        
    float4 color = payload.color;

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = color;
}

    
[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    bool isComplex = InstanceID() >= CUBE_INSTANCE_COUNT;
    float3 hitPosition = HitWorldPosition();

    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;
    
    // Load up 3 16 bit indices for the triangle.
    uint3 indices = isComplex ? Load3x16BitIndices(PrimitiveIndex() * 3 * 2, IndicesComplex) : Load3x16BitIndices(PrimitiveIndex() * 3 * 2, IndicesCube);
        
    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3];
    if (isComplex)
    {
        vertexNormals[0] = VerticesComplex[indices.x].normal;
        vertexNormals[1] = VerticesComplex[indices.y].normal;
        vertexNormals[2] = VerticesComplex[indices.z].normal;
    }
    else
    {
        vertexNormals[0] = VerticesCube[indices.x].normal;
        vertexNormals[1] = VerticesCube[indices.y].normal;
        vertexNormals[2] = VerticesCube[indices.z].normal;
    }

    // Compute the triangle's normal.
    // This is redundant and done for illustration purposes 
    // as all the per-vertex normals are the same and match triangle's normal in this sample. 
    
    // Albedo is defined per  material
    float3 albedo = g_objectCB.albedo; 
    float4 sampled = float4(1.0, 1.0, 1.0, 1.0);

    // Compute the triangle's normal
    float3 triangleNormal = HitAttribute(vertexNormals, attr);
    
    // If material ID is 0, use the checker texture (to illustrate differing workloads that is used as sortKey in raygeneration)
    if (g_objectCB.materialID == 1)
    {
        float2 uv = float2(
        frac(hitPosition.x * 0.5 + 0.5),
        frac(hitPosition.z * 0.5 + 0.5)
        );

        // Sample the texture
        float3 colorSum = float3(0, 0, 0);
        [unroll]
        for (int i = 0; i < 18; ++i)
        {
            float2 offset = float2(i * 0.01, i * 0.01);
            colorSum += MaterialTexture.SampleLevel(TextureSampler, uv + offset, 0).rgb;
        }
        sampled.rgb = colorSum / 18.0;

    }

    // Calculate diffuse lighting
    float3 baseColor = albedo * sampled.rgb;

    float3 lightDir = normalize(g_sceneCB.lightPosition.xyz - hitPosition);
    float NdotL = saturate(dot(triangleNormal, lightDir));
    float3 finalColor = baseColor * g_sceneCB.lightDiffuseColor.rgb * NdotL;
    payload.color = float4(finalColor, 1.0f);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    payload.color = background;
}

#endif // RAYTRACING_HLSL