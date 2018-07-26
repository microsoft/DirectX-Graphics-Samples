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

RWByteAddressBuffer g_SortBuffer : register(u0);

cbuffer Constants : register(b0)
{
    uint k;    // k >= 4096
    uint j;    // j >= 2048 && j < k
};

#ifdef BITONICSORT_64BIT
    #define Element uint2
    #define LoadElement(idx) g_SortBuffer.Load2(idx * 8)
    #define StoreElement(idx, elem) g_SortBuffer.Store2(idx * 8, elem)
#else
    #define Element uint
    #define LoadElement(idx) g_SortBuffer.Load(idx * 4)
    #define StoreElement(idx, elem) g_SortBuffer.Store(idx * 4, elem)
#endif

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

    Element A = LoadElement(Index1);
    Element B = LoadElement(Index2);

    if (ShouldSwap(A, B))
    {
        StoreElement(Index1, B);
        StoreElement(Index2, A);
    }
}
