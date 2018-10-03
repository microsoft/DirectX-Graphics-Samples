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

float4 GrayScale(float2 uv)
{
    float3 luminance = float3(0.21f, 0.72f, 0.07f);
    float3 color = g_tex.Sample(g_samp, uv).xyz;
    float output = dot(luminance, color);

    return float4(output, output, output, 1.0f);
}

float4 mainGray(PSInput input) : SV_TARGET
{
    return GrayScale(input.uv);
}
