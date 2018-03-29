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

cbuffer CB1 : register(b1)
{
    // A sort key that will end up at the end of the list; to be used to pad
    // lists in LDS (always 2048 items).
    //   Descending:  0x00000000
    //   Ascending:   0xffffffff
    // Also used by the ShouldSwap() function to invert ordering.
    uint NullItem; 
    uint ListCount;
}

// Takes Value and widens it by one bit at the location of the bit
// in the mask.  A one is inserted in the space.  OneBitMask must
// have one and only one bit set.
uint InsertOneBit( uint Value, uint OneBitMask )
{
    uint Mask = OneBitMask - 1;
    return (Value & ~Mask) << 1 | (Value & Mask) | OneBitMask;
}

// Determines if two sort keys should be swapped in the list.  NullItem is
// either 0 or 0xffffffff.  XOR with the NullItem will either invert the bits
// (effectively a negation) or leave the bits alone.  When the the NullItem is
// 0, we are sorting descending, so when A < B, they should swap.  For an
// ascending sort, ~A < ~B should swap.
bool ShouldSwap(uint A, uint B, uint indexA, uint indexB)
{
    if (A == B)
    {
        return indexA > indexB;
    }
    else
    {
        return (A ^ NullItem) < (B ^ NullItem);
    }
}
