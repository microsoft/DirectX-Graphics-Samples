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

#include "ShaderUtility.hlsli"
#include "TemporalRS.hlsli"

Texture2D<float4> TemporalColor : register(t0);
RWTexture2D<float3> OutColor : register(u0);

SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1);

cbuffer InlineConstants : register(b0)
{
    float WA, WB;
}

#define BORDER_SIZE 1
#define GROUP_SIZE_X 8
#define GROUP_SIZE_Y 8
#define GROUP_SIZE (GROUP_SIZE_X * GROUP_SIZE_Y)
#define TILE_SIZE_X (GROUP_SIZE_X + 2 * BORDER_SIZE)
#define TILE_SIZE_Y (GROUP_SIZE_Y + 2 * BORDER_SIZE)
#define TILE_PIXEL_COUNT (TILE_SIZE_X * TILE_SIZE_Y)

groupshared float gs_R[TILE_PIXEL_COUNT];
groupshared float gs_G[TILE_PIXEL_COUNT];
groupshared float gs_B[TILE_PIXEL_COUNT];
groupshared float gs_W[TILE_PIXEL_COUNT];

float3 LoadSample(uint ldsIndex)
{
    return float3(gs_R[ldsIndex], gs_G[ldsIndex], gs_B[ldsIndex]);
}

[RootSignature(Temporal_RootSig)]
[numthreads( GROUP_SIZE_X, GROUP_SIZE_Y, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex )
{
    int2 GroupUL = Gid.xy * uint2(GROUP_SIZE_X, GROUP_SIZE_Y) - BORDER_SIZE;
    for (uint i = GI; i < TILE_PIXEL_COUNT; i += GROUP_SIZE)
    {
        int2 ST = GroupUL + int2(i % TILE_SIZE_X, i / TILE_SIZE_X);
        float4 Color = TemporalColor[ST];
        Color.rgb = log2(1.0 + Color.rgb / max(Color.w, 1e-6));
        gs_R[i] = Color.r;
        gs_G[i] = Color.g;
        gs_B[i] = Color.b;
        gs_W[i] = Color.w;
    }

    GroupMemoryBarrierWithGroupSync();

    uint ldsIndex = (GTid.x + BORDER_SIZE) + (GTid.y + BORDER_SIZE) * TILE_SIZE_X;

    float3 Center = LoadSample(ldsIndex);
    float3 Neighbors = LoadSample(ldsIndex - 1) + LoadSample(ldsIndex + 1) +
        LoadSample(ldsIndex - TILE_SIZE_X) + LoadSample(ldsIndex + TILE_SIZE_X);

    // If the temporal weight is less than 0.5, it might be a brand new pixel.  Brand new pixels
    // have not been antialiased at all and can be jarring.  Here we change the weights to actually
    // blur rather than sharpen those pixels.
    float TemporalWeight = gs_W[ldsIndex];
    float CenterWeight = TemporalWeight <= 0.5 ? 0.5 : WA;
    float LateralWeight = TemporalWeight <= 0.5 ? 0.125 : WB;

    OutColor[DTid.xy] = exp2(max(0, WA * Center - WB * Neighbors)) - 1.0;
}
