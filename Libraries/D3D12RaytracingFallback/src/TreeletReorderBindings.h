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
#pragma once
#include "RaytracingHlslCompat.h"
#ifdef HLSL
#include "ShaderUtil.hlsli"
#endif

struct InputConstants
{
    uint NumberOfElements;
    uint MinTrianglesPerTreelet;
};

// CBVs
#define ConstantsRegister 0

// UAVs
#define HierarchyBufferRegister 0
#define NumTrianglesBufferRegister 1
#define AABBBufferRegister 2
#define ElementBufferRegister 3
#define BaseTreeletsCountBufferRegister 4
#define BaseTreeletsIndexBufferRegister 5

static const uint FullTreeletSize = 7;

#ifdef HLSL
// These need to be UAVs despite being read-only because the fallback layer only gets a 
// GPU VA and the API doesn't allow any way to transition that GPU VA from UAV->SRV
RWStructuredBuffer<Primitive> InputBuffer : UAV_REGISTER(ElementBufferRegister);

globallycoherent RWByteAddressBuffer NumTrianglesBuffer : UAV_REGISTER(NumTrianglesBufferRegister);
globallycoherent RWStructuredBuffer<HierarchyNode> hierarchyBuffer : UAV_REGISTER(HierarchyBufferRegister);
globallycoherent RWStructuredBuffer<AABB> AABBBuffer : UAV_REGISTER(AABBBufferRegister);

globallycoherent RWByteAddressBuffer BaseTreeletsCountBuffer : UAV_REGISTER(BaseTreeletsCountBufferRegister);
RWStructuredBuffer<uint> BaseTreeletsIndexBuffer : UAV_REGISTER(BaseTreeletsIndexBufferRegister);

cbuffer TreeletConstants : CONSTANT_REGISTER(ConstantsRegister)
{
    InputConstants Constants;
};

#define BIT(x) (1 << (x))

static const uint NumInternalTreeletNodes = FullTreeletSize - 1;

static const uint NumTreeletSplitPermutations = BIT(FullTreeletSize);
static const uint FullPartitionMask = NumTreeletSplitPermutations - 1;

static const uint RootNodeIndex = 0;

// All results of FullTreeletSize choose i 
static const uint FullTreeletSizeChoose[FullTreeletSize + 1] = { 1, 7, 21, 35, 35, 21, 7, 1 };

// Precalculated bit permutations with k bits set, for k = 2 to FullTreeletSize - 1.
// Permutations with k = 0, 1, and FullTreeletSize are calculated in GetBitPermutation.
static const uint BitPermutations[FullTreeletSize - 2][35] = {
    { 0x03, 0x05, 0x06, 0x09, 0x0a, 0x0c, 0x11, 0x12, 0x14, 0x18, 0x21, 0x22, 0x24, 0x28, 0x30, 0x41, 0x42, 0x44, 0x48, 0x50, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x07, 0x0b, 0x0d, 0x0e, 0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x31, 0x32, 0x34, 0x38, 0x43, 0x45, 0x46, 0x49, 0x4a, 0x4c, 0x51, 0x52, 0x54, 0x58, 0x61, 0x62, 0x64, 0x68, 0x70 },
    { 0x0f, 0x17, 0x1b, 0x1d, 0x1e, 0x27, 0x2b, 0x2d, 0x2e, 0x33, 0x35, 0x36, 0x39, 0x3a, 0x3c, 0x47, 0x4b, 0x4d, 0x4e, 0x53, 0x55, 0x56, 0x59, 0x5a, 0x5c, 0x63, 0x65, 0x66, 0x69, 0x6a, 0x6c, 0x71, 0x72, 0x74, 0x78 },
    { 0x1f, 0x2f, 0x37, 0x3b, 0x3d, 0x3e, 0x4f, 0x57, 0x5b, 0x5d, 0x5e, 0x67, 0x6b, 0x6d, 0x6e, 0x73, 0x75, 0x76, 0x79, 0x7a, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x7d, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

uint GetBitPermutation(uint numBitsSet, uint n)
{
    if (numBitsSet == 0)
    {
        return 0;
    }
    else if (numBitsSet == 1)
    {
        return n < FullTreeletSizeChoose[1] ? BIT(n) : 0;
    }
    else if (numBitsSet == FullTreeletSize)
    {
        return n == 0 ? BIT(FullTreeletSize) - 1 : 0;
    }
    return BitPermutations[numBitsSet - 2][n];
}

bool IsLeafIndex(uint nodeIndex)
{
    const uint NumberOfInternalNodes = GetNumInternalNodes(Constants.NumberOfElements);
    return nodeIndex >= NumberOfInternalNodes;
}

float ComputeBoxSurfaceArea(AABB aabb)
{
    float3 dim = aabb.max - aabb.min;
    return 2.0f * (dim.x * dim.y + dim.x * dim.z + dim.y * dim.z);
}

AABB CombineAABB(AABB aabb0, AABB aabb1)
{
    AABB parentAABB;
    parentAABB.min = min(aabb0.min, aabb1.min);
    parentAABB.max = max(aabb0.max, aabb1.max);
    return parentAABB;
}

#endif