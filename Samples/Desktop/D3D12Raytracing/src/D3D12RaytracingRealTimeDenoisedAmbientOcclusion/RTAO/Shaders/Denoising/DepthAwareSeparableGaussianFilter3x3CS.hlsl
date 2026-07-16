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


// Desc: Filters values via a depth aware separable gaussian filter and input blur strength input.
// The input pixels are interleaved such that kernel cells are at cb.step offsets
// and the results are scatter wrote to memory. The interleaved layout
// allows for a separable filtering via shared memory.
// The purpose of the filter is to apply a strong blur and thus the depth test
// is more relaxed than the one used in AtrousWaveletTransform filter.
// It still however does a relaxed depth test to prevent blending surfaces too far apart.
// Supports up to 9x9 kernels.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "RTAO/Shaders/RTAO.hlsli"

#define GAUSSIAN_KERNEL_3X3
#include "Kernels.hlsli"

Texture2D<float> g_inDepth : register(t1);
Texture2D<float> g_inBlurStrength: register(t2);
RWTexture2D<float> g_inOutValue : register(u0);

RWTexture2D<float4> g_outDebug1 : register(u3);
RWTexture2D<float4> g_outDebug2 : register(u4);


ConstantBuffer<FilterConstantBuffer> cb: register(b0);

// Group shared memory cache for the row aggregated results.
static const uint NumValuesToLoadPerRowOrColumn =
    DefaultComputeShaderParams::ThreadGroup::Width
    + (FilterKernel::Width - 1);
groupshared uint PackedValueDepthCache[NumValuesToLoadPerRowOrColumn][8];   // 16bit float value, depth.
groupshared float FilteredResultCache[NumValuesToLoadPerRowOrColumn][8];    // 32 bit float filteredValue.

// Cache loaded (value, depth) pairs so each output thread can read its
// kernel-window neighbours without relying on wave-lane ordering.
groupshared float2 ValueDepthCache[NumValuesToLoadPerRowOrColumn][NumValuesToLoadPerRowOrColumn];


// Find a DTID with steps in between the group threads and groups interleaved to cover all pixels.
uint2 GetPixelIndex(in uint2 Gid, in uint2 GTid)
{
    uint2 GroupDim = uint2(8, 8);
    uint2 groupBase = (Gid / cb.step) * GroupDim * cb.step + Gid % cb.step;
    uint2 groupThreadOffset = GTid * cb.step;
    uint2 sDTid = groupBase + groupThreadOffset;

    return sDTid;
}

// Load up to 16x16 pixels and filter them horizontally.
// The output is cached in Shared Memory and contains NumRows x 8 results.
void FilterHorizontally(in uint2 Gid, in uint GI)
{
    const uint2 GroupDim = uint2(8, 8);

    // Processes the thread group as row-major 4x16, where each sub group of 16 threads processes one row.
    // Each thread loads up to 4 values, with the sub groups loading rows interleaved.
    // Loads up to 4x16x4 == 256 input values.
    uint2 GTid4x16_row0 = uint2(GI % 16, GI / 16);
    int2 GroupKernelBasePixel = GetPixelIndex(Gid, 0) - int(FilterKernel::Radius * cb.step);
    const uint NumRowsToLoadPerThread = 4;

    // Exchange each row's (value, depth) pairs through groupshared memory.
    // This is independent of wave size and lane-to-SV_GroupIndex ordering.

    // Phase 1: cooperatively load values/depths into shared memory and cache kernel centers.
    [unroll]
    for (uint i = 0; i < NumRowsToLoadPerThread; i++)
    {
        uint2 GTid4x16 = GTid4x16_row0 + uint2(0, i * 4);
        if (GTid4x16.y < NumValuesToLoadPerRowOrColumn && GTid4x16.x < NumValuesToLoadPerRowOrColumn)
        {
            int2 pixel = GroupKernelBasePixel + GTid4x16 * cb.step;
            float value = RTAO::InvalidAOCoefficientValue;
            float depth = 0;
            if (IsWithinBounds(pixel, cb.textureDim))
            {
                value = g_inOutValue[pixel];
                depth = g_inDepth[pixel];
            }
            ValueDepthCache[GTid4x16.y][GTid4x16.x] = float2(value, depth);

            // Cache the kernel center values for the vertical pass.
            if (IsInRange(GTid4x16.x, FilterKernel::Radius, FilterKernel::Radius + GroupDim.x - 1))
            {
                PackedValueDepthCache[GTid4x16.y][GTid4x16.x - FilterKernel::Radius] = Float2ToHalf(float2(value, depth));
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // Phase 2: depth-aware gaussian accumulation for the first GroupDim (8) columns of each row.
    [unroll]
    for (uint j = 0; j < NumRowsToLoadPerThread; j++)
    {
        uint2 GTid4x16 = GTid4x16_row0 + uint2(0, j * 4);
        if (GTid4x16.y >= NumValuesToLoadPerRowOrColumn || GTid4x16.x >= GroupDim.x)
        {
            continue;
        }

        // Kernel center values.
        float2 kc = ValueDepthCache[GTid4x16.y][GTid4x16.x + FilterKernel::Radius];
        float kcValue = kc.x;
        float kcDepth = kc.y;

        float weightedValueSum = 0;
        float weightSum = 0;
        float gaussianWeightedValueSum = 0;
        float gaussianWeightedSum = 0;

        // Kernel center contribution.
        if (kcValue != RTAO::InvalidAOCoefficientValue && kcDepth != 0)
        {
            float w_h = FilterKernel::Kernel1D[FilterKernel::Radius];
            gaussianWeightedValueSum = w_h * kcValue;
            gaussianWeightedSum = w_h;
            weightedValueSum = gaussianWeightedValueSum;
            weightSum = w_h;
        }

        // Remaining kernel cells.
        for (uint k = 0; k < FilterKernel::Width; k++)
        {
            if (k == FilterKernel::Radius)
            {
                continue;   // center already accumulated
            }

            float2 cvd = ValueDepthCache[GTid4x16.y][GTid4x16.x + k];
            float cValue = cvd.x;
            float cDepth = cvd.y;

            if (cValue != RTAO::InvalidAOCoefficientValue && kcDepth != 0 && cDepth != 0)
            {
                float w_h = FilterKernel::Kernel1D[k];

                // Simple depth test with tolerance growing as the kernel radius increases.
                float depthThreshold = 0.05 + cb.step * 0.001 * abs(int(FilterKernel::Radius) - int(k));
                float w_d = abs(kcDepth - cDepth) <= depthThreshold * kcDepth;
                float w = w_h * w_d;

                weightedValueSum += w * cValue;
                weightSum += w;
                gaussianWeightedValueSum += w_h * cValue;
                gaussianWeightedSum += w_h;
            }
        }

        float gaussianFilteredValue = gaussianWeightedSum > 1e-6 ? gaussianWeightedValueSum / gaussianWeightedSum : RTAO::InvalidAOCoefficientValue;
        float filteredValue = weightSum > 1e-6 ? weightedValueSum / weightSum : gaussianFilteredValue;

        FilteredResultCache[GTid4x16.y][GTid4x16.x] = filteredValue;
    }
}

void FilterVertically(uint2 DTid, in uint2 GTid, in float blurStrength)
{
    // Kernel center values.
    float2 kcValueDepth = HalfToFloat2(PackedValueDepthCache[GTid.y + FilterKernel::Radius][GTid.x]);
    float kcValue = kcValueDepth.x;
    float kcDepth = kcValueDepth.y;

    float filteredValue = kcValue;
    if (blurStrength >= 0.01 && kcDepth != 0)
    {
        float weightedValueSum = 0;
        float weightSum = 0;
        float gaussianWeightedValueSum = 0;
        float gaussianWeightSum = 0;

        // For all rows in the kernel.
        [unroll]
        for (uint r = 0; r < FilterKernel::Width; r++)
        {
            uint rowID = GTid.y + r;

            float2 rUnpackedValueDepth = HalfToFloat2(PackedValueDepthCache[rowID][GTid.x]);
            float rDepth = rUnpackedValueDepth.y;
            float rFilteredValue = FilteredResultCache[rowID][GTid.x];

            if (rDepth != 0 && rFilteredValue != RTAO::InvalidAOCoefficientValue)
            {
                float w_h = FilterKernel::Kernel1D[r];

                // Simple depth test with tolerance growing as the kernel radius increases.
                // Goal is to prevent values too far apart to blend together, while having 
                // the test being relaxed enough to get a strong blurring result.
                float depthThreshold = 0.05 + cb.step * 0.001 * abs(int(FilterKernel::Radius) - int(r));
                float w_d = abs(kcDepth - rDepth) <= depthThreshold * kcDepth;
                float w = w_h * w_d;

                weightedValueSum += w * rFilteredValue;
                weightSum += w;
                gaussianWeightedValueSum += w_h * rFilteredValue;
                gaussianWeightSum += w_h;
            }
        }
        float gaussianFilteredValue = gaussianWeightSum > 1e-6 ? gaussianWeightedValueSum / gaussianWeightSum : RTAO::InvalidAOCoefficientValue;
        filteredValue = weightSum > 1e-6 ? weightedValueSum / weightSum : gaussianFilteredValue;
        filteredValue = filteredValue != RTAO::InvalidAOCoefficientValue ? lerp(kcValue, filteredValue, blurStrength) : filteredValue;
    }
    g_inOutValue[DTid] = filteredValue;
}


[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 Gid : SV_GroupID, uint2 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    uint2 sDTid = GetPixelIndex(Gid, GTid);
    // Pass through if all pixels have 0 blur strength set.
    float blurStrength;
    {
        if (GI == 0)
            FilteredResultCache[0][0] = 0;
        GroupMemoryBarrierWithGroupSync();

        blurStrength = g_inBlurStrength[sDTid];

        float MinBlurStrength = 0.01;
        bool valueNeedsFiltering = blurStrength >= MinBlurStrength;
        if (valueNeedsFiltering)
            FilteredResultCache[0][0] = 1;

        GroupMemoryBarrierWithGroupSync();

        if (FilteredResultCache[0][0] == 0)
        {
            return;
        }
    }


    FilterHorizontally(Gid, GI);
    GroupMemoryBarrierWithGroupSync();

    FilterVertically(sDTid, GTid, blurStrength);
}
