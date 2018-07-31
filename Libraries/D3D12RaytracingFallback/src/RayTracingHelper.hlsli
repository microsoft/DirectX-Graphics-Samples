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
static const int IsLeafFlag               = 0x80000000;
static const int IsProceduralGeometryFlag = 0x40000000;
static const int IsDummyFlag              = 0x20000000;
static const int LeafFlags = IsLeafFlag | IsProceduralGeometryFlag | IsDummyFlag;
static const int MinNumberOfPrimitives = 1;
static const int MinNumberOfLeafNodeBVHs = 1;

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
int GetOffsetToAABBNodes(RWByteAddressBufferPointer pointer)
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

bool IsLeaf(uint2 info)
{
    return (info.y & IsLeafFlag);
}

bool IsProceduralGeometry(uint2 info)
{
    return (info.y & IsProceduralGeometryFlag);
}

bool IsDummy(uint2 info)
{
    return (info.y & IsDummyFlag);
}

uint GetLeafIndexFromInfo(uint2 info)
{
    return info.x;
}

uint GetChildIndexFromInfo(uint2 info)
{
    return info.x;
}

uint GetLeafFlagsFromPrimitiveFlags(uint flags)
{
    return flags & LeafFlags;
}

uint GetNumPrimitivesFromPrimitiveFlags(uint flags)
{
    return flags & ~LeafFlags;
}

uint GetNumPrimitivesFromInfo(uint2 info)
{
    return GetNumPrimitivesFromPrimitiveFlags(info.y);
}

uint CombinePrimitiveFlags(uint flags1, uint flags2)
{
    uint combinedFlags =     GetLeafFlagsFromPrimitiveFlags(flags1)
                           | GetLeafFlagsFromPrimitiveFlags(flags2);
    combinedFlags &= ~(IsLeafFlag | IsDummyFlag);

    uint combinedPrimitives = GetNumPrimitivesFromPrimitiveFlags(flags1)
                            + GetNumPrimitivesFromPrimitiveFlags(flags2);

    return combinedFlags | combinedPrimitives;
}

uint GetAABBNodeAddress(uint startAddress, uint boxIndex)
{
    return startAddress + boxIndex * SizeOfAABBNode;
}

BoundingBox CreateDummyBox(out uint2 info)
{
    BoundingBox box;
    info = uint2(0, IsLeafFlag | IsDummyFlag);
    return box;
}

static
void CompressBox(BoundingBox box, uint childIndex, uint primitiveFlags, out uint4 data1, out uint4 data2)
{
    data1.x = childIndex;
    data1.y = asuint(box.center.x);
    data1.z = asuint(box.center.y);
    data1.w = asuint(box.center.z);

    data2.x = asuint(box.halfDim.x);
    data2.y = asuint(box.halfDim.y);
    data2.z = asuint(box.halfDim.z);
    data2.w = primitiveFlags;
}

static
void WriteBoxToBuffer(
    RWByteAddressBuffer buffer,
    uint boxAddress,
    BoundingBox box,
    uint2 extraInfo)
{
    uint4 data1, data2;
    CompressBox(box, extraInfo.x, extraInfo.y, data1, data2);

    buffer.Store4(boxAddress, data1);
    buffer.Store4(boxAddress + 16, data2);
}

static
void WriteLeftBoxToBuffer(
    RWByteAddressBuffer buffer, 
    uint nodeStartOffset, 
    uint nodeIndex, 
    BoundingBox box, 
    uint2 extraInfo)
{
    uint boxAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex);
    WriteBoxToBuffer(buffer, boxAddress, box, extraInfo);
}

static
void WriteRightBoxToBuffer(
    RWByteAddressBuffer buffer, 
    uint nodeStartOffset, 
    uint nodeIndex, 
    BoundingBox box, 
    uint2 extraInfo)
{
    uint boxAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex) + SizeOfAABBNodeSibling;
    WriteBoxToBuffer(buffer, boxAddress, box, extraInfo);
}

static
BoundingBox RawDataToBoundingBox(uint4 a, uint4 b, out uint2 extraInfo)
{
    BoundingBox box;
    box.center.x = asfloat(a.y);
    box.center.y = asfloat(a.z);
    box.center.z = asfloat(a.w);
    box.halfDim.x = asfloat(b.x);
    box.halfDim.y = asfloat(b.y);
    box.halfDim.z = asfloat(b.z);
    extraInfo.x = a.x;
    extraInfo.y = b.w;
    return box;
}

static
BoundingBox GetBoxFromBuffer(RWByteAddressBuffer buffer, uint aabbNodeAddress, out uint2 extraInfo)
{
    uint4 data1 = buffer.Load4(aabbNodeAddress);
    uint4 data2 = buffer.Load4(aabbNodeAddress + 16);

    return RawDataToBoundingBox(data1, data2, extraInfo);
}

static
BoundingBox GetLeftBoxFromBuffer(
    RWByteAddressBuffer buffer, 
    uint nodeStartOffset, 
    uint parentNodeIndex,
    out uint2 extraInfo)
{
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, parentNodeIndex);
    return GetBoxFromBuffer(buffer, aabbNodeAddress, extraInfo);
}

static
BoundingBox GetRightBoxFromBuffer(
    RWByteAddressBuffer buffer, 
    uint nodeStartOffset, 
    uint parentNodeIndex,
    out uint2 extraInfo)
{
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, parentNodeIndex) + SizeOfAABBNodeSibling;
    return GetBoxFromBuffer(buffer, aabbNodeAddress, extraInfo);
}

static
BoundingBox GetLeftBoxFromBVH(RWByteAddressBufferPointer pointer, int nodeIndex, out uint2 extraInfo)
{
    uint nodeStartOffset = GetOffsetToAABBNodes(pointer);
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex);
    return GetBoxFromBuffer(pointer.buffer, aabbNodeAddress, extraInfo);
}

static
BoundingBox GetRightBoxFromBVH(RWByteAddressBufferPointer pointer, int nodeIndex, out uint2 extraInfo)
{
    uint nodeStartOffset = GetOffsetToAABBNodes(pointer);
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex) + SizeOfAABBNodeSibling;
    return GetBoxFromBuffer(pointer.buffer, aabbNodeAddress, extraInfo);
}


#define GetBVHMetadataAddress(byteAddressBufferPointer, offsetToInstanceDescs, leafIndex) \
    offsetToInstanceDescs + leafIndex * SizeOfBVHMetadata

#define GetBVHMetadataFromLeafIndex(byteAddressBufferPointer, offsetToLeafNodeMetaData, leafIndex) \
    LoadBVHMetadata(byteAddressBufferPointer.buffer, GetBVHMetadataAddress(byteAddressBufferPointer, offsetToLeafNodeMetaData, leafIndex))

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

static
BoundingBox AABBtoBoundingBox(AABB aabb)
{
    BoundingBox box;
    box.center = (aabb.min + aabb.max) * 0.5f;
    box.halfDim = aabb.max - box.center;
    return box;
}

static
AABB BoundingBoxToAABB(BoundingBox boundingBox)
{
    AABB aabb;
    aabb.min = boundingBox.center - boundingBox.halfDim;
    aabb.max = boundingBox.center + boundingBox.halfDim;
    return aabb;
}

static
AABB RawDataToAABB(int4 a, int4 b)
{
    uint2 unusedInfo;
    return BoundingBoxToAABB(RawDataToBoundingBox(a, b, unusedInfo));
}

static
BoundingBox GetBoxDataFromTriangle(float3 v0, float3 v1, float3 v2, int triangleIndex, out uint2 triangleInfo)
{
    AABB aabb;
    aabb.min = min(min(v0, v1), v2);
    aabb.max = max(max(v0, v1), v2);

    aabb.min = min(aabb.min, aabb.max - AABB_Min_Padding);

    BoundingBox box = AABBtoBoundingBox(aabb);
    triangleInfo.x = triangleIndex;
    triangleInfo.y = IsLeafFlag | MinNumberOfPrimitives;
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

AABB GetAABBFromChildBoxes(BoundingBox boxA, BoundingBox boxB)
{
    AABB aabb;
    aabb.min = min(GetMinCorner(boxA), GetMinCorner(boxB));
    aabb.max = max(GetMaxCorner(boxA), GetMaxCorner(boxB));

    return aabb;
}

BoundingBox GetBoxFromChildBoxes(BoundingBox boxA, BoundingBox boxB)
{
    return AABBtoBoundingBox(GetAABBFromChildBoxes(boxA, boxB));
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
