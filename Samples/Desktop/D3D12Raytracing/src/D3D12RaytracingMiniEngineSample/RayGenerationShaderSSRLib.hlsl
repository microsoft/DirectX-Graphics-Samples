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
#include "ModelViewerRaytracing.h"

Texture2D<float>    depth    : register(t1);
Texture2D<float3>   normals  : register(t2);

[shader("raygeneration")]
void RayGen()
{
    uint2 DTid = DispatchRaysIndex();
    float2 xy = DTid.xy + 0.5;

    // Screen position for the ray
    float2 screenPos = xy / g_dynamic.resolution * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates
    screenPos.y =  -screenPos.y;

    float2 readGBufferAt = xy;

    // Read depth and normal
    float sceneDepth = depth.Load(int3(readGBufferAt, 0));
    float3 normalData = normals.Load(int3(readGBufferAt, 0)).xyz;
    if (length(normalData) < 0.1) return;

    float3 normal = normalize(normalData);

    // Unproject into the world position using depth
    float4 unprojected = mul(g_dynamic.cameraToWorld, float4(screenPos, sceneDepth, 1));
    float3 world = unprojected.xyz / unprojected.w;

    float3 primaryRayDirection = normalize(g_dynamic.worldCameraPosition - world);

    // R
    float3 direction = normalize(-primaryRayDirection - 2 * dot(-primaryRayDirection, normal) * normal);
    float3 origin = world - primaryRayDirection * 0.1f;     // Lift off the surface a bit

    RayDesc rayDesc = { origin,
        0.0f,
        direction,
        FLT_MAX };

    RayPayload payload;
    payload.SkipShading = false;
    payload.RayHitT = FLT_MAX;
    TraceRay(g_accel, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0,0,1,0, rayDesc, payload);
}

