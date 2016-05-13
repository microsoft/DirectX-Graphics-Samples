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
Texture2D<float3> TileClass : register(t2);
StructuredBuffer<uint> WorkQueue : register(t3);
RWTexture2D<float3> OutputColor : register(u0);
RWTexture2D<float> OutputAlpha : register(u1);

groupshared uint gs_RB[24 * 24];
groupshared float gs_Gr[24 * 24];

void PrefetchPixel(int2 Corner, uint2 Offset)
{
	uint ldsIdx = Offset.x + Offset.y * 24;
	uint2 st = clamp(Corner + Offset, 0, HalfDimensionMinusOne);
	float3 Color = ColorBuffer[st];
	gs_RB[ldsIdx] = f32tof16(Color.r) | f32tof16(Color.b) << 16;
	gs_Gr[ldsIdx] = Color.g;
}

float3 LoadColor( uint ldsIdx )
{
	uint RB = gs_RB[ldsIdx];
	return float3( f16tof32(RB), gs_Gr[ldsIdx], f16tof32(RB >> 16) );
}

float4 AccumulateOneRing( uint ldsIdx )
{
	float3 RingSamples = 0;
	[unroll]
	for (uint i = 0; i < 8; i++)
		RingSamples += LoadColor(ldsIdx + s_Ring1Q[i]);
	return float4(RingSamples, 8);
}

float4 AccumulateTwoRings( uint ldsIdx )
{
	float3 RingSamples = 0;
	[unroll]
	for (uint i = 0; i < 16; i++)
		RingSamples += LoadColor(ldsIdx + s_Ring2Q[i]);
	return float4(RingSamples, 16);
}

float4 AccumulateThreeRings( uint ldsIdx )
{
	float3 RingSamples = 0;
	[unroll]
	for (uint i = 0; i < 24; i++)
		RingSamples += LoadColor(ldsIdx + s_Ring3Q[i]);
	return float4(RingSamples, 24);
}

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID )
{
	uint TileCoord = WorkQueue[Gid.x];
	uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);

	int2 TileUL = Tile * 8 - 8;
	PrefetchPixel(TileUL, GTid.xy + uint2( 0,  0));
	PrefetchPixel(TileUL, GTid.xy + uint2( 0,  8));
	PrefetchPixel(TileUL, GTid.xy + uint2( 0, 16));
	PrefetchPixel(TileUL, GTid.xy + uint2( 8,  0));
	PrefetchPixel(TileUL, GTid.xy + uint2( 8,  8));
	PrefetchPixel(TileUL, GTid.xy + uint2( 8, 16));
	PrefetchPixel(TileUL, GTid.xy + uint2(16,  0));
	PrefetchPixel(TileUL, GTid.xy + uint2(16,  8));
	PrefetchPixel(TileUL, GTid.xy + uint2(16, 16));
	GroupMemoryBarrierWithGroupSync();

	uint2 st = Tile * 8 + GTid.xy;
	uint ldsIdx = GTid.x + GTid.y * 24 + 25 * 8;

	float3 CenterColor = LoadColor(ldsIdx);
	float TileCoC = TileClass[Tile].x;
	float RingCount = (TileCoC - 1.0) / 5.0;

	float4 Foreground = float4(CenterColor, 1);

	Foreground += saturate(RingCount) * AccumulateOneRing(ldsIdx);

	if (RingCount > 1.0)
		Foreground += saturate(RingCount - 1.0) * AccumulateTwoRings(ldsIdx);

	if (RingCount > 2.0)
		Foreground += saturate(RingCount - 2.0) * AccumulateThreeRings(ldsIdx);

	OutputColor[st] = Foreground.rgb / Foreground.w;
	OutputAlpha[st] = 1.0;
}