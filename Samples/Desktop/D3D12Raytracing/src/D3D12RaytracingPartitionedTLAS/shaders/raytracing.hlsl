//*********************************************************
//
// Copyright (c) 2022-2026, NVIDIA CORPORATION. All rights reserved.
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// Rendering adapts vk_partitioned_tlas ray-tracing shaders. RNG/hash, basis and
// simple-sky helpers/settings adapt nvpro_core2/nvshaders implementations.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

RaytracingAccelerationStructure Scene : register(t0);
ByteAddressBuffer DynamicPartitions : register(t1);
ByteAddressBuffer ActivePartitions : register(t2);
ByteAddressBuffer PhysicsState : register(t3);
ByteAddressBuffer TriangleData : register(t4);
// RayGen output contract (see compositing.hlsl for subsequent channel reuse):
// TraceImage: secondary lighting RGB (sky on a miss), ambient occlusion in A.
// PrimaryImage: direct lighting RGB, primaryHitId bit-cast to float in A.
// DepthImage: primary ray distance in R. PickResult uses the application InstanceID.
RWTexture2D<float4> TraceImage : register(u0);
RWTexture2D<float4> PrimaryImage : register(u1);
RWTexture2D<float4> DepthImage : register(u2);
RWByteAddressBuffer PickResult : register(u3);

#define HLSL 1
#include "RaytracingHlslCompat.h"
ConstantBuffer<FrameConstants> Frame : register(b0);

// Adapted from nvpro_core2/nvshaders/random.h.slang; also used in the Vulkan sample.
// PCG: https://www.pcg-random.org/index.html
uint pcg(inout uint state)
{
    uint value = state * 747796405u + 2891336453u;
    uint word = ((value >> ((value >> 28u) + 4u)) ^ value) * 277803737u;
    state = value;
    return (word >> 22u) ^ word;
}

// Origin references retained from nvpro_core2/nvshaders/random.h.slang:
// https://github.com/Cyan4973/xxHash and https://www.shadertoy.com/view/XlGcRh.
uint xxhash32(uint3 value)
{
    const uint4 primes = uint4(2246822519u, 3266489917u, 668265263u, 374761393u);
    uint hash = value.z + primes.w + value.x * primes.y;
    hash = primes.z * ((hash << 17) | (hash >> 15));
    hash += value.y * primes.y;
    hash = primes.z * ((hash << 17) | (hash >> 15));
    hash = primes.x * (hash ^ (hash >> 15));
    hash = primes.y * (hash ^ (hash >> 13));
    return hash ^ (hash >> 16);
}

float3 partitionColor(uint partition)
{
    // Map the global API sentinel to the palette entry after the spatial grid.
    if(partition == 0xffffffffu)
        partition = Frame.PartitionAxis * Frame.PartitionAxis;
    uint hash = partition * 747796405u + 2891336453u;
    hash = ((hash >> ((hash >> 28) + 4)) ^ hash) * 277803737u;
    hash = (hash >> 22) ^ hash;
    return float3(hash & 255u, (hash >> 8) & 255u, (hash >> 16) & 255u) / 255.0;
}

float3 desaturate(float3 color, float amount)
{
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    return lerp(color, gray.xxx, amount);
}

float3 schlickFresnel(float3 f0, float3 f90, float viewDotHalf)
{
    return f0 + (f90 - f0) * pow(1.0 - saturate(viewDotHalf), 5.0);
}

float3 ggxEvaluate(float3 viewDirection, float3 normal, float3 lightDirection, float3 albedo)
{
    const float metallic = Frame.MaterialParameters.x;
    const float roughness = Frame.MaterialParameters.y;
    float3 halfVector = normalize(lightDirection + viewDirection);
    float nDotL = saturate(dot(normal, lightDirection));
    float nDotV = saturate(dot(normal, viewDirection));
    float nDotH = saturate(dot(normal, halfVector));
    float vDotH = saturate(dot(viewDirection, halfVector));
    float alphaSquared = roughness * roughness;
    float distributionDenominator = nDotH * nDotH * (alphaSquared - 1.0) + 1.0;
    float distribution = alphaSquared /
        (3.14159265 * distributionDenominator * distributionDenominator);
    float ggxV = nDotL * sqrt(nDotV * nDotV * (1.0 - alphaSquared) + alphaSquared);
    float ggxL = nDotV * sqrt(nDotL * nDotL * (1.0 - alphaSquared) + alphaSquared);
    float visibility = 0.5 / max(ggxV + ggxL, 0.001);
    float3 fresnel = schlickFresnel(lerp(0.04.xxx, albedo, metallic), 1.0.xxx, vDotH);
    float3 glossy = fresnel * (visibility * distribution * nDotL);
    float3 diffuse = albedo * schlickFresnel(0.96.xxx, 0.0.xxx, vDotH)
        * ((1.0 - metallic) * nDotL / 3.14159265);
    return glossy + diffuse;
}

bool occluded(float3 origin, float3 direction, float maxDistance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.001;
    ray.TMax = maxDistance;
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> query;
    query.TraceRayInline(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, ray);
    while (query.Proceed()) {}
    return query.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}

float3 secondaryColor(float3 origin, float3 direction, float minimumDistance, uint depth)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = minimumDistance;
    ray.TMax = 100.0;
    RayPayload secondary;
    secondary.color = 0;
    secondary.normal = 0;
    secondary.position = 0;
    secondary.hit = 0;
    secondary.primaryHitId = 255u;
    secondary.primaryDepth = ray.TMax;
    secondary.instanceId = 0xffffffffu;
    secondary.depth = depth;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, 0, 0, 0, ray, secondary);
    return secondary.color;
}

[shader("raygeneration")]
void RayGen()
{
    uint2 pixel = DispatchRaysIndex().xy;
    uint2 size = DispatchRaysDimensions().xy;
    float2 uv = (float2(pixel) + 0.5) / float2(size);
    float2 ndc = float2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
    float4 farPoint = mul(Frame.InvViewProjection, float4(ndc, 1.0, 1.0));
    farPoint.xyz /= farPoint.w;

    RayDesc ray;
    ray.Origin = Frame.CameraPosition.xyz;
    ray.Direction = normalize(farPoint.xyz - Frame.CameraPosition.xyz);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload payload;
    payload.color = 0;
    payload.normal = 0;
    payload.position = 0;
    payload.hit = 0;
    payload.primaryHitId = 255u;
    payload.primaryDepth = ray.TMax;
    payload.instanceId = 0xffffffffu;
    payload.depth = 0;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, 0, 0, 0, ray, payload);
    if(Frame.PickActive != 0 && all(pixel == uint2(Frame.PickX, Frame.PickY)))
        PickResult.Store(0, payload.instanceId);
    float3 color = payload.color;
    float ao = 1.0;
    if (payload.hit != 0)
    {
        float3 sunDirection = normalize(Frame.SunDirection.xyz);
        float diffuse = saturate(dot(payload.normal, sunDirection));
        float3 origin = payload.position;
        bool shadowed = diffuse > 0.0 && occluded(origin, sunDirection, 10000.0);

        // nvpro_core2 orthonormalBasis: Nelson Max, "Improved accuracy when
        // building an orthonormal basis", https://jcgt.org/published/0006/01/02.
        float3 tangent, bitangent;
        if(payload.normal.z < -0.99998796)
        {
            tangent = float3(0, -1, 0);
            bitangent = float3(-1, 0, 0);
        }
        else
        {
            float basisA = 1.0 / (1.0 + payload.normal.z);
            float basisB = -payload.normal.x * payload.normal.y * basisA;
            tangent = float3(1.0 - payload.normal.x * payload.normal.x * basisA,
                             basisB, -payload.normal.x);
            bitangent = float3(basisB, 1.0 - payload.normal.y * payload.normal.y * basisA,
                               -payload.normal.y);
        }
        ao = 0.0;
        uint seed = pixel.y * size.x + pixel.x;
        [unroll] for (uint sample = 0; sample < 2; ++sample)
        {
            float radius = sqrt(float(pcg(seed)) / 4294967295.0);
            float azimuth = (float(pcg(seed)) / 4294967295.0) * 6.2831853;
            float3 hemisphere = payload.normal * sqrt(max(0.0, 1.0 - radius * radius))
                + radius * (cos(azimuth) * tangent + sin(azimuth) * bitangent);
            ao += occluded(origin, hemisphere, 5.0) ? 0.0 : 1.0;
        }
        ao = min(0.99, max(0.2, ao * 0.5));
        color = ggxEvaluate(-ray.Direction, payload.normal, sunDirection, color) * Frame.MaterialParameters.z;
        if(shadowed)
            color *= payload.hit == 2u ? 0.7 : 0.3;
        float3 reflected = reflect(ray.Direction, payload.normal);
        float3 secondary = Frame.MaterialParameters.x * secondaryColor(payload.position, reflected, 0.0001, 1);
        uint kind = (payload.instanceId >> 21) & 3u;
        bool isDomino = (payload.instanceId & 0x800000u) == 0;
        float alpha = !isDomino && kind == 1u ? 0.4 : 1.0;
        if((isDomino && (payload.instanceId + Frame.StaticInstanceCount) % 10u == 0u) || alpha < 1.0)
        {
            float3 refractNormal = payload.normal;
            float phase = Frame.FrameIndex / 50.0;
            refractNormal.x += 0.01 * sin(100.0 * payload.position.x + phase)
                * cos(320.0 * payload.position.z + phase);
            refractNormal.y += 0.01 * cos(237.0 * payload.position.y + phase)
                * sin(20.0 * payload.position.x + phase);
            refractNormal.z += 0.01 * sin(75.0 * payload.position.z + phase)
                * cos(52.0 * payload.position.y + phase);
            float3 direction = refract(ray.Direction, normalize(refractNormal), 1.2);
            if(dot(direction, direction) > 1e-8)
                secondary = secondary * 0.2 + (alpha < 1.0 ? alpha : Frame.MaterialParameters.x)
                    * secondaryColor(payload.position, direction, 0.001, 1);
        }
        TraceImage[pixel] = float4(secondary, ao);
        PrimaryImage[pixel] = float4(color, asfloat(payload.primaryHitId));
    }
    else
    {
        TraceImage[pixel] = float4(color, 1.0);
        PrimaryImage[pixel] = float4(0.0, 0.0, 0.0, asfloat(255u));
    }
    DepthImage[pixel] = float4(payload.primaryDepth, 0.0, 0.0, 0.0);
}

// Simple sky evaluation/defaults adapt nvpro_core2/nvshaders/sky_functions.h.slang
// and sky_io.h.slang. See THIRD_PARTY_NOTICES.md for the pinned sources.
[shader("miss")]
void Miss(inout RayPayload payload)
{
    float3 direction = WorldRayDirection();
    float elevation = asin(clamp(direction.y, -1.0, 1.0));
    float top = smoothstep(0.0, 0.5, elevation);
    float bottom = smoothstep(0.0, 0.5, -elevation);
    float3 environment = lerp(lerp(float3(0.50, 0.70, 0.92),
                                   float3(0.62, 0.59, 0.55), bottom),
                              float3(0.17, 0.37, 0.65), top);
    float angleToSun = acos(clamp(dot(direction, normalize(Frame.SunDirection.xyz)), 0.0, 1.0));
    float glowInput = saturate(2.0 * (1.0 - smoothstep(0.059 * 0.5 - 0.091,
                                                    0.059 * 0.5 + 0.091, angleToSun)));
    payload.color = environment + 0.9 * pow(glowInput, 4.0);
    payload.hit = 0;
    payload.primaryHitId = 255u;
    payload.primaryDepth = 10000.0;
    payload.instanceId = 0xffffffffu;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    if(payload.depth >= uint(Frame.MaterialParameters.w) - 1u)
    {
        payload.color = 0;
        return;
    }
    // CreateSceneAndTopLevels encodes a 24-bit application ID:
    // bit 23 clear = domino index; bit 23 set = static object,
    // bits 22:21 = kind (plane, icosahedron, tile, tetrahedron),
    // bits 20:0 = static index. PTLAS InstanceIndex is a separate stable slot.
    uint id = InstanceID();
    float3 tint = float3(0.5, 0.5, 0.5);
    if ((id & 0x800000u) != 0)
    {
        uint kind = (id >> 21) & 3u;
        if (kind == 0u) tint = float3(0.7, 0.7, 0.7);
        if (kind == 1u) tint = float3(0.4, 0.7, 0.4);
    }
    bool activePartition = false;
    uint activePartitionId = 0;
    bool colorRed = false;
    if((id & 0x800000u) == 0)
    {
        uint firstContact = PhysicsState.Load(id * 96 + 84);
        colorRed = firstContact != 0xffffffffu && Frame.FrameIndex - firstContact < 10u;
        if(colorRed)
            tint = float3(1.0, 0.0, 0.0);
    }
    if (Frame.ShowPartitionColors != 0)
    {
        if ((id & 0x800000u) == 0)
        {
            uint partition = DynamicPartitions.Load(id * 4);
            if(!colorRed)
                tint = partitionColor(partition);
        }
        else
        {
            float3x4 objectToWorld = ObjectToWorld3x4();
            float2 center = float2(objectToWorld[0][3], objectToWorld[2][3]);
            uint2 cell = min((uint2)clamp(floor((center + Frame.SceneExtent) *
                (float(Frame.PartitionAxis) / (2.0 * Frame.SceneExtent))), 0.0, float(Frame.PartitionAxis - 1)),
                Frame.PartitionAxis - 1);
            uint partition = cell.x + cell.y * Frame.PartitionAxis;
            activePartitionId = partition;
            float3 color = partitionColor(partition) * 0.7;
            uint lastActiveFrame = ActivePartitions.Load(partition * 4);
            activePartition = lastActiveFrame != 0u && Frame.FrameIndex - lastActiveFrame < 10u;
            tint = activePartition ? color : lerp(tint, desaturate(color, 0.8), 0.4);
            // Match the Vulkan sample: desaturate static scenery even in active
            // partitions so the moving dominoes remain easier to distinguish.
            tint = lerp(tint, desaturate(color, 0.8), 0.4);
        }
    }
    payload.color = tint;
    uint normalOffset = 0;
    if ((id & 0x800000u) != 0)
    {
        // Triangle starts in BuildGeometryAndBLAS's concatenated metadata:
        // domino 0 (12 triangles), plane 12 (200), icosahedron 212 (20),
        // tile 232 (12), tetrahedron 244 (4). Keep these offsets and the CPU
        // append order in sync when changing mesh topology or plane subdivisions.
        const uint offsets[4] = {12, 212, 232, 244};
        normalOffset = offsets[(id >> 21) & 3u];
    }
    // Each triangle has three float3s: shading normal, edge1, edge2 (36 bytes).
    uint normalAddress = (normalOffset + PrimitiveIndex()) * 36;
    float3 faceNormal = asfloat(TriangleData.Load3(normalAddress));
    float3 edge1 = asfloat(TriangleData.Load3(normalAddress + 12));
    float3 edge2 = asfloat(TriangleData.Load3(normalAddress + 24));
    float3 geometricNormal = normalize(cross(edge1, edge2));
    geometricNormal = normalize(mul(geometricNormal, (float3x3)WorldToObject3x4()));
    payload.normal = normalize(mul(faceNormal, (float3x3)WorldToObject3x4()));
    if (dot(payload.normal, WorldRayDirection()) > 0.0)
        payload.normal *= -1.0;
    payload.position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    payload.hit = activePartition ? 2u : 1u;
    uint normalHash = xxhash32(asuint(geometricNormal));
    // A lossy 8-bit grouping key lets the compositor smooth AO and detect edges.
    // Misses also use 255; picking uses the separate full instanceId field.
    payload.primaryHitId = (normalHash + (activePartition ? activePartitionId : 0u)) & 255u;
    payload.primaryDepth = RayTCurrent();
    payload.instanceId = id;
    if(payload.depth != 0u)
    {
        float3 incoming = WorldRayDirection();
        float3 light = normalize(Frame.SunDirection.xyz);
        float3 direct = ggxEvaluate(-incoming, payload.normal, light, tint) * Frame.MaterialParameters.z;
        if(occluded(payload.position, light, 10000.0))
            direct *= activePartition ? 0.7 : 0.3;
        float3 reflected = reflect(incoming, payload.normal);
        float3 indirect = Frame.MaterialParameters.x * secondaryColor(payload.position, reflected, 0.0001,
                                                payload.depth + 1u);
        uint kind = (id >> 21) & 3u;
        bool isDomino = (id & 0x800000u) == 0u;
        float alpha = !isDomino && kind == 1u ? 0.4 : 1.0;
        if((isDomino && (id + Frame.StaticInstanceCount) % 10u == 0u) || alpha < 1.0)
        {
            float3 refractNormal = payload.normal;
            float phase = Frame.FrameIndex / 50.0;
            refractNormal.x += 0.01 * sin(100.0 * payload.position.x + phase)
                * cos(320.0 * payload.position.z + phase);
            refractNormal.y += 0.01 * cos(237.0 * payload.position.y + phase)
                * sin(20.0 * payload.position.x + phase);
            refractNormal.z += 0.01 * sin(75.0 * payload.position.z + phase)
                * cos(52.0 * payload.position.y + phase);
            float3 direction = refract(incoming, normalize(refractNormal), 1.2);
            if(dot(direction, direction) > 1e-8)
                indirect = indirect * 0.2 + (alpha < 1.0 ? alpha : Frame.MaterialParameters.x)
                    * secondaryColor(payload.position, direction, 0.001, payload.depth + 1u);
        }
        payload.color = direct + indirect;
    }
}
