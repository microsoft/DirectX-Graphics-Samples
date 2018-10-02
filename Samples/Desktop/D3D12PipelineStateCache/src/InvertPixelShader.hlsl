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

float4 InvertPixel(float2 uv)
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f) - g_tex.Sample(g_samp, uv);
}

float4 mainInvert(PSInput input) : SV_TARGET
{
    return InvertPixel(input.uv);
}
