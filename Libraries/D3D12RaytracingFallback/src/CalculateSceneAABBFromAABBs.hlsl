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

AABB CalculateSceneAABB(uint baseElementIndex)
{
    uint aabbsToRead = min(Constants.NumberOfElements - baseElementIndex, ElementsSummedPerThread);

    AABB sceneAABB;
    sceneAABB.min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
    sceneAABB.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (uint i = 0; i < aabbsToRead; i++)
    {
        AABB aabb = AABBBuffer[baseElementIndex + i];
        sceneAABB.min = min(aabb.min, sceneAABB.min);
        sceneAABB.max = max(aabb.max, sceneAABB.max);
    }
    return sceneAABB;
}
