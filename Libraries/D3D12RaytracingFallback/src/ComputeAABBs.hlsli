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
uint DivideAndRoundUp(uint dividend, uint divisor) { return (dividend - 1) / divisor + 1; }

static const uint rootNodeIndex = 0;
static const int offsetToBoxes = SizeOfBVHOffsets;
static const int MaxLeavesPerNode = 4;

uint GetLeafCount(uint boxIndex)
{
    uint nodeAddress = GetBoxAddress(offsetToBoxes, boxIndex);
    return outputBVH.Load(nodeAddress + 28);
}

uint4 GetNodeData(uint boxIndex, out uint2 flags)
{
    uint nodeAddress = GetBoxAddress(offsetToBoxes, boxIndex);
    uint4 data1 = outputBVH.Load4(nodeAddress);
    uint4 data2 = outputBVH.Load4(nodeAddress + 16);
    flags = float2(data1.w, data2.w);
    return data1;
}

uint2 GetNodeFlags(uint boxIndex) 
{
    uint2 flags;
    GetNodeData(boxIndex, flags);
    return flags;
}

uint GetLeftChildIndex(uint boxIndex) 
{
    if (ShouldPerformUpdate)
    {
        return GetLeftNodeIndex(GetNodeFlags(boxIndex));
    }

    return hierarchyBuffer[boxIndex].LeftChildIndex;
}

uint GetRightChildIndex(uint boxIndex) 
{
    if (ShouldPerformUpdate)
    {
        return GetRightNodeIndex(GetNodeFlags(boxIndex));
    }

    return hierarchyBuffer[boxIndex].RightChildIndex;
}

uint GetParentIndex(uint boxIndex) 
{
    if (ShouldPerformUpdate)
    {
        return aabbParentBuffer[boxIndex];
    }

    return GetActualParentIndex(hierarchyBuffer[boxIndex].ParentIndex);
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int NumberOfInternalNodes = GetNumInternalNodes(Constants.NumberOfElements);
    int NumberOfAABBs = NumberOfInternalNodes + Constants.NumberOfElements;

    if (DTid.x >= Constants.NumberOfElements)
    {
        return;
    }
    
    uint threadScratchAddress = DTid.x * SizeOfUINT32;
    uint nodeIndex = scratchMemory.Load(threadScratchAddress);
    if (nodeIndex == InvalidNodeIndex) { return; }

    int offsetToPrimitives = outputBVH.Load(OffsetToPrimitivesOffset);

    uint dummyFlag;
    uint nodeAddress = GetBoxAddress(offsetToBoxes, nodeIndex);
    if (nodeIndex >= NumberOfAABBs) return;

    uint numTriangles = 1;
    bool swapChildIndices = false;
    while (true)
    {
        BoundingBox boxData;
        uint2 outputFlag;
        bool isLeaf = nodeIndex >= NumberOfInternalNodes;
        if (isLeaf)
        {
            uint leafIndex = nodeIndex - NumberOfInternalNodes;
            boxData = ComputeLeafAABB(leafIndex, offsetToPrimitives, outputFlag);
        }
        else
        {
            uint leftNodeIndex = GetLeftChildIndex(nodeIndex);
            uint rightNodeIndex = GetRightChildIndex(nodeIndex);
            if (swapChildIndices)
            {
                uint temp = leftNodeIndex;
                leftNodeIndex = rightNodeIndex;
                rightNodeIndex = temp;
            }

            BoundingBox leftBox = GetBoxFromBuffer(outputBVH, offsetToBoxes, leftNodeIndex);
            BoundingBox rightBox = GetBoxFromBuffer(outputBVH, offsetToBoxes, rightNodeIndex);

            boxData = GetBoxFromChildBoxes(leftBox, leftNodeIndex, rightBox, rightNodeIndex, outputFlag);

#if COMBINE_LEAF_NODES_2ND_PART_TBD
            uint2 leftFlags;
            uint4 leftData = GetNodeData(leftNodeIndex, leftFlags);

            uint2 rightFlags;
            uint4 rightData = GetNodeData(rightNodeIndex, rightFlags);
            if (IsLeaf(rightFlags) && IsLeaf(leftFlags))
            {
                uint leftLeafNodeCount = GetLeafCount(leftNodeIndex);
                uint rightLeafNodeCount = GetLeafCount(rightNodeIndex);
                uint totalLeafCount = leftLeafNodeCount + rightLeafNodeCount;
                if (totalLeafCount <= MaxLeavesPerNode)
                {
                    outputFlag.x = rightFlags.x;
                    outputFlag.y = totalLeafCount;
                }
            }
#endif
        }

        WriteBoxToBuffer(outputBVH, offsetToBoxes, nodeIndex, boxData, outputFlag);

        if (nodeIndex == rootNodeIndex)
        {
            break;
        }

        uint parentNodeIndex = GetParentIndex(nodeIndex);
        uint trianglesFromOtherChild;
        // If this counter was already incremented, that means both children for the parent 
        // node have computed their AABB, and the parent is ready to be processed
        childNodesProcessedCounter.InterlockedAdd(parentNodeIndex * SizeOfUINT32, numTriangles, trianglesFromOtherChild);
        if (trianglesFromOtherChild != 0)
        {
            // Prioritize having the smaller nodes on the left
            // TODO: Investigate better heuristics for node ordering
            bool IsLeft = GetLeftChildIndex(parentNodeIndex) == nodeIndex;
            bool IsOtherSideSmaller = numTriangles > trianglesFromOtherChild;
            swapChildIndices = IsLeft ? IsOtherSideSmaller : !IsOtherSideSmaller;
            nodeIndex = parentNodeIndex;
            numTriangles += trianglesFromOtherChild;

            if (ShouldPrepareUpdate)
            {
                aabbParentBuffer[GetLeftChildIndex(nodeIndex)] = nodeIndex;
                aabbParentBuffer[GetRightChildIndex(nodeIndex)] = nodeIndex;
            }
        }
        else
        {
            break;
        }
    }
    
}
