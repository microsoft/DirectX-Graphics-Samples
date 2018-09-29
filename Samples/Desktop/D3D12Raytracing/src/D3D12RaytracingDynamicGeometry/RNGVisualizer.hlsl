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
    uint seed = rngCB.dispatchDimensions.x * DTid.y + DTid.x + rngCB.seed;

    uint RNGState = RNG::SeedThread(seed);
    float randomValue = RNG::Random01(RNGState);

    // Write the raytraced color to the output texture.
    float4 color = (float4) randomValue;
    g_renderTarget[rngCB.uavOffset + DTid.xy] = color;
}