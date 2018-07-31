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

RWStructuredBuffer<AABBNodeSibling> InputBuffer : UAV_REGISTER(MortonCodeCalculatorInputBufferRegister);

float3 GetCentroid(uint elementIndex)
{
	return float3(InputBuffer[elementIndex].center);
}

#include "CalculateMortonCodes.hlsli"