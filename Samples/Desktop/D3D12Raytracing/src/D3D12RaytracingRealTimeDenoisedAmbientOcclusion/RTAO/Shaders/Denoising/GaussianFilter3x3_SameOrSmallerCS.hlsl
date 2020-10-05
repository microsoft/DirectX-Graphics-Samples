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

// Desc: Filters invalid values for a checkerboard filled input from neighborhood.
// The compute shader is to be run with (width, height / 2) dimensions as 
// it scales Y coordinate by 2 to process only the inactive pixels in the checkerboard filled input.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"

#define GAUSSIAN_KERNEL_3X3
#include "Kernels.hlsli"

RWTexture2D<uint> g_inOutValues : register(u0);
ConstantBuffer<TextureDimConstantBuffer> cb: register(b0);


[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    float weightedValueSum = 0;
    float weightSum = 0;

    // Add contributions from the neighborhood.
    [unroll]
    for (int r = 0; r < FilterKernel::Width; r++)
        [unroll]
        for (int c = 0; c < FilterKernel::Width; c++)
        {
            if (IsWithinBounds(DTid, cb.textureDim))
            {
                float w = FilterKernel::Kernel[r][c];
                int2 id = int2(DTid)-int2(1, 1) + int2(r, c);
                uint v = g_inOutValues[id];

                weightedValueSum += w * v;
                weightSum += w;
            }
        }

    uint initialValue = g_inOutValues[DTid];
    g_inOutValues[DTid] = min(initialValue, round(weightedValueSum / weightSum));
}
