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

#define MAX(x, y) (x > y ? x : y)
#define ROUNDUP(x, y) ((x + y - 1) & ~(y - 1))

#define MAX_VERTS 64
#define MAX_PRIMS 126
#define MAX_LOD_LEVELS 8

#define THREADS_PER_WAVE 32 // Assumes availability of wave size of 32 threads

// Pre-defined threadgroup sizes for AS & MS stages
#define AS_GROUP_SIZE THREADS_PER_WAVE
#define MS_GROUP_SIZE ROUNDUP(MAX(MAX_VERTS, MAX_PRIMS), THREADS_PER_WAVE)


#ifdef __cplusplus 
using float4x4 = DirectX::XMFLOAT4X4;
using float4 = DirectX::XMFLOAT4;
using float3 = DirectX::XMFLOAT3;
using float2 = DirectX::XMFLOAT2;
using uint = uint32_t;
#endif

#ifdef __cplusplus 
_declspec(align(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
#endif
struct Constants
{
    float4x4 View;
    float4x4 ViewProj;

    float4 Planes[6];
    float3 ViewPosition;
    float RecipTanHalfFovy;

    uint RenderMode;
    uint LODCount;
};

struct DrawParams
{
    uint InstanceOffset;
    uint InstanceCount;
};

struct Instance
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4   BoundingSphere;
};
