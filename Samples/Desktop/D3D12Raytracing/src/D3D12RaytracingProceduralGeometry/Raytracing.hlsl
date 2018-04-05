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

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);

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

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
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

    return g_cubeCB.diffuseColor * g_sceneCB.lightDiffuseColor * fNDotL;
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

void swap(inout float a, inout float b)
{
    float temp = a;
    a = b;
    b = temp;
}

struct Ray
{
    float3 origin, direction;       // ray origin and direction 
    float3 inv_direction;
    int sign[3];
};

Ray make_ray(float3 origin, float3 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.inv_direction = 1 / direction;
    ray.sign[0] = (ray.inv_direction.x < 0);
    ray.sign[1] = (ray.inv_direction.y < 0);
    ray.sign[2] = (ray.inv_direction.z < 0);
    return ray;
}
#if 0
// https://github.com/hpicgs/cgsee/wiki/Ray-Box-Intersection-on-the-GPU
bool intersectBox(Ray ray, out float thit, float rayTMin, float rayTMax)
{
    float3 aabb[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (aabb[ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tmax = (aabb[1 - ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tymin = (aabb[ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tymax = (aabb[1 - ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tzmin = (aabb[ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tzmax = (aabb[1 - ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tmin = max(max(tmin, tymin), tzmin);
    tmax = min(min(tmax, tymax), tzmax);

    thit = tmin;
    return tmin < tmax;
}
#else
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool intersectBox(Ray ray, out float thit, float rayTMin, float rayTMax)
{
    float3 bounds[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
#if 0
    tmin = (bounds[ray.sign[0]].x - ray.origin.x) / ray.direction.x;
    tmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) / ray.direction.x;
    if (ray.direction.x < 0) swap(tmin, tmax);
    tymin = (bounds[ray.sign[1]].y - ray.origin.y) / ray.direction.y;
    tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) / ray.direction.y;
    if (ray.direction.y < 0) swap(tymin, tymax);
    tzmin = (bounds[ray.sign[1]].z - ray.origin.z) / ray.direction.z;
    tzmax = (bounds[1 - ray.sign[1]].z - ray.origin.z) / ray.direction.z;
    if (ray.direction.z < 0) swap(tzmin, tzmax);
#elif 1
#if 0
    const int sign = ray.direction.x < 0 ? 1 : 0;
    tmin = (bounds[1 - sign].x - ray.origin.x) / ray.direction.x;
    tmax = (bounds[sign].x - ray.origin.x) / ray.direction.x;
    if (ray.direction.x < 0) swap(tmin, tmax);
#else
    tmin = (bounds[0].x - ray.origin.x) / ray.direction.x;
    tmax = (bounds[1].x - ray.origin.x) / ray.direction.x;
    if (ray.direction.x < 0) swap(tmin, tmax);
#endif
    tymin = (bounds[0].y - ray.origin.y) / ray.direction.y;
    tymax = (bounds[1].y - ray.origin.y) / ray.direction.y;
    if (ray.direction.y < 0) swap(tymin, tymax);
    tzmin = (bounds[0].z - ray.origin.z) / ray.direction.z;
    tzmax = (bounds[1].z - ray.origin.z) / ray.direction.z;
    if (ray.direction.z < 0) swap(tzmin, tzmax);
#elif 1
    if (ray.direction.x < 0)
    {
        tmin = (bounds[ray.sign[0]].x - ray.origin.x) / ray.direction.x;
        tmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) / ray.direction.x;
    }
    else
    {
        tmax = (bounds[ray.sign[0]].x - ray.origin.x) / ray.direction.x;
        tmin = (bounds[1 - ray.sign[0]].x - ray.origin.x) / ray.direction.x;
    }
    if (ray.direction.y < 0)
    {
        tymin = (bounds[ray.sign[1]].y - ray.origin.y) / ray.direction.y;
        tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) / ray.direction.y;
    }
    else
    {
        tymax = (bounds[ray.sign[1]].y - ray.origin.y) / ray.direction.y;
        tymin = (bounds[1 - ray.sign[1]].y - ray.origin.y) / ray.direction.y;
    }
    if (ray.direction.z < 0)
    {
        tzmin = (bounds[ray.sign[2]].z - ray.origin.z) / ray.direction.z;
        tzmax = (bounds[1 - ray.sign[2]].z - ray.origin.z) / ray.direction.z;
    }
    else
    {
        tzmax = (bounds[ray.sign[2]].z - ray.origin.z) / ray.direction.z;
        tzmin = (bounds[1 - ray.sign[2]].z - ray.origin.z) / ray.direction.z;
    }
#endif

    tmin = max(max(tmin, tymin), tzmin);
    tmax = min(min(tmax, tymax), tzmax);

    thit = tmin;
    return tmax > tmin;
}
#endif

bool IntersectCustomPrimitiveFrontToBack(
    float3 origin, float3 direction,
    float rayTMin, float rayTMax, inout float curT,
    out MyAttributes attr)
{
    attr.barycentrics = float2(1.0, 0);

    Ray ray = make_ray(origin, direction);
    return intersectBox(ray, curT, rayTMin, rayTMax);
}

[shader("intersection")]
void MyIntersectionShader()
{
    float THit = RayTCurrent();
    MyAttributes attr;
    if (IntersectCustomPrimitiveFrontToBack(
        WorldRayOrigin(), WorldRayDirection(),
        RayTMin(), RayTCurrent(), THit, attr))
    {
        ReportHit(THit, /*hitKind*/ 0, attr);
    }
}

[shader("closesthit")]
void MyClosestHitShader(inout HitData payload : SV_RayPayload, in MyAttributes attr : SV_IntersectionAttributes)
{
    float array[2] = { 1.0, 0.0 };
    float tmin;
    float3 direction = float3(1, 1, 1);// WorldRayDirection();
#if 0
    tmin = array[0];
#else  // CreateStateObject fails
    tmin = array[direction.x > 0 ? 1 : 0];
#endif 
    
    payload.color = float4(tmin, 0, 0, 1);

#if 1
#elif 1

#if 0   // CreateStateObject fails
    int sign = direction.x < 0 ? 1 : 0;
    tmin = (bounds[sign].x - origin.x) / direction.x;
    tmax = (bounds[1 - sign].x - origin.x) / direction.x;
#else
    tmin = (bounds[0].x - origin.x) / direction.x;
    tmax = (bounds[1].x - origin.x) / direction.x;
    if (direction.x < 0) swap(tmin, tmax);
#endif
    payload.color = float4(tmin, tmax, 0, 1);
#else
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
    float3 triangleNormal = HitAttribute(vertexNormals, attr);

    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    payload.color = color;
#endif
}

[shader("miss")]
void MyMissShader(inout HitData payload : SV_RayPayload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    payload.color = background;
}