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

Texture2D<float> LNDepthBuffer : register(t0);
Texture2D<float3> TileClass : register(t1);
Texture2D<float3> ColorBuffer : register(t2);
StructuredBuffer<uint> WorkQueue : register(t3);

// Half res
RWTexture2D<float3> PresortBuffer : register(u0);
RWTexture2D<float3> OutputBuffer : register(u1);

groupshared uint RGBuffer[100];
groupshared uint BWBuffer[100];
groupshared float DepthBuffer[100];

void LoadSample( out float4 Color, out float Depth, uint LsIdx )
{
    uint RG = RGBuffer[LsIdx];
    uint BW = BWBuffer[LsIdx];
    Color.r = f16tof32(RG >> 16);
    Color.g = f16tof32(RG);
    Color.b = f16tof32(BW >> 16);
    Color.w = f16tof32(BW);
    Depth = DepthBuffer[LsIdx];
}

float4 WeightByInverseLuminance( float3 Color )
{
    float Luminance = dot(Color, float3(0.212671, 0.715160, 0.072169));
    return float4(Color, 1) * rcp(Luminance * AntiSparkleFilterStrength + 1.0);
}

float ComputeSampleWeight( float SampleDepth, float CmpDepth )
{
    return 1.0 - saturate(abs(CmpDepth - SampleDepth) * RcpForegroundRange - 1.0);
}

float4 GetWeightedSample( uint LsIdx, float CenterDepth )
{
    float4 Color; float SampleDepth;
    LoadSample(Color, SampleDepth, LsIdx);
    return Color * ComputeSampleWeight(SampleDepth, CenterDepth);
}

float4 AccumulateOneRing( uint LsIdx, float CenterDepth )
{
    float4 AccumColor = 
        GetWeightedSample(LsIdx - 1,  CenterDepth) +
        GetWeightedSample(LsIdx + 1,  CenterDepth) +
        GetWeightedSample(LsIdx - 10, CenterDepth) +
        GetWeightedSample(LsIdx + 10, CenterDepth);

    AccumColor += 0.75 * (
        GetWeightedSample(LsIdx - 9,  CenterDepth) +
        GetWeightedSample(LsIdx - 11, CenterDepth) +
        GetWeightedSample(LsIdx + 9,  CenterDepth) +
        GetWeightedSample(LsIdx + 11, CenterDepth)
        );

    return AccumColor;
}

void LoadBlurriestSample( uint lsIdx, int2 st )
{
    float2 uv = st * RcpBufferDim;

    float4 Depths = LNDepthBuffer.Gather(ClampSampler, uv);
    float4 Reds = ColorBuffer.GatherRed(ClampSampler, uv);
    float4 Greens = ColorBuffer.GatherGreen(ClampSampler, uv);
    float4 Blues = ColorBuffer.GatherBlue(ClampSampler, uv);

    Depths = clamp(Depths, FocalMinDist, FocalMaxDist);
    float MaxDepth = Max4(Depths);
    DepthBuffer[lsIdx] = MaxDepth;

    float WeightX = ComputeSampleWeight(Depths.x, MaxDepth);
    float WeightY = ComputeSampleWeight(Depths.y, MaxDepth);
    float WeightZ = ComputeSampleWeight(Depths.z, MaxDepth);
    float WeightW = ComputeSampleWeight(Depths.w, MaxDepth);

    float3 Color = (
        WeightX * float3(Reds.x, Greens.x, Blues.x) +
        WeightY * float3(Reds.y, Greens.y, Blues.y) +
        WeightZ * float3(Reds.z, Greens.z, Blues.z) +
        WeightW * float3(Reds.w, Greens.w, Blues.w)
        ) / (WeightX + WeightY + WeightZ + WeightW);

    float4 WeightedColor = WeightByInverseLuminance(Color);
    RGBuffer[lsIdx] = f32tof16(WeightedColor.r) << 16 | f32tof16(WeightedColor.g);
    BWBuffer[lsIdx] = f32tof16(WeightedColor.b) << 16 | f32tof16(WeightedColor.w);
}

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint TileCoord = WorkQueue[Gid.x];
    uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);
    uint2 st = Tile * 8 + GTid.xy;

    int2 TileCorner = Tile * 16 - 1;

    LoadBlurriestSample(GI, TileCorner + uint2((GI % 10) * 2, (GI / 10) * 2));
    if (GI < 36)
    {
        uint idx = GI + 64;
        LoadBlurriestSample(idx, TileCorner + uint2((idx % 10) * 2, (idx / 10) * 2));
    }
    GroupMemoryBarrierWithGroupSync();

    uint LsIdx = GTid.x + GTid.y * 10 + 11;
    float4 Color; float Depth;
    LoadSample(Color, Depth, LsIdx);

    float CoC = ComputeCoC(Depth);

    PresortBuffer[st] = float3(max(0, (CoC - 1.0) / 5.0), SampleAlpha(CoC), Depth);

    if (CoC >= 1.0 && DisablePreFilter == 0)
        Color += saturate(CoC - 1.0) * AccumulateOneRing(LsIdx, Depth);

    OutputBuffer[st] = Color.rgb / Color.w;
}
