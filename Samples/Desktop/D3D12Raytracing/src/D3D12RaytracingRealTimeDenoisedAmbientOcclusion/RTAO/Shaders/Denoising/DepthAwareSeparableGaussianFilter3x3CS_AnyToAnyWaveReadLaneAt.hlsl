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
// Requirements:
//  - Wave lane size 16 or higher.
//  - WaveReadLaneAt() with any to any to wave read lane support.

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
    const uint Row_BaseWaveLaneIndex = (WaveGetLaneIndex() / 16) * 16;

    [unroll]
    for (uint i = 0; i < NumRowsToLoadPerThread; i++)
    {
        uint2 GTid4x16 = GTid4x16_row0 + uint2(0, i * 4);
        if (GTid4x16.y >= NumValuesToLoadPerRowOrColumn)
        {
            break;
        }

        // Load all the contributing columns for each row.
        int2 pixel = GroupKernelBasePixel + GTid4x16 * cb.step;
        float value = RTAO::InvalidAOCoefficientValue;
        float depth = 0;

        // The lane is out of bounds of the GroupDim + kernel, 
        // but could be within bounds of the input texture,
        // so don't read it from the texture.
        // However, we need to keep it as an active lane for a below split sum.
        if (GTid4x16.x < NumValuesToLoadPerRowOrColumn && IsWithinBounds(pixel, cb.textureDim))
        {
            value = g_inOutValue[pixel];
            depth = g_inDepth[pixel];
        }

        // Cache the kernel center values.
        if (IsInRange(GTid4x16.x, FilterKernel::Radius, FilterKernel::Radius + GroupDim.x - 1))
        {
            PackedValueDepthCache[GTid4x16.y][GTid4x16.x - FilterKernel::Radius] = Float2ToHalf(float2(value, depth));
        }

        // Filter the values for the first GroupDim columns.
        {
            // Accumulate for the whole kernel width.
            float weightedValueSum = 0;
            float weightSum = 0;
            float gaussianWeightedValueSum = 0;
            float gaussianWeightedSum = 0;

            // Since a row uses 16 lanes, but we only need to calculate the aggregate for the first half (8) lanes,
            // split the kernel wide aggregation among the first 8 and the second 8 lanes, and then combine them.


            // Get the lane index that has the first value for a kernel in this lane.
            uint Row_KernelStartLaneIndex =
                (Row_BaseWaveLaneIndex + GTid4x16.x)
                - (GTid4x16.x < GroupDim.x
                    ? 0
                    : GroupDim.x);

            // Get values for the kernel center.
            uint kcLaneIndex = Row_KernelStartLaneIndex + FilterKernel::Radius;
            float kcValue = WaveReadLaneAt(value, kcLaneIndex);
            float kcDepth = WaveReadLaneAt(depth, kcLaneIndex);

            // Initialize the first 8 lanes to the center cell contribution of the kernel. 
            // This covers the remainder of 1 in FilterKernel::Width / 2 used in the loop below. 
            if (GTid4x16.x < GroupDim.x && kcValue != RTAO::InvalidAOCoefficientValue && kcDepth != 0)
            {
                float w_h = FilterKernel::Kernel1D[FilterKernel::Radius];
                gaussianWeightedValueSum = w_h * kcValue;
                gaussianWeightedSum = w_h;
                weightedValueSum = gaussianWeightedValueSum;
                weightSum = w_h;
            }

            // Second 8 lanes start just past the kernel center.
            uint KernelCellIndexOffset =
                GTid4x16.x < GroupDim.x
                ? 0
                : (FilterKernel::Radius + 1); // Skip over the already accumulated center cell of the kernel.


            // For all columns in the kernel.
            for (uint c = 0; c < FilterKernel::Radius; c++)
            {
                uint kernelCellIndex = KernelCellIndexOffset + c;

                uint laneToReadFrom = Row_KernelStartLaneIndex + kernelCellIndex;
                float cValue = WaveReadLaneAt(value, laneToReadFrom);
                float cDepth = WaveReadLaneAt(depth, laneToReadFrom);

                if (cValue != RTAO::InvalidAOCoefficientValue && kcDepth != 0 && cDepth != 0)
                {
                    float w_h = FilterKernel::Kernel1D[kernelCellIndex];

                    // Simple depth test with tolerance growing as the kernel radius increases.
                    // Goal is to prevent values too far apart to blend together, while having 
                    // the test being relaxed enough to get a strong blurring result.
                    float depthThreshold = 0.05 + cb.step * 0.001 * abs(int(FilterKernel::Radius) - c);
                    float w_d = abs(kcDepth - cDepth) <= depthThreshold * kcDepth;
                    float w = w_h * w_d;

                    weightedValueSum += w * cValue;
                    weightSum += w;
                    gaussianWeightedValueSum += w_h * cValue;
                    gaussianWeightedSum += w_h;
                }
            }

            // Combine the sub-results.
            uint laneToReadFrom = min(WaveGetLaneCount() - 1, Row_BaseWaveLaneIndex + GTid4x16.x + GroupDim.x);
            weightedValueSum += WaveReadLaneAt(weightedValueSum, laneToReadFrom);
            weightSum += WaveReadLaneAt(weightSum, laneToReadFrom);
            gaussianWeightedValueSum += WaveReadLaneAt(gaussianWeightedValueSum, laneToReadFrom);
            gaussianWeightedSum += WaveReadLaneAt(gaussianWeightedSum, laneToReadFrom);

            // Store only the valid results, i.e. first GroupDim columns.
            if (GTid4x16.x < GroupDim.x)
            {
                float gaussianFilteredValue = gaussianWeightedSum > 1e-6 ? gaussianWeightedValueSum / gaussianWeightedSum : RTAO::InvalidAOCoefficientValue;
                float filteredValue = weightSum > 1e-6 ? weightedValueSum / weightSum : gaussianFilteredValue;

                FilteredResultCache[GTid4x16.y][GTid4x16.x] = filteredValue;
            }
        }
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
