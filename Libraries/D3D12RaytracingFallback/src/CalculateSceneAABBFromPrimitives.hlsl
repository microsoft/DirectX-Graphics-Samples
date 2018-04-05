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
#include "CalculateSceneAABBBindings.h"
#include "RayTracingHelper.hlsli"

RWStructuredBuffer<Primitive> InputBuffer : UAV_REGISTER(SceneAABBCalculatorInputBufferRegister);
AABB CalculateSceneAABB(uint baseElementIndex)
{
    uint primitivesToRead = min(Constants.NumberOfElements - baseElementIndex, ElementsSummedPerThread);

    AABB sceneAABB;
    sceneAABB.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
    sceneAABB.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (uint i = 0; i < primitivesToRead; i++)
    {
        Primitive primitive = InputBuffer[baseElementIndex + i];
        if (primitive.PrimitiveType == TRIANGLE_TYPE)
        {
            Triangle tri = GetTriangle(primitive);
            sceneAABB.min = min(min(min(tri.v0, sceneAABB.min), tri.v1), tri.v2);
            sceneAABB.max = max(max(max(tri.v0, sceneAABB.max), tri.v1), tri.v2);
        }
        else // if(primitive.PrimitiveType == PROCEDURAL_PRIMITIVE_TYPE)
        {
            AABB aabb = GetProceduralPrimitiveAABB(primitive);
            sceneAABB.min = min(sceneAABB.min, aabb.min);
            sceneAABB.max = max(sceneAABB.max, aabb.max);
        }
    }
    return sceneAABB;
}
