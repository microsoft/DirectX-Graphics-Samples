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
Texture2D<float4> g_texGBufferPositionHit : register(t2);
Texture2D<float4> g_texGBufferPositionRT : register(t3);
Texture2D<float4> g_texGBufferNormal : register(t0);


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float sampleRadius = 0.015f;
    float4 color = (float4) 0.0f;
    
#if 1
    // Polar angles
    // Get pixel position in [-1,1] from [0,1]
    static const float TwoPI = 2.f* 3.14159265f;
    float2 pixelPosition = 2.f * ((float2)DTid.xy / rngCB.dispatchDimensions - (float2)0.5);
    float distToCenter = length(pixelPosition);
    float angle = fmod(atan2(pixelPosition.y, pixelPosition.x) + TwoPI, TwoPI) * 360.f / TwoPI;
    float lineWidth = 0.01;
    float lineRadius = 1.f;
    if (distToCenter >= lineRadius - lineWidth / 2 && distToCenter <= lineRadius + lineWidth / 2 &&
        fmod (angle, 3.f) >=1.0f)
        color = 0.f;
    else
        color = 1.f;
#else
    // Stratum grid lines
    {
        float2 dist = (float2)0.5 - fmod((float2)DTid.xy * rngCB.stratums / rngCB.dispatchDimensions, 1.0);
        float2 dots = fmod((float2) DTid.xy * 32.0 / rngCB.dispatchDimensions, 1.0);
        
        if (dist.x > dist.y)
            color = dist.x > 0.46 && dots.y < 0.50 ? 0.0 : 1.0;
        else
            color = dist.y > 0.46 && dots.x < 0.50 ? 0.0 : 1.0;
        
    }
    // Grid lines
    {
        float2 dist = (float2)0.5 - fmod((float2)DTid.xy * rngCB.grid / rngCB.dispatchDimensions, 1.0);
      
        if (dist.x > dist.y)
            color = dist.x > 0.47 ? 0.5 : color;
        else
            color = dist.y > 0.47 ? 0.5 : color;

    }
#endif

    // Border
    {
        color = (min(DTid.x, DTid.y) <= 2
            || DTid.x >= rngCB.dispatchDimensions.x - 3
            || DTid.y >= rngCB.dispatchDimensions.y - 3)
            ? 1.0 : color;
    }

    // Samples
    {
        float2 pixelPosition = (float2) DTid.xy / (rngCB.dispatchDimensions - (uint2)1);
        for (uint i = 0; i < rngCB.numSamplesToShow; i++)
        {
            if (length(g_sampleSets[i + rngCB.sampleSetBase].value - pixelPosition) <= sampleRadius)
            {
                color = 0.0f;
            }
        }
    }

    g_renderTarget[rngCB.uavOffset + DTid.xy] = color;
}