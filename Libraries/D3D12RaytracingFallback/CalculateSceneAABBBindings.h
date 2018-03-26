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
#include "ShaderUtil.hlsli"
#endif
#include "RaytracingHlslCompat.h"

#define ElementsSummedPerThread 8


struct SceneAABBCalculatorConstants
{
    uint NumberOfElements;
};

// UAVs
#define SceneAABBCalculatorAABBBuffer 0
#define SceneAABBCalculatorOutputBuffer 1
#define SceneAABBCalculatorInputBufferRegister 2

// CBVs
#define SceneAABBCalculatorConstantsRegister 0

#ifdef HLSL
RWStructuredBuffer<AABB> AABBBuffer : UAV_REGISTER(SceneAABBCalculatorAABBBuffer);
RWStructuredBuffer<AABB> OutputBuffer : UAV_REGISTER(SceneAABBCalculatorOutputBuffer);
cbuffer SceneAABBCalculatorConstants : CONSTANT_REGISTER(SceneAABBCalculatorConstantsRegister)
{
    SceneAABBCalculatorConstants Constants;
}

AABB CalculateSceneAABB(uint baseElementIndex);

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint outputAABBIndex = DTid.x;
    uint BaseElementIndex = DTid.x * ElementsSummedPerThread;
    if (BaseElementIndex >= Constants.NumberOfElements) return;

    OutputBuffer[outputAABBIndex] = CalculateSceneAABB(BaseElementIndex);
}
#endif
