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

// This constant is pulled from this paper, http://research.nvidia.com/sites/default/files/pubs/2013-07_Fast-Parallel-Construction/karras2013hpg_paper.pdf
static const float CostOfRayBoxIntersection = 1.2;
static const float CostOfRayTriangleIntersection = 1.0;

float CalculateCost(AABB nodeAABB, float parentAABBSurfaceArea)
{
    // TODO consider caching rcp(parentAABBSurfaceArea)
    return CostOfRayBoxIntersection * ComputeBoxSurfaceArea(nodeAABB) / parentAABBSurfaceArea;
}

// Must be at least FullTreeletSize
#define NumThreadsInGroup 32

groupshared uint nodeIndex;
groupshared float optimalCost[NumTreeletSplitPermutations];
groupshared uint optimalPartition[NumTreeletSplitPermutations];
groupshared uint treeletToReorder[FullTreeletSize];
groupshared uint internalNodes[NumInternalTreeletNodes];
groupshared bool finished;

void FormTreelet(in uint groupThreadId)
{
    if (groupThreadId == 0)
    {
        internalNodes[0] = nodeIndex;

        treeletToReorder[0] = hierarchyBuffer[nodeIndex].LeftChildIndex;
        treeletToReorder[1] = hierarchyBuffer[nodeIndex].RightChildIndex;

#if USE_EXPLICIT_UNROLL_IN_FORMTREELET
        [unroll]
#endif
    for (uint treeletSize = 2; treeletSize < FullTreeletSize; treeletSize++)
        {
            float largestSurfaceArea = 0.0;
            uint nodeIndexToTraverse = 0;
            uint indexOfNodeIndexToTraverse = 0;
            [unroll]
            for (uint i = 0; i < treeletSize; i++)
            {
                uint treeletNodeIndex = treeletToReorder[i];
                // Leaf nodes can't be split so skip these
                if (!IsLeafIndex(treeletNodeIndex))
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
    GroupMemoryBarrierWithGroupSync();
}

void FindOptimalPartitions(in uint threadId)
{
    uint numBitmasksPerThread;
    uint extraBitmasks;
    uint bitmasksStart;
    uint bitmasksEnd;

    // For every combination of bitmasks (representing which leaves are included), ie. 0000001, 0000010, 0000011, ..., 1111111, calculate its Surface Area
    if (threadId < NumTreeletSplitPermutations)
    {
        numBitmasksPerThread = max(NumTreeletSplitPermutations / NumThreadsInGroup, 1);
        extraBitmasks = NumTreeletSplitPermutations > NumThreadsInGroup ? NumTreeletSplitPermutations % NumThreadsInGroup : 0;

        bitmasksStart = numBitmasksPerThread * threadId + min(threadId, extraBitmasks);
        bitmasksEnd = bitmasksStart + numBitmasksPerThread + (threadId < extraBitmasks ? 1 : 0);

        // Now that a treelet has been formed, try to reorder
        for (uint treeletBitmask = bitmasksStart; treeletBitmask < bitmasksEnd; treeletBitmask++)
        {
            if (treeletBitmask == 0)
            {
                continue;
            }

            AABB aabb;
            aabb.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
            aabb.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

            [unroll]
            for (uint i = 0; i < FullTreeletSize; i++)
            {
                if (BIT(i) & treeletBitmask)
                {
                    aabb = CombineAABB(aabb, AABBBuffer[treeletToReorder[i]]);
                }
            }

            // Intermediate value.
            optimalCost[treeletBitmask] = ComputeBoxSurfaceArea(aabb);
        }
    }

    AABB nodeAABB = AABBBuffer[nodeIndex];
    float rootAABBSurfaceArea = ComputeBoxSurfaceArea(nodeAABB);

    // For every individual leaf [0-6], calculate its Surface Area Heuristic Cost, and store it in array where leaf's bitmask is the index
    if (threadId < FullTreeletSize)
    {
        optimalCost[BIT(threadId)] = CalculateCost(AABBBuffer[treeletToReorder[threadId]], rootAABBSurfaceArea);
    }

    GroupMemoryBarrierWithGroupSync();

    // Dynamic programming from 'treelet/subset' of size 2 up to FullTreeletSize, calculate and store optimal (lowest) cost and its partition bitmask
    [unroll]
    for (uint subsetSize = 2; subsetSize <= FullTreeletSize; subsetSize++)
    {
        // eg. In 'treelet/subset' of size 2, there are (7 Choose 2) distinct 'treelets' in the original treelet of 7 leaves, ie. 0000011, 0000101, ..., 1100000
        uint numTreeletBitmasks = FullTreeletSizeChoose[subsetSize];
        if (threadId < numTreeletBitmasks)
        {
            numBitmasksPerThread = max(numTreeletBitmasks / NumThreadsInGroup, 1);
            extraBitmasks = numTreeletBitmasks > NumThreadsInGroup ? numTreeletBitmasks % NumThreadsInGroup : 0;

            bitmasksStart = numBitmasksPerThread * threadId + min(threadId, extraBitmasks);
            bitmasksEnd = bitmasksStart + numBitmasksPerThread + (threadId < extraBitmasks ? 1 : 0);

            // For each subset with [subsetSize] bits set
            for (uint i = bitmasksStart; i < bitmasksEnd; i++)
            {
                uint treeletBitmask = GetBitPermutation(subsetSize, i);

                float lowestCost = FLT_MAX;
                uint bestPartition = 0;

                uint delta = (treeletBitmask - 1) & treeletBitmask;
                uint partitionBitmask = (-delta) & treeletBitmask;

                do
                {
                    const float cost = optimalCost[partitionBitmask] + optimalCost[treeletBitmask ^ partitionBitmask];
                    if (cost < lowestCost)
                    {
                        lowestCost = cost;
                        bestPartition = partitionBitmask;
                    }
                    partitionBitmask = (partitionBitmask - delta) & treeletBitmask;
                } while (partitionBitmask != 0);

#if COMBINE_LEAF_NODES
                float costAsLeafNode = CostOfRayTriangleIntersection * optimalCost[treeletBitmask] * subsetSize;
                float costAsInternalNode = CostOfRayBoxIntersection * optimalCost[treeletBitmask] + lowestCost;
                optimalCost[treeletBitmask] = min(costAsInternalNode, costAsLeafNode);
                optimalPartition[treeletBitmask] = bestPartition;
                if (costAsLeafNode < costAsInternalNode)
                {
                    // Consider cost of flattening to triangle list as a leaf node
                    optimalPartition[treeletBitmask] |= BIT(FullTreeletSize); // Set the unused bit, as a bCollapseChildren flag
                }
#else
                optimalCost[treeletBitmask] = CostOfRayBoxIntersection * optimalCost[treeletBitmask] + lowestCost; // TODO: Consider cost of flattening to triangle list
                optimalPartition[treeletBitmask] = bestPartition;
#endif
            }
        }

        GroupMemoryBarrierWithGroupSync();
    }
}

void ReformTree(in uint groupThreadId)
{
    if (groupThreadId != 0)
    {
        return;
    }

    // Now that a reordering has been calculated, reform the tree
    struct PartitionEntry
    {
        uint Mask;
        uint NodeIndex;
    };
    uint nodesAllocated = 1;
    uint partitionStackSize = 1;
    PartitionEntry partitionStack[FullTreeletSize];
    partitionStack[0].Mask = FullPartitionMask;
    partitionStack[0].NodeIndex = internalNodes[0];

    while (partitionStackSize > 0)
    {
        PartitionEntry partition = partitionStack[--partitionStackSize];

        PartitionEntry leftEntry;
        leftEntry.Mask = optimalPartition[partition.Mask];
        bool bCollapseChildren = leftEntry.Mask & BIT(FullTreeletSize);
        leftEntry.Mask &= FullPartitionMask;
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
#if COMBINE_LEAF_NODES
        if (bCollapseChildren)
        {
            // Only possible if this node was calculated to be more optimal to be flattened as a leaf with triangle list
            hierarchyBuffer[leftEntry.NodeIndex].ParentIndex |= HierarchyNode::IsCollapseChildren;
            hierarchyBuffer[rightEntry.NodeIndex].ParentIndex |= HierarchyNode::IsCollapseChildren;
        }
#endif
    }

    // Start from the back. This is optimizing since the previous traversal went from
    // top-down, the reverse order is guaranteed to be bottom-up
    [unroll]
    for (int j = NumInternalTreeletNodes - 1; j >= 0; j--)
    {
        uint internalNodeIndex = internalNodes[j];
        AABB leftAABB = AABBBuffer[hierarchyBuffer[internalNodeIndex].LeftChildIndex];
        AABB rightAABB = AABBBuffer[hierarchyBuffer[internalNodeIndex].RightChildIndex];
        AABBBuffer[internalNodeIndex] = CombineAABB(leftAABB, rightAABB);
    }
}

void TraverseToParent(in uint groupThreadId)
{
    if (groupThreadId == 0)
    {
        if (nodeIndex == RootNodeIndex)
        {
            finished = true;
        }
        else
        {
            uint parentNodeIndex = GetActualParentIndex(hierarchyBuffer[nodeIndex].ParentIndex);

            uint ourNumTriangles = NumTrianglesBuffer.Load(nodeIndex * SizeOfUINT32);
            uint numTrianglesFromOtherNode = 0;
            NumTrianglesBuffer.InterlockedAdd(parentNodeIndex * SizeOfUINT32, ourNumTriangles, numTrianglesFromOtherNode);

            // Wait for sibling in tree
            if (numTrianglesFromOtherNode == 0)
            {
                finished = true;
            }
            else
            {
                // Build parents bounding box
                AABB leftAABB = AABBBuffer[hierarchyBuffer[parentNodeIndex].LeftChildIndex];
                AABB rightAABB = AABBBuffer[hierarchyBuffer[parentNodeIndex].RightChildIndex];
                AABBBuffer[parentNodeIndex] = CombineAABB(leftAABB, rightAABB);

                nodeIndex = parentNodeIndex;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();
}

[numthreads(NumThreadsInGroup, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadId)
{
    if (GTid.x == 0)
    {
        nodeIndex = BaseTreeletsIndexBuffer[Gid.x];
        finished = false;
    }

    GroupMemoryBarrierWithGroupSync();

    const uint NumberOfAABBs = GetNumInternalNodes(Constants.NumberOfElements) + Constants.NumberOfElements;

    if (nodeIndex >= NumberOfAABBs)
    {
        return;
    }

    do
    {
        FormTreelet(GTid.x);

        FindOptimalPartitions(GTid.x);

        ReformTree(GTid.x);

        TraverseToParent(GTid.x);

        if (finished)
        {
            return;
        }

        DeviceMemoryBarrierWithGroupSync();
    } while (true);
}