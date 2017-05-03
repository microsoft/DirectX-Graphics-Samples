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
// Author:  James Stanard 
//
// The CS for extracting bright pixels and saving a log-luminance map (quantized to 8 bits).  This
// is then used to generate an 8-bit histogram.

#include "ShaderUtility.hlsli"
#include "PostEffectsRS.hlsli"

SamplerState BiLinearClamp : register( s0 );
Texture2D<float3> SourceTex : register( t0 );
StructuredBuffer<float> Exposure : register( t1 );
RWTexture2D<uint> LumaResult : register( u0 );

cbuffer cb0
{
    float2 g_inverseOutputSize;
}

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // We need the scale factor and the size of one pixel so that our four samples are right in the middle
    // of the quadrant they are covering.
    float2 uv = DTid.xy * g_inverseOutputSize;
    float2 offset = g_inverseOutputSize * 0.25f;

    // Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
    float3 color1 = SourceTex.SampleLevel( BiLinearClamp, uv + float2(-offset.x, -offset.y), 0 );
    float3 color2 = SourceTex.SampleLevel( BiLinearClamp, uv + float2( offset.x, -offset.y), 0 );
    float3 color3 = SourceTex.SampleLevel( BiLinearClamp, uv + float2(-offset.x,  offset.y), 0 );
    float3 color4 = SourceTex.SampleLevel( BiLinearClamp, uv + float2( offset.x,  offset.y), 0 );

    // Compute average luminance
    float luma = RGBToLuminance(color1 + color2 + color3 + color4) * 0.25;

    // Prevent log(0) and put only pure black pixels in Histogram[0]
    if (luma == 0.0)
    {
        LumaResult[DTid.xy] = 0;
    }
    else
    {
        const float MinLog = Exposure[4];
        const float RcpLogRange = Exposure[7];
        float logLuma = saturate((log2(luma) - MinLog) * RcpLogRange);	// Rescale to [0.0, 1.0]
        LumaResult[DTid.xy] = logLuma * 254.0 + 1.0;					// Rescale to [1, 255]
    }
}
