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
	return max(x, 0) * 219.0 / 255.0 + 16.0 / 255.0;
}

float3 RGBLimitedToFull( float3 x )
{
	return saturate((x - 16.0 / 255.0) * 255.0 / 219.0);
}

float3 LinearToFrameBufferFormat( float3 x, int bufferFormat = 0 )
{
	switch (bufferFormat)
	{
	default:
	case 0:		// Identity
		return x;
	case 1:		// 10-bit UNORM
		return LinearToSRGB_Exact(x);
	case 2:		// 7e3 float biased exponent
		return x * 16.0;
	case 3:		// 6e4 float biased exponent
		return x * 256.0;
	case 4:		// DCP disabled (already in REC709 RGB Limited)
		return RGBFullToLimited( LinearToREC709_Exact(x) );
	};
}

float3 FrameBufferFormatToLinear( float3 x, int bufferFormat = 0 )
{
	switch (bufferFormat)
	{
	default:
	case 0:		// Identity
		return x;
	case 1:		// 10-bit UNORM
		return SRGBToLinear_Exact(x);
	case 2:		// 7e3 float biased exponent
		return x / 16.0;
	case 3:		// 6e4 float biased exponent
		return x / 256.0;
	case 4:		// DCP disabled (already in REC709 RGB Limited)
		return REC709ToLinear_Exact( RGBLimitedToFull(x) );
	};
}