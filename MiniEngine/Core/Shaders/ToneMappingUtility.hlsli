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

float3 ToneMapACES( float3 L )
{
	L *= 0.6;
	const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
	return saturate((L * (A * L + B)) / (L * (C * L + D) + E));
}

float3 InverseToneMapACES( float3 N )
{
	const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
	float3 L = 0.5 * (D * N - sqrt(((D*D - 4*C*E) * N + 4*A*E-2*B*D) * N + B*B) - B) / (A - C * N);
	// The compiler will turn this into the following code, but if you want to change the parameters
	// to the curve, it's best to stick with the general formula.
	//float3 L = 0.5 * (0.59 * N - sqrt((-1.0127 * N + 1.3702) * N + 0.0009) - 0.03) / (2.51 - 2.43 * N);
	return L /= 0.6;
}

#endif // __TONE_MAPPING_UTILITY_HLSLI__