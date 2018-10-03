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
#ifndef RAYTACING_HELPER_H_INCLUDED
#define RAYTACING_HELPER_H_INCLUDED

#include "EmulatedPointer.hlsli"
#include "RayTracingHlslCompat.h"
#include "ShaderUtil.hlsli"

// Hidden
#define AABB_Min_Padding 0.001
#define NumberOfFloatsPerVertex 3
#define SizeOfVertex (NumberOfFloatsPerVertex * SizeOfFloat)
#define NumberOfVerticesPerTriangle 3
#define InvalidNodeIndex 9999999
static const int IsLeafFlag = 0x80000000;
static const int IsProceduralGeometryFlag = 0x40000000;
static const int LeafFlags = IsLeafFlag | IsProceduralGeometryFlag;

#define COMBINE_LEAF_NODES 1

// BVH description for the traversal shader
//struct BVHOffsets
//{
//    uint    offsetToBoxes;
//    uint    offsetToVertices;
//};

// No offsetof macro, so unfortunately need to do this hack

// Size of the above struct that is placed at the beginning of
// every BVH
static const int OffsetToBoxesOffset = 0;

// Bottom Level
static const int OffsetToPrimitivesOffset = 4;
static const int OffsetToPrimitiveMetaDataOffset = 8;

// Top Level
static const int OffsetToLeafNodeMetaDataOffset = 4;

static const int OffsetToTotalSize = 12;


int GetLeafIndexFromFlag(uint2 flag)
{
    return flag.x & ~LeafFlags;
}

uint GetActualParentIndex(uint index)
{
#if COMBINE_LEAF_NODES
    // Zero out HierarchyNode::bCollapseChildren flag
    return index & ~HierarchyNode::IsCollapseChildren;
#else
    return index;
#endif
}

// Reorganized AABB for faster intersection testing
struct BoundingBox
{
    float3 center;
    float3 halfDim;
};

#define GetOffsetToOffset(pointer, offsetOfOffset) \
    pointer.buffer.Load(offsetOfOffset + pointer.offsetInBytes) + pointer.offsetInBytes

#define GetOffsetToInstanceDesc(pointer) \
    GetOffsetToOffset(pointer, OffsetToLeafNodeMetaDataOffset)

static
int GetOffsetToBoxes(RWByteAddressBufferPointer pointer)
{
    // Optimization, Boxes are always at a fixed location
    // after the header so no need to read the offset 
    // from the BVH
    return pointer.offsetInBytes + SizeOfBVHOffsets;
}

static
int GetOffsetToVertices(RWByteAddressBufferPointer pointer)
{
    return GetOffsetToOffset(pointer, OffsetToPrimitivesOffset);
}

static
int GetOffsetToPrimitiveMetaData(RWByteAddressBufferPointer pointer)
{
    return GetOffsetToOffset(pointer, OffsetToPrimitiveMetaDataOffset);
}

bool IsLeaf(uint2 flag)
{
    return (flag.x & IsLeafFlag);
}

bool IsProceduralGeometry(uint2 flag)
{
    return (flag.x & IsProceduralGeometryFlag);
}

uint2 CreateFlag(uint leftNodeIndex, uint rightNodeIndex)
{
    uint2 flag;
    flag.x = leftNodeIndex & 0x00ffffff;
    flag.y = rightNodeIndex;
    return flag;
}

uint GetLeftNodeIndex(uint2 flag)
{
    return  flag.x & 0x00ffffff;
}

uint GetRightNodeIndex(uint2 flag)
{
    return flag.y;
}

uint GetBoxAddress(uint startAddress, uint boxIndex)
{
    return startAddress + boxIndex * SizeOfAABBNode;
}

static
BoundingBox RawDataToBoundingBox(int4 a, int4 b, out uint2 flags)
{
    BoundingBox box;
    box.center.x = asfloat(a.x);
    box.center.y = asfloat(a.y);
    box.center.z = asfloat(a.z);
    box.halfDim.x = asfloat(b.x);
    box.halfDim.y = asfloat(b.y);
    box.halfDim.z = asfloat(b.z);

    flags = uint2(a.w, b.w);

    return box;
}

static
BoundingBox GetBoxFromBuffer(RWByteAddressBuffer buffer, uint boxStartOffset, uint boxIndex)
{
    uint boxAddress = GetBoxAddress(boxStartOffset, boxIndex);

    int4 data1 = buffer.Load4(boxAddress);
    int4 data2 = buffer.Load4(boxAddress + 16);

    uint2 dummyFlag;
    return RawDataToBoundingBox(data1, data2, dummyFlag);
}

#define GetBVHMetadataAddress(byteAddressBufferPointer, offsetToInstanceDescs, leafIndex) \
    offsetToInstanceDescs + leafIndex * SizeOfBVHMetadata

#define GetBVHMetadataFromLeafIndex(byteAddressBufferPointer, offsetToLeafNodeMetaData, leafIndex) \
    LoadBVHMetadata(byteAddressBufferPointer.buffer, GetBVHMetadataAddress(byteAddressBufferPointer, offsetToLeafNodeMetaData, leafIndex))

static
BoundingBox BVHReadBoundingBox(RWByteAddressBufferPointer pointer, int nodeIndex, out uint2 flags)
{
    const uint boxAddress = GetBoxAddress(GetOffsetToBoxes(pointer), nodeIndex);

    const uint4 a = pointer.buffer.Load4(boxAddress);
    const uint4 b = pointer.buffer.Load4(boxAddress + 16);
    return RawDataToBoundingBox(a, b, flags);
}

void CompressBox(BoundingBox box, uint2 flags, out uint4 data1, out uint4 data2)
{
    data1.x = asuint(box.center.x);
    data1.y = asuint(box.center.y);
    data1.z = asuint(box.center.z);
    data1.w = flags.x;

    data2.x = asuint(box.halfDim.x);
    data2.y = asuint(box.halfDim.y);
    data2.z = asuint(box.halfDim.z);
    data2.w = flags.y;
}

uint GetPrimitiveMetaDataAddress(uint startAddress, uint triangleIndex)
{
    return startAddress + triangleIndex * SizeOfPrimitiveMetaData;
}

static
PrimitiveMetaData BVHReadPrimitiveMetaData(RWByteAddressBufferPointer pointer, int primitiveIndex)
{
    const uint readAddress = GetPrimitiveMetaDataAddress(GetOffsetToPrimitiveMetaData(pointer), primitiveIndex);

    PrimitiveMetaData metadata;
    const uint3 a = pointer.buffer.Load3(readAddress);
    metadata.GeometryContributionToHitGroupIndex = a.x;
    metadata.PrimitiveIndex = a.y;
    metadata.GeometryFlags = a.z;
    return metadata;
}

static
void WriteOnlyFlagToBuffer(RWByteAddressBuffer buffer, uint boxStartOffset, uint boxIndex, uint2 flags)
{
    uint boxAddress = GetBoxAddress(boxStartOffset, boxIndex);

    buffer.Store(boxAddress + 4 * 3, flags.x);
    buffer.Store(boxAddress + 4 * 7, flags.y);
}

static
void WriteBoxToBuffer(RWByteAddressBuffer buffer, uint boxStartOffset, uint boxIndex, BoundingBox box, uint2 flags)
{
    uint boxAddress = GetBoxAddress(boxStartOffset, boxIndex);

    uint4 data1, data2;
    CompressBox(box, flags, data1, data2);

    buffer.Store4(boxAddress, data1);
    buffer.Store4(boxAddress + 16, data2);
}

static
void BVHReadTriangle(
    RWByteAddressBufferPointer pointer,
    out float3 v0,
    out float3 v1,
    out float3 v2,
    uint triId)
{
    uint baseOffset = GetOffsetToVertices(pointer) + triId * SizeOfPrimitive
        + OffsetToPrimitiveData;

    const float4 a = asfloat(pointer.buffer.Load4(baseOffset));
    const float4 b = asfloat(pointer.buffer.Load4(baseOffset + 16));
    const float c = asfloat(pointer.buffer.Load(baseOffset + 32));

    v0 = a.xyz;
    v1 = float3(a.w, b.xy);
    v2 = float3(b.zw, c);
}

BoundingBox AABBtoBoundingBox(AABB aabb)
{
    BoundingBox box;
    box.center = (aabb.min + aabb.max) * 0.5f;
    box.halfDim = aabb.max - box.center;
    return box;
}

AABB BoundingBoxToAABB(BoundingBox boundingBox)
{
    AABB aabb;
    aabb.min = boundingBox.center - boundingBox.halfDim;
    aabb.max = boundingBox.center + boundingBox.halfDim;
    return aabb;
}

AABB RawDataToAABB(int4 a, int4 b)
{
    uint2 unusedFlags;
    return BoundingBoxToAABB(RawDataToBoundingBox(a, b, unusedFlags));
}

BoundingBox GetBoxDataFromTriangle(float3 v0, float3 v1, float3 v2, int triangleIndex, out uint2 flag)
{
    AABB aabb;
    aabb.min = min(min(v0, v1), v2);
    aabb.max = max(max(v0, v1), v2);

    aabb.min = min(aabb.min, aabb.max - AABB_Min_Padding);

    BoundingBox box = AABBtoBoundingBox(aabb);
    flag.x = triangleIndex | IsLeafFlag;
    flag.y = 1; // NumTriangles
    return box;
}

float3 GetMinCorner(BoundingBox box)
{
    return box.center - box.halfDim;
}

float3 GetMaxCorner(BoundingBox box)
{
    return box.center + box.halfDim;
}

BoundingBox GetBoxFromChildBoxes(BoundingBox boxA, int leftBoxIndex, BoundingBox boxB, int rightBoxIndex, out uint2 flag)
{
    AABB aabb;
    aabb.min = min(GetMinCorner(boxA), GetMinCorner(boxB));
    aabb.max = max(GetMaxCorner(boxA), GetMaxCorner(boxB));

    BoundingBox box = AABBtoBoundingBox(aabb);
    flag = CreateFlag(leftBoxIndex, rightBoxIndex);

    return box;
}

float Determinant(in AffineMatrix transform)
{
    return transform[0][0] * transform[1][1] * transform[2][2] -
        transform[0][0] * transform[2][1] * transform[1][2] -
        transform[1][0] * transform[0][1] * transform[2][2] +
        transform[1][0] * transform[2][1] * transform[0][2] +
        transform[2][0] * transform[0][1] * transform[1][2] -
        transform[2][0] * transform[1][1] * transform[0][2];
}

AffineMatrix InverseAffineTransform(AffineMatrix transform)
{
    const float invDet = rcp(Determinant(transform));

    AffineMatrix invertedTransform;
    invertedTransform[0][0] = invDet * (transform[1][1] * (transform[2][2] * 1.0f - 0.0f  * transform[2][3]) + transform[2][1] * (0.0f  * transform[1][3] - transform[1][2] * 1.0f) + 0.0f  * (transform[1][2] * transform[2][3] - transform[2][2] * transform[1][3]));
    invertedTransform[1][0] = invDet * (transform[1][2] * (transform[2][0] * 1.0f - 0.0f  * transform[2][3]) + transform[2][2] * (0.0f  * transform[1][3] - transform[1][0] * 1.0f) + 0.0f  * (transform[1][0] * transform[2][3] - transform[2][0] * transform[1][3]));
    invertedTransform[2][0] = invDet * (transform[1][3] * (transform[2][0] * 0.0f - 0.0f  * transform[2][1]) + transform[2][3] * (0.0f  * transform[1][1] - transform[1][0] * 0.0f) + 1.0f  * (transform[1][0] * transform[2][1] - transform[2][0] * transform[1][1]));
    invertedTransform[0][1] = invDet * (transform[2][1] * (transform[0][2] * 1.0f - 0.0f  * transform[0][3]) + 0.0f  * (transform[2][2] * transform[0][3] - transform[0][2] * transform[2][3]) + transform[0][1] * (0.0f  * transform[2][3] - transform[2][2] * 1.0f));
    invertedTransform[1][1] = invDet * (transform[2][2] * (transform[0][0] * 1.0f - 0.0f  * transform[0][3]) + 0.0f  * (transform[2][0] * transform[0][3] - transform[0][0] * transform[2][3]) + transform[0][2] * (0.0f  * transform[2][3] - transform[2][0] * 1.0f));
    invertedTransform[2][1] = invDet * (transform[2][3] * (transform[0][0] * 0.0f - 0.0f  * transform[0][1]) + 1.0f  * (transform[2][0] * transform[0][1] - transform[0][0] * transform[2][1]) + transform[0][3] * (0.0f  * transform[2][1] - transform[2][0] * 0.0f));
    invertedTransform[0][2] = invDet * (0.0f  * (transform[0][2] * transform[1][3] - transform[1][2] * transform[0][3]) + transform[0][1] * (transform[1][2] * 1.0f - 0.0f  * transform[1][3]) + transform[1][1] * (0.0f  * transform[0][3] - transform[0][2] * 1.0f));
    invertedTransform[1][2] = invDet * (0.0f  * (transform[0][0] * transform[1][3] - transform[1][0] * transform[0][3]) + transform[0][2] * (transform[1][0] * 1.0f - 0.0f  * transform[1][3]) + transform[1][2] * (0.0f  * transform[0][3] - transform[0][0] * 1.0f));
    invertedTransform[2][2] = invDet * (1.0f  * (transform[0][0] * transform[1][1] - transform[1][0] * transform[0][1]) + transform[0][3] * (transform[1][0] * 0.0f - 0.0f  * transform[1][1]) + transform[1][3] * (0.0f  * transform[0][1] - transform[0][0] * 0.0f));
    invertedTransform[0][3] = invDet * (transform[0][1] * (transform[2][2] * transform[1][3] - transform[1][2] * transform[2][3]) + transform[1][1] * (transform[0][2] * transform[2][3] - transform[2][2] * transform[0][3]) + transform[2][1] * (transform[1][2] * transform[0][3] - transform[0][2] * transform[1][3]));
    invertedTransform[1][3] = invDet * (transform[0][2] * (transform[2][0] * transform[1][3] - transform[1][0] * transform[2][3]) + transform[1][2] * (transform[0][0] * transform[2][3] - transform[2][0] * transform[0][3]) + transform[2][2] * (transform[1][0] * transform[0][3] - transform[0][0] * transform[1][3]));
    invertedTransform[2][3] = invDet * (transform[0][3] * (transform[2][0] * transform[1][1] - transform[1][0] * transform[2][1]) + transform[1][3] * (transform[0][0] * transform[2][1] - transform[2][0] * transform[0][1]) + transform[2][3] * (transform[1][0] * transform[0][1] - transform[0][0] * transform[1][1]));
    
    return invertedTransform;
}

AABB TransformAABB(AABB box, AffineMatrix transform)
{
    // TODO: This will largely inflate AABBs larger then they need to be.
    // Consider techniques that store coarse versions of the acc struct that 
    // can be transformed instead
    const uint verticesPerAABB = 8;
    float4 boxVertices[verticesPerAABB];
    boxVertices[0] = float4(box.min, 1.0);
    boxVertices[1] = float4(box.min.xy, box.max.z, 1.0);
    boxVertices[2] = float4(box.min.x, box.max.yz, 1.0);
    boxVertices[3] = float4(box.min.x, box.max.y, box.min.z, 1.0);
    boxVertices[4] = float4(box.max.x, box.min.yz, 1.0);
    boxVertices[6] = float4(box.max.x, box.min.y, box.max.z, 1.0);
    boxVertices[5] = float4(box.max.xy, box.min.z, 1.0);
    boxVertices[7] = float4(box.max, 1.0);

    AABB transformedBox;
    transformedBox.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
    transformedBox.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (uint i = 0; i < verticesPerAABB; i++)
    {
        float3 tranformedVertex = mul(transform, boxVertices[i]);
        transformedBox.min = min(transformedBox.min, tranformedVertex);
        transformedBox.max = max(transformedBox.max, tranformedVertex);
    }
    return transformedBox;
}

static const uint OffsetToAnyHitStateId = 4;
static const uint OffsetToIntersectionStateId = 8;
static
uint GetAnyHitStateId(ByteAddressBuffer shaderTable, uint recordOffset)
{
    return shaderTable.Load(recordOffset + OffsetToAnyHitStateId);
}

static
void GetAnyHitAndIntersectionStateId(ByteAddressBuffer shaderTable, uint recordOffset, out uint AnyHitStateId, out uint IntersectionStateId)
{
    uint2 stateIds = shaderTable.Load2(recordOffset + OffsetToAnyHitStateId);
    AnyHitStateId = stateIds.x;
    IntersectionStateId = stateIds.y;
}
#endif // RAYTACING_HELPER_H_INCLUDED