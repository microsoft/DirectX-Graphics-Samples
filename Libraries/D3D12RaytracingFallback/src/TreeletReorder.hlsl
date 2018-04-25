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

static const uint rootNodeIndex = 0;
static const uint MaxTreeletSize = 7;
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

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x >= Constants.NumberOfElements) return;

    const uint NumberOfInternalNodes = GetNumInternalNodes(Constants.NumberOfElements);
    const uint NumberOfAABBs = NumberOfInternalNodes + Constants.NumberOfElements;
    
    // Start from the leaf nodes and go bottom-up
    uint nodeIndex = NumberOfAABBs - DTid.x - 1;
    uint numTriangles = 1;
    bool isLeaf = true;
    do
    {
        AABB nodeAABB;
        if (isLeaf)
        {
            uint leafIndex = nodeIndex - NumberOfInternalNodes;
            nodeAABB = ComputeLeafAABB(leafIndex);
        }
        else
        {
            AABB leftAABB = AABBBuffer[hierarchyBuffer[nodeIndex].LeftChildIndex];
            AABB rightAABB = AABBBuffer[hierarchyBuffer[nodeIndex].RightChildIndex];
            nodeAABB = CombineAABB(leftAABB, rightAABB);
        }
        
        if (numTriangles >= Constants.MinTrianglesPerTreelet)
        {
            // Form a treelet
            uint nodesToReorder[MaxTreeletSize];
            const uint numInternalTreeletNodes = MaxTreeletSize - 1;
            uint internalNodes[numInternalTreeletNodes];
            internalNodes[0] = nodeIndex;

            nodesToReorder[0] = hierarchyBuffer[nodeIndex].LeftChildIndex;
            nodesToReorder[1] = hierarchyBuffer[nodeIndex].RightChildIndex;
            uint treeletSize = 2;
            while(treeletSize < MaxTreeletSize)
            {
                float largestSurfaceArea = 0.0;
                uint nodeIndexToTraverse = 0;
                uint indexOfNodeIndexToTraverse = 0;
                for (uint i = 0; i < treeletSize; i++)
                {
                    uint treeletNodeIndex = nodesToReorder[i];
                    // Leaf nodes can't be split so skip these
                    if (!IsLeaf(treeletNodeIndex))
                    {
                        // TODO: Only needs to be recalculated on the nodes that were added
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
                nodesToReorder[indexOfNodeIndexToTraverse] = nodeToTraverse.LeftChildIndex;
                nodesToReorder[treeletSize] = nodeToTraverse.RightChildIndex;
                treeletSize++;
            }

            // Now that a treelet has been formed, try to reorder
            const uint numTreeletSplitPermutations = 1 << MaxTreeletSize;
            float area[numTreeletSplitPermutations];
            float optimalCost[numTreeletSplitPermutations];
            uint optimalPartition[numTreeletSplitPermutations];

            const uint FullPartitionMask = numTreeletSplitPermutations - 1;
            for (uint treeletBitmask = 1; treeletBitmask < numTreeletSplitPermutations; treeletBitmask++)
            {
                AABB aabb;
                aabb.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
                aabb.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
                for (uint i = 0; i < MaxTreeletSize; i++)
                {
                    if (BIT(i) & treeletBitmask)
                    {
                        // TODO: Reading from AABBBuffer a lot, should cache the treelet's AABBs
                        aabb = CombineAABB(aabb, AABBBuffer[nodesToReorder[i]]);
                    }
                }
                area[treeletBitmask] = ComputeBoxSurfaceArea(aabb);
            }

            float rootAABBSurfaceArea = ComputeBoxSurfaceArea(nodeAABB);
            for (uint i = 0; i < MaxTreeletSize; i++)
            {
                optimalCost[BIT(i)] = CalculateCost(AABBBuffer[nodesToReorder[i]], rootAABBSurfaceArea);
            }

            for (uint bits = 2; bits <= MaxTreeletSize; bits++)
            {
                for (uint treeletBitmask = BIT(bits) - 1; treeletBitmask < numTreeletSplitPermutations; treeletBitmask++)
                {
                    if (countbits(treeletBitmask) == bits)
                    {
                        float lowestCost = FLT_MAX;
                        uint bestPartition = 0;
                        for (uint partitionBitmask = 1; partitionBitmask < treeletBitmask; partitionBitmask++)
                        {
                            if ((treeletBitmask | partitionBitmask) == treeletBitmask)
                            {
                                const float cost = optimalCost[partitionBitmask] + optimalCost[treeletBitmask ^ partitionBitmask];
                                if (cost < lowestCost)
                                {
                                    lowestCost = cost;
                                    bestPartition = partitionBitmask;
                                }
                            }
                        }
                        // TODO: Can the CostOfRayBoxIntersection be baked into area[s]?
                        optimalCost[treeletBitmask] = CostOfRayBoxIntersection * area[treeletBitmask] + lowestCost; // TODO: Consider cost of flattening to triangle list
                        optimalPartition[treeletBitmask] = bestPartition;
                    }
                }
            }

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
                leftEntry.Mask = optimalPartition[partition.Mask];
                if (countbits(leftEntry.Mask) > 1)
                {
                    leftEntry.NodeIndex = internalNodes[nodesAllocated++];
                    partitionStack[partitionStackSize++] = leftEntry;
                }
                else
                {
                    leftEntry.NodeIndex = nodesToReorder[log2(leftEntry.Mask)];
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
                    rightEntry.NodeIndex = nodesToReorder[log2(rightEntry.Mask)];
                }

                hierarchyBuffer[partition.NodeIndex].LeftChildIndex = leftEntry.NodeIndex;
                hierarchyBuffer[partition.NodeIndex].RightChildIndex = rightEntry.NodeIndex;
                hierarchyBuffer[leftEntry.NodeIndex].ParentIndex = partition.NodeIndex;
                hierarchyBuffer[rightEntry.NodeIndex].ParentIndex = partition.NodeIndex;
            }

            // Start from the back. This is optimizing since the previous traversal went from
            // top-down, the reverse order is guaranteed to be bottom-up
            for (int j = numInternalTreeletNodes - 1; j >= 0; j--)
            {
                uint internalNodeIndex = internalNodes[j];
                AABB leftAABB = AABBBuffer[hierarchyBuffer[internalNodeIndex].LeftChildIndex];
                AABB rightAABB = AABBBuffer[hierarchyBuffer[internalNodeIndex].RightChildIndex];
                AABBBuffer[internalNodeIndex] = CombineAABB(leftAABB, rightAABB);
            }
        }

        AABBBuffer[nodeIndex] = nodeAABB;
        DeviceMemoryBarrier(); // Ensure AABBs have been written out and are visible to all waves

        if (nodeIndex == rootNodeIndex) return;

        uint parentNodeIndex = hierarchyBuffer[nodeIndex].ParentIndex;

        uint numTrianglesFromOtherNode = 0;
        NumTrianglesBuffer.InterlockedAdd(parentNodeIndex * SizeOfUINT32, numTriangles, numTrianglesFromOtherNode);
        if (numTrianglesFromOtherNode == 0)
        {
            // Other child hasn't finished calculating yet
            return;
        }
        numTriangles = numTrianglesFromOtherNode + numTriangles;
        nodeIndex = parentNodeIndex;
        isLeaf = false;
    } while (true);
}
