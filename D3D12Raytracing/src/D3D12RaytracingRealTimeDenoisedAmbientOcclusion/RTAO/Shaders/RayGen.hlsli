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
#ifndef RAYGEN_HLSLI
#define RAYGEN_HLSLI

#include "RaytracingShaderHelper.hlsli"

float3 GetRandomRayDirection(in uint2 srcRayIndex, in float3 surfaceNormal, in uint2 textureDim, in uint raySampleIndexOffset)
{
    // Calculate coordinate system for the hemisphere.
    float3 u, v, w;
    w = surfaceNormal;

    // Get a vector that's not parallel to w.
    float3 right = 0.3f * w + float3(-0.72f, 0.56f, -0.34f);
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
        uint numSampleSetsInX = (textureDim.x + cb.numPixelsPerDimPerSet - 1) / cb.numPixelsPerDimPerSet;
        uint2 sampleSetId = srcRayIndex / cb.numPixelsPerDimPerSet;

        // Get a common hitPosition to adjust the sampleSeed by. 
        // This breaks noise correlation on camera movement which otherwise results 
        // in noise pattern swimming across the screen on camera movement.
        uint2 pixelZeroId = sampleSetId * cb.numPixelsPerDimPerSet;
        float3 pixelZeroHitPosition = g_texRayOriginPosition[pixelZeroId].xyz; 
        uint sampleSetSeed = (sampleSetId.y * numSampleSetsInX + sampleSetId.x) * hash(pixelZeroHitPosition) + cb.seed;
        uint RNGState = RNG::SeedThread(sampleSetSeed);

        sampleSetJump = RNG::Random(RNGState, 0, cb.numSampleSets - 1) * cb.numSamplesPerSet;

        // Get a pixel ID within the shared set across neighboring pixels.
        uint2 pixeIDPerSet2D = srcRayIndex % cb.numPixelsPerDimPerSet;
        uint pixeIDPerSet = pixeIDPerSet2D.y * cb.numPixelsPerDimPerSet + pixeIDPerSet2D.x;

        // Randomize starting sample position within a sample set per neighbor group 
        // to break group to group correlation resulting in square alias.
        uint numPixelsPerSet = cb.numPixelsPerDimPerSet * cb.numPixelsPerDimPerSet;
        sampleJump = pixeIDPerSet + RNG::Random(RNGState, 0, numPixelsPerSet - 1) + raySampleIndexOffset;
    }

    // Load a pregenerated random sample from the sample set.
    float3 sample = g_sampleSets[sampleSetJump + (sampleJump % cb.numSamplesPerSet)].value;
    float3 rayDirection = normalize(sample.x * u + sample.y * v + sample.z * w);

    return rayDirection;
}

#endif // RAYGEN_HLSLI