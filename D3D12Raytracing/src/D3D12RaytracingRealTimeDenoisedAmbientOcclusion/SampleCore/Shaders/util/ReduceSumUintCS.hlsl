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

Texture2D<uint> g_input : register(t0);
RWTexture2D<uint> g_output : register(u0);


groupshared uint gShared[ReduceSumCS::ThreadGroup::Size];

// Reduce sum kernel
// - Sums values from a 2D input across a group and writes out the result once per group.
// ReduceSumCS performance
// Ref: http://on-demand.gputechconf.com/gtc/2010/presentations/S12312-DirectCompute-Pre-Conference-Tutorial.pdf
//  N [uint] element loads per thread - 1080p | 4K [us gpu time]: 
//		N = 1:   44   | 114.5 us
//		N = 4:   23.2 | 70 us
//		N = 8:   23.5 | 42.5 us
//		N = 10:  23	  | 42.5 us
//  Bandwidth: 361 | 781 GB/s.
//  GPU: RTX 2080 Ti
//  ThreadGroup: [8, 16]
[numthreads(ReduceSumCS::ThreadGroup::Width, ReduceSumCS::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID, uint GIndex: SV_GroupIndex, uint2 Gid : SV_GroupID)
{
	uint ThreadGroupSize = ReduceSumCS::ThreadGroup::Size;
	uint sum = 0;

	// Load the input data.
	uint2 index = DTid + uint2(Gid.x * ((ReduceSumCS::ThreadGroup::NumElementsToLoadPerThread - 1) * ReduceSumCS::ThreadGroup::Width), 0);
	for (UINT i = 0; i < ReduceSumCS::ThreadGroup::NumElementsToLoadPerThread; i++)
	{
		sum += g_input[index];
		index += uint2(ReduceSumCS::ThreadGroup::Width, 0);
	}

	// Aggregate values across the wave.
	sum = WaveActiveSum(sum);

	// Aggregate values across the whole group.
	for (UINT s = WaveGetLaneCount(); s < ThreadGroupSize; s*= WaveGetLaneCount())
	{
		// Store the value in the group shared memory and wait for all threads in group to finish.
		gShared[GIndex] = sum;
		GroupMemoryBarrierWithGroupSync();

		// Let through only so many lanes as there are unique sum values from the last store.
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
		g_output[Gid] = sum;
	}
}