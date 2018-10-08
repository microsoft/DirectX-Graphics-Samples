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

float4 Pixelate(float2 uv)
{
    uint2 var = uint2(uv.x * 100, uv.y * 100);
    uv = float2((float)var.x / 100, (float)var.y / 100);

    return g_tex.Sample(g_samp, uv);
}

float4 mainPixel(PSInput input) : SV_TARGET
{
    return Pixelate(input.uv);
}
