//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#define HLSL
#include "RaytracingHlslCompat.h"

ConstantBuffer<ReduceSumCSCB> gCB: register(b0);
Texture2D<uint> g_texInput : register(t0);
RWTexture2D<uint> g_texOutput : register(u1);


// ToDo - dxc fails on
//groupshared uint gShared[ReduceSumCS::ThreadGroup::Width*ReduceSumCS::ThreadGroup::Height];
groupshared uint gShared[ReduceSumCS::ThreadGroup::Size];

// ReduceSumCS performance
//  N element loads per thread - Time: 
//		N = 1 - 41.5 us
//		N = 3 -	26.5 us	
//		N = 4 - 24.5 us
//		N = 5 - 24.5 us
//  Bandwidth: 1354 GB/s.
//  GPU: RTX 2080 Ti
//  Resolution: 2160p fullscreen
//  ThreadGroup: [16, 16]
[numthreads(ReduceSumCS::ThreadGroup::Width, ReduceSumCS::ThreadGroup::Height, 1)]
void main(
	uint3 DTid : SV_DispatchThreadID, 
	uint3 GTid : SV_GroupThreadID,
	uint GIndex: SV_GroupIndex,
	uint2 Gid : SV_GroupID)
{
	uint ThreadGroupSize = ReduceSumCS::ThreadGroup::Size;

	// Load the input data
	uint2 index = DTid.xy + uint2(Gid.x * ((ReduceSumCS::ThreadGroup::NumElementsToLoadPerThread - 1) * ReduceSumCS::ThreadGroup::Width), 0);
	UINT i = 0;
	uint sum = 0;
	while (i++ < ReduceSumCS::ThreadGroup::NumElementsToLoadPerThread)
	{
		sum += g_texInput[index].x;
		index += uint2(ReduceSumCS::ThreadGroup::Width, 0);
	}

	// Aggregate values across the wave.
	sum = WaveActiveSum(sum);

	for (UINT s = WaveGetLaneCount(); s < ThreadGroupSize; s*= WaveGetLaneCount())
	{
		// Store in shared memory and wait for all threads in group to finish.
		gShared[GIndex] = sum;		// ToDo test conditional write if (WaveIsFirstLane())
		GroupMemoryBarrierWithGroupSync();

		uint numLanesToProcess = (ThreadGroupSize + s - 1) / s;
		if (GIndex >= numLanesToProcess)
		{
			break;
		}
		// Load and aggregate values across the wave.
		sum = gShared[GIndex * WaveGetLaneCount()];
		sum = WaveActiveSum(sum);
	}

	// Write out summed result for each thread group.
	if (GIndex == 0)
	{
		g_texOutput[Gid] = sum;
	}
}