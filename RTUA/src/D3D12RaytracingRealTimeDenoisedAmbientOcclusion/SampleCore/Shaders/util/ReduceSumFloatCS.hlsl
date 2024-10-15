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

Texture2D<float> g_input : register(t0);
RWTexture2D<float> g_output : register(u0);

groupshared float gShared[ReduceSumCS::ThreadGroup::Size];

// Reduce sum kernel
// Sums values from a 2D input across a group and writes out the result once per group.
[numthreads(ReduceSumCS::ThreadGroup::Width, ReduceSumCS::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID, uint GIndex: SV_GroupIndex, uint2 Gid : SV_GroupID)
{
	uint ThreadGroupSize = ReduceSumCS::ThreadGroup::Size;
	float sum = 0;

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