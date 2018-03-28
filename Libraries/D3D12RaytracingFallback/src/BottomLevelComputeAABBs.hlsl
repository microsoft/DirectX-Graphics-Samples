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

BoundingBox ComputeLeafAABB(uint triangleIndex, uint offsetToVertices, out uint2 flags)
{
    uint offsetToReadTriangles = offsetToVertices + triangleIndex * SizeOfTriangle;
    float3 v[NumberOfVerticesPerTriangle];

    [unroll]
    for (uint i = 0; i < NumberOfVerticesPerTriangle; i++)
    {
        v[i] = asfloat(outputBVH.Load3(offsetToReadTriangles + i * SizeOfVertex));
    }

    return GetBoxDataFromTriangle(v[0], v[1], v[2], triangleIndex, flags);
}

#define BOTTOM_LEVEL 1

#include "ComputeAABBs.hlsli"
