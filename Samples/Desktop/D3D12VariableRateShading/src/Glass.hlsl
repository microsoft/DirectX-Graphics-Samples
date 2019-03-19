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

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t2);
Texture2D refractionMap : register(t3);

SamplerState sampleClamp : register(s0);
SamplerState sampleWrap : register(s1);

cbuffer GlassConstantBuffer : register(b1)
{
    float refractionScale;
    bool flipNormal;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 refractionPosition : TEXCOORD1;
};

#include "Common.hlsli"

PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT)
{
    PSInput result;

    float4 inputPosition = float4(position, 1.0f);

    normal.z *= -1.0f;

    float4 newPosition = mul(inputPosition, model);
    result.worldpos = newPosition;

    newPosition = mul(newPosition, view);
    newPosition = mul(newPosition, projection);

    result.position = newPosition;
    result.uv = uv;
    result.normal = normal;
    result.tangent = tangent;

    result.refractionPosition = result.position;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 diffuseColor = diffuseMap.Sample(sampleWrap, input.uv);
    float3 pixelNormal = CalcPerPixelNormal(input.uv, input.normal, input.tangent);
    float4 totalLight = ambientColor;

    if (flipNormal)
    {
        pixelNormal.z = -pixelNormal.z;
    }

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        float4 lightPass = CalcLightingColor(lights[i].position, lights[i].direction, lights[i].color, lights[i].falloff, input.worldpos.xyz, pixelNormal);
        if (sampleShadowMap && i == 0)
        {
            lightPass *= CalcUnshadowedAmountPCF2x2(i, input.worldpos);
        }
        totalLight += lightPass;
    }

    // Bump up the blue color a bit.
    diffuseColor.z += .3f;

    float2 refractionUV;
    refractionUV.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractionUV.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;
    refractionUV = refractionUV + (pixelNormal.xy * refractionScale);

    // Blend refraction color and diffuse color.
    float4 refractionColor = refractionMap.Sample(sampleClamp, refractionUV);
    diffuseColor = lerp(refractionColor, diffuseColor * refractionColor, .5f);

    return diffuseColor * saturate(totalLight);
}
