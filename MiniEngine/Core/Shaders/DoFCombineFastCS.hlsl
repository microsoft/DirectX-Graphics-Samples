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
#include "PixelPacking.hlsli"

Texture2D<float3> DoFColorBuffer : register(t0);
Texture2D<float> DoFAlphaBuffer : register(t1);
Texture2D<float3> TileClass : register(t2);
Texture2D<float> LNFullDepth : register(t3);
StructuredBuffer<uint> WorkQueue : register(t4);
#if SUPPORT_TYPED_UAV_LOADS
	RWTexture2D<float3> DstColor : register(u0);
#else
	RWTexture2D<uint> DstColor : register(u0);
#endif

[RootSignature(DoF_RootSig)]
[numthreads( 16, 16, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID )
{
	uint TileCoord = WorkQueue[Gid.x];
	uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);
	uint2 st = Tile * 16 + GTid.xy;

	float2 UV = (st + 0.5) * RcpBufferDim;
	float Depth = LNFullDepth[st];
	float3 DoFColor = DoFColorBuffer.SampleLevel(BilinearSampler, UV, 0);
	float FgAlpha = DoFAlphaBuffer.SampleLevel(BilinearSampler, UV, 0);

	float TileMinDepth = TileClass[Tile].y;
	float BgPercent = BackgroundPercent(Depth, TileMinDepth);
	float PixelBlurriness = saturate((ComputeCoC(Depth) - 1.0) / 1.5);
	float CombinedFactor = lerp(PixelBlurriness, lerp(FgAlpha, 1.0, PixelBlurriness), BgPercent);

#if SUPPORT_TYPED_UAV_LOADS
	DstColor[st] = lerp(DstColor[st], DoFColor, CombinedFactor);
#else
	DstColor[st] = Pack_R11G11B10_FLOAT(lerp(Unpack_R11G11B10_FLOAT(DstColor[st]), DoFColor, CombinedFactor));
#endif
}