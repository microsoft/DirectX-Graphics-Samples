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

BoundingBox ComputeLeafAABB(uint leafIndex, uint unused, out uint2 flags)
{
    RWByteAddressBufferPointer topLevelAccelerationStructure = CreateRWByteAddressBufferPointer(outputBVH, 0);
    
    uint offsetToInstanceLevelDesc = GetOffsetToInstanceDesc(topLevelAccelerationStructure);
    BVHMetadata metadata = GetBVHMetadataFromLeafIndex(
        topLevelAccelerationStructure, offsetToInstanceLevelDesc, leafIndex);

    RWByteAddressBufferPointer bottomLevelAccelerationStructure = CreateRWByteAddressBufferPointerFromGpuVA(metadata.instanceDesc.AccelerationStructure);

    uint2 unusedFlag;
    AABB box = BoundingBoxToAABB(BVHReadBoundingBox(bottomLevelAccelerationStructure, 0, unusedFlag));
    AffineMatrix ObjectToWorld = CreateMatrix(metadata.ObjectToWorld);

    flags.x = leafIndex | IsLeafFlag;
    flags.y = 1;
    return AABBtoBoundingBox(TransformAABB(box, ObjectToWorld));
}

#include "ComputeAABBs.hlsli"
