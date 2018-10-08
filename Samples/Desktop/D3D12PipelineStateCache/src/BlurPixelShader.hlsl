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

#include "QuadVertexShader.hlsl"

static const float weights[] = {
    0.1061154f,
    0.102850571f,
    0.102850571f,
    0.09364651f,
    0.09364651f,
    0.0801001f,
    0.0801001f,
    0.06436224f,
    0.06436224f,
    0.0485831723f,
    0.0485831723f,
    0.0344506279f,
    0.0344506279f,
    0.0229490642f,
    0.0229490642f,
};

static const float2 offsets[] = {
    float2(0.0f, 0.0f),
    float2(0.00375f, 0.006250001f),
    float2(-0.00375f, -0.006250001f),
    float2(0.00875f, 0.01458333f),
    float2(-0.00875f, -0.01458333f),
    float2(0.01375f, 0.02291667f),
    float2(-0.01375f, -0.02291667f),
    float2(0.01875f, 0.03125f),
    float2(-0.01875f, -0.03125f),
    float2(0.02375f, 0.03958334f),
    float2(-0.02375f, -0.03958334f),
    float2(0.02875f, 0.04791667f),
    float2(-0.02875f, -0.04791667f),
    float2(0.03375f, 0.05625f),
    float2(-0.03375f, -0.05625f),
};

float4 Blur(float2 uv)
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 15; i++)
    {
        color += g_tex.Sample(g_samp, uv + offsets[i]) * weights[i];
    }

    return color;
}

float4 mainBlur(PSInput input) : SV_TARGET
{
    return Blur(input.uv);
}
