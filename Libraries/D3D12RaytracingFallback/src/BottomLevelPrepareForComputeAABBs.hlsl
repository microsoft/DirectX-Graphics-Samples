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

uint GetVertexAddress(int startAddress, uint triangleIndex, uint vertexIndex)
{
    return startAddress + triangleIndex * SizeOfTriangle + SizeOfVertex * vertexIndex;
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint triangleIndex = DTid.x;
    if (triangleIndex >= Constants.NumberOfElements)
    {
        return;
    }

    uint NumberOfInternalNodes = Constants.NumberOfElements - 1;
    uint NumberOfAABBs = NumberOfInternalNodes + Constants.NumberOfElements;

    // Initialize arguement for the AABB building phase, easier than uploading them
    // but probably not the best place for this logic
    {
        uint nodeIndex = NumberOfAABBs - DTid.x - 1;
        uint threadScratchAddress = DTid.x * SizeOfUINT32;
        scratchMemory.Store(threadScratchAddress, nodeIndex);

        // Initialize count buffer to 0
        if (DTid.x < NumberOfInternalNodes)
        {
            childNodesProcessedCounter.Store(threadScratchAddress, 0);
        }
    }

    uint offsetToBoxes = SizeOfBVHOffsets;
    uint offsetToTriangles = GetOffsetToTriangles(Constants.NumberOfElements);
    uint offsetToTriangleMetadata = offsetToTriangles + GetOffsetFromTrianglesToTriangleMetadata(Constants.NumberOfElements);
    uint totalSize = offsetToTriangleMetadata + Constants.NumberOfElements * SizeOfTriangleMetaData;

    if (DTid.x == 0)
    {
        outputBVH.Store(OffsetToBoxesOffset, offsetToBoxes);
        outputBVH.Store(OffsetToVerticesOffset, offsetToTriangles);
        outputBVH.Store(OffsetToTriangleMetadataOffset, offsetToTriangleMetadata);
        outputBVH.Store(OffsetToTotalSize, totalSize);
    }
}
