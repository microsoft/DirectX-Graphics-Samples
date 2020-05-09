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

ToDo fix or remove
#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "Kernels.hlsli"

Texture2D<float> g_inValue : register(t0); // ToDo input is 3841x2161 instead of 2160p..

Texture2D<NormalDepthTexFormat> g_inNormalDepth : register(t1);
Texture2D<float> g_inVariance : register(t4);   // ToDo remove
Texture2D<float> g_inSmoothedVariance : register(t5);   // ToDo rename
Texture2D<float> g_inHitDistance : register(t6);   // ToDo remove?
Texture2D<float2> g_inPartialDistanceDerivatives : register(t7);   // ToDo remove?

RWTexture2D<float> g_outFilteredValues : register(u0);
RWTexture2D<float> g_outFilteredVariance : register(u1);
ConstantBuffer<AtrousWaveletTransformFilterConstantBuffer> cb: register(b0);


float DepthThreshold(float distance, float2 ddxy, float2 pixelOffset)
{
    float depthThreshold;

    // Pespective correction for the non-linear interpolation
    if (cb.perspectiveCorrectDepthInterpolation)
    {
        // Calculate depth with perspective correction.
        // Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
        // Given depth buffer interpolation for finding z at offset q along z0 to z1
        //      z =  1 / (1 / z0 * (1 - q) + 1 / z1 * q)
        // and z1 = z0 + ddxy, where z1 is at a unit pixel offset [1, 1]
        // z can be calculated via ddxy as
        //
        //      z = (z0 + ddxy) / (1 + (1-q) / z0 * ddxy) 
         
        float z0 = distance;
        float2 zxy = (z0 + ddxy) / (1 + ((1 - pixelOffset) / z0) * ddxy);
        depthThreshold = dot(1, abs(zxy - z0)); // ToDo this should be sqrt(dot(zxy - z0, zxy - z0))
    }
    else
    {
        // ToDo explain
        depthThreshold = dot(1, abs(pixelOffset * ddxy));
    }

    return depthThreshold;
}

void AddFilterContribution(
    inout float weightedValueSum,
    inout float weightedVarianceSum,
    inout float weightSum, 
    in float value, 
    in float stdDeviation,
    in float depth, 
    in float3 normal, 
    in float2 ddxy,
    in uint row, 
    in uint col,
    in uint2 DTid)
{
    const float valueSigma = cb.valueSigma;
    const float normalSigma = cb.normalSigma;
    const float depthSigma = cb.depthSigma; 
    float kernelWidth;

    int2 pixelOffset = int2(row - FilterKernel::Radius, col - FilterKernel::Radius);
    int2 id = int2(DTid) + pixelOffset;

    if (IsWithinBounds(id, cb.textureDim))
    {
        float iValue = g_inValue[id];
        float iVariance = g_inVariance[id];
        float3 iNormal;
        float iDepth;
        DecodeNormalDepth(g_inNormalDepth[id], iNormal, iDepth);

        bool iIsValidValue = iValue != RTAO::InvalidAOCoefficientValue;
        if (!iIsValidValue || iDepth == 0)
        {
            return;
        }
        
        // Normal based weight.
        float w_n = 1;
        if (normalSigma > 0)
        {
            w_n = pow(max(0, dot(normal, iNormal)), normalSigma);
        }

        // Depth based weight.
        float w_d = 1;
        if (depthSigma > 0)
        {
            // ToDo dedupe with CrossBilateralWeights.hlsli?
            // ToDo why is 2x needed to get rid of banding?
            // ToDo test perf overhead
            float depthFloatPrecision = 2.0f * FloatPrecision(max(depth, iDepth), cb.DepthNumMantissaBits);

            float2 pixelOffsetForDepthTreshold = abs(pixelOffset);
            // Account for sample offset in bilateral downsampled partial depth derivative buffer.
            if (cb.usingBilateralDownsampledBuffers)
            {
                pixelOffsetForDepthTreshold += float2(0.5, 0.5);
            }
            float depthTolerance = depthSigma * DepthThreshold(depth, ddxy, pixelOffsetForDepthTreshold) + depthFloatPrecision;

            // ToDo compare to exp version from SVGF.
            w_d = min(depthTolerance / (abs(depth - iDepth) + FLT_EPSILON), 1);

            // ToDo Explain
            w_d *= w_d >= cb.depthWeightCutoff;
        }

        // Value based weight.
        float w_x = 1;
        if (valueSigma > 0)
        {
            const float errorOffset = 0.005f;
            float e_x = -abs(value - iValue) / (valueSigma * stdDeviation + errorOffset);
            w_x = exp(e_x);
        }

        // Filter kernel weight.
        float w_h = FilterKernel::Kernel[row][col];

        float w = w_h * w_n * w_x * w_d;

        weightedValueSum += w * iValue;
        weightSum += w;

        if (cb.outputFilteredVariance)
        {
            weightedVarianceSum += w * w * iVariance;   // ToDo rename to sqWeight...
        }
    }
}

// Atrous Wavelet Transform Cross Bilateral Filter
// Ref: Dammertz 2010, Edge-Avoiding A-Trous Wavelet Transform for Fast Global Illumination Filtering
[numthreads(AtrousWaveletTransformFilterCS::ThreadGroup::Width, AtrousWaveletTransformFilterCS::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID, uint2 Gid : SV_GroupID)
{
    // ToDo add early exit if this pixel is processing inactive result.
    // ToDo double check all CS for out of bounds.
    if (DTid.x >= cb.textureDim.x || DTid.y >= cb.textureDim.y)
        return;

    // Initialize values to the current pixel / center filter kernel value.
    float value = g_inValue[DTid];
    float4 packedNormalDepth = g_inNormalDepth[DTid];
    float3 normal = DecodeNormal(packedNormalDepth.xy);
    float depth = packedNormalDepth.z;
    float2 ddxy = g_inPartialDistanceDerivatives[DTid];
    float variance = g_inSmoothedVariance[DTid];
    float stdDeviation = sqrt(variance);

    float weightSum = FilterKernel::Kernel[FilterKernel::Radius][FilterKernel::Radius];
    float weightedValueSum = weightSum * value;
    float weightedVarianceSum = FilterKernel::Kernel[FilterKernel::Radius][FilterKernel::Radius] * FilterKernel::Kernel[FilterKernel::Radius][FilterKernel::Radius]
                            * variance;

    // Add contributions from the neighborhood.
    [unroll]
    for (UINT r = 0; r < FilterKernel::Width; r++)
    [unroll]
    for (UINT c = 0; c < FilterKernel::Width; c++)
        if (r != FilterKernel::Radius || c != FilterKernel::Radius)
             AddFilterContribution(weightedValueSum, weightedVarianceSum, weightSum, value, stdDeviation, depth, normal, ddxy, r, c, DTid);

    float outputValue = (cb.outputFilterWeightSum) ? weightSum : weightedValueSum / weightSum;
    g_outFilteredValues[DTid] = outputValue;


    if (cb.outputFilteredVariance)
    {
        g_outFilteredVariance[DTid] = weightedVarianceSum / (weightSum * weightSum);
    }
}