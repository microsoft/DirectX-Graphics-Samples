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
#include "Denoising/CrossBilateralWeights.hlsli"

Texture2D<ValueType> g_inValue : register(t0);
Texture2D<NormalDepthTexFormat> g_inLowResNormalDepth : register(t1);
Texture2D<NormalDepthTexFormat> g_inHiResNormalDepth : register(t2);
Texture2D<float2> g_inHiResPartialDepthDerivative : register(t3);
RWTexture2D<ValueType> g_outValue : register(u0);
RWTexture2D<float4> g_outDebug1 : register(u1);
RWTexture2D<float4> g_outDebug2 : register(u2);

ConstantBuffer<DownAndUpsampleFilterConstantBuffer> cb : register(b0);

SamplerState ClampSampler : register(s0);

// Returns normalized weights for Bilateral Upsample.
float4 BilateralUpsampleWeights(
    in float TargetDepth, 
    in float3 TargetNormal, 
    in float2 TargetOffset,
    in float2 Ddxy,
    in float4 SampleDepths, 
    in float3 SampleNormals[4])
{
    // Account for the fact that the high-res sample corresponding to a low-res sample 
    // may be up to 2 pixels away in the high-res grid from the target.
    float2 samplesOffset = 2;

    CrossBilateral::BilinearDepthNormal::Parameters params;
    params.Depth.Sigma = 1;
    params.Depth.WeightCutoff = 0.98;
    params.Depth.NumMantissaBits = 10;
    params.Normal.Sigma = 1.1;      // Bump the sigma a bit to add tolerance for slight geometry misalignments and/or format precision limitations.
    params.Normal.SigmaExponent = 64; 

    float4 bilinearDepthNormalWeights = CrossBilateral::BilinearDepthNormal::GetWeights(
        TargetDepth,
        TargetNormal,
        TargetOffset,
        Ddxy,
        SampleDepths,
        SampleNormals,
        samplesOffset,
        params);

    bool4 isActive = SampleDepths != HitDistanceOnMiss;
    float4 weights = isActive * bilinearDepthNormalWeights;
    float weightSum = dot(weights, 1);

    // Default to an average if all weights are too small.
    float4 nWeights = weightSum > 1e-6f ? weights / weightSum : isActive / (dot(1, isActive) + FLT_EPSILON); 

    return nWeights;
}

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    // Process each 2x2 high res quad at a time, starting from [-1,-1] 
    // so that subsequent internal high-res pixel quads are within low-res quads.
    int2 topLeftHiResIndex = (DTid << 1) + int2(-1, -1);
    int2 topLeftLowResIndex = (topLeftHiResIndex + int2(-1, -1)) >> 1;
    const uint2 srcIndexOffsets[4] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
 
    float4 vHiResDepths;
    float3 hiResNormals[4];
    float2 hiResTexturePos = (topLeftHiResIndex + 0.5) * cb.invHiResTextureDim;
    {
        uint4 packedEncodedNormalDepths = g_inHiResNormalDepth.GatherRed(ClampSampler, hiResTexturePos).wzxy;
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            DecodeNormalDepth(packedEncodedNormalDepths[i], hiResNormals[i], vHiResDepths[i]);
        }
    }

    float4 vLowResDepths;
    float3 lowResNormals[4];
    float2 lowResTexturePos = (topLeftLowResIndex + 0.5) * cb.invLowResTextureDim;
    {
        uint4 packedEncodedNormalDepths = g_inLowResNormalDepth.GatherRed(ClampSampler, lowResTexturePos).wzxy;
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            DecodeNormalDepth(packedEncodedNormalDepths[i], lowResNormals[i], vLowResDepths[i]);
        }
    }

#if VALUE_NUM_COMPONENTS == 1
    float4 vLowResValues = g_inValue.GatherRed(ClampSampler, lowResTexturePos).wzxy;
#elif VALUE_NUM_COMPONENTS == 2
    float2x4 vLowResValues = {
        g_inValue.GatherRed(ClampSampler, lowResTexturePos).wzxy,
        g_inValue.GatherGreen(ClampSampler, lowResTexturePos).wzxy
    };
#endif

    float offset = 0.25;
    float2 targetOffsets[4] = {
        float2(offset, offset),
        float2(1 - offset, offset),
        float2(offset, 1 - offset),
        float2(1 - offset, 1 - offset)
    };
    
    float2x4 ddxy2x4 = {
        g_inHiResPartialDepthDerivative.GatherRed(ClampSampler, hiResTexturePos).wzxy,
        g_inHiResPartialDepthDerivative.GatherGreen(ClampSampler, hiResTexturePos).wzxy,
    };

    float4x2 ddxy = {
        ddxy2x4._11, ddxy2x4._21,
        ddxy2x4._12, ddxy2x4._22,
        ddxy2x4._13, ddxy2x4._23,
        ddxy2x4._14, ddxy2x4._24,
    };

    {
        [unroll]
        for (uint i = 0; i < 4; i++)
        {
            float targetDepth = vHiResDepths[i];
            float3 targetNormal = hiResNormals[i];
            float4 nWeights = BilateralUpsampleWeights(targetDepth, targetNormal, targetOffsets[i], ddxy[i], vLowResDepths, lowResNormals);

#if VALUE_NUM_COMPONENTS == 1
            float outValue = dot(nWeights, vLowResValues);
            g_outValue[topLeftHiResIndex + srcIndexOffsets[i]] = targetDepth != HitDistanceOnMiss ? outValue : vLowResValues[i];
#elif VALUE_NUM_COMPONENTS == 2
            float2 outValue = float2(dot(nWeights, vLowResValues[0]), dot(nWeights, vLowResValues[1]));
            g_outValue[topLeftHiResIndex + srcIndexOffsets[i]] = targetDepth != HitDistanceOnMiss ? outValue : vLowResValues._11_21;
#endif
        }
    }
}