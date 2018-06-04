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
#include "LoadInstancesBindings.h"
#include "RayTracingHelper.hlsli"
#include "EmulatedPointerIntrinsics.hlsli"

static const uint sizeOfGpuVA = 8;
static const uint sizeOfUint4 = 16;

struct ByteAddressBufferPointer
{
    ByteAddressBuffer buffer;
    uint offsetInBytes;
};

ByteAddressBuffer PointerGetSRVBuffer(GpuVA address)
{
    return DescriptorHeapSRVBufferTable[NonUniformResourceIndex(address[EmulatedPointerDescriptorHeapIndex])];
}

ByteAddressBufferPointer CreateByteAddressBufferPointerFromGpuVA(GpuVA address)
{
    ByteAddressBufferPointer pointer;
    pointer.buffer = PointerGetSRVBuffer(address);
    pointer.offsetInBytes = PointerGetBufferStartOffset(address);
    return pointer;
}

GpuVA GetInstanceAddress(int instanceIndex)
{
    return instanceDescs.Load2(instanceIndex * sizeOfGpuVA);
}

RaytracingInstanceDesc GetInstanceDesc(uint instanceIndex)
{
#ifdef ARRAY_OF_POINTERS
    GpuVA instanceDescAddress = GetInstanceAddress(instanceIndex);
    ByteAddressBufferPointer byteAddressPointer = CreateByteAddressBufferPointerFromGpuVA(instanceDescAddress);
    ByteAddressBuffer buffer = byteAddressPointer.buffer;
    uint offsetInBytes = byteAddressPointer.offsetInBytes;
#endif
#ifdef ARRAY_OF_INSTANCES
    ByteAddressBuffer buffer = instanceDescs;
    uint offsetInBytes = instanceIndex * SizeOfRaytracingInstanceDesc;
#endif
    return LoadRaytracingInstanceDesc(buffer, offsetInBytes);
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint instanceIndex = DTid.x;
    if (instanceIndex >= Constants.NumberOfElements)
    {
        return;
    }

    uint outputIndex = GetOutputIndex(instanceIndex);

    uint totalSizeOfAABBNodes = Constants.NumberOfElements * SizeOfAABBNode;
    const uint offsetToLeafNodeMetadata = totalSizeOfAABBNodes;

    RaytracingInstanceDesc instanceDesc = GetInstanceDesc(instanceIndex);

    RWByteAddressBufferPointer bottomLevelByteAddressPointer = CreateRWByteAddressBufferPointerFromGpuVA(instanceDesc.AccelerationStructure);

    int2 unusedFlag;
    AABB box = BoundingBoxToAABB(BVHReadBoundingBox(bottomLevelByteAddressPointer, 0, unusedFlag));
    AffineMatrix ObjectToWorld = CreateMatrix(instanceDesc.Transform);

    // Convert the instance transform from ObjectToWorld->WorldToObject which is all we'll need for BVH build/Raytracing
    AffineMatrix WorldToObject = InverseAffineTransform(ObjectToWorld);
    instanceDesc.Transform[0] = WorldToObject[0];
    instanceDesc.Transform[1] = WorldToObject[1];
    instanceDesc.Transform[2] = WorldToObject[2];
   
    // The AABBs for all top level nodes needs to be in world-space
    AABB transformedBox = TransformAABB(box, ObjectToWorld);

    int leafFlag = IsLeafFlag | instanceIndex;
    WriteBoxToBuffer(outputBVH, 0, outputIndex, AABBtoBoundingBox(transformedBox), leafFlag);
    
    BVHMetadata metadata;
    metadata.instanceDesc = instanceDesc;
    metadata.InstanceIndex = instanceIndex;

    metadata.ObjectToWorld[0] = ObjectToWorld[0];
    metadata.ObjectToWorld[1] = ObjectToWorld[1];
    metadata.ObjectToWorld[2] = ObjectToWorld[2];
    StoreBVHMetadataToRawData(outputBVH, offsetToLeafNodeMetadata + outputIndex * SizeOfBVHMetadata, metadata);
}
