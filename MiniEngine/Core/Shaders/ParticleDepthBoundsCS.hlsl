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
//			Alex Nankervis 
//

#include "ParticleUtility.hlsli"

Texture2D<float>		g_Input		: register(t0);
RWTexture2D<uint>		g_Output8	: register(u0);
RWTexture2D<uint>		g_Output16	: register(u1);
RWTexture2D<uint>		g_Output32	: register(u2);

groupshared float gs_Buffer[128];

void Max4( inout float MaxZ, uint GI, uint Dx )
{
	float MM1 = gs_Buffer[GI + 1 * Dx];
	float MM2 = gs_Buffer[GI + 8 * Dx];
	float MM3 = gs_Buffer[GI + 9 * Dx];
	MaxZ = max(max(MaxZ, MM1), max(MM2, MM3));
}

uint PackMinMax( float2 MinMax )
{
	return f32tof16(MinMax.y) << 16 | f32tof16(saturate(MinMax.x - 0.001));
}

[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
	float2 UV1 = (DTid.xy * 4 + 1) * gRcpBufferDim;
	float2 UV2 = UV1 + float2(2, 0) * gRcpBufferDim;
	float2 UV3 = UV1 + float2(0, 2) * gRcpBufferDim;
	float2 UV4 = UV1 + float2(2, 2) * gRcpBufferDim;
	
	float4 ZQuad1 = g_Input.Gather(gSampPointClamp, UV1);
	float4 ZQuad2 = g_Input.Gather(gSampPointClamp, UV2);
	float4 ZQuad3 = g_Input.Gather(gSampPointClamp, UV3);
	float4 ZQuad4 = g_Input.Gather(gSampPointClamp, UV4);

	float4 MaxQuad = max(max(ZQuad1, ZQuad2), max(ZQuad3, ZQuad4));
	float4 MinQuad = min(min(ZQuad1, ZQuad2), min(ZQuad3, ZQuad4));

	float maxZ = max(max(MaxQuad.x, MaxQuad.y), max(MaxQuad.z, MaxQuad.w));
	float minZ = min(min(MinQuad.x, MinQuad.y), min(MinQuad.z, MinQuad.w));

	gs_Buffer[GI] = maxZ;
	gs_Buffer[GI + 64] = -minZ;

	GroupMemoryBarrierWithGroupSync();

	const uint This = GI * 2;
	float MaxZ = gs_Buffer[This];

	Max4(MaxZ, This, 1);
	gs_Buffer[This] = MaxZ;

	GroupMemoryBarrierWithGroupSync();

	// if (X % 2 == 0 && Y % 2 == 0 && Y < 8)
	if ((This & 0x49) == 0)	
	{
		uint2 SubTile = uint2(This >> 1, This >> 4) & 3;
		g_Output8[Gid.xy * 4 + SubTile] = PackMinMax(float2(-gs_Buffer[This + 64], MaxZ));
	}

	Max4(MaxZ, This, 2);
	gs_Buffer[This] = MaxZ;

	GroupMemoryBarrierWithGroupSync();

	// if (X % 4 == 0 && Y % 4 == 0 && Y < 8)
	if ((This & 0x5B) == 0)	
	{
		uint2 SubTile = uint2(This >> 2, This >> 5) & 1;
		g_Output16[Gid.xy * 2 + SubTile] = PackMinMax(float2(-gs_Buffer[This + 64], MaxZ));
	}

	Max4(MaxZ, This, 4);
	gs_Buffer[This] = MaxZ;

	GroupMemoryBarrierWithGroupSync();

	if (This == 0)
		g_Output32[Gid.xy] = PackMinMax(float2(-gs_Buffer[64], MaxZ));
}
