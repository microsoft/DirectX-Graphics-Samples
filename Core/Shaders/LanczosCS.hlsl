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
// Author(s):  James Stanard
//             Jack Elliott
//

#include "LanczosFunctions.hlsli"
#include "PresentRS.hlsli"
#include "ShaderUtility.hlsli"

Texture2D<float3> Source : register(t0);
RWTexture2D<float3> Dest : register(u0);

cbuffer CB0 : register(b0)
{
    float2 kRcpScale;
};

#ifndef ENABLE_FAST_PATH
  #define TILE_DIM_X 16
  #define TILE_DIM_Y 16
#endif
#define GROUP_COUNT (TILE_DIM_X * TILE_DIM_Y)

// The fast path can be enabled when the source tile plus the extra border pixels fit
// within the destination tile size.  For 16x16 destination tiles and 4 taps, you can
// upsample 13x13 tiles and smaller using the fast path.  Src/Dest <= 13/16 --> FAST
#ifdef ENABLE_FAST_PATH
  #define SAMPLES_X TILE_DIM_X
  #define SAMPLES_Y TILE_DIM_Y
#else
  #define SAMPLES_X (TILE_DIM_X + 3)
  #define SAMPLES_Y (TILE_DIM_Y + 3)
#endif

#define TOTAL_SAMPLES (SAMPLES_X * SAMPLES_Y)

// De-interleaved to avoid LDS bank conflicts
groupshared float g_R[TOTAL_SAMPLES];
groupshared float g_G[TOTAL_SAMPLES];
groupshared float g_B[TOTAL_SAMPLES];

// Store pixel to LDS (local data store)
void StoreLDS(uint LdsIdx, float3 rgb)
{
    g_R[LdsIdx] = rgb.r;
    g_G[LdsIdx] = rgb.g;
    g_B[LdsIdx] = rgb.b;
}

// Load four pixel samples from LDS.  Stride determines horizontal or vertical groups.
float3x4 LoadSamples(uint idx, uint Stride)
{
    uint i0 = idx, i1 = idx+Stride, i2 = idx+2*Stride, i3=idx+3*Stride;
    return float3x4(
        g_R[i0], g_R[i1], g_R[i2], g_R[i3],
        g_G[i0], g_G[i1], g_G[i2], g_G[i3],
        g_B[i0], g_B[i1], g_B[i2], g_B[i3]);
}

[RootSignature(Present_RootSig)]
[numthreads(TILE_DIM_X, TILE_DIM_Y, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    uint3 GTid : SV_GroupThreadID,
    uint3 Gid  : SV_GroupID,
    uint  GI   : SV_GroupIndex)
{
    // Number of samples needed from the source buffer to generate the output tile dimensions.
    const uint2 SampleSpace = ceil(float2(TILE_DIM_X, TILE_DIM_Y) * kRcpScale + 3.0);

    // Pre-Load source pixels
    int2 UpperLeft = floor((Gid.xy * uint2(TILE_DIM_X, TILE_DIM_Y) + 0.5) * kRcpScale - 1.5);
#ifdef ENABLE_FAST_PATH
    // NOTE: If bandwidth is more of a factor than ALU, uncomment this condition.
    //if (all(GTid.xy < SampleSpace))
        StoreLDS(GI, Source[UpperLeft + GTid.xy]);
#else
    for (uint i = GI; i < TOTAL_SAMPLES; i += GROUP_COUNT)
        StoreLDS(i, Source[UpperLeft + int2(i % SAMPLES_X, i / SAMPLES_Y)]);
#endif

    GroupMemoryBarrierWithGroupSync();

    // The coordinate of the top-left sample from the 4x4 kernel (offset by -0.5
    // so that whole numbers land on a pixel center.)  This is in source texture space.
    float2 TopLeftSample = (DTid.xy + 0.5) * kRcpScale - 1.5;

    // Position of samples relative to pixels used to evaluate the Sinc function.
    float2 Phase = frac(TopLeftSample);

    // LDS tile coordinate for the top-left sample (for this thread)
    uint2 TileST = int2(floor(TopLeftSample)) - UpperLeft;

    // Convolution weights, one per sample (in each dimension)
    float4 xWeights = GetUpscaleFilterWeights(Phase.x);
    float4 yWeights = GetUpscaleFilterWeights(Phase.y);

    // Horizontally convolve the first N rows
    uint ReadIdx = TileST.x + GTid.y * SAMPLES_X;
#ifdef ENABLE_FAST_PATH
    StoreLDS(GI, mul(LoadSamples(ReadIdx, 1), xWeights));
#else
    uint WriteIdx = GTid.x + GTid.y * SAMPLES_X;
    StoreLDS(WriteIdx, mul(LoadSamples(ReadIdx, 1), xWeights));

    // If the source tile plus border is larger than the destination tile, we
    // have to convolve a few more rows.
    if (GI + GROUP_COUNT < SampleSpace.y * TILE_DIM_X)
    {
        ReadIdx += TILE_DIM_Y * SAMPLES_X;
        WriteIdx += TILE_DIM_Y * SAMPLES_X;
        StoreLDS(WriteIdx, mul(LoadSamples(ReadIdx, 1), xWeights));
    }
#endif

    GroupMemoryBarrierWithGroupSync();

    // Convolve vertically N columns
    ReadIdx = GTid.x + TileST.y * SAMPLES_X;
    float3 Result = mul(LoadSamples(ReadIdx, SAMPLES_X), yWeights);

    // Transform to display settings
    Result = RemoveDisplayProfile(Result, LDR_COLOR_FORMAT);
    Dest[DTid.xy] = ApplyDisplayProfile(Result, DISPLAY_PLANE_FORMAT);
}