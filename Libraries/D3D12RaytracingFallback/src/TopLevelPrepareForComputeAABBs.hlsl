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
#define HLSL
#include "ConstructAABBBindings.h"
#include "RayTracingHelper.hlsli"
#include "EmulatedPointerIntrinsics.hlsli"

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint instanceIndex = DTid.x;
    const uint NumberOfElements = Constants.NumberOfElements;
    bool IsEmptyAccelerationStructure = NumberOfElements == 0;
    if (instanceIndex >= NumberOfElements ||
        (IsEmptyAccelerationStructure && DTid.x != 0)) // Special case for constructing empty AS
    {
        return;
    }

    const uint OffsetToAABBNodes = SizeOfBVHOffsets;
    const uint NumberOfAABBs = GetNumAABBNodes(NumberOfElements);
    const uint OffsetToBVHMetadata = OffsetToAABBNodes + NumberOfAABBs * SizeOfAABBNode;
    const uint TotalSize = OffsetToBVHMetadata + NumberOfElements * SizeOfBVHMetadata;

    if (DTid.x == 0)
    {
        outputBVH.Store(OffsetToBoxesOffset, OffsetToAABBNodes);
        outputBVH.Store(OffsetToLeafNodeMetaDataOffset, OffsetToBVHMetadata);
        outputBVH.Store(OffsetToTotalSize, TotalSize);

        if (IsEmptyAccelerationStructure)
        {
            uint2 dummyFlags;
            BoundingBox dummyBox = CreateDummyBox(dummyFlags); 
            dummyBox.center = 0; dummyBox.halfDim = 0;
            WriteLeftBoxToBuffer(outputBVH, OffsetToAABBNodes, 0, dummyBox, dummyFlags);
            WriteRightBoxToBuffer(outputBVH, OffsetToAABBNodes, 0, dummyBox, dummyFlags);
            return;
        }
    }

    // Initialize argument for the AABB building phase, easier than uploading them
    // but probably not the best place for this logic
    const uint TotalNumNodes = GetNumInternalNodes(NumberOfElements) + NumberOfElements;
    {
        uint nodeIndex = TotalNumNodes - DTid.x - 1;
        uint threadScratchAddress = DTid.x * SizeOfUINT32;
        scratchMemory.Store(threadScratchAddress, nodeIndex);

        // Initialize count buffer to 0
        if (DTid.x < NumberOfAABBs)
        {
            childNodesProcessedCounter.Store(threadScratchAddress, 0);
        }
    }
}
