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

// Desc: Filters/fills-in invalid values for a checkerboard filled input from neighborhood.
// The compute shader is to be run with (width, height / 2) dimensions as 
// it scales Y coordinate by 2 to process only the inactive pixels in the checkerboard filled input.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "RTAO/Shaders/RTAO.hlsli"

RWTexture2D<float2> g_inOutValues : register(u0);

RWTexture2D<float4> g_outDebug1 : register(u3);
RWTexture2D<float4> g_outDebug2 : register(u4);

ConstantBuffer<CalculateMeanVarianceConstantBuffer> cb: register(b0);

// Adjust an index in Y coordinate to a same/next pixel that has an invalid value generated for it.
int2 GetInactivePixelIndex(int2 pixel)
{
    bool isEvenPixel = ((pixel.x + pixel.y) & 1) == 0;
    return
        cb.areEvenPixelsActive == isEvenPixel
        ? pixel + int2(0, 1)
        : pixel;
}

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    int2 pixel = GetInactivePixelIndex(int2(DTid.x, DTid.y * 2));

    // Load 4 valid neighbors.
    const int2 srcIndexOffsets[4] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };
    float4x2 inValues_4x2;
    {
        [unroll]
        for (uint i = 0; i < 4; i++)
        {
            inValues_4x2[i] = g_inOutValues[pixel + srcIndexOffsets[i]];
        }
    }

    // Average valid inputs.
    float4 weights = inValues_4x2._11_21_31_41 != RTAO::InvalidAOCoefficientValue;
    float weightSum = dot(1, weights);
    float2 filteredValue =  weightSum > 1e-3 ? mul(weights, inValues_4x2) / weightSum : RTAO::InvalidAOCoefficientValue;

    g_inOutValues[pixel] = filteredValue;
}
