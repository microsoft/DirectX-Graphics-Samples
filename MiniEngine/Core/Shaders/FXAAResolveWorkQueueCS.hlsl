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
// Used with FXAA to resolve the lengths of the two work queues and to generate DispatchIndirect parameters.
// The work queues are also padded out to a multiple of 64 with dummy work items.
//

RWStructuredBuffer<uint4> WorkQueueH : register(u0);
RWStructuredBuffer<uint4> WorkQueueV : register(u1);
RWByteAddressBuffer  IndirectParams : register(u2);

groupshared uint PixelCountH;
groupshared uint PixelCountV;

[numthreads( 64, 1, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	if (GI == 0)
	{
		// Increment the counter so we can read the current value.
		PixelCountH = WorkQueueH.IncrementCounter();
		PixelCountV = WorkQueueV.IncrementCounter();
	}

	GroupMemoryBarrierWithGroupSync();

	uint PaddedCountH = (PixelCountH + 63) & ~63;
	uint PaddedCountV = (PixelCountV + 63) & ~63;

	// Write out padding to the buffer
	if (GI + PixelCountH < PaddedCountH)
		WorkQueueH[PixelCountH + GI] = uint4(0xffffffff, 0xffffffff, 0, 0xffffffff);

	// Write out padding to the buffer
	if (GI + PixelCountV < PaddedCountV)
		WorkQueueV[PixelCountV + GI] = uint4(0xffffffff, 0xffffffff, 0, 0xffffffff);

	if (GI == 0)
	{
		IndirectParams.Store(0 , PaddedCountH >> 6);
		IndirectParams.Store(12, PaddedCountV >> 6);
	}
}