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
#include "RayTracingHelper.hlsli"
#endif

struct MortonCodeCalculatorConstants
{
    uint NumberOfElements;
};

// UAVs
#define MortonCodeCalculatorCalculatorOutputIndices 0
#define MortonCodeCalculatorCalculatorOutputMortonCodes 1
#define MortonCodeCalculatorSceneAABBRegister 2
#define MortonCodeCalculatorInputBufferRegister 3

// CBVs
#define MortonCodeCalculatorConstantsRegister 0

#ifdef HLSL
RWStructuredBuffer<uint> OutputIndicesBuffer : UAV_REGISTER(MortonCodeCalculatorCalculatorOutputIndices);
RWStructuredBuffer<uint> OutputMortonCodesBuffer : UAV_REGISTER(MortonCodeCalculatorCalculatorOutputMortonCodes);
RWByteAddressBuffer SceneAABB : UAV_REGISTER(MortonCodeCalculatorSceneAABBRegister);
cbuffer MortonCodeCalculatorConstants : CONSTANT_REGISTER(MortonCodeCalculatorConstantsRegister)
{
    MortonCodeCalculatorConstants Constants;
}

#define BIT(x) (1 << (x))

AABB GetSceneAABB()
{
    uint4 data1 = SceneAABB.Load4(0);
    uint2 data2 = SceneAABB.Load2(16);
    AABB sceneAAB;
    sceneAAB.min = asfloat(data1.xyz);
    sceneAAB.max = asfloat(uint3(data1.w, data2.xy));
    return sceneAAB;
}
#endif
