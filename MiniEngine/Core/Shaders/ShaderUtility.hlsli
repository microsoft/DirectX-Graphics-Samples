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

#ifndef __SHADER_UTILITY_HLSLI__
#define __SHADER_UTILITY_HLSLI__

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

// This is the same as above, but converts the linear luminance value to a more subjective "perceived luminance",
// which could be called the Log-Luminance.
float RGBToLogLuminance( float3 x, float gamma = 4.0 )
{
    return LinearToLogLuminance( RGBToLuminance(x), gamma );
}

// A fast invertible tone map that preserves color (Reinhard)
float3 TM( float3 rgb )
{
    return rgb / (1 + RGBToLuminance(rgb));
}

// Inverse of preceding function
float3 ITM( float3 rgb )
{
    return rgb / (1 - RGBToLuminance(rgb));
}

// 8-bit should range from 16 to 235
float3 RGBFullToLimited8bit( float3 x )
{
    return saturate(x) * 219.0 / 255.0 + 16.0 / 255.0;
}

float3 RGBLimitedToFull8bit( float3 x )
{
    return saturate((x - 16.0 / 255.0) * 255.0 / 219.0);
}

// 10-bit should range from 64 to 940
float3 RGBFullToLimited10bit( float3 x )
{
    return saturate(x) * 876.0 / 1023.0 + 64.0 / 1023.0;
}

float3 RGBLimitedToFull10bit( float3 x )
{
    return saturate((x - 64.0 / 1023.0) * 1023.0 / 876.0);
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

float3 ApplyDisplayProfile( float3 x, int DisplayFormat )
{
    switch (DisplayFormat)
    {
    default:
    case COLOR_FORMAT_LINEAR:
        return x;
    case COLOR_FORMAT_sRGB_FULL:
        return ApplySRGBCurve(x);
    case COLOR_FORMAT_sRGB_LIMITED:
        return RGBFullToLimited10bit(ApplySRGBCurve(x));
    case COLOR_FORMAT_Rec709_FULL:
        return ApplyREC709Curve(x);
    case COLOR_FORMAT_Rec709_LIMITED:
        return RGBFullToLimited10bit(ApplyREC709Curve(x));
    case COLOR_FORMAT_HDR10:
        return ApplyREC2084Curve(REC709toREC2020(x));
    };
}

float3 RemoveDisplayProfile( float3 x, int DisplayFormat )
{
    switch (DisplayFormat)
    {
    default:
    case COLOR_FORMAT_LINEAR:
        return x;
    case COLOR_FORMAT_sRGB_FULL:
        return RemoveSRGBCurve(x);
    case COLOR_FORMAT_sRGB_LIMITED:
        return RemoveSRGBCurve(RGBLimitedToFull10bit(x));
    case COLOR_FORMAT_Rec709_FULL:
        return RemoveREC709Curve(x);
    case COLOR_FORMAT_Rec709_LIMITED:
        return RemoveREC709Curve(RGBLimitedToFull10bit(x));
    case COLOR_FORMAT_HDR10:
        return REC2020toREC709(RemoveREC2084Curve(x));
    };
}

float3 ConvertColor( float3 x, int FromFormat, int ToFormat )
{
    if (FromFormat == ToFormat)
        return x;

    return ApplyDisplayProfile(RemoveDisplayProfile(x, FromFormat), ToFormat);
}

#endif // __SHADER_UTILITY_HLSLI__
