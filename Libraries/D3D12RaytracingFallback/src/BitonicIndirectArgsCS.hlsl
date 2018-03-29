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

#include "BitonicSortCommon.hlsli"

RWByteAddressBuffer g_IndirectArgsBuffer : register(u0);

cbuffer Constants : register(b0)
{
    uint MaxIterations;
}

uint NextPow2( uint Val )
{
    uint Mask = (1 << firstbithigh(Val)) - 1;
    return (Val + Mask) & ~Mask;
}

[numthreads(22, 1, 1)]
void main( uint GI : SV_GroupIndex )
{
    if (GI >= MaxIterations)
        return;

    uint k = 2048 << GI;

    uint ElementCount = ListCount;
    // We need one more iteration every time the number of thread groups doubles
    if (k > NextPow2((ElementCount + 2047) & ~2047))
        ElementCount = 0;

    uint PrevDispatches = GI * (GI + 1) / 2;
    uint Offset = 12 * PrevDispatches;

    // Generate outer sort dispatch arguments
    for (uint j = k / 2; j > 1024; j /= 2)
    {
        // All of the groups of size 2j that are full
        uint CompleteGroups = (ElementCount & ~(2 * j - 1)) / 2048;

        // Remaining items must only be sorted if there are more than j of them
        uint PartialGroups = ((uint)max(int(ElementCount - CompleteGroups * 2048 - j), 0) + 1023) / 1024;

        g_IndirectArgsBuffer.Store3(Offset, uint3(CompleteGroups + PartialGroups, 1, 1));

        Offset += 12;
    }

    // The inner sort always sorts all groups (rounded up to multiples of 2048)
    g_IndirectArgsBuffer.Store3(Offset, uint3((ElementCount + 2047) / 2048, 1, 1));
}
