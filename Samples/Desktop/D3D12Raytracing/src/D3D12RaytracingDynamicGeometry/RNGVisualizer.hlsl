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
#include "RandomNumberGenerator.hlsli"
#include "RaytracingHlslCompat.h"


ConstantBuffer<RNGConstantBuffer> rngCB: register(b0);
RWTexture2D<float4> g_renderTarget : register(u0);
StructuredBuffer<AlignedUnitSquareSample2D> g_sampleSets : register(t1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
#define RNG_GPUONLY 0
#define RNG_CPUSAMPLES 1

#if RNG_CPUSAMPLES
    float sampleRadius = 0.025f;
    float4 color = (float4) 0.0f;
    float2 pixelPosition = (float2) DTid.xy / (rngCB.dispatchDimensions - (uint2)1);
    for (uint i = 0; i < rngCB.numSamples; i++)
    {
       // if (g_sampleSets[i + 9 * rngCB.seed].value.x > 0.5 && g_sampleSets[i + 9 * rngCB.seed].value.y <0.5)
        if (length(g_sampleSets[i+ 9*rngCB.seed].value - pixelPosition) <= sampleRadius)
        {
            color = (float4) 1.0f;
        }
    }
    float2 dist = (float2)0.5 - fmod((float2) DTid.xy * 3.0 / rngCB.dispatchDimensions, 1.0);
    color += max(dist.x, dist.y) > 0.45f ? 1.0 : 0.0;
    color += (min(DTid.x, DTid.y) == 0
                || DTid.x == rngCB.dispatchDimensions.x - 1
                || DTid.y == rngCB.dispatchDimensions.y - 1) 
           ? 1.0 : 0.0;

#endif 

#if RNG_GPUONLY
    uint seed = rngCB.dispatchDimensions.x * DTid.y + DTid.x + rngCB.seed;

    uint RNGState = RNG::SeedThread(seed);
    float randomValue = RNG::Random01(RNGState);

    // Write the raytraced color to the output texture.
    float4 color = (float4) randomValue;
#endif
    g_renderTarget[rngCB.uavOffset + DTid.xy] = color;
}