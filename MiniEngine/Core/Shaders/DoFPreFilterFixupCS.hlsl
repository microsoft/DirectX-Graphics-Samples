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

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	uint TileCoord = WorkQueue[Gid.x];
	uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);
	uint2 st = Tile * 8 + GTid.xy;

	float2 uv = (2 * st + 1) * RcpBufferDim;

	OutputBuffer[st] = ColorBuffer.SampleLevel(BilinearSampler, uv, 0);
	float Depth = LNDepthBuffer.SampleLevel(PointSampler, uv, 0);
	PresortBuffer[st] = float3(0.0, 1.0, Depth);

}