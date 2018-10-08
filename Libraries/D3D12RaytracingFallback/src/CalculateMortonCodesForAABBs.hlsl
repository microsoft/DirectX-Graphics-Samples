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
#include "CalculateMortonCodesBindings.h"
RWByteAddressBuffer InputBuffer : UAV_REGISTER(MortonCodeCalculatorInputBufferRegister);

BoundingBox ReadBottomLevelBoundingBox(uint bottomLevelIndex)
{
    uint address = bottomLevelIndex * SizeOfAABBNode;
    uint4 dataA = InputBuffer.Load4(address);
    uint4 dataB = InputBuffer.Load4(address + 16);

    uint2 unusedFlags;
    return RawDataToBoundingBox(dataA, dataB, unusedFlags);
}

float3 GetCentroid(uint elementIndex)
{
    return ReadBottomLevelBoundingBox(elementIndex).center;
}

#include "CalculateMortonCodes.hlsli"