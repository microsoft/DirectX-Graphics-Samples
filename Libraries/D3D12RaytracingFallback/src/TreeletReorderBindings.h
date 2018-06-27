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

#define GlobalDescriptorHeapRegister 0
#define GlobalDescriptorHeapRegisterSpace 1

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

static const uint NumTreeletSplitPermutations = 1 << FullTreeletSize;
static const uint NumInternalTreeletNodes = FullTreeletSize - 1;
static const uint RootNodeIndex = 0; 

// All results of [FullTreeletSize] choose [i]
static const uint FullTreeletSizeChoose[FullTreeletSize + 1] = { 1, 7, 21, 35, 35, 21, 7, 1 };

static const uint BitPermutations[FullTreeletSize + 1][35] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 4, 8, 16, 32, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 3, 5, 6, 9, 10, 12, 17, 18, 20, 24, 33, 34, 36, 40, 48, 65, 66, 68, 72, 80, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 7, 11, 13, 14, 19, 21, 22, 25, 26, 28, 35, 37, 38, 41, 42, 44, 49, 50, 52, 56, 67, 69, 70, 73, 74, 76, 81, 82, 84, 88, 97, 98, 100, 104, 112 },
    { 15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120 },
    { 31, 47, 55, 59, 61, 62, 79, 87, 91, 93, 94, 103, 107, 109, 110, 115, 117, 118, 121, 122, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 63, 95, 111, 119, 123, 125, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
#endif