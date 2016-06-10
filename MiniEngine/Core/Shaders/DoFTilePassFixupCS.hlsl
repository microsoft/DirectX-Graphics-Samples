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

Texture2D<float3> InputClass : register(t0);
RWStructuredBuffer<uint> FixupQueue : register(u0);

groupshared float gs_MaxCoC[100];

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	if (GTid.x < 5 && GTid.y < 5)
	{
		float2 PrefetchUV = (DTid.xy + GTid.xy) * InvTiledDimension;
		float4 MaxCoCs = InputClass.GatherRed(ClampSampler, PrefetchUV);
		int destIdx = GTid.x * 2 + GTid.y * 2 * 10;
		gs_MaxCoC[destIdx   ] = MaxCoCs.w;
		gs_MaxCoC[destIdx+ 1] = MaxCoCs.z;
		gs_MaxCoC[destIdx+10] = MaxCoCs.x;
		gs_MaxCoC[destIdx+11] = MaxCoCs.y;
	}

	GroupMemoryBarrierWithGroupSync();

	if (any(DTid.xy >= TiledDimension))
		return;

	uint ulIdx = GTid.x + GTid.y * 10;

	float TileMaxCoC = gs_MaxCoC[ulIdx+11];

	float MaxCoC0 = Max3(gs_MaxCoC[ulIdx   ], gs_MaxCoC[ulIdx+ 1], gs_MaxCoC[ulIdx+ 2]);
	float MaxCoC1 = max(gs_MaxCoC[ulIdx+10], gs_MaxCoC[ulIdx+12]);
	float MaxCoC2 = Max3(gs_MaxCoC[ulIdx+20], gs_MaxCoC[ulIdx+21], gs_MaxCoC[ulIdx+22]);
	float MaxNeighborCoC = Max3(MaxCoC0, MaxCoC1, MaxCoC2);

	if (TileMaxCoC < 1.0 && MaxNeighborCoC >= 1.0)
		FixupQueue[FixupQueue.IncrementCounter()] = DTid.x | DTid.y << 16;
}