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
// Author(s):  James Stanard
//
// Bitonic Sort is a highly parallel sorting algorithm well-suited to the
// GPU.  It has a complexity of O( N*(log N)^2 ), which is inferior to most
// traditional sorting algorithms, but because GPUs have so many threads,
// and because each thread can be utilized, the algorithm can fully load
// the GPU, taking advantage of its high ALU and bandwidth capabilities.
// 
// Another reason why sorting on the GPU is useful is when the GPU is creating
// work on its own timeline and needs to sort the work without CPU intervention.
// One example in MiniEngine is with GPU simulated particle systems.  Before
// rendering the particles, it is beneficial to sort the particles and render
// them either front-to-back or back-to-front.
//
// The way a bitonic sort works is by iteratively sorting groups of increasing
// size and then blending the sorted groups together to form larger sorted
// groups.  The core of the algorithm can be expressed like so:
//
// for (k = 2; k < NumItems; k *= 2)      // k = group size
//     for (j = k / 2; j > 0; j /= 2)     // j = compare distance
//         for (i = 0; i < NumItems; ++i) // i = element index
//             if (ShouldSwap(i, i ^ j))  // Are the two in proper order?
//                 Swap(i, i^j)           // If not, swap them
//
// To determine if two elements should swap, one more property of the bitonic
// sort must be understood.  For groups of size k, even groups should sort in
// ascending order, and odd groups should sort in descending order.  Two items
// which are being compared always come from the same group because they have
// a distance of j, which is always less than k.
//
// Because of the parallel nature and powers of two dimensionality, we must
// ultimately sort a list that has a length with a power of two.  This forces
// us to pad out unused elements with sort keys that will eventually sort to
// the end (or remain in place.)  To enforce this, we have the concept of "pre-
// sorting".  During the pre-sort, we fill unused items with null sort keys
// (e.g. 0xffffffff) that are guaranteed to sort to the end of the list.  As
// an optimization, we can also pre-sort the list for k values up to 2048 by
// sorting in LDS before writing to memory.  It is always better for the caller
// of this system to pre-sort their list as they create it.
//
// The expected usage of this API is that you will have an array of data of
// unspecified stride.  You will generate a list of sort keys and array index
// pairs to pass to the system to be sorted.  The sorted key/index pairs can
// then be used to reorder your data array (with a double buffer) or you can
// read your data array in place using indirection.
//
// We also expect that consumers of this system have a GPU-visible count of
// items.  This probably came from an AppendBuffer (structured buffer with an
// atomic counter).  So you will need to provide the counter buffer as well as
// the offset to the counter.  Lastly, due to the variable amount of work a
// list of variable size requires, the implementation uses ExecuteIndirect().
// The exact number of thread groups needed for each dispatch is determined by
// the list count, but we need a buffer to hold the DispatchIndirect arguments.
// Pre-allocate an IndirectArgsBuffer and provide it to the system.  Allocating
// 256 bytes for args will allow for sorting up to 2 billion items, which should
// be enough. :)
//
// Also note that key/index pairs may be packed into 32-bit words with the key
// in the most significant bits.  Sorting 32-bit elements is faster than sorting
// 64-bit elements because it uses less bandwidth.

#pragma once

#include "GpuBuffer.h"

namespace BitonicSort
{
    void Sort(
        // An existing compute context
        ComputeContext& Context,

        // List to be sorted.  If element size is 4 bytes, it is assumed the key and index are packed
        // together with the key in the most significant bytes.  If element size is 8 bytes, the key
        // is assumed in the upper 4 bytes (i.e. uint2.y).
        GpuBuffer& KeyIndexList,

        // A buffer containing the count of items to be sorted.
        GpuBuffer& CountBuffer,

        // Offset into counter buffer to find count for this list.  Must be a multiple of 4 bytes.
        uint32_t CounterOffset,

        // Buffer to hold ExecuteIndirect arguments.  Allocate 256 bytes to be safe.
        IndirectArgsBuffer& IndirectArgsBuffer,

        // Pass 'true' if the list has already been sorted for k values up to 2048.  That is, each
        // group of 2048 is already sorted.  This is common because you can combine generating a
        // sort key list with sorting thread groups in groupshared memory.
        bool IsPartiallyPreSorted,

        // True to sort in ascending order (smallest to largest).  False to sort in descending order.
        bool SortAscending
    );

} // namespace BitonicSort
