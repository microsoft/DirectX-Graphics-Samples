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
#include "BlitPixelShader.hlsl"
#include "InvertPixelShader.hlsl"
#include "GrayScalePixelShader.hlsl"
#include "EdgeDetectPixelShader.hlsl"
#include "BlurPixelShader.hlsl"
#include "WarpPixelShader.hlsl"
#include "PixelatePixelShader.hlsl"
#include "DistortPixelShader.hlsl"
#include "WavePixelShader.hlsl"

cbuffer cb : register(b0)
{
    uint shaderType;
};

#define PostBlit 2
#define PostInvert 3
#define PostGrayScale 4
#define PostEdgeDetect 5
#define PostBlur 6
#define PostWarp 7
#define PostPixelate 8
#define PostDistort 9
#define PostWave 10

float4 mainUber(PSInput input) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

    [branch] switch (shaderType)
    {
    case PostBlit:
        color = Blit(input.uv);
        break;

    case PostInvert:
        color = InvertPixel(input.uv);
        break;

    case PostGrayScale:
        color = GrayScale(input.uv);
        break;

    case PostEdgeDetect:
        color = EdgeDetect(input.uv);
        break;

    case PostBlur:
        color = Blur(input.uv);
        break;

    case PostWarp:
        color = Warp(input.uv);
        break;

    case PostPixelate:
        color = Pixelate(input.uv);
        break;

    case PostDistort:
        color = Distort(input.uv);
        break;

    case PostWave:
        color = Wave(input.uv);
        break;

    default:
        break;
    }

    // Indicate that the Uber shader is running.
    color *= 0.4f;

    return color;
}
