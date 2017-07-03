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

ByteAddressBuffer g_CounterBuffer : register(t0);
RWByteAddressBuffer g_IndirectArgsBuffer : register(u0);

cbuffer Constants : register(b0)
{
    uint MaxBufferCountPow2;
    uint CounterOffset;
}

[RootSignature(BitonicSort_RootSig)]
[numthreads(64, 1, 1)]
void main( uint GI : SV_GroupIndex )
{
    uint ListCount = g_CounterBuffer.Load(CounterOffset);

    uint k = 1 << (GI + 11);

    uint NextPow2 = (1 << firstbithigh(ListCount)) - 1;
    NextPow2 = (ListCount + NextPow2) & ~NextPow2;
    NextPow2 = (NextPow2 + 2047) & ~2047;

    uint NumElements = k > NextPow2 ? 0 : (ListCount + k - 1) & ~(k - 1);
    uint NumGroups = (GI == 0 ? NextPow2 : NumElements) / 2048;

    if (k <= MaxBufferCountPow2)
        g_IndirectArgsBuffer.Store3(GI * 12, uint3(NumGroups, 1, 1));
}
