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

float3 LinearToSRGB( float3 x )
{
	// This can be 9 cycles faster than the "precise" version
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float3 LinearToSRGB_Exact( float3 x )
{
	return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

float3 SRGBToLinear_Exact( float3 x )
{
	return x < 0.04045 ? x / 12.92 : pow( (abs(x) + 0.055) / 1.055, 2.4 );
}

float3 LinearToREC709_Exact( float3 x )
{
	return x < 0.0018 ? 4.5 * x : 1.099 * pow(abs(x), 0.45) - 0.099;
}

float3 REC709ToLinear_Exact( float3 x )
{
	return x < 0.0081 ? x / 4.5 : pow(abs((x + 0.099) / 1.099), 1.0 / 0.45);
}

// Encodes a smooth logarithmic gradient for even distribution of precision natural to vision
float LinearToLogLuminance( float x, float gamma = 4.0 )
{
	return log2(lerp(1, exp2(gamma), x)) / gamma;
}

// This assumes the default color gamut found in sRGB and REC709.  The color primaries determine these
// coefficients.  Note that this operates on linear values, not gamma space.
float RGBToLuminance( float3 x )
{
	return dot( x, float3(0.212671, 0.715160, 0.072169) );		// Defined by sRGB gamut
//	return dot( x, float3(0.299, 0.587, 0.114) );				// Old CRT phosphor luma measurements
}

// Assumes the "white point" is 1.0.  Prescale your HDR values if otherwise.  'E' affects the rate
// at which colors blow out to white.
float3 ToneMap( float3 hdr, float E = 4.0 )
{
	return (1 - exp2(-E * hdr)) / (1 - exp2(-E));
}

// This variant rescales only the luminance of the color to fit in the [0, 1] range while preserving hue.
float3 ToneMap2( float3 hdr, float E = 4.0 )
{
	float luma = RGBToLuminance(hdr);
	return hdr * (1 - exp2(-E * luma)) / (1 - exp2(-E)) / (luma + 0.0001);
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
#define COLOR_FORMAT_7e3_FLOAT_FULL	5
#define COLOR_FORMAT_6e4_FLOAT_FULL	6
#define COLOR_FORMAT_TV_DEFAULT		COLOR_FORMAT_Rec709_LIMITED
#define COLOR_FORMAT_PC_DEFAULT		COLOR_FORMAT_sRGB_FULL

#define HDR_COLOR_FORMAT			COLOR_FORMAT_LINEAR
#define LDR_COLOR_FORMAT			COLOR_FORMAT_LINEAR
#if _XBOX_ONE
	#define DISPLAY_PLANE_FORMAT	COLOR_FORMAT_TV_DEFAULT
	#define OVERLAY_PLANE_FORMAT	COLOR_FORMAT_sRGB_FULL
#else
	#define DISPLAY_PLANE_FORMAT	COLOR_FORMAT_PC_DEFAULT
#endif

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
		return LinearToREC709_Exact(x);
	case COLOR_FORMAT_Rec709_LIMITED:
		return RGBFullToLimited(LinearToREC709_Exact(x));

	// Xbox formats:  10-bit floats with biased exponents; range: [0, 2)
	case COLOR_FORMAT_7e3_FLOAT_FULL:
		return x * 16.0;
	case COLOR_FORMAT_6e4_FLOAT_FULL:
		return x * 256.0;
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
		return SRGBToLinear_Exact(x);
	case COLOR_FORMAT_sRGB_LIMITED:
		return SRGBToLinear_Exact(RGBLimitedToFull(x));
	case COLOR_FORMAT_Rec709_FULL:
		return REC709ToLinear_Exact(x);
	case COLOR_FORMAT_Rec709_LIMITED:
		return REC709ToLinear_Exact(RGBLimitedToFull(x));

	// Xbox formats:  10-bit floats with biased exponents; range: [0, 2)
	case COLOR_FORMAT_7e3_FLOAT_FULL:
		return x / 16.0;
	case COLOR_FORMAT_6e4_FLOAT_FULL:
		return x / 256.0;
	};
}

float3 ConvertColor( float3 x, int FromFormat, int ToFormat )
{
	if (FromFormat == ToFormat)
		return x;

	return ApplyColorProfile(LinearizeColor(x, FromFormat), ToFormat);
}