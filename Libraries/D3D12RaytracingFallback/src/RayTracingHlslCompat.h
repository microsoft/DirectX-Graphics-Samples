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

#define     TRAVERSAL_MAX_STACK_DEPTH       32

#define     MAX_TRIS_IN_LEAF                1

#ifdef HLSL
#include "EmulatedPointer.hlsli"
#else

#pragma once

#include "HlslCompat.h"
#pragma pack(push, 1)
#endif

struct HierarchyNode
{
    uint ParentIndex;
    uint LeftChildIndex;
    uint RightChildIndex;
};

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
#ifndef HLSL
static_assert(sizeof(Triangle) == SizeOfTriangle, L"Incorrect sizeof for Triangle");
#endif


struct TriangleMetaData
{
    uint GeometryContributionToHitGroupIndex;
    uint PrimitiveIndex;
};
#define SizeOfTriangleMetaData (4 * 2)
#ifndef HLSL
static_assert(sizeof(TriangleMetaData) == SizeOfTriangleMetaData, L"Incorrect sizeof for TriangleMetaData");
#endif

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

#define SizeOfRaytracingInstanceDesc 64
#define SizeOfBVHMetadata 112
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
};

#ifdef HLSL
#define Store4StrideInBytes 16
void StoreBVHMetadataToRawData(RWByteAddressBuffer buffer, uint offset, BVHMetadata metadata)
{
    uint4 data[7];
        
    data[0] = asuint(metadata.instanceDesc.Transform[0]);
    data[1] = asuint(metadata.instanceDesc.Transform[1]);
    data[2] = asuint(metadata.instanceDesc.Transform[2]);
    data[3].x = metadata.instanceDesc.InstanceIDAndMask;
    data[3].y = metadata.instanceDesc.InstanceContributionToHitGroupIndexAndFlags;
    data[3].zw = metadata.instanceDesc.AccelerationStructure;
    data[4] = asuint(metadata.ObjectToWorld[0]);
    data[5] = asuint(metadata.ObjectToWorld[1]);
    data[6] = asuint(metadata.ObjectToWorld[2]);

    [unroll]
    for (uint i = 0; i < 7; i++)
    {
        buffer.Store4(offset + Store4StrideInBytes * i, data[i]);
    }
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

BVHMetadata LoadBVHMetadata(RWByteAddressBuffer buffer, uint offset)
{
    uint4 data[7];
    [unroll]
    for (uint i = 0; i < 7; i++)
    {
        data[i] = buffer.Load4(offset + Store4StrideInBytes * i);
    }
    BVHMetadata metadata;
    metadata.instanceDesc = RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
    metadata.ObjectToWorld[0] = asfloat(data[4]);
    metadata.ObjectToWorld[1] = asfloat(data[5]);
    metadata.ObjectToWorld[2] = asfloat(data[6]);
    return metadata;
}

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
    uint    offsetToTriangleMetadata;
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
uint GetOffsetFromTrianglesToTriangleMetadata(uint numTriangles)
{
    return SizeOfTriangle * numTriangles;
}

inline
uint GetOffsetToLeafNodeAABBs(uint numElements)
{
    uint numInternalLeafNodes = GetNumInternalNodes(numElements);
    return SizeOfBVHOffsets + SizeOfAABBNode * numInternalLeafNodes;
}

inline
uint GetOffsetToTriangles(uint numTriangles)
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

