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

// Desc: Generates AO rays for each pixel and stores them in a texture.
// Supports 1 rays per pixel (spp) and 0.5 spp (via checkerboard distribution).
// 0.5 spp:
//  - generates 1 spp for every other pixel in a checkerboard distribution.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "RandomNumberGenerator.hlsli"
#include "Ray sorting/RaySorting.hlsli"
#include "RTAO.hlsli"

Texture2D<NormalDepthTexFormat> g_texRayOriginSurfaceNormalDepth : register(t0);
Texture2D<float4> g_texRayOriginPosition : register(t1);

RWTexture2D<NormalDepthTexFormat> g_rtRaysDirectionOriginDepth : register(u0);

ConstantBuffer<RayGenConstantBuffer> cb: register(b0);
StructuredBuffer<AlignedHemisphereSample3D> g_sampleSets : register(t3);

// Delay the include so that resource references resolve.
#include "RayGen.hlsli"

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID, uint2 GTid : SV_GroupThreadID)
{
    uint2 DTidFullRes = DTid;

    if (cb.doCheckerboardRayGeneration)
    {
        UINT pixelStepX = 2;
        bool isEvenPixelY = (DTid.y & 1) == 0;
        UINT pixelOffsetX = isEvenPixelY != cb.checkerboardGenerateRaysForEvenPixels;
        DTidFullRes.x = DTid.x * pixelStepX + pixelOffsetX;
    }

    float3 surfaceNormal;
    float rayOriginDepth;
    DecodeNormalDepth(g_texRayOriginSurfaceNormalDepth[DTidFullRes], surfaceNormal, rayOriginDepth);

    float3 rayDirection = 0;
    if (rayOriginDepth != INVALID_RAY_ORIGIN_DEPTH)
    {
        rayDirection = GetRandomRayDirection(DTid, surfaceNormal, cb.textureDim, 0);
    }

    g_rtRaysDirectionOriginDepth[DTid] = EncodeNormalDepth(rayDirection, rayOriginDepth);
}
