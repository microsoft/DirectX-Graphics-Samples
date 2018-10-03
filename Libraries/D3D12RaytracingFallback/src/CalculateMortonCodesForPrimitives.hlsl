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

RWStructuredBuffer<Primitive> InputBuffer : UAV_REGISTER(MortonCodeCalculatorInputBufferRegister);

float3 GetCentroid(uint elementIndex)
{
    Primitive primitive = InputBuffer[elementIndex];
    if(primitive.PrimitiveType == TRIANGLE_TYPE)
    {
        Triangle tri = GetTriangle(primitive);
        return (tri.v0 + tri.v1 + tri.v2) / 3.0;
    }
    else //if(primitive.PrimitiveType == PROCEDURAL_PRIMITIVE_TYPE)
    {
        AABB aabb = GetProceduralPrimitiveAABB(primitive);
        return (aabb.min + aabb.max) / 2.0;
    }
}

#include "CalculateMortonCodes.hlsli"
