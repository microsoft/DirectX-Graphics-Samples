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

float4 Distort(float2 uv)
{
    // Calculate the distance of this pixel from the center.
    float pixelDistance = distance(float2(0.5f, 0.5f), uv);

    // Weight the pixel by its distance.
    float weight = 1.0f + pixelDistance;

    float x = (uv.x - 0.5f) + 0.5f;
    float y = (uv.y - 0.5f) + 0.5f;

    x = pow(abs(x), weight);
    y = pow(abs(y), weight);

    return g_tex.Sample(g_samp, float2(x, y));
}

float4 mainDistort(PSInput input) : SV_TARGET
{
    return Distort(input.uv);
}
