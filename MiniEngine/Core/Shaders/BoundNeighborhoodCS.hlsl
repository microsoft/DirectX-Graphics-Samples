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

#include "TemporalRS.hlsli"

Texture2D<float3> InputColor : register(t0);
RWTexture2D<float3> OutMin : register(u0);
RWTexture2D<float3> OutMax : register(u1);

#define BORDER_SIZE 1
#define GROUP_SIZE_X 8
#define GROUP_SIZE_Y 8
#define GROUP_SIZE (GROUP_SIZE_X * GROUP_SIZE_Y)
#define TILE_SIZE_X (GROUP_SIZE_X + 2 * BORDER_SIZE)
#define TILE_SIZE_Y (GROUP_SIZE_Y + 2 * BORDER_SIZE)
#define TILE_PIXEL_COUNT (TILE_SIZE_X * TILE_SIZE_Y)

groupshared float gs_MinR[TILE_PIXEL_COUNT];
groupshared float gs_MinG[TILE_PIXEL_COUNT];
groupshared float gs_MinB[TILE_PIXEL_COUNT];
groupshared float gs_MaxR[TILE_PIXEL_COUNT];
groupshared float gs_MaxG[TILE_PIXEL_COUNT];
groupshared float gs_MaxB[TILE_PIXEL_COUNT];

void ConvolveH( uint Idx )
{
    gs_MinR[Idx] = min(min(gs_MinR[Idx - 1], gs_MinR[Idx]), gs_MinR[Idx + 1]);
    gs_MinG[Idx] = min(min(gs_MinG[Idx - 1], gs_MinG[Idx]), gs_MinG[Idx + 1]);
    gs_MinB[Idx] = min(min(gs_MinB[Idx - 1], gs_MinB[Idx]), gs_MinB[Idx + 1]);
    gs_MaxR[Idx] = max(max(gs_MaxR[Idx - 1], gs_MaxR[Idx]), gs_MaxR[Idx + 1]);
    gs_MaxG[Idx] = max(max(gs_MaxG[Idx - 1], gs_MaxG[Idx]), gs_MaxG[Idx + 1]);
    gs_MaxB[Idx] = max(max(gs_MaxB[Idx - 1], gs_MaxB[Idx]), gs_MaxB[Idx + 1]);
}

void ConvolveV( uint Idx, uint2 st )
{
    float minR = min(min(gs_MinR[Idx - TILE_SIZE_X], gs_MinR[Idx]), gs_MinR[Idx + TILE_SIZE_X]);
    float minG = min(min(gs_MinG[Idx - TILE_SIZE_X], gs_MinG[Idx]), gs_MinG[Idx + TILE_SIZE_X]);
    float minB = min(min(gs_MinB[Idx - TILE_SIZE_X], gs_MinB[Idx]), gs_MinB[Idx + TILE_SIZE_X]);
    OutMin[st] = float3(minR, minG, minB);

    float maxR = max(max(gs_MaxR[Idx - TILE_SIZE_X], gs_MaxR[Idx]), gs_MaxR[Idx + TILE_SIZE_X]);
    float maxG = max(max(gs_MaxG[Idx - TILE_SIZE_X], gs_MaxG[Idx]), gs_MaxG[Idx + TILE_SIZE_X]);
    float maxB = max(max(gs_MaxB[Idx - TILE_SIZE_X], gs_MaxB[Idx]), gs_MaxB[Idx + TILE_SIZE_X]);
    OutMax[st] = float3(maxR, maxG, maxB);
}

[RootSignature(Temporal_RootSig)]
[numthreads( GROUP_SIZE_X, GROUP_SIZE_Y, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex )
{
    // Load tile pixels

    int2 GroupUL = Gid.xy * uint2(GROUP_SIZE_X, GROUP_SIZE_Y) - BORDER_SIZE;
    for (uint i = GI; i < TILE_PIXEL_COUNT; i += GROUP_SIZE)
    {
        float3 Color = InputColor[GroupUL + uint2(i % TILE_SIZE_X, i / TILE_SIZE_X)];
        gs_MinR[i] = gs_MaxR[i] = Color.r;
        gs_MinG[i] = gs_MaxG[i] = Color.g;
        gs_MinB[i] = gs_MaxB[i] = Color.b;
    }

    GroupMemoryBarrierWithGroupSync();

    uint Idx = (GTid.x + BORDER_SIZE) + (GTid.y + BORDER_SIZE) * TILE_SIZE_X;

    float R0 = gs_MinR[Idx], R1 = gs_MinR[Idx - 1], R2 = gs_MinR[Idx + 1], R3 = gs_MinR[Idx - TILE_SIZE_X], R4 = gs_MinR[Idx + TILE_SIZE_X];
    float minR = min(min(R0, R1), min(min(R2, R3), R4));
    float maxR = max(max(R0, R1), max(max(R2, R3), R4));

    float G0 = gs_MinG[Idx], G1 = gs_MinG[Idx - 1], G2 = gs_MinG[Idx + 1], G3 = gs_MinG[Idx - TILE_SIZE_X], G4 = gs_MinG[Idx + TILE_SIZE_X];
    float minG = min(min(G0, G1), min(min(G2, G3), G4));
    float maxG = max(max(G0, G1), max(max(G2, G3), G4));

    float B0 = gs_MinB[Idx], B1 = gs_MinB[Idx - 1], B2 = gs_MinB[Idx + 1], B3 = gs_MinB[Idx - TILE_SIZE_X], B4 = gs_MinB[Idx + TILE_SIZE_X];
    float minB = min(min(B0, B1), min(min(B2, B3), B4));
    float maxB = max(max(B0, B1), max(max(B2, B3), B4));

    OutMin[DTid.xy] = float3(minR, minG, minB);
    OutMax[DTid.xy] = float3(maxR, maxG, maxB);
}
