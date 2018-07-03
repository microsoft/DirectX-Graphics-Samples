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

static uint NumberOfInternalNodes;
static uint NumberOfAABBs;

uint GetLeftChildIndex(uint nodeIndex) 
{
    if (ShouldPerformUpdate)
    {
        uint2 boxInfo;
        GetLeftBoxFromBuffer(outputBVH, offsetToBoxes, nodeIndex, boxInfo);
        if (IsLeaf(boxInfo))
        {
            return GetLeafIndexFromInfo(boxInfo) + NumberOfInternalNodes;
        }
        else
        {
            return GetChildIndexFromInfo(boxInfo);   
        }
    }

    return hierarchyBuffer[nodeIndex].LeftChildIndex;
}

uint GetRightChildIndex(uint nodeIndex) 
{
    if (ShouldPerformUpdate)
    {
        uint2 boxInfo;
        GetRightBoxFromBuffer(outputBVH, offsetToBoxes, nodeIndex, boxInfo);
        if (IsLeaf(boxInfo))
        {
            return GetLeafIndexFromInfo(boxInfo) + NumberOfInternalNodes;
        }
        else
        {
            return GetChildIndexFromInfo(boxInfo);   
        }
    }

    return hierarchyBuffer[nodeIndex].RightChildIndex;
}

uint GetParentIndex(uint nodeIndex) 
{
    if (ShouldPerformUpdate)
    {
        return aabbParentBuffer[nodeIndex];
    }

    return hierarchyBuffer[nodeIndex].ParentIndex;
}

void CacheParentIndex(uint nodeIndex)
{
    aabbParentBuffer[GetLeftChildIndex(nodeIndex)] = nodeIndex;
    aabbParentBuffer[GetRightChildIndex(nodeIndex)] = nodeIndex;
}

BoundingBox BuildLeafBox(uint nodeIndex, uint offsetToPrimitives, out uint2 boxInfo)
{
    uint leafIndex = nodeIndex - NumberOfInternalNodes;
    return ComputeLeafAABB(leafIndex, offsetToPrimitives, boxInfo);
}

BoundingBox BuildCombinedBox(uint nodeIndex, out uint2 boxInfo)
{
    uint2 leftBoxInfo;
    BoundingBox leftBox = GetLeftBoxFromBuffer(outputBVH, offsetToBoxes, nodeIndex, leftBoxInfo);
    uint2 rightBoxInfo;
    BoundingBox rightBox = GetRightBoxFromBuffer(outputBVH, offsetToBoxes, nodeIndex, rightBoxInfo);

    boxInfo.x = nodeIndex;
    boxInfo.y = CombinePrimitiveFlags(leftBoxInfo.y, rightBoxInfo.y);

    return GetBoxFromChildBoxes(leftBox, rightBox);
}

BoundingBox BuildChildBox(uint nodeIndex, uint offsetToPrimitives, out uint2 boxInfo)
{
    bool isLeaf = nodeIndex >= NumberOfInternalNodes;

    if (isLeaf)
    {
        return BuildLeafBox(nodeIndex, offsetToPrimitives, boxInfo);
    }
    else
    {
        return BuildCombinedBox(nodeIndex, boxInfo);
    }
}

void BuildBoxFromChildren(uint nodeIndex, uint offsetToPrimitives, bool swapChildIndices) 
{
    uint leftChildIndex = GetLeftChildIndex(nodeIndex);
    uint rightChildIndex = GetRightChildIndex(nodeIndex);

    // Prioritize having the smaller nodes on the left
    // TODO: Investigate better heuristics for node ordering
    if (swapChildIndices)
    {
        uint temp = leftChildIndex;
        leftChildIndex = rightChildIndex;
        rightChildIndex = temp;
    }

    BoundingBox leftBox, rightBox;
    uint2 leftBoxInfo, rightBoxInfo;

    leftBox = BuildChildBox(leftChildIndex, offsetToPrimitives, leftBoxInfo);
    rightBox = BuildChildBox(rightChildIndex, offsetToPrimitives, rightBoxInfo);

    WriteLeftBoxToBuffer(outputBVH, offsetToBoxes, nodeIndex, leftBox, leftBoxInfo);
    WriteRightBoxToBuffer(outputBVH, offsetToBoxes, nodeIndex, rightBox, rightBoxInfo);
}

void BuildRootBoxWithDummy(uint nodeIndex, uint offsetToPrimitives)
{
    BoundingBox leafBox, dummyBox;
    uint2 leafBoxInfo, dummyBoxInfo;

    leafBox = BuildLeafBox(0, offsetToPrimitives, leafBoxInfo);
    dummyBox = CreateDummyBox(dummyBoxInfo);

    WriteLeftBoxToBuffer(outputBVH, offsetToBoxes, nodeIndex, leafBox, leafBoxInfo);
    WriteRightBoxToBuffer(outputBVH, offsetToBoxes, nodeIndex, leafBox, dummyBoxInfo);
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= Constants.NumberOfElements)
    {
        return;
    }

    NumberOfInternalNodes = GetNumInternalNodes(Constants.NumberOfElements);
    NumberOfAABBs = NumberOfInternalNodes + Constants.NumberOfElements;

    uint threadScratchAddress = DTid.x * SizeOfUINT32;
    uint nodeIndex = scratchMemory.Load(threadScratchAddress);

    if (nodeIndex >= NumberOfAABBs) 
    {
        return; 
    }

    uint offsetToPrimitives = outputBVH.Load(OffsetToPrimitivesOffset);

    uint numTriangles = 1;
    bool swapChildIndices = false;

    do
    {
        bool hasChildren = nodeIndex < NumberOfInternalNodes;

        if (hasChildren)
        {
            BuildBoxFromChildren(nodeIndex, offsetToPrimitives, swapChildIndices);

            if (ShouldPrepareUpdate)
            {
                CacheParentIndex(nodeIndex);
            }
        } 
        else if (nodeIndex == rootNodeIndex)
        {
            BuildRootBoxWithDummy(nodeIndex, offsetToPrimitives);
        }

        if (nodeIndex == rootNodeIndex)
        {
            return;
        }

        uint parentNodeIndex = GetParentIndex(nodeIndex);
        uint siblingTriangles;
        childNodesProcessedCounter.InterlockedAdd(parentNodeIndex * SizeOfUINT32, numTriangles, siblingTriangles);
        if (siblingTriangles == 0) // Need both children loaded to process parent
        {
            return;
        }

        bool isLeft = GetLeftChildIndex(parentNodeIndex) == nodeIndex;
        bool areSidesEven = numTriangles == siblingTriangles;
        bool isThisSideSmaller = numTriangles < siblingTriangles;
        swapChildIndices = (!areSidesEven) && (isLeft != isThisSideSmaller);
        
        nodeIndex = parentNodeIndex;
        numTriangles += siblingTriangles;
    } while (true);   
}

/*
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

#if COMBINE_LEAF_NODES
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
*/