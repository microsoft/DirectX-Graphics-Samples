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

// Desc: Generates AO rays for RTAO
// Supports 1 rays per pixel (rpp) and 0.5 rpp (via checkerboard pattern) 
//
#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "RandomNumberGenerator.hlsli"
#include "RaySorting.hlsli"

Texture2D<NormalDepthTexFormat> g_texRayOriginSurfaceNormalDepth : register(t0);
Texture2D<float4> g_texRayOriginPosition : register(t1);

// ToDo use higher bit format?
RWTexture2D<NormalDepthTexFormat> g_rtRaysDirectionOriginDepth : register(u0);

ConstantBuffer<AdaptiveRayGenConstantBuffer> CB: register(b0);
StructuredBuffer<AlignedHemisphereSample3D> g_sampleSets : register(t3);

groupshared uint FrameAgeCache[DefaultComputeShaderParams::ThreadGroup::Height][DefaultComputeShaderParams::ThreadGroup::Width];

float3 GetRandomRayDirection(in uint2 srcRayIndex, in float3 surfaceNormal)
{
    // Calculate coordinate system for the hemisphere.
    // ToDo AO has square alias due to same hemisphere
    float3 u, v, w;
    w = surfaceNormal;

    // ToDo revisit this
    // Get a vector that's not parallel to w;
#if 0
    float3 right = float3(0.0072f, 0.999994132f, 0.0034f);
#else
    float3 right = 0.3f * w + float3(-0.72f, 0.56f, -0.34f);
#endif
    v = normalize(cross(w, right));
    u = cross(v, w);


    // Calculate offsets to the pregenerated sample set.
    uint sampleSetJump;     // Offset to the start of the sample set
    uint sampleJump;        // Offset to the first sample for this pixel within a sample set.
    {
        // Neighboring samples NxN share a sample set, but use different samples within a set.
        // Sharing a sample set lets the pixels in the group get a better coverage of the hemisphere 
        // than if each pixel used a separate sample set with less samples pregenerated per set.

        // Get a common sample set ID and seed shared across neighboring pixels.
        uint numSampleSetsInX = (CB.textureDim.x + CB.numPixelsPerDimPerSet - 1) / CB.numPixelsPerDimPerSet;
        uint2 sampleSetId = srcRayIndex / CB.numPixelsPerDimPerSet;

        // Get a common hitPosition to adjust the sampleSeed by. 
        // This breaks noise correlation on camera movement which otherwise results 
        // in noise pattern swimming across the screen on camera movement.
        uint2 pixelZeroId = sampleSetId * CB.numPixelsPerDimPerSet;
        float3 pixelZeroHitPosition = g_texRayOriginPosition[pixelZeroId].xyz;      // ToDo remove?
        uint sampleSetSeed = (sampleSetId.y * numSampleSetsInX + sampleSetId.x) * hash(pixelZeroHitPosition) + CB.seed;
        uint RNGState = RNG::SeedThread(sampleSetSeed);

        sampleSetJump = RNG::Random(RNGState, 0, CB.numSampleSets - 1) * CB.numSamplesPerSet;

        // Get a pixel ID within the shared set across neighboring pixels.
        uint2 pixeIDPerSet2D = srcRayIndex % CB.numPixelsPerDimPerSet;
        uint pixeIDPerSet = pixeIDPerSet2D.y * CB.numPixelsPerDimPerSet + pixeIDPerSet2D.x;

        // Randomize starting sample position within a sample set per neighbor group 
        // to break group to group correlation resulting in square alias.
        uint numPixelsPerSet = CB.numPixelsPerDimPerSet * CB.numPixelsPerDimPerSet;
        sampleJump = pixeIDPerSet + RNG::Random(RNGState, 0, numPixelsPerSet - 1);
    }

    // Load a pregenerated random sample from the sample set.
    float3 sample = g_sampleSets[sampleSetJump + (sampleJump % CB.numSamplesPerSet)].value;

    // ToDo remove unnecessary normalize()
    float3 rayDirection = normalize(sample.x * u + sample.y * v + sample.z * w);

    return rayDirection;
}


// ToDo
// Limitations:
// -    TextureDim and CsDim must be a multiple of QuadDim
// Address:
// - pixel neighborhood sampling. Carry sample set id with each pixel?
// - support 2+ spp per ray
// Comment
// - rename frameAge to numTemporalSamples?
[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID, uint2 GTid : SV_GroupThreadID)
{
    uint2 DTidFullRes = DTid;
    if (CB.doCheckerboardRayGeneration)
    {
        UINT pixelStepX = 2;
        bool isEvenPixelY = (DTid.y & 1) == 0;
        UINT pixelOffsetX = isEvenPixelY != CB.checkerboardGenerateRaysForEvenPixels;
        DTidFullRes.x = DTid.x * pixelStepX + pixelOffsetX;
    }


    float3 surfaceNormal;
    float rayOriginDepth;
    DecodeNormalDepth(g_texRayOriginSurfaceNormalDepth[DTidFullRes], surfaceNormal, rayOriginDepth);

    float3 rayDirection = 0;
    if (rayOriginDepth != INVALID_RAY_ORIGIN_DEPTH)
    {
        rayDirection = GetRandomRayDirection(DTid, surfaceNormal);
    }

    g_rtRaysDirectionOriginDepth[DTid] = EncodeNormalDepth(rayDirection, rayOriginDepth);
}
