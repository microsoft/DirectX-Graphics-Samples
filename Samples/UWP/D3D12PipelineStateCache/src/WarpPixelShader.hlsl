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

float4 Warp(float2 uv)
{
    float2 dir = uv - float2(0.5f, 0.5f);
    float len = length(dir) * 6.0f;
    float2 newCoord = uv + dir * sin(pow(2.0f, len));

    return g_tex.Sample(g_samp, newCoord);
}

float4 mainWarp(PSInput input) : SV_TARGET
{
    return Warp(input.uv);
}
