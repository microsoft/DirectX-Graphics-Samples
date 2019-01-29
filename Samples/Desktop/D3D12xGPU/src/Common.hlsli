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

#define NUM_LIGHTS 1
#define SHADOW_DEPTH_BIAS 0.00005f

struct LightState
{
    float3 position;
    float3 direction;
    float4 color;
    float4 falloff;
    float4x4 view;
    float4x4 projection;
};

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4 ambientColor;
    bool sampleShadowMap;
    LightState lights[NUM_LIGHTS];
    float4 viewport;
    float4 clipPlane;
};

// Sample normal map, convert to signed, apply tangent-to-world space transform.
float3 CalcPerPixelNormal(float2 vTexcoord, float3 vVertNormal, float3 vVertTangent)
{
    // Compute per-pixel normal.
    float3 vBumpNormal = (float3)normalMap.Sample(sampleWrap, vTexcoord);
    vBumpNormal = 2.0f * vBumpNormal - 1.0f;

    // Compute tangent frame.
    vVertNormal = normalize(vVertNormal);
    vVertTangent = normalize(vVertTangent);

    float3 vVertBinormal = normalize(cross(vVertTangent, vVertNormal));
    float3x3 mTangentSpaceToWorldSpace = float3x3(vVertTangent, vVertBinormal, vVertNormal);

    return mul(vBumpNormal, mTangentSpaceToWorldSpace);
}

// Diffuse lighting calculation, with angle and distance falloff.
float4 CalcLightingColor(float3 vLightPos, float3 vLightDir, float4 vLightColor, float4 vFalloffs, float3 vPosWorld, float3 vPerPixelNormal)
{
    float3 vLightToPixelUnNormalized = vPosWorld - vLightPos;

    // Dist falloff = 0 at vFalloffs.x, 1 at vFalloffs.x - vFalloffs.y
    float fDist = length(vLightToPixelUnNormalized);

    float fDistFalloff = saturate((vFalloffs.x - fDist) / vFalloffs.y);

    // Normalize from here on.
    float3 vLightToPixelNormalized = vLightToPixelUnNormalized / fDist;

    // Angle falloff = 0 at vFalloffs.z, 1 at vFalloffs.z - vFalloffs.w
    float fCosAngle = dot(vLightToPixelNormalized, vLightDir / length(vLightDir));
    float fAngleFalloff = saturate((fCosAngle - vFalloffs.z) / vFalloffs.w);

    // Diffuse contribution.
    float fNDotL = saturate(-dot(vLightToPixelNormalized, vPerPixelNormal));

    // Ignore angle falloff for a point light.
    fAngleFalloff = 1.0f;  

    return vLightColor * fNDotL * fDistFalloff * fAngleFalloff;
}

// Test how much pixel is in shadow, using 2x2 percentage-closer filtering.
float4 CalcUnshadowedAmountPCF2x2(int lightIndex, float4 vPosWorld)
{
    // Compute pixel position in light space.
    float4 vLightSpacePos = vPosWorld;
    vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].view);
    vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].projection);

    vLightSpacePos.xyz /= vLightSpacePos.w;

    // Translate from homogeneous coords to texture coords.
    float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
    vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;

    // Depth bias to avoid pixel self-shadowing.
    float vLightSpaceDepth = vLightSpacePos.z - SHADOW_DEPTH_BIAS;

    // Find sub-pixel weights.
    float2 vShadowMapDims = float2(viewport.x, viewport.y); //float2(1280.0f, 720.0f);             // need to keep in sync with .cpp file
    float4 vSubPixelCoords = float4(1.0f, 1.0f, 1.0f, 1.0f);
    vSubPixelCoords.xy = frac(vShadowMapDims * vShadowTexCoord);
    vSubPixelCoords.zw = 1.0f - vSubPixelCoords.xy;
    float4 vBilinearWeights = vSubPixelCoords.zxzx * vSubPixelCoords.wwyy;

    // 2x2 percentage closer filtering.
    float2 vTexelUnits = 1.0f / vShadowMapDims;
    float4 vShadowDepths;
    if (vShadowTexCoord.x <= 0.0f || vShadowTexCoord.x >= 1.0f || vShadowTexCoord.y <= 0.0f || vShadowTexCoord.y >= 1.0f || vLightSpaceDepth > 1.0f)
    {
        // Hack for a point light with a shadow map only for a single face.
        // Don't apply any shadow outside the shadow map.
        return float4(1.0, 1.0, 1.0, 1.0);
    }
    else
    {
        vShadowDepths.x = shadowMap.Sample(sampleClamp, vShadowTexCoord).x;
        vShadowDepths.y = shadowMap.Sample(sampleClamp, vShadowTexCoord + float2(vTexelUnits.x, 0.0f)).x;
        vShadowDepths.z = shadowMap.Sample(sampleClamp, vShadowTexCoord + float2(0.0f, vTexelUnits.y)).x;
        vShadowDepths.w = shadowMap.Sample(sampleClamp, vShadowTexCoord + vTexelUnits).x;
    }
    // What weighted fraction of the 4 samples are nearer to the light than this pixel?
    float4 vShadowTests = (vShadowDepths >= vLightSpaceDepth) ? 1.0f : 0.0f;
    return dot(vBilinearWeights, vShadowTests);
}