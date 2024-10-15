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
#include "RTAO.hlsli"

// Output.
RWTexture2D<float4> g_renderTarget : register(u0);

// Input.
ConstantBuffer<ComposeRenderPassesConstantBuffer> cb : register(b0);
Texture2D<float4> g_texGBufferPositionRT : register(t2);
Texture2D<NormalDepthTexFormat> g_texGBufferNormalDepth : register(t3);
Texture2D<float> g_texAO : register(t5);
StructuredBuffer<PrimitiveMaterialBuffer> g_materials : register(t7);
Texture2D<float> g_texRayHitDistance : register(t9);
Texture2D<uint> g_texTspp : register(t10);
Texture2D<float4> g_texColor : register(t11);
Texture2D<float4> g_texAOSurfaceAlbedo : register(t12);
Texture2D<float4> g_texVariance : register(t13);
Texture2D<float4> g_texLocalMeanVariance : register(t14);


float4 RenderPBRResult(in uint2 DTid)
{
    float4 color;

    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);

    float3 PBRcolor = g_texColor[DTid].xyz;

    float ambientCoef = 0;
    bool hit = depth != HitDistanceOnMiss;
    if (hit && cb.isAOEnabled)
    {
        // Subtract the default ambient illumination that has already been added to the color in pathtracing pass.
        ambientCoef = g_texAO[DTid] - cb.defaultAmbientIntensity;
    }

    float3 ambientColor = ambientCoef * g_texAOSurfaceAlbedo[DTid].xyz;
    color = float4(PBRcolor + ambientColor, 1);

    // Apply visibility falloff.
    float3 hitPosition = g_texGBufferPositionRT[DTid].xyz;
    float t = length(hitPosition);
    float distFalloff = 0.000000005;
    color = lerp(color, BackgroundColor, 1.0 - exp(-distFalloff * t * t * t * t));

    return color;
}

float4 RenderAOResult(in uint2 DTid)
{
    float4 color = float4(1, 1, 1, 1);
    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);
    bool hit = depth != HitDistanceOnMiss;
    if (hit)
    {
        float ambientCoef = g_texAO[DTid];
        color = ambientCoef != RTAO::InvalidAOCoefficientValue ? ambientCoef : 1;
        float4 albedo = float4(1, 1, 1, 1);
        color *= albedo;
    }

    return color;
}

float4 RenderVariance(in uint2 DTid)
{
    float4 color = float4(1, 1, 1, 1);
    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);
    bool hit = depth != HitDistanceOnMiss;
    if (hit)
    {
        float variance;
        if (cb.compositionType == CompositionType::AmbientOcclusionVariance)
            variance = g_texVariance[DTid].x;
        else
            variance = g_texLocalMeanVariance[DTid].y;

        float3 minSampleColor = float3(20, 20, 20) / 255;
        float3 maxSampleColor = float3(255, 255, 255) / 255;
        if (cb.variance_visualizeStdDeviation)
            variance = sqrt(variance);
        variance *= cb.variance_scale;
        color = float4(lerp(minSampleColor, maxSampleColor, variance), 1);
    }

    return color;
}

float4 RenderRayHitDistance(in uint2 DTid)
{
    float4 color = float4(1, 1, 1, 1);
    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);
    bool hit = depth != HitDistanceOnMiss;
    if (hit)
    {
        float3 minDistanceColor = float3(15, 18, 153) / 255;
        float3 maxDistanceColor = float3(170, 220, 200) / 255;
        float hitDistance = g_texRayHitDistance[DTid].x;
        float hitCoef = hitDistance / cb.RTAO_MaxRayHitDistance;
        color = hitCoef >= 0.0f ? float4(lerp(minDistanceColor, maxDistanceColor, hitCoef), 1) : float4(1, 1, 1, 1);
    }

    return color;
}

float4 RenderNormalOrDepth(in uint2 DTid)
{
    float4 color = float4(1, 1, 1, 1);
    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);
    bool hit = depth != HitDistanceOnMiss;
    if (hit)
    {
        float depth;
        float3 surfaceNormal;
        DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);

        if (cb.compositionType == CompositionType::NormalsOnly)
            color = float4(surfaceNormal, 1);
        else 
            color = depth / 120;
    }

    return color;
}

float4 RenderAlbedo(in uint2 DTid)
{
    float4 color = float4(1, 1, 1, 1);
    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);
    bool hit = depth != HitDistanceOnMiss;
    if (hit)
    {
        float3 albedo = g_texAOSurfaceAlbedo[DTid].xyz;
        color = float4(albedo, 1);
    }

    return color;
}

float4 RenderDisocclusionMap(in uint2 DTid)
{
    float4 color = float4(1, 1, 1, 1);
    float depth;
    float3 surfaceNormal;
    DecodeNormalDepth(g_texGBufferNormalDepth[DTid], surfaceNormal, depth);
    bool hit = depth != HitDistanceOnMiss;
    if (hit)
    {
        uint tspp = g_texTspp[DTid].x;
        color = tspp == 1 ? float4(1, 0, 0, 1) : color;


        float normalizedTspp = min(1.f, tspp / 32.f);
        float3 minTsppColor = float3(153, 18, 15) / 255;
        float3 maxTsppColor = float3(170, 220, 200) / 255;
        color = float4(lerp(minTsppColor, maxTsppColor, normalizedTspp), 1);
    }

    return color;
}

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID )
{
    float4 color;
    switch(cb.compositionType)
    {
    case CompositionType::PBRShading: 
        color = RenderPBRResult(DTid);
        break;

    case CompositionType::AmbientOcclusionOnly_Denoised:
    case CompositionType::AmbientOcclusionOnly_RawOneFrame:
        color = RenderAOResult(DTid);
        break;

    case CompositionType::AmbientOcclusionAndDisocclusionMap:
        color = RenderDisocclusionMap(DTid);
        break;

    case CompositionType::AmbientOcclusionVariance:
    case CompositionType::AmbientOcclusionLocalVariance:
        color = RenderVariance(DTid);
        break;

    case CompositionType::RTAOHitDistance:
        color = RenderRayHitDistance(DTid);
        break;

    case CompositionType::NormalsOnly:
    case CompositionType::DepthOnly:
        color = RenderNormalOrDepth(DTid);
        break;

    case CompositionType::Albedo:
    case CompositionType::BaseMaterialAlbedo:
        color = RenderAlbedo(DTid);
        break;

    default:
        color = float4(1, 0, 0, 0);
        break;
    }

	// Write the composited color to the output texture.
    g_renderTarget[DTid] = color;
}

