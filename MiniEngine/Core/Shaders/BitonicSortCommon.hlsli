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

#define BitonicSort_RootSig \
    "RootFlags(0), " \
    "RootConstants(b0, num32BitConstants = 2)," \
    "DescriptorTable(SRV(t0, numDescriptors = 1))," \
    "DescriptorTable(UAV(u0, numDescriptors = 1))," \
    "RootConstants(b1, num32BitConstants = 1)"


cbuffer CB1 : register(b1)
{
    // A sort key that will end up at the end of the list, to be used to pad lists to the proper length
    // for a bitonic sort (a multiple of 2048):  0 for descending sorts, 0xffffffff for ascending sorts.
    // Also used as a mask to determine sort order in the ShouldSwap() function.
    uint NullItem; 
}

// Takes Value and widens it by one bit at the location of the bit
// in the mask.  A zero is inserted in the space.  OneBitMask must
// have one and only one bit set.
uint InsertZeroBit( uint Value, uint OneBitMask )
{
    uint Mask = OneBitMask - 1;
    return (Value & ~Mask) << 1 | (Value & Mask);
}

// Determines if two sort keys should be swapped in the list.  The
// bitonic sort algorithm sorts groups of size k in ascending and
// then descending order, alternatingly.  To invert the sort, i.e. to
// sort in descending and then ascending order, the value of NullItem
// (0x00000000 or 0xffffffff) is used to invert the meaning of an odd
// or even group.
bool ShouldSwap(uint A, uint B, uint Index, uint k)
{
    return (A < B) == ((Index & k) == (NullItem & k));
}

// Same as above, but only compares the upper 32-bit word.
bool ShouldSwap(uint2 A, uint2 B, uint Index, uint k)
{
    return (A.y < B.y) == ((Index & k) == (NullItem & k));
}