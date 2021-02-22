//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//             Jack Elliott
//

#include "LanczosFunctions.hlsli"
#include "PresentRS.hlsli"
#include "ShaderUtility.hlsli"

Texture2D<float3> Source : register(t0);

cbuffer CB0 : register(b0)
{
    float2 kSrcResolution;
};

float4x3 LoadSamples(int2 ST, uint2 Stride)
{
    int2 st0 = ST, st1 = ST+Stride, st2 = ST+2*Stride, st3 = ST+3*Stride;
    return float4x3(Source[st0], Source[st1], Source[st2], Source[st3]);
}

[RootSignature(Present_RootSig)]
float3 main(float4 Pos : SV_Position, float2 UV : TexCoord0) : SV_Target0
{
    // We subtract 0.5 because that represents the center of the pixel.  We need to know where
    // we lie between two pixel centers, and we will use frac() for that.  We subtract another
    // 1.0 so that our start index is one pixel to the left.
    float2 TopLeft = UV * kSrcResolution - 1.5;

#ifdef LANCZOS_VERTICAL
    float4 Weights = GetUpscaleFilterWeights(frac(TopLeft.y));
    float4x3 Samples = LoadSamples(int2(floor(Pos.x), TopLeft.y), uint2(0, 1));
#else
    float4 Weights = GetUpscaleFilterWeights(frac(TopLeft.x));
    float4x3 Samples = LoadSamples(int2(TopLeft.x, floor(Pos.y)), uint2(1, 0));
#endif

    float3 Result = mul(Weights, Samples);

#ifdef LANCZOS_VERTICAL
    // Transform to display settings
    Result = RemoveDisplayProfile(Result, LDR_COLOR_FORMAT);
    Result = ApplyDisplayProfile(Result, DISPLAY_PLANE_FORMAT);
#endif

    return Result;
}
