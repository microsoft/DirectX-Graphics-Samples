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

BoundingBox ComputeLeafAABB(uint primitiveIndex, uint offsetToPrimitives, out uint2 extraInfo)
{
    uint offsetToReadPrimitive = offsetToPrimitives + primitiveIndex * SizeOfPrimitive;
    uint primitiveType = outputBVH.Load(offsetToReadPrimitive);
    offsetToReadPrimitive += OffsetToPrimitiveData;

    if (primitiveType == TRIANGLE_TYPE)
    {
        float3 v[NumberOfVerticesPerTriangle];
        [unroll]
        for (uint i = 0; i < NumberOfVerticesPerTriangle; i++)
        {
            v[i] = asfloat(outputBVH.Load3(offsetToReadPrimitive + i * SizeOfVertex));
        }

        return GetBoxDataFromTriangle(v[0], v[1], v[2], primitiveIndex, extraInfo);
    }
    else // if(primitiveType == PROCEDURAL_PRIMITIVE_TYPE)
    {
        extraInfo.x = primitiveIndex;
        extraInfo.y = IsLeafFlag | IsProceduralGeometryFlag | MinNumberOfPrimitives;
    
        AABB aabb;
        aabb.min = asfloat(outputBVH.Load3(offsetToReadPrimitive));
        aabb.max = asfloat(outputBVH.Load3(offsetToReadPrimitive + 12));
        return AABBtoBoundingBox(aabb);
    }
}

#define BOTTOM_LEVEL 1

#include "ComputeAABBs.hlsli"
