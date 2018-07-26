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

float4 Wave(float2 uv)
{
    float angle = distance(float2(0.5f, 0.5f), uv) * 50.0f;
    uv.x = 0.5f + (uv.x - 0.5f) * cos(angle) - (uv.y - 0.5f) * sin(angle);
    uv.y = 0.5f + (uv.x - 0.5f) * sin(angle) + (uv.y - 0.5f) * cos(angle);

    return g_tex.Sample(g_samp, uv);
}

float4 mainWave(PSInput input) : SV_TARGET
{
    return Wave(input.uv);
}
