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

struct MyAttributes
{
    float2 barycentrics;
    float4 normal;
};


struct ProceduralPrimitiveAttributes
{
    float3 normal;
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
float3 HitAttribute(float3 vertexAttribute[3], MyAttributes attr)
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

bool solveQuadratic(float a, float b, float c, out float x0, out float x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = -0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) swap(x0, x1);

    return true;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool intersectSphere(float3 origin, float3 direction, out float thit, inout ProceduralPrimitiveAttributes attr)
{
    float t0, t1; // solutions for t if the ray intersects 
    float3 center = float3(0, 0, 0);
    float radius = 1;
    float radius2 = pow(radius, 2);
    
    // analytic solution
    float3 L = origin - center;
    float a = dot(direction, direction);
    float b = 2 * dot(direction, L);
    float c = dot(L, L) - radius2;
    if (!solveQuadratic(a, b, c, t0, t1)) return false;

    if (t0 > t1) swap(t0, t1);

    if (t0 < 0) 
    {
        t0 = t1; // if t0 is negative, let's use t1 instead 
        if (t0 < 0) return false; // both t0 and t1 are negative 
    }

    thit = t0;

    float3 hitPosition = origin + thit * direction;
    attr.normal = mul(ObjectToWorld(), normalize(hitPosition - center));

    //attr.barycentrics = float2(0.5, 0.9);
    return true;
}

// https://github.com/hpicgs/cgsee/wiki/Ray-Box-Intersection-on-the-GPU
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool intersectBox(float3 origin, float3 direction, out float thit, inout ProceduralPrimitiveAttributes attr)
{
    float3 bounds[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    tmin = (bounds[0].x - origin.x) / direction.x;
    tmax = (bounds[1].x - origin.x) / direction.x;
    if (direction.x < 0) swap(tmin, tmax);
    tymin = (bounds[0].y - origin.y) / direction.y;
    tymax = (bounds[1].y - origin.y) / direction.y;
    if (direction.y < 0) swap(tymin, tymax);
    tzmin = (bounds[0].z - origin.z) / direction.z;
    tzmax = (bounds[1].z - origin.z) / direction.z;
    if (direction.z < 0) swap(tzmin, tzmax);
    tmin = max(max(tmin, tymin), tzmin);
    tmax = min(min(tmax, tymax), tzmax);
    thit = tmin;
    
    // Calculate cube face normal
    float3 center = float3(0, 0, 0);
    float3 hitPosition = origin + thit * direction;
    float3 sphereNormal = normalize(hitPosition - center);
    // take the largest dimension and normalize it
    if (abs(sphereNormal.x) > abs(sphereNormal.y))
    {
        if (abs(sphereNormal.x) > abs(sphereNormal.z))
        {
            attr.normal = float3(sphereNormal.x, 0, 0);
        }
        else
        {
            attr.normal = float3(0, 0, sphereNormal.z);
        }
    }
    else
    {
        if (abs(sphereNormal.y) > abs(sphereNormal.z))
        {
            attr.normal = float3(0, sphereNormal.y, 0);
        }
        else
        {
            attr.normal = float3(0, 0, sphereNormal.z);
        }
    }
    attr.normal = mul(ObjectToWorld(),normalize(attr.normal));
    
    return tmax > tmin;
}

bool IntersectCustomPrimitiveFrontToBack(
    float3 origin, float3 direction,
    float rayTMin, float rayTMax, inout float curT,
    out ProceduralPrimitiveAttributes attr)
{
    if (InstanceIndex() == 0)
        return intersectBox(origin, direction, curT, attr);
    else
        return intersectSphere(origin, direction, curT, attr);
}

[shader("intersection")]
void MyIntersectionShader()
{
    float THit = RayTCurrent();
    ProceduralPrimitiveAttributes attr;
    if (IntersectCustomPrimitiveFrontToBack(
        ObjectRayOrigin(), ObjectRayDirection(),
        RayTMin(), RayTCurrent(), THit, attr))
    {
        ReportHit(THit, /*hitKind*/ 0, attr);
    }
}

[shader("closesthit")]
void MyClosestHitShader(inout HitData payload : SV_RayPayload, in ProceduralPrimitiveAttributes attr : SV_IntersectionAttributes)
{
    float3 hitPosition = HitWorldPosition();
#if 0
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
#else
    float3 triangleNormal = attr.normal;
#endif
    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    //payload.color = float4(0, attr.barycentrics, 1);
    payload.color = float4(attr.normal, 1);
    payload.color = color;
}

[shader("miss")]
void MyMissShader(inout HitData payload : SV_RayPayload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    payload.color = background;
}