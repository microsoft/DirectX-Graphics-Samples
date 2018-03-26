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
#ifndef HLSL
#include "HlslCompat.h"
#else
#include "RaytracingHlslCompat.h"
#include "ShaderUtil.hlsli"
#endif

struct LoadTrianglesInputConstants
{
    uint VertexBufferStride;
    uint IndexBufferOffset;
    uint NumTrianglesBound;

    uint TriangleOffset;
    uint TotalTriangleCount;

    uint GeometryContributionToHitGroupIndex;

    // TODO: Consider inlining into separate shaders
    uint HasValidTransform; 
};

// UAVs
#define OutputTriangleBufferRegister 0
#define OutputMetadataBufferRegister 1

//SRVs 
#define VertexBufferRegister 0
#define IndexBufferRegister 1
#define TransformRegister 2

// CBVs
#define LoadInstancesConstantsRegister 0

#ifdef HLSL

RWStructuredBuffer<Triangle> triangeBuffer : UAV_REGISTER(OutputTriangleBufferRegister);
RWStructuredBuffer<TriangleMetaData> MetadataBuffer : UAV_REGISTER(OutputMetadataBufferRegister);

ByteAddressBuffer vertexBuffer : SRV_REGISTER(VertexBufferRegister);
ByteAddressBuffer indexBuffer : SRV_REGISTER(IndexBufferRegister);
StructuredBuffer<float4> TransformBuffer : SRV_REGISTER(TransformRegister);
cbuffer LoadTrianglesConstants : CONSTANT_REGISTER(LoadInstancesConstantsRegister)
{
    LoadTrianglesInputConstants Constants;
}

#endif
