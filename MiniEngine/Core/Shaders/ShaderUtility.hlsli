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

#pragma warning( disable : 3571 )

#include "ColorSpaceUtility.hlsli"
#include "PixelPacking.hlsli"

// Encodes a smooth logarithmic gradient for even distribution of precision natural to vision
float LinearToLogLuminance( float x, float gamma = 4.0 )
{
	return log2(lerp(1, exp2(gamma), x)) / gamma;
}

// This assumes the default color gamut found in sRGB and REC709.  The color primaries determine these
// coefficients.  Note that this operates on linear values, not gamma space.
float RGBToLuminance( float3 x )
{
	return dot( x, float3(0.212671, 0.715160, 0.072169) );		// Defined by sRGB/Rec.709 gamut
}

float MaxChannel(float3 x)
{
	return max(x.x, max(x.y, x.z));
}

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

// This is the same as above, but converts the linear luminance value to a more subjective "perceived luminance",
// which could be called the Log-Luminance.
float RGBToLogLuminance( float3 x, float gamma = 4.0 )
{
	return LinearToLogLuminance( RGBToLuminance(x), gamma );
}

float3 RGBFullToLimited( float3 x )
{
	return saturate(x) * 219.0 / 255.0 + 16.0 / 255.0;
}

float3 RGBLimitedToFull( float3 x )
{
	return saturate((x - 16.0 / 255.0) * 255.0 / 219.0);
}

#define COLOR_FORMAT_LINEAR			0
#define COLOR_FORMAT_sRGB_FULL		1
#define COLOR_FORMAT_sRGB_LIMITED	2
#define COLOR_FORMAT_Rec709_FULL	3
#define COLOR_FORMAT_Rec709_LIMITED	4
#define COLOR_FORMAT_HDR10			5
#define COLOR_FORMAT_TV_DEFAULT		COLOR_FORMAT_Rec709_LIMITED
#define COLOR_FORMAT_PC_DEFAULT		COLOR_FORMAT_sRGB_FULL

#define HDR_COLOR_FORMAT			COLOR_FORMAT_LINEAR
#define LDR_COLOR_FORMAT			COLOR_FORMAT_LINEAR
#define DISPLAY_PLANE_FORMAT		COLOR_FORMAT_PC_DEFAULT

float3 ApplyColorProfile( float3 x, int Format )
{
	switch (Format)
	{
	default:
	case COLOR_FORMAT_LINEAR:
		return x;
	case COLOR_FORMAT_sRGB_FULL:
		return LinearToSRGB(x);
	case COLOR_FORMAT_sRGB_LIMITED:
		return RGBFullToLimited(LinearToSRGB(x));
	case COLOR_FORMAT_Rec709_FULL:
		return LinearToREC709(x);
	case COLOR_FORMAT_Rec709_LIMITED:
		return RGBFullToLimited(LinearToREC709(x));
	case COLOR_FORMAT_HDR10:
		return LinearToREC2084(ConvertCS_709to2020(x));
	};
}

float3 LinearizeColor( float3 x, int Format )
{
	switch (Format)
	{
	default:
	case COLOR_FORMAT_LINEAR:
		return x;
	case COLOR_FORMAT_sRGB_FULL:
		return SRGBToLinear(x);
	case COLOR_FORMAT_sRGB_LIMITED:
		return SRGBToLinear(RGBLimitedToFull(x));
	case COLOR_FORMAT_Rec709_FULL:
		return REC709ToLinear(x);
	case COLOR_FORMAT_Rec709_LIMITED:
		return REC709ToLinear(RGBLimitedToFull(x));
	case COLOR_FORMAT_HDR10:
		return ConvertCS_2020to709(REC2084ToLinear(x));
	};
}

float3 ConvertColor( float3 x, int FromFormat, int ToFormat )
{
	if (FromFormat == ToFormat)
		return x;

	return ApplyColorProfile(LinearizeColor(x, FromFormat), ToFormat);
}