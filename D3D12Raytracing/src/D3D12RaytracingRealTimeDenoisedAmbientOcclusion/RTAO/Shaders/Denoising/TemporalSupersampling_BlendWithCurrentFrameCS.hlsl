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

// 2nd stage of temporal supersampling. Blends current frame values
// with values reprojected from previous frame in stage 1.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "RTAO\Shaders\RTAO.hlsli"

Texture2D<float> g_inCurrentFrameValue : register(t0);
Texture2D<float2> g_inCurrentFrameLocalMeanVariance : register(t1);
Texture2D<float> g_inCurrentFrameRayHitDistance : register(t2);
Texture2D<uint4> g_inReprojected_Tspp_Value_SquaredMeanValue_RayHitDistance : register(t3);

RWTexture2D<float> g_inOutValue : register(u0);
RWTexture2D<uint> g_inOutTspp : register(u1);
RWTexture2D<float> g_inOutSquaredMeanValue : register(u2);
RWTexture2D<float> g_inOutRayHitDistance : register(u3);
RWTexture2D<float> g_outVariance : register(u4);
RWTexture2D<float> g_outBlurStrength: register(u5);

RWTexture2D<float4> g_outDebug1 : register(u10);
RWTexture2D<float4> g_outDebug2 : register(u11);

ConstantBuffer<TemporalSupersampling_BlendWithCurrentFrameConstantBuffer> cb : register(b0);

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    uint4 encodedCachedValues = g_inReprojected_Tspp_Value_SquaredMeanValue_RayHitDistance[DTid];
    uint Tspp = encodedCachedValues.x;
    float4 cachedValues = float4(Tspp, f16tof32(encodedCachedValues.yzw));

    bool isCurrentFrameValueActive = true;
    if (cb.checkerboard_enabled)
    {
        bool isEvenPixel = ((DTid.x + DTid.y) & 1) == 0;
        isCurrentFrameValueActive = cb.checkerboard_areEvenPixelsActive == isEvenPixel;
    }

    float value = isCurrentFrameValueActive ? g_inCurrentFrameValue[DTid] : RTAO::InvalidAOCoefficientValue;
    bool isValidValue = value != RTAO::InvalidAOCoefficientValue;
    float valueSquaredMean = isValidValue ? value * value : RTAO::InvalidAOCoefficientValue;
    float rayHitDistance = RTAO::InvalidAOCoefficientValue;
    float variance = RTAO::InvalidAOCoefficientValue;
    
    if (Tspp > 0)
    {     
        uint maxTspp = 1 / cb.minSmoothingFactor;
        Tspp = isValidValue ? min(Tspp + 1, maxTspp) : Tspp;

        float cachedValue = cachedValues.y;

        float2 localMeanVariance = g_inCurrentFrameLocalMeanVariance[DTid];
        float localMean = localMeanVariance.x;
        float localVariance = localMeanVariance.y;
        if (cb.clampCachedValues)
        {
            float localStdDev = max(cb.stdDevGamma * sqrt(localVariance), cb.clamping_minStdDevTolerance);
            float nonClampedCachedValue = cachedValue;

            // Clamp value to mean +/- std.dev of local neighborhood to surpress ghosting on value changing due to other occluder movements.
            // Ref: Salvi2016, Temporal Super-Sampling
            cachedValue = clamp(cachedValue, localMean - localStdDev, localMean + localStdDev);

            // Scale down the tspp based on how strongly the cached value got clamped to give more weight to new samples.
            float TsppScale = saturate(cb.clampDifferenceToTsppScale * abs(cachedValue - nonClampedCachedValue));
            Tspp = lerp(Tspp, 0, TsppScale);
        }
        float invTspp = 1.f / Tspp;
        float a = cb.forceUseMinSmoothingFactor ? cb.minSmoothingFactor : max(invTspp, cb.minSmoothingFactor);
        float MaxSmoothingFactor = 1;
        a = min(a, MaxSmoothingFactor);

        // TODO: use average weighting instead of exponential for the first few samples 
        //  to even out the weights for the noisy start instead of giving first samples much more weight than the rest.
        //  Ref: Koskela2019, Blockwise Multi-Order Feature Regression for Real-Time Path-Tracing Reconstruction

        // Value.
        value = isValidValue ? lerp(cachedValue, value, a) : cachedValue;

        // Value Squared Mean.
        float cachedSquaredMeanValue = cachedValues.z; 
        valueSquaredMean = isValidValue ? lerp(cachedSquaredMeanValue, valueSquaredMean, a) : cachedSquaredMeanValue;

        // Variance.
        float temporalVariance = valueSquaredMean - value * value;
        temporalVariance = max(0, temporalVariance);    // Ensure variance doesn't go negative due to imprecision.
        variance = Tspp >= cb.minTsppToUseTemporalVariance ? temporalVariance : localVariance;
        variance = max(0.1, variance);

        // RayHitDistance.
        rayHitDistance = isValidValue ? g_inCurrentFrameRayHitDistance[DTid] : 0;
        float cachedRayHitDistance = cachedValues.w;
        rayHitDistance = isValidValue ? lerp(cachedRayHitDistance, rayHitDistance, a) : cachedRayHitDistance;
    }
    else if (isValidValue)
    {
        Tspp = 1;
        value = value;

        rayHitDistance = g_inCurrentFrameRayHitDistance[DTid];
        variance = g_inCurrentFrameLocalMeanVariance[DTid].y;
        valueSquaredMean = valueSquaredMean;
    }

    float TsppRatio = min(Tspp, cb.blurStrength_MaxTspp) / float(cb.blurStrength_MaxTspp);
    float blurStrength = pow(1 - TsppRatio, cb.blurDecayStrength);

    g_inOutTspp[DTid] = Tspp;
    g_inOutValue[DTid] = value;
    g_inOutSquaredMeanValue[DTid] = valueSquaredMean;
    g_inOutRayHitDistance[DTid] = rayHitDistance;
    g_outVariance[DTid] = variance; 
    g_outBlurStrength[DTid] = blurStrength;
}