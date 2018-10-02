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
#ifndef RAYTRACING_HLSL_COMPAT_H_INCLUDED
#define RAYTRACING_HLSL_COMPAT_H_INCLUDED
#include "WaveDimensions.h"

//*********----- AMD driver limitation workarounds ------******************
//  
// Set following to 0 to get MiniEngineSample to run on AMD.
//
// Enables an unroll in treelet. Fails to compile on AMD.
#define USE_EXPLICIT_UNROLL_IN_FORMTREELET 1

// Enables treelet BVH optimization. TDRs on AMD.
#define ENABLE_TREELET_REORDERING 1
//
//*************************************************************************


#define     TRAVERSAL_MAX_STACK_DEPTH       32

#define     MAX_TRIS_IN_LEAF                1

#ifdef HLSL
#include "EmulatedPointer.hlsli"
#else

#pragma once

#include "HlslCompat.h"
#pragma pack(push, 1)
#endif

#define SizeOfFloat 4
#define SizeOfUINT16 2
#define SizeOfUINT32 4

struct HierarchyNode
{
#ifdef HLSL
    uint ParentIndex;
#else
    uint ParentIndex : 31;
    uint bCollapseChildren : 1;
#endif
    uint LeftChildIndex;
    uint RightChildIndex;

    static const int IsCollapseChildren = 0x80000000; // for extracting HierarchyNode::bCollapseChildren
};

struct AABB
{
#ifdef HLSL
    float3  min;
    float3  max;
#else
    union
    {
        float3  min;
        float   minArr[3];
    };
    union
    {
        float3  max;
        float   maxArr[3];
    };
#endif
};
#define SizeOfAABB (6 * 4)
#ifdef HLSL
AABB RawDataToAABB(uint4 a, uint2 b)
{
    AABB aabb;
    aabb.min = asfloat(a.xyz);
    aabb.max = asfloat(uint3(a.w, b.xy));

    return aabb;
}

void AABBToRawData(in AABB aabb, out uint4 a, out uint2 b)
{
    a = asuint(float4(aabb.min.xyz, aabb.max.x));
    b = asuint(float2(aabb.max.yz));
}
#else
static_assert(sizeof(AABB) == SizeOfAABB, L"Incorrect sizeof for AABB");
#endif

struct Triangle
{
#ifdef HLSL
    float3 v0;
    float3 v1;
    float3 v2;
#else
    union
    {
        struct
        {
            float3 v0;
            float3 v1;
            float3 v2;
        };
        float3 v[3];
    };
#endif
};

#define SizeOfTriangle (9 * 4)
#ifdef HLSL
Triangle RawDataToTriangle(uint4 a, uint4 b, uint c)
{
    Triangle tri;
    tri.v0 = asfloat(a.xyz);
    tri.v1 = asfloat(uint3(a.w, b.xy));
    tri.v2 = asfloat(uint3(b.zw, c));

    return tri;
}

void TriangleToRawData(in Triangle tri, out uint4 a, out uint4 b, out uint c)
{
    a = asuint(float4(tri.v0.xyz, tri.v1.x));
    b = asuint(float4(tri.v1.yz, tri.v2.xy));
    c = asuint(tri.v2.z);
}
#else
static_assert(sizeof(Triangle) == SizeOfTriangle, L"Incorrect sizeof for Triangle");
#endif

#define TRIANGLE_TYPE 0x1
#define PROCEDURAL_PRIMITIVE_TYPE 0x2
struct Primitive
{
    uint PrimitiveType;
#ifdef HLSL
    uint4 data0;
    uint4 data1;
    uint data2;
#else
    union
    {
        Triangle triangle;
        AABB aabb;
    };
#endif
};
#ifdef HLSL
Primitive NullPrimitive()
{
    Primitive primitive;
    primitive.PrimitiveType = 0;
    primitive.data0 = 0;
    primitive.data1 = 0;
    primitive.data2 = 0;
    return primitive;
}

Primitive CreateProceduralGeometryPrimitive(AABB aabb)
{
    Primitive primitive = NullPrimitive();
    primitive.PrimitiveType = PROCEDURAL_PRIMITIVE_TYPE;
    AABBToRawData(aabb, primitive.data0, primitive.data1.xy);
    return primitive;
}

Primitive CreateTrianglePrimitive(Triangle tri)
{
    Primitive primitive = NullPrimitive();
    primitive.PrimitiveType = TRIANGLE_TYPE;
    TriangleToRawData(tri, primitive.data0, primitive.data1, primitive.data2);
    return primitive;
}

Triangle GetTriangle(Primitive prim)
{
    return RawDataToTriangle(prim.data0, prim.data1, prim.data2);
}

AABB GetProceduralPrimitiveAABB(Primitive prim)
{
    return RawDataToAABB(prim.data0, prim.data1.xy);
}

#endif
#define SizeOfPrimitive 40
#define OffsetToPrimitiveData 4
#ifndef HLSL
static_assert(sizeof(Primitive) == SizeOfPrimitive, L"Incorrect sizeof for Primitive");
static_assert(offsetof(Primitive, triangle) == OffsetToPrimitiveData, L"Incorrect offset to Primitive data");
#endif

struct PrimitiveMetaData
{
    uint GeometryContributionToHitGroupIndex;
    uint PrimitiveIndex;
    uint GeometryFlags;
};
#define SizeOfPrimitiveMetaData (4 * 3)
#ifndef HLSL
static_assert(sizeof(PrimitiveMetaData) == SizeOfPrimitiveMetaData, L"Incorrect sizeof for PrimitiveMetaData");
#endif

#define SizeOfRaytracingInstanceDesc 64
#define SizeOfBVHMetadata 116
#define RaytracingInstanceDescOffsetToPointer 56
#ifdef HLSL
#define AffineMatrix float3x4
AffineMatrix CreateMatrix(float4 rows[3])
{
    AffineMatrix mat;
    mat[0] = rows[0];
    mat[1] = rows[1];
    mat[2] = rows[2];
    return mat;
}

static const uint D3D12_RAYTRACING_INSTANCE_FLAG_NONE = 0;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE = 0x4;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8;

static const uint D3D12_RAYTRACING_GEOMETRY_FLAG_NONE = 0;
static const uint D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE = 0x1;
static const uint D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2;

struct RaytracingInstanceDesc
{
    float4 Transform[3];
    uint InstanceIDAndMask;
    uint InstanceContributionToHitGroupIndexAndFlags;
    GpuVA AccelerationStructure;
};
#endif 

struct BVHMetadata
{
#ifdef HLSL
    RaytracingInstanceDesc instanceDesc;
#else
    D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDesc;
#endif
    float4 ObjectToWorld[3];
    uint InstanceIndex;
};

#ifdef HLSL
#define Store4StrideInBytes 16
static
void StoreBVHMetadataToRawData(RWByteAddressBuffer buffer, uint offset, BVHMetadata metadata)
{
    uint4 data[7];
    uint dataRemainder;
        
    data[0] = asuint(metadata.instanceDesc.Transform[0]);
    data[1] = asuint(metadata.instanceDesc.Transform[1]);
    data[2] = asuint(metadata.instanceDesc.Transform[2]);
    data[3].x = metadata.instanceDesc.InstanceIDAndMask;
    data[3].y = metadata.instanceDesc.InstanceContributionToHitGroupIndexAndFlags;
    data[3].zw = metadata.instanceDesc.AccelerationStructure;
    data[4] = asuint(metadata.ObjectToWorld[0]);
    data[5] = asuint(metadata.ObjectToWorld[1]);
    data[6] = asuint(metadata.ObjectToWorld[2]);
    dataRemainder = metadata.InstanceIndex;

    [unroll]
    for (uint i = 0; i < 7; i++)
    {
        buffer.Store4(offset, data[i]);
        offset += Store4StrideInBytes;
    }
    buffer.Store(offset, dataRemainder);
}

RaytracingInstanceDesc RawDataToRaytracingInstanceDesc(uint4 a, uint4 b, uint4 c, uint4 d)
{
    RaytracingInstanceDesc desc;
    desc.Transform[0] = asfloat(a);
    desc.Transform[1] = asfloat(b);
    desc.Transform[2] = asfloat(c);

    desc.InstanceIDAndMask = d.x;
    desc.InstanceContributionToHitGroupIndexAndFlags = d.y;
    desc.AccelerationStructure = d.zw;

    return desc;
}

static
BVHMetadata LoadBVHMetadata(RWByteAddressBuffer buffer, uint offset)
{
    uint4 data[7];
    [unroll]
    for (uint i = 0; i < 7; i++)
    {
        data[i] = buffer.Load4(offset);
        offset += Store4StrideInBytes;
    }
    BVHMetadata metadata;
    metadata.instanceDesc = RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
    metadata.ObjectToWorld[0] = asfloat(data[4]);
    metadata.ObjectToWorld[1] = asfloat(data[5]);
    metadata.ObjectToWorld[2] = asfloat(data[6]);
    metadata.InstanceIndex = buffer.Load(offset);

    return metadata;
}

static
RaytracingInstanceDesc LoadRaytracingInstanceDesc(RWByteAddressBuffer buffer, uint offset)
{
    uint4 data[4];
    [unroll]
    for (uint i = 0; i < 4; i++)
    {
        data[i] = buffer.Load4(offset + Store4StrideInBytes * i);
    }
    return RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
}

static
RaytracingInstanceDesc LoadRaytracingInstanceDesc(ByteAddressBuffer buffer, uint offset)
{
    uint4 data[4];
    [unroll]
    for (uint i = 0; i < 4; i++)
    {
        data[i] = buffer.Load4(offset + Store4StrideInBytes * i);
    }
    return RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
}

uint GetInstanceContributionToHitGroupIndex(RaytracingInstanceDesc desc)
{
    return desc.InstanceContributionToHitGroupIndexAndFlags & 0xffffff;
}

uint GetInstanceFlags(RaytracingInstanceDesc desc)
{
    return desc.InstanceContributionToHitGroupIndexAndFlags >> 24;
}

uint GetInstanceMask(RaytracingInstanceDesc desc)
{
    return desc.InstanceIDAndMask >> 24;
}

uint GetInstanceID(RaytracingInstanceDesc desc)
{
  return desc.InstanceIDAndMask & 0xFFFFFF;
}
#else
static_assert(sizeof(BVHMetadata) == SizeOfBVHMetadata, L"Incorrect sizeof for BVHMetadata");
static_assert(sizeof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC) == SizeOfRaytracingInstanceDesc, L"Incorrect sizeof for RaytracingInstanceDesc");
static_assert(offsetof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC, AccelerationStructure) == RaytracingInstanceDescOffsetToPointer, L"Incorrect offset calculated for RaytracingInstanceDesc");
#endif

struct AABBNode
{
    float    center[3];
#ifdef HLSL
    uint    flags;
#else
    union
    {
        struct
        {
            uint    leftNodeIndex : 24;
            uint    separatingAxis : 3;
        } internalNode;

        struct
        {
            uint    firstTriangleId : 24;
            uint    numTriangleIds  : 7;
        } leafNode;

        uint nodeAllBits;

        struct
        {
            uint         : 31;
            uint    leaf : 1;
        };
    };
#endif

    float halfDim[3];
#ifdef HLSL
    uint    rightNodeIndex;
#else
    union
    {
        uint    rightNodeIndex;
        uint    numTriangles;
    };
#endif
};
#define SizeOfAABBNode (4 * 8)
#ifndef HLSL
static_assert(sizeof(AABBNode) == SizeOfAABBNode, L"Incorrect sizeof for AABB");
#endif

// BVH description for the traversal shader
struct BVHOffsets
{
    uint    offsetToBoxes;
    uint    offsetToVertices;
    uint    offsetToPrimitiveMetaData;
    uint    totalSize;
};
#define SizeOfBVHOffsets (4 * 4)
#ifndef HLSL
static_assert(sizeof(BVHOffsets) == SizeOfBVHOffsets, L"Incorrect sizeof for BVHOffsets");
#endif

inline
uint GetNumInternalNodes(uint numLeaves)
{
    return numLeaves - 1;
}

inline
uint GetOffsetFromSortedIndicesToAABBParents(uint numPrimitives) {
    return SizeOfUINT32 * numPrimitives;
}

inline
uint GetOffsetToBVHSortedIndices(uint numElements) {
    uint totalNodes = numElements + GetNumInternalNodes(numElements);
    return SizeOfBVHOffsets + SizeOfAABBNode * totalNodes + SizeOfBVHMetadata * numElements;
}

inline
uint GetOffsetFromPrimitiveMetaDataToSortedIndices(uint numPrimitives)
{
    return SizeOfPrimitiveMetaData * numPrimitives;
}

inline
uint GetOffsetFromPrimitivesToPrimitiveMetaData(uint numPrimitives)
{
    return SizeOfPrimitive * numPrimitives;
}

inline
uint GetOffsetToLeafNodeAABBs(uint numElements)
{
    uint numInternalLeafNodes = GetNumInternalNodes(numElements);
    return SizeOfBVHOffsets + SizeOfAABBNode * numInternalLeafNodes;
}

inline
uint GetOffsetToPrimitives(uint numTriangles)
{
    uint numAABBs = numTriangles + GetNumInternalNodes(numTriangles);
    return SizeOfBVHOffsets + SizeOfAABBNode * numAABBs;
}

inline
uint GetOffsetFromLeafNodesToBottomLevelMetadata(uint numElements)
{
    return SizeOfAABBNode * numElements;
}

#ifndef HLSL
#pragma pack(pop)
#endif
#endif // RAYTRACING_HLSL_COMPAT_H_INCLUDED

