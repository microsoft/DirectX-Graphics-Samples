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
StructuredBuffer<uint> WorkQueue : register(t3);
RWTexture2D<float3> OutputColor : register(u0);
RWTexture2D<float> OutputAlpha : register(u1);

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID )
{
	uint TileCoord = WorkQueue[Gid.x];
	uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);

	uint2 st = Tile * 8 + GTid.xy;

	float Alpha = saturate(PresortBuffer[st].z);

	OutputColor[st] = ColorBuffer[st];
	OutputAlpha[st] = 1.0;//lerp(Alpha, 1.0, 0.75);
}