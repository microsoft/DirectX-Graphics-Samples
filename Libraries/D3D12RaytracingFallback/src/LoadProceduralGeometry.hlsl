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
#include "LoadPrimitivesBindings.h"
#include "RayTracingHelper.hlsli"

AABB GetAABB(ByteAddressBuffer aabbBuffer, uint aabbIndex, uint stride)
{
    AABB aabb;
    uint readOffset = stride * aabbIndex;
    aabb.min = asfloat(aabbBuffer.Load3(readOffset));
    aabb.max = asfloat(aabbBuffer.Load3(readOffset + 12));
    return aabb;
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint localPrimitiveIndex = DTid.x;
    if (localPrimitiveIndex >= Constants.NumTrianglesBound)
    {
        return;
    }
    uint globalPrimitiveIndex = localPrimitiveIndex + Constants.TriangleOffset;
    AABB aabb = GetAABB(elementBuffer, localPrimitiveIndex, Constants.VertexBufferStride);

    PrimitiveBuffer[globalPrimitiveIndex] = CreateProceduralGeometryPrimitive(aabb);

    PrimitiveMetaData metaData;
    metaData.GeometryContributionToHitGroupIndex = Constants.GeometryContributionToHitGroupIndex;
    metaData.PrimitiveIndex = localPrimitiveIndex;
    MetadataBuffer[globalPrimitiveIndex] = metaData;
}