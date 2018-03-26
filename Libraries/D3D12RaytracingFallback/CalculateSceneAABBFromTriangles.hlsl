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

RWStructuredBuffer<Triangle> InputBuffer : UAV_REGISTER(SceneAABBCalculatorInputBufferRegister);
AABB CalculateSceneAABB(uint baseElementIndex)
{
    uint trianglesToRead = min(Constants.NumberOfElements - baseElementIndex, ElementsSummedPerThread);

    AABB sceneAABB;
    sceneAABB.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
    sceneAABB.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (uint i = 0; i < trianglesToRead; i++)
    {
        Triangle tri = InputBuffer[baseElementIndex + i];
        sceneAABB.min = min(min(min(tri.v0, sceneAABB.min), tri.v1), tri.v2);
        sceneAABB.max = max(max(max(tri.v0, sceneAABB.max), tri.v1), tri.v2);
    }
    return sceneAABB;
}
