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
Texture2D<float4> g_texGBufferNormal : register(t4);


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

    if (length(g_sampleSets[i + rngCB.sampleSetBase].value - pixelPosition) <= sampleRadius)
    {
        color = 0.0f;
    }
    g_renderTarget[rngCB.uavOffset + DTid.xy] = color;
}