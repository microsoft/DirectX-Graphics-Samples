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

// Using the Karras's 2012 parallel BVH construction algorithm outlined 
// in "Maximizing Parallelism in the Construction of BVHs, Octrees,
// and k-d Trees"
#include "RayTracingHelper.hlsli"

#define offsetToBoxes SizeOfBVHOffsets
int CountLeadingZeroes(uint num)
{
    return 31 - firstbithigh(num);
}

void WriteChild(uint childIndex, uint parentIndex)
{
    // Constructing new hierarchy, so the ParentIndex is already accurate, ie. doesn't need GetActualParentIndex()
    hierarchyBuffer[childIndex].ParentIndex = parentIndex;
}

void WriteParent(uint parentIndex, int leftBoxIndex, int rightBoxIndex)
{
    hierarchyBuffer[parentIndex].LeftChildIndex = leftBoxIndex;
    hierarchyBuffer[parentIndex].RightChildIndex = rightBoxIndex;
}

int GetLongestCommonPrefix(uint indexA, uint indexB)
{
    if (indexA >= Constants.NumberOfElements || indexB >= Constants.NumberOfElements)
    {
        return -1;
    }
    else
    {
        uint mortonCodeA = mortonCodes[indexA];
        uint mortonCodeB = mortonCodes[indexB];
        if (mortonCodeA != mortonCodeB)
        {
            return CountLeadingZeroes(mortonCodes[indexA] ^ mortonCodes[indexB]);
        }
        else
        {
            // TODO: Technically this should be primitive ID
            return CountLeadingZeroes(indexA ^ indexB) + 31;
        }
    }
}

uint2 DetermineRange(uint idx)
{
    int d = GetLongestCommonPrefix(idx, idx + 1) - GetLongestCommonPrefix(idx, idx - 1);
    d = clamp(d, -1, 1);
    int minPrefix = GetLongestCommonPrefix(idx, idx - d);

    // TODO: Consider starting this at a higher number
    int maxLength = 2;
    while (GetLongestCommonPrefix(idx, idx + maxLength * d) > minPrefix)
    {
        maxLength *= 4;
    }

    int length = 0;
    for (int t = maxLength / 2; t > 0; t /= 2)
    {
        if (GetLongestCommonPrefix(idx, idx + (length + t) * d) > minPrefix)
        {
            length = length + t;
        }
    }

    int j = idx + length * d;
    return uint2(min(idx, j), max(idx, j));
}


int FindSplit(int first, uint last)
{
    int commonPrefix = GetLongestCommonPrefix(first, last);
    int split = first;
    int step = last - first;

    do
    {
        step = (step + 1) >> 1;
        int newSplit = split + step;

        if (newSplit < last)
        {
            int splitPrefix = GetLongestCommonPrefix(first, newSplit);
            if (splitPrefix > commonPrefix)
                split = newSplit;
        }
    } while (step > 1);

    return split;
}

void GenerateHierarchy(unsigned int idx)
{
    uint2 range = DetermineRange(idx);
    uint first = range.x;
    uint last = range.y;

    uint split = FindSplit(first, last);

    uint internalNodeOffset = 0;
    uint leafNodeOffset = Constants.NumberOfElements - 1;
    uint childAIndex;
    if (split == first)
        childAIndex = leafNodeOffset + split;
    else
        childAIndex = internalNodeOffset + split;

    uint childBIndex;
    if (split + 1 == last)
        childBIndex = leafNodeOffset + split + 1;
    else
        childBIndex = internalNodeOffset + split + 1;

    WriteParent(idx, childAIndex, childBIndex);
    WriteChild(childAIndex, idx);
    WriteChild(childBIndex, idx);
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int NumberOfInternalNodes = Constants.NumberOfElements - 1;
    int NumberOfAABBs = NumberOfInternalNodes + Constants.NumberOfElements;

    if (DTid.x >= NumberOfInternalNodes) return;

    GenerateHierarchy(DTid.x);
}
