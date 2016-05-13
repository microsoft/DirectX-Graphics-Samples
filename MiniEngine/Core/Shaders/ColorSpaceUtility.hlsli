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

//
// Gamma ramps and encoding transfer functions
//
// Agnostic to color space though usually tightly coupled.  For instance, sRGB is both a
// color space (defined by three basis vectors and a white point) and a gamma ramp.  Gamma
// ramps are designed to reduce perceptual error when compressing floats to small integer
// values.  More precision is needed in darker gradients as the human eye is more sensitive
// to contrast with dark values.  A logarithmic curve is often used.
//

// This approximates sRGB sufficiently enough that for 8-bit encodings is indistinguishable
// from the "slow" version.  This can be a lot faster due to avoiding three pow() calls.
float3 LinearToSRGB_Fast( float3 x )
{
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(x - 0.00228) - 0.13448 * x + 0.005719;
}

float3 LinearToSRGB( float3 x )
{
	return x < 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

float3 SRGBToLinear( float3 x )
{
	return x < 0.04045 ? x / 12.92 : pow( (x + 0.055) / 1.055, 2.4 );
}

float3 LinearToREC709( float3 x )
{
	return x < 0.018 ? 4.5 * x : 1.099 * pow(x, 0.45) - 0.099;
}

float3 REC709ToLinear( float3 x )
{
	return x < 0.081 ? x / 4.5 : pow((x + 0.099) / 1.099, 1.0 / 0.45);
}

// Same as Rec.709 transfer but more precise (intended for 12-bit rather than 10-bit)
float3 LinearToREC2020(float3 x)
{
	return x < 0.0181 ? 4.5 * x : 1.0993 * pow(x, 0.45) - 0.0993;
}

float3 REC2020ToLinear(float3 x)
{
	return x < 0.08145 ? x / 4.5 : pow((x + 0.0993) / 1.0993, 1.0 / 0.45);
}

// This is the new HDR OETF (transfer function)
float3 LinearToREC2084(float3 L)
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float3 Lp = pow(L, m1);
	return pow((c1 + c2 * Lp) / (1 + c3 * Lp), m2);
}

float3 REC2084ToLinear(float3 N)
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float3 Np = pow(N, 1 / m2);
	return pow(max(Np - c1, 0) / (c2 - c3 * Np), 1 / m1);
}

//
// Color space conversions
//
// These assume linear (not gamma-encoded) values.  A color space conversion is a change
// of basis (like in Linear Algebra).  Since a color space is defined by three vectors--
// the basis vectors--changing space involves a matrix-vector multiplication.  Note that
// changing the color space may result in colors that are "out of bounds" because some
// color spaces have larger gamuts than others.  When converting some colors from a wide
// gamut to small gamut, negative values may result, which are inexpressible in that new
// color space.
//
// It would be ideal to build a color pipeline which never throws away inexpressible (but
// perceivable) colors.  This means using a color space that is as wide as possible.  The
// XYZ color space is the neutral, all-encompassing color space, but it has the unfortunate
// property of having negative values (specifically in X and Z).  To correct this, a further
// transformation can be made to X and Z to make them always positive.  They can have their
// precision needs reduced by dividing by Y, allowing X and Z to be packed into two UNORM8s.
// This color space is called YUV for lack of a better name.
//

float3 ConvertCS_2020toXYZ( float3 RGB2020 )
{
	static const float3x3 ConvMat = 
	{
		6.36953507e-01, 1.44619185e-01, 1.68855854e-01,
		2.62698339e-01, 6.78008766e-01, 5.92928953e-02,
		4.99407097e-17, 2.80731358e-02, 1.06082723e+00
	};
	return mul(ConvMat, RGB2020);
}


float3 ConvertCS_XYZto2020( float3 XYZ )
{
	static const float3x3 ConvMat =
	{
		1.71666343, -0.35567332, -0.25336809,
		-0.66667384, 1.61645574, 0.0157683,
		0.01764248, -0.04277698, 0.94224328
	};
	return mul(ConvMat, XYZ);
}

float3 ConvertCS_709toXYZ(float3 RGB709)
{
	static const float3x3 ConvMat =
	{
		0.41238656, 0.35759149, 0.18045049,
		0.21263682, 0.71518298, 0.0721802,
		0.01933062, 0.11919716, 0.95037259
	};
	return mul(ConvMat, RGB709);
}


float3 ConvertCS_XYZto709(float3 XYZ)
{
	static const float3x3 ConvMat =
	{
		3.24100323, -1.53739897, -0.49861588,
		-0.96922425, 1.87592998, 0.04155423,
		0.05563942, -0.20401121, 1.05714898
	};
	return mul(ConvMat, XYZ);
}

// One assumes the shader compiler will fold the matrices together since they are
// constant literals.
float3 ConvertCS_709to2020( float3 RGB709 )
{
	//return ConvertCS_XYZto2020(ConvertCS_709toXYZ(RGB709));
	static const float3x3 ConvMat =
	{
		0.62740192, 0.32929197, 0.0433061,
		0.06909549, 0.91954428, 0.01136023,
		0.01639371, 0.08802816, 0.89557813
	};
	return mul(ConvMat, RGB709);
}

float3 ConvertCS_2020to709(float3 RGB2020)
{
	return ConvertCS_XYZto709(ConvertCS_2020toXYZ(RGB2020));
}