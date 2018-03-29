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
//
// Description:  A bitonic sort must eventually sort the power-of-two
// ceiling of items.  E.g. 391 items -> 512 items.  Because of this
// "null items" must be used as padding at the end of the list so that
// they can participate in the sort but remain at the end of the list.
//
// The pre-sort does two things.  It appends null items as need, and
// it does the initial sort for k values up to 2048.  This is because
// we can run 1024 threads, each of of which can compare and swap two
// elements without contention.  And because we can always fit 2048
// keys & indices in LDS with occupancy greater than one.  (A single
// thread group can use as much as 32KB of LDS.)


#include "BitonicSortCommon.hlsli"

RWByteAddressBuffer g_SortBuffer : register(u0);
RWByteAddressBuffer g_IndexBuffer : register(u1);

groupshared uint gs_SortIndices[2048];
groupshared uint gs_SortKeys[2048];

void FillSortKey( uint Element, uint ListCount )
{
    // Unused elements must sort to the end
    if (Element < ListCount)
    {
        gs_SortKeys[Element & 2047] = g_SortBuffer.Load(Element * 4);
        gs_SortIndices[Element & 2047] = g_IndexBuffer.Load(Element * 4);
    }
    else
    {
        gs_SortKeys[Element & 2047] = NullItem;
    }
}

void StoreKeyIndexPair( uint Element, uint ListCount)
{
    if (Element < ListCount)
    {
        g_SortBuffer.Store(Element * 4, gs_SortKeys[Element & 2047]);
        g_IndexBuffer.Store(Element * 4, gs_SortIndices[Element & 2047]);
    }
}

[numthreads(1024, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
    // Item index of the start of this group
    const uint GroupStart = Gid.x * 2048;

    FillSortKey(GroupStart + GI, ListCount);
    FillSortKey(GroupStart + GI + 1024, ListCount);

    GroupMemoryBarrierWithGroupSync();

    uint k;

    // This is better unrolled because it reduces ALU and because some
    // architectures can load/store two LDS items in a single instruction
    // as long as their separation is a compile-time constant.
    [unroll]
    for (k = 2; k <= 2048; k <<= 1)
    {
        [unroll]
        for (uint j = k / 2; j > 0; j /= 2)
        {
            uint Index2 = InsertOneBit(GI, j);
            uint Index1 = Index2 ^ (k == 2 * j ? k - 1 : j);

            uint A = gs_SortKeys[Index1];
            uint B = gs_SortKeys[Index2];
            uint indexA = gs_SortIndices[Index1];
            uint indexB = gs_SortIndices[Index2];
            if (ShouldSwap(A, B, indexA, indexB))
            {
                // Swap the keys
                gs_SortKeys[Index1] = B;
                gs_SortKeys[Index2] = A;

                // Then swap the indices (for 64-bit sorts)
                A = gs_SortIndices[Index1];
                B = gs_SortIndices[Index2];
                gs_SortIndices[Index1] = indexB;
                gs_SortIndices[Index2] = indexA;
            }

            GroupMemoryBarrierWithGroupSync();
        }
    }

    // Write sorted results to memory
    StoreKeyIndexPair(GroupStart + GI, ListCount);
    StoreKeyIndexPair(GroupStart + GI + 1024, ListCount);
}
