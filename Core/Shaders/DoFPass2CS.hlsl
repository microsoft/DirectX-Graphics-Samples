//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "DoFCommon.hlsli"

Texture2D<float3> ColorBuffer : register(t0);
Texture2D<float3> PresortBuffer : register(t1);
Texture2D<float3> TileClass : register(t2);
StructuredBuffer<uint> WorkQueue : register(t3);
RWTexture2D<float3> OutputColor : register(u0);
RWTexture2D<float> OutputAlpha : register(u1);

groupshared uint gs_RG[24 * 24];    // Red, Green
groupshared uint gs_BW[24 * 24];    // Blue, Weight
groupshared uint gs_CF[24 * 24];    // CoC, FG Weight

void PrefetchPixel(int2 Corner, uint2 Offset, float FgRenormFactor, float TileMinDepth)
{
    uint ldsIdx = Offset.x + Offset.y * 24;
    uint2 st = clamp(Corner + Offset, 0, HalfDimensionMinusOne);
    float3 PreSortData = PresortBuffer[st];
    float SampleAlpha = PreSortData.y;
    float4 Color = float4(ColorBuffer[st], 1) * SampleAlpha * FgRenormFactor;
    gs_RG[ldsIdx] = f32tof16(Color.r) | f32tof16(Color.g) << 16;
    gs_BW[ldsIdx] = f32tof16(Color.b) | f32tof16(Color.w) << 16;
    float NumRings = PreSortData.x;
    float FgPercent = ForegroundPercent(PreSortData.z, TileMinDepth);
    gs_CF[ldsIdx] = f32tof16(NumRings) | f32tof16(FgPercent) << 16;
}

void AccumulateSample( uint ldsIdx, float SampleRadius, inout float4 BackgroundAccum, inout float4 ForegroundAccum )
{
    uint RG = gs_RG[ldsIdx];
    uint BW = gs_BW[ldsIdx];
    float4 SampleColor = float4(f16tof32(RG), f16tof32(RG >> 16), f16tof32(BW), f16tof32(BW >> 16));
    uint CF = gs_CF[ldsIdx];
    float CoC = f16tof32(CF);
    float Fg = f16tof32(CF >> 16);

    float Weight = saturate(1.0 - (SampleRadius - CoC));
    BackgroundAccum += SampleColor * (1 - Fg) * Weight;
    ForegroundAccum += SampleColor * Fg * Weight;
}

void AccumulateOneRing( uint ldsIdx, inout float4 Background, inout float4 Foreground )
{
    [unroll]
    for (uint i = 0; i < 8; ++i)
        AccumulateSample(ldsIdx + s_Ring1Q[i], 1, Background, Foreground);
}

void AccumulateTwoRings( uint ldsIdx, inout float4 Background, inout float4 Foreground )
{
    [unroll]
    for (uint i = 0; i < 16; ++i)
        AccumulateSample(ldsIdx + s_Ring2Q[i], 2, Background, Foreground);
}

void AccumulateThreeRings( uint ldsIdx, inout float4 Background, inout float4 Foreground )
{
    [unroll]
    for (uint i = 0; i < 24; ++i)
        AccumulateSample(ldsIdx + s_Ring3Q[i], 3, Background, Foreground);
}

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID )
{
    uint TileCoord = WorkQueue[Gid.x];
    uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);

    float TileMinDepth = TileClass[Tile].y;
    float FgRenormFactor = TileClass[Tile].z;

    int2 TileUL = Tile * 8 - 8;
    PrefetchPixel(TileUL, GTid.xy + uint2( 0,  0), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2( 0,  8), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2( 0, 16), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2( 8,  0), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2( 8,  8), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2( 8, 16), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2(16,  0), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2(16,  8), FgRenormFactor, TileMinDepth);
    PrefetchPixel(TileUL, GTid.xy + uint2(16, 16), FgRenormFactor, TileMinDepth);
    GroupMemoryBarrierWithGroupSync();

    uint2 st = Tile * 8 + GTid.xy;
    uint ldsIdx = GTid.x + GTid.y * 24 + 25 * 8;

    float RingCount = (TileClass[Tile].x - 1.0) / 5.0;

    uint RG = gs_RG[ldsIdx];
    uint BW = gs_BW[ldsIdx];
    float4 CenterColor = float4(f16tof32(RG), f16tof32(RG >> 16), f16tof32(BW), f16tof32(BW >> 16));
    float Fg = f16tof32(gs_CF[ldsIdx] >> 16);

    float4 Background = CenterColor * (1.01 - Fg);
    float4 Foreground = CenterColor * Fg;

#ifdef SUPPORT_DEBUGGING
    if (DebugMode == 4)
    {
        float Rings = clamp(RingCount, 0.0, 2.99999);
        OutputColor[st] = frac(float3(saturate(Rings), saturate(Rings - 1.0), saturate(Rings - 2.0)));
        OutputAlpha[st] = 1.0;
        return;
    }
#endif

    AccumulateOneRing(ldsIdx, Background, Foreground);

    if (RingCount > 1.0)
        AccumulateTwoRings(ldsIdx, Background, Foreground);

    if (RingCount > 2.0)
        AccumulateThreeRings(ldsIdx, Background, Foreground);

    Background.rgb /= (Background.a + 0.00001);
    Foreground.rgb /= (Foreground.a + 0.00001);

    float Alpha = saturate(Foreground.a);

#ifdef SUPPORT_DEBUGGING
    if (DebugMode == 1)
    {
        OutputColor[st] = Foreground.rgb;
        OutputAlpha[st] = 1.0;
    }
    else if (DebugMode == 2)
    {
        OutputColor[st] = Background.rgb;
        OutputAlpha[st] = 1.0;
    }
    else if (DebugMode == 3)
    {
        OutputColor[st] = Alpha.xxx;
        OutputAlpha[st] = 1.0;
    }
    else
#endif
    {
        OutputColor[st] = lerp(Background.rgb, Foreground.rgb, Alpha);
        OutputAlpha[st] = lerp(Alpha, 1.0, 0.5);
    }
}
