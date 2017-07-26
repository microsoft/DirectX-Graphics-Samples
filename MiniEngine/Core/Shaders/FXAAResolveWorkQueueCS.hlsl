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

#include "FXAARootSignature.hlsli"

RWByteAddressBuffer IndirectParams : register(u0);
RWByteAddressBuffer WorkQueue : register(u1);
RWByteAddressBuffer WorkCounts : register(u2);

[RootSignature(FXAA_RootSig)]
[numthreads( 64, 1, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    uint2 PixelCounts = WorkCounts.Load2(0);

    // Write out padding to the buffer
    uint PixelCountH = PixelCounts.x;
    uint PaddedCountH = (PixelCountH + 63) & ~63;
    if (GI + PixelCountH < PaddedCountH)
        WorkQueue.Store(PixelCountH + GI, 0xffffffff);

    // Write out padding to the buffer
    uint PixelCountV = PixelCounts.y;
    uint PaddedCountV = (PixelCountV + 63) & ~63;
    if (GI + PixelCountV < PaddedCountV)
        WorkQueue.Store(LastQueueIndex - PixelCountV - GI, 0xffffffff);

    DeviceMemoryBarrierWithGroupSync();

    if (GI == 0)
    {
        IndirectParams.Store(0 , PaddedCountH >> 6);
        IndirectParams.Store(12, PaddedCountV >> 6);
        WorkCounts.Store2(0, 0);
    }
}
