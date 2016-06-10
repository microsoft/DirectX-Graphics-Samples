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
RWTexture2D<float3> TileClass : register(u0);
RWStructuredBuffer<uint> WorkQueue : register(u1);
RWStructuredBuffer<uint> FastQueue : register(u2);

groupshared float gs_MaxCoC[100];
groupshared float gs_MinDepth[100];
groupshared float gs_MaxDepth[100];

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	if (GTid.x < 5 && GTid.y < 5)
	{
		float2 PrefetchUV = (DTid.xy + GTid.xy) * InvTiledDimension;
		float4 MaxCoCs = InputClass.GatherRed(ClampSampler, PrefetchUV);
		float4 MinDepths = InputClass.GatherGreen(ClampSampler, PrefetchUV);
		float4 MaxDepths = InputClass.GatherBlue(ClampSampler, PrefetchUV);
		int destIdx = GTid.x * 2 + GTid.y * 2 * 10;
		gs_MaxCoC[destIdx   ] = MaxCoCs.w;
		gs_MaxCoC[destIdx+ 1] = MaxCoCs.z;
		gs_MaxCoC[destIdx+10] = MaxCoCs.x;
		gs_MaxCoC[destIdx+11] = MaxCoCs.y;
		gs_MinDepth[destIdx   ] = MinDepths.w;
		gs_MinDepth[destIdx+ 1] = MinDepths.z;
		gs_MinDepth[destIdx+10] = MinDepths.x;
		gs_MinDepth[destIdx+11] = MinDepths.y;
		gs_MaxDepth[destIdx   ] = MaxDepths.w;
		gs_MaxDepth[destIdx+ 1] = MaxDepths.z;
		gs_MaxDepth[destIdx+10] = MaxDepths.x;
		gs_MaxDepth[destIdx+11] = MaxDepths.y;
	}

	GroupMemoryBarrierWithGroupSync();

	if (any(DTid.xy >= TiledDimension))
		return;

	uint ulIdx = GTid.x + GTid.y * 10;

	float MaxCoC0 = Max3(gs_MaxCoC[ulIdx   ], gs_MaxCoC[ulIdx+ 1], gs_MaxCoC[ulIdx+ 2]);
	float MaxCoC1 = Max3(gs_MaxCoC[ulIdx+10], gs_MaxCoC[ulIdx+11], gs_MaxCoC[ulIdx+12]);
	float MaxCoC2 = Max3(gs_MaxCoC[ulIdx+20], gs_MaxCoC[ulIdx+21], gs_MaxCoC[ulIdx+22]);
	float FinalMaxCoC = Max3(MaxCoC0, MaxCoC1, MaxCoC2);

	float MinDepth0 = Min3(gs_MinDepth[ulIdx   ], gs_MinDepth[ulIdx+ 1], gs_MinDepth[ulIdx+ 2]);
	float MinDepth1 = Min3(gs_MinDepth[ulIdx+10], gs_MinDepth[ulIdx+11], gs_MinDepth[ulIdx+12]);
	float MinDepth2 = Min3(gs_MinDepth[ulIdx+20], gs_MinDepth[ulIdx+21], gs_MinDepth[ulIdx+22]);
	float FinalMinDepth = Min3(MinDepth0, MinDepth1, MinDepth2);
	FinalMinDepth = clamp(FinalMinDepth, FocalMinDist, FocalMaxDist);

	float MaxDepth0 = Max3(gs_MaxDepth[ulIdx   ], gs_MaxDepth[ulIdx+ 1], gs_MaxDepth[ulIdx+ 2]);
	float MaxDepth1 = Max3(gs_MaxDepth[ulIdx+10], gs_MaxDepth[ulIdx+11], gs_MaxDepth[ulIdx+12]);
	float MaxDepth2 = Max3(gs_MaxDepth[ulIdx+20], gs_MaxDepth[ulIdx+21], gs_MaxDepth[ulIdx+22]);
	float FinalMaxDepth = Max3(MaxDepth0, MaxDepth1, MaxDepth2);
	FinalMaxDepth = clamp(FinalMaxDepth, FocalMinDist, FocalMaxDist);

	float FgAlphaNormalizationTerm = ComputeRenormalizationFactor( FinalMinDepth, FinalMaxCoC );

	TileClass[DTid.xy] = float3(FinalMaxCoC, FinalMinDepth, FgAlphaNormalizationTerm);

	if (FinalMaxCoC >= 1.0)
	{
		if (FinalMaxDepth - FinalMinDepth > ForegroundRange)
			WorkQueue[WorkQueue.IncrementCounter()] = DTid.x | DTid.y << 16;
		else
			FastQueue[FastQueue.IncrementCounter()] = DTid.x | DTid.y << 16;
	}
}