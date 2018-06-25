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

// Using the Karras/Aila paper on treelet reoordering:
// "Fast Parallel Construction of High-Quality Bounding Volume
// Hierarchies"



#define HLSL
#include "TreeletReorderBindings.h"
#include "RayTracingHelper.hlsli"

#define BREAK_POINT hierarchyBuffer[4294967291] = hierarchyBuffer[4294967294];


static const uint FullPartitionMask = numTreeletSplitPermutations - 1;

static const float CostOfRayBoxIntersection = 1.0;

float ComputeBoxSurfaceArea(AABB aabb)
{
    float3 dim = aabb.max - aabb.min;
    return 2.0f * (dim.x * dim.y + dim.x * dim.z + dim.y * dim.z);
}

float CalculateCost(AABB nodeAABB, float parentAABBSurfaceArea)
{
    // TODO consider caching rcp(parentAABBSurfaceArea)
    return CostOfRayBoxIntersection * ComputeBoxSurfaceArea(nodeAABB) / parentAABBSurfaceArea;
}

AABB ComputeLeafAABB(uint triangleIndex)
{
    uint2 unused;
    Primitive primitive = InputBuffer[triangleIndex];
    if (primitive.PrimitiveType == TRIANGLE_TYPE)
    {
        Triangle tri = GetTriangle(primitive);
        return BoundingBoxToAABB(GetBoxDataFromTriangle(tri.v0, tri.v1, tri.v2, triangleIndex, unused));
    }
    else // if(primitiveType == PROCEDURAL_PRIMITIVE_TYPE)
    {
        return GetProceduralPrimitiveAABB(primitive);
    }
}

AABB CombineAABB(AABB aabb0, AABB aabb1)
{
    AABB parentAABB;
    parentAABB.min = min(aabb0.min, aabb1.min);
    parentAABB.max = max(aabb0.max, aabb1.max);
    return parentAABB;
}

bool IsLeaf(uint nodeIndex)
{
    const uint NumberOfInternalNodes = GetNumInternalNodes(Constants.NumberOfElements);
    return nodeIndex >= NumberOfInternalNodes;
}

#define BIT(x) (1 << (x))

typedef uint byte;

uint setByte(uint src, uint data, uint byteIndex)
{
    uint bitIndex = byteIndex << 3;
    return ((src & (~(0xff << bitIndex))) | (data << bitIndex));
}

uint getByte(uint src, uint byteIndex)
{
    uint bitIndex = byteIndex << 3;
    return ((src & (0xff << bitIndex)) >> bitIndex);
}

// http://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
uint NextBitPermutation(uint bits)
{
    const uint t = bits | (bits - 1);
    return (t + 1) | (((~t & -~t) - 1) >> (firstbitlow(bits) + 1));
}

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{   
    if (GTid.x != 0) return;

    float optimalCost[numTreeletSplitPermutations];
    byte optimalPartition[numTreeletSplitPermutations >> 2]; // Treated as an array of bytes

    const uint NumberOfAABBs = GetNumInternalNodes(Constants.NumberOfElements) + Constants.NumberOfElements;
    const uint MaxNumTreelets = Constants.NumberOfElements / MaxTreeletSize;

    uint nodeIndex = ReorderBubbleBuffer.Load((Gid.x + 1) * SizeOfUINT32);
    
    while (nodeIndex >= rootNodeIndex && nodeIndex < NumberOfAABBs)
    {
        uint treeletToReorder[MaxTreeletSize];
        uint internalNodes[numInternalTreeletNodes];

        // Form Treelet
        {
            internalNodes[0] = nodeIndex;

            treeletToReorder[0] = hierarchyBuffer[nodeIndex].LeftChildIndex;
            treeletToReorder[1] = hierarchyBuffer[nodeIndex].RightChildIndex;

            [unroll]
            for (uint treeletSize = 2; treeletSize < MaxTreeletSize; treeletSize++)
            {
                float largestSurfaceArea = 0.0;
                uint nodeIndexToTraverse = 0;
                uint indexOfNodeIndexToTraverse = 0;
                [unroll]
                for (uint i = 0; i < treeletSize; i++)
                {
                    uint treeletNodeIndex = treeletToReorder[i];
                    // Leaf nodes can't be split so skip these
                    if (!IsLeaf(treeletNodeIndex))
                    {
                        float surfaceArea = ComputeBoxSurfaceArea(AABBBuffer[treeletNodeIndex]);
                        if (surfaceArea > largestSurfaceArea)
                        {
                            largestSurfaceArea = surfaceArea;
                            nodeIndexToTraverse = treeletNodeIndex;
                            indexOfNodeIndexToTraverse = i;
                        }
                    }
                }
                // Replace the original node with its left child and add the right child to the end
                HierarchyNode nodeToTraverse = hierarchyBuffer[nodeIndexToTraverse];
                internalNodes[treeletSize - 1] = nodeIndexToTraverse;
                treeletToReorder[indexOfNodeIndexToTraverse] = nodeToTraverse.LeftChildIndex;
                treeletToReorder[treeletSize] = nodeToTraverse.RightChildIndex;
            }
        }

        // Reorder treelet
        {
            AABB nodeAABB = AABBBuffer[nodeIndex];
            // Now that a treelet has been formed, try to reorder
            [unroll]
            for (uint treeletBitmask = 1; treeletBitmask < numTreeletSplitPermutations; treeletBitmask++)
            {
                AABB aabb;
                aabb.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
                aabb.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
                
                [unroll]
                for (uint i = 0; i < MaxTreeletSize; i++)
                {
                    if (BIT(i) & treeletBitmask)
                    {
                        aabb = CombineAABB(aabb, AABBBuffer[treeletToReorder[i]]);
                    }
                }

                // Intermediate value.
                optimalCost[treeletBitmask] = ComputeBoxSurfaceArea(aabb);
            }

            float rootAABBSurfaceArea = ComputeBoxSurfaceArea(nodeAABB);
            [unroll]
            for (uint i = 0; i < MaxTreeletSize; i++)
            {
                optimalCost[BIT(i)] = CalculateCost(AABBBuffer[treeletToReorder[i]], rootAABBSurfaceArea);
            }

            [unroll]
            for (uint subsetSize = 2; subsetSize <= MaxTreeletSize; subsetSize++)
            {
                uint treeletBitmask = BIT(subsetSize) - 1;
                uint numTreeletBitmasks = MaxTreeletSizeChoose[subsetSize];
                // For each subset with [subsetSize] bits set
                for (uint i = 0; i < numTreeletBitmasks; i++, treeletBitmask = NextBitPermutation(treeletBitmask))
                {
                    float lowestCost = FLT_MAX;
                    uint bestPartition = 0;

                    uint delta;
                    uint partitionBitmask;

                    delta = (treeletBitmask - 1) & treeletBitmask;
                    partitionBitmask = (-delta) & treeletBitmask;
                    do // [Invariant] countbits(treeletBitmask) == subsetSize
                    {
                        const float cost = optimalCost[partitionBitmask] + optimalCost[treeletBitmask ^ partitionBitmask];
                        if (cost < lowestCost)
                        {
                            lowestCost = cost;
                            bestPartition = partitionBitmask;
                        }
                        partitionBitmask = (partitionBitmask - delta) & treeletBitmask;
                    } while (partitionBitmask != 0);

                    optimalCost[treeletBitmask] = CostOfRayBoxIntersection * optimalCost[treeletBitmask] + lowestCost; // TODO: Consider cost of flattening to triangle list
                    optimalPartition[treeletBitmask >> 2] = setByte(optimalPartition[treeletBitmask >> 2], bestPartition & 0xff, treeletBitmask & 3);
                }
            }
        }

        // Reform tree
        {
            // Now that a reordering has been calculated, reform the tree
            struct PartitionEntry
            {
                uint Mask;
                uint NodeIndex;
            };
            uint nodesAllocated = 1;
            uint partitionStackSize = 1;
            PartitionEntry partitionStack[MaxTreeletSize];
            partitionStack[0].Mask = FullPartitionMask;
            partitionStack[0].NodeIndex = internalNodes[0];

            while (partitionStackSize > 0)
            {
                PartitionEntry partition = partitionStack[partitionStackSize - 1];
                partitionStackSize--;

                PartitionEntry leftEntry;
                leftEntry.Mask = getByte(optimalPartition[partition.Mask >> 2], partition.Mask & 3);

                if (countbits(leftEntry.Mask) > 1)
                {
                    leftEntry.NodeIndex = internalNodes[nodesAllocated++];
                    partitionStack[partitionStackSize++] = leftEntry;
                }
                else
                {
                    leftEntry.NodeIndex = treeletToReorder[firstbitlow(leftEntry.Mask)];
                }

                PartitionEntry rightEntry;
                rightEntry.Mask = partition.Mask ^ leftEntry.Mask;
                if (countbits(rightEntry.Mask) > 1)
                {
                    rightEntry.NodeIndex = internalNodes[nodesAllocated++];
                    partitionStack[partitionStackSize++] = rightEntry;
                }
                else
                {
                    rightEntry.NodeIndex = treeletToReorder[firstbitlow(rightEntry.Mask)];
                }

                hierarchyBuffer[partition.NodeIndex].LeftChildIndex = leftEntry.NodeIndex;
                hierarchyBuffer[partition.NodeIndex].RightChildIndex = rightEntry.NodeIndex;
                hierarchyBuffer[leftEntry.NodeIndex].ParentIndex = partition.NodeIndex;
                hierarchyBuffer[rightEntry.NodeIndex].ParentIndex = partition.NodeIndex;
            }

            // Start from the back. This is optimizing since the previous traversal went from
            // top-down, the reverse order is guaranteed to be bottom-up
            [unroll]
            for (int j = numInternalTreeletNodes - 1; j >= 0; j--)
            {
                uint internalNodeIndex = internalNodes[j];
                AABB leftAABB = AABBBuffer[hierarchyBuffer[internalNodeIndex].LeftChildIndex];
                AABB rightAABB = AABBBuffer[hierarchyBuffer[internalNodeIndex].RightChildIndex];
                AABBBuffer[internalNodeIndex] = CombineAABB(leftAABB, rightAABB);
            }
        }

        // Build parent boxes
        {
            if (nodeIndex == rootNodeIndex)
            {
                return;
            }

            uint parentNodeIndex = hierarchyBuffer[nodeIndex].ParentIndex;
          
                uint ourNumTriangles = NumTrianglesBuffer.Load(nodeIndex * SizeOfUINT32);
                uint numTrianglesFromOtherNode = 0;
                NumTrianglesBuffer.InterlockedAdd(parentNodeIndex * SizeOfUINT32, ourNumTriangles, numTrianglesFromOtherNode);
 
                // Wait for sibling in tree
                if (numTrianglesFromOtherNode == 0)
                {
                    return;
                }

                // Build parents bounding box
                AABB leftAABB = AABBBuffer[hierarchyBuffer[parentNodeIndex].LeftChildIndex];
                AABB rightAABB = AABBBuffer[hierarchyBuffer[parentNodeIndex].RightChildIndex];
                AABBBuffer[parentNodeIndex] = CombineAABB(leftAABB, rightAABB);

            nodeIndex = parentNodeIndex;
        }

        DeviceMemoryBarrierWithGroupSync();
    }
}
