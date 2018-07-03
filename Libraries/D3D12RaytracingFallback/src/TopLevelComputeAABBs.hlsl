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

BoundingBox ComputeLeafAABB(uint leafIndex, uint unused, out uint2 boxInfo)
{
    RWByteAddressBufferPointer topLevelAccelerationStructure = CreateRWByteAddressBufferPointer(outputBVH, 0);
    
    uint offsetToInstanceLevelDesc = GetOffsetToInstanceDesc(topLevelAccelerationStructure);
    BVHMetadata metadata = GetBVHMetadataFromLeafIndex(
        topLevelAccelerationStructure, offsetToInstanceLevelDesc, leafIndex);

    RWByteAddressBufferPointer bottomLevelAccelerationStructure = CreateRWByteAddressBufferPointerFromGpuVA(metadata.instanceDesc.AccelerationStructure);

    // Combine top two from bottom-level AABB
    uint2 leftRootBoxInfo, rightRootBoxInfo;
    BoundingBox leftRootBox = GetLeftBoxFromBVH(bottomLevelAccelerationStructure, 0, leftRootBoxInfo);
    BoundingBox rightRootBox = GetRightBoxFromBVH(bottomLevelAccelerationStructure, 0, rightRootBoxInfo);
    
    boxInfo.x = leafIndex;
    boxInfo.y = IsLeafFlag | MinNumberOfLeafNodeBVHs;

    AABB box = GetAABBFromChildBoxes(leftRootBox, rightRootBox);
    AffineMatrix ObjectToWorld = CreateMatrix(metadata.ObjectToWorld);

    return AABBtoBoundingBox(TransformAABB(box, ObjectToWorld));
}

#include "ComputeAABBs.hlsli"
