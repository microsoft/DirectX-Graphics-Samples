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

#ifndef __TONE_MAPPING_UTILITY_HLSLI__
#define __TONE_MAPPING_UTILITY_HLSLI__

#include "ShaderUtility.hlsli"

float3 ToneMapRGB( float3 hdr )
{
	return 1 - exp2(-hdr);
}

float ToneMapLuma( float luma )
{
	return 1 - exp2(-luma);
}

float InverseToneMapLuma(float luma)
{
	return -log2(max(1e-6, 1 - luma));
}

float3 InverseToneMapRGB(float3 ldr)
{
	return -log2(max(1e-6, 1 - ldr));
}

// This variant rescales only the luminance of the color to fit in the [0, 1] range while
// preserving hue.
float3 ToneMap( float3 hdr )
{
	float luma = RGBToLuminance(hdr);//MaxChannel(hdr);
	return hdr / max(luma, 1e-6) * ToneMapLuma(luma);
}

float3 InverseToneMap( float3 ldr )
{
	float luma = RGBToLuminance(ldr);//MaxChannel(ldr);
	return ldr / max(luma, 1e-6) * InverseToneMapLuma(luma);
}

float3 ToneMapACES( float3 hdr )
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return saturate((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E));
}

float3 ApplyToeRGB( float3 ldr, float ToeStrength )
{
	return ldr * ToneMap(ldr * ToeStrength);
}

float3 ApplyToe(float3 ldr, float ToeStrength)
{
	float luma = RGBToLuminance(ldr);//MaxChannel(ldr);
	return ldr * ToneMapLuma(luma * ToeStrength);
}

// It's possible to rescale tonemapped values without inverting the tone operator and
// applying a new one.  This will compute the desired rescale factor which can be used
// with the ReToneMap* functions.
float ComputeHDRRescale(float PW, float MB, float N = 0.25)
{
	return log2(1 - N * PW / MB) / log2(1 - N);
}

float ReToneMapLuma(float luma, float Rescale)
{
	return 1 - pow(1 - luma, Rescale);
}

float3 ReToneMapRGB(float3 ldr, float Rescale)
{
	return ldr / max(ldr, 1e-6) * (1 - pow(1 - ldr, Rescale));
}

float3 ReToneMap(float3 ldr, float Rescale)
{
	float luma = RGBToLuminance(ldr);//MaxChannel(ldr);
	return ldr / max(luma, 1e-6) * ReToneMapLuma(luma, Rescale);
}

#endif // __TONE_MAPPING_UTILITY_HLSLI__