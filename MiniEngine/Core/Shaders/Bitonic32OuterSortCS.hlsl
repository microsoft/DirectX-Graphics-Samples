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

#include "BitonicSortCommon.hlsli"

#ifdef BITONICSORT_64BIT
RWStructuredBuffer<uint2> g_SortBuffer : register(u0);
#else
RWStructuredBuffer<uint> g_SortBuffer : register(u0);
#endif

cbuffer Constants : register(b0)
{
    uint k;	// k >= 4096
    uint j;	// j >= 2048 && j < k
};

[RootSignature(BitonicSort_RootSig)]
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID  )
{
    const uint ListCount = g_CounterBuffer.Load(CounterOffset);

    // Form unique index pair from dispatch thread ID
    uint Index2 = InsertOneBit(DTid.x, j);
    uint Index1 = Index2 ^ (k == 2 * j ? k - 1 : j);

    if (Index2 >= ListCount)
        return;

#ifdef BITONICSORT_64BIT
    uint2 A = g_SortBuffer[Index1];
    uint2 B = g_SortBuffer[Index2];
#else
    uint A = g_SortBuffer[Index1];
    uint B = g_SortBuffer[Index2];
#endif

    if (ShouldSwap(A, B))
    {
        g_SortBuffer[Index1] = B;
        g_SortBuffer[Index2] = A;
    }
}
