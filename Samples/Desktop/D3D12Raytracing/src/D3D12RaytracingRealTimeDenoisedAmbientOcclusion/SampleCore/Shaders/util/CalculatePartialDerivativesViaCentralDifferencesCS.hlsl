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

#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"

Texture2D<float> g_inValue : register(t0);
RWTexture2D<float2> g_outValue : register(u0);

ConstantBuffer<CalculatePartialDerivativesConstantBuffer> cb : register(b0);

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    //                x
    //        ----------------->
    //    |    x     [top]     x
    // y  |  [left]   DTiD   [right]
    //    v    x    [bottom]   x
    //
    uint2 top = clamp(DTid.xy + uint2(0, -1), 0, cb.textureDim - 1);
    uint2 bottom = clamp(DTid.xy + uint2(0, 1), 0, cb.textureDim - 1);
    uint2 left = clamp(DTid.xy + uint2(-1, 0), 0, cb.textureDim - 1);
    uint2 right = clamp(DTid.xy + uint2(1, 0), 0, cb.textureDim - 1);

    float centerValue = g_inValue[DTid.xy];
    float2 backwardDifferences = centerValue - float2(g_inValue[left], g_inValue[top]);
    float2 forwardDifferences = float2(g_inValue[right], g_inValue[bottom]) - centerValue;

    // Calculates partial derivatives as the min of absolute backward and forward differences. 

    // Find the absolute minimum of the backward and foward differences in each axis
    // while preserving the sign of the difference.
    float2 ddx = float2(backwardDifferences.x, forwardDifferences.x);
    float2 ddy = float2(backwardDifferences.y, forwardDifferences.y);

    uint2 minIndex = {
        GetIndexOfValueClosestToTheReference(0, ddx),
        GetIndexOfValueClosestToTheReference(0, ddy)
    };
    float2 ddxy = float2(ddx[minIndex.x], ddy[minIndex.y]);

    // Clamp ddxy to a reasonable value to avoid ddxy going over surface boundaries
    // on thin geometry and getting background/foreground blended together on blur.
    float maxDdxy = 1;
    float2 _sign = sign(ddxy);
    ddxy = _sign * min(abs(ddxy), maxDdxy);

    g_outValue[DTid] = ddxy;
}