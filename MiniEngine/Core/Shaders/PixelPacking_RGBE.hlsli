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

#ifndef __PIXEL_PACKING_RGBE_HLSLI__
#define __PIXEL_PACKING_RGBE_HLSLI__

#include "ColorSpaceUtility.hlsli"

// RGBE packs 9 bits per color channel while encoding the multiplier as a perfect power of 2
// (just the exponent).  What's nice about this is that it gives you a lot more range than RGBM.
// "Precise rounding" will round rather than truncate bits.  It's more correct (like IEEE) but
// unnecessary for real-time rendering.
uint PackRGBE(float3 rgb)
{
    // The stored exponent will be +1 of the maximum exponent, so [-15, +16].  (There are
    // no floating point specials, so we get all 32 exponents.)
    const float kMaxVal = asfloat(0x477F8000); // 1.FF x 2^+15
    const float kMinVal = asfloat(0x37800000); // 1.00 x 2^-16

    rgb = clamp(rgb, 0, kMaxVal);

    float MaxChannel = max(max(kMinVal, rgb.r), max(rgb.g, rgb.b));

    // Always uses precise rounding
    uint Exp = (asuint(MaxChannel) + 0x4000) & 0x7F800000;
    float ScaleR = asfloat(0x83000000 - Exp);	// 2^(-exp + 8)   (negate exponent and add 8)
    uint R = rgb.r * ScaleR + 0.5;
    uint G = rgb.g * ScaleR + 0.5;
    uint B = rgb.b * ScaleR + 0.5;
    uint E = (Exp - 0x37800000) << 4;	// E = exp - 127 + 15 + 1  (exponent - bias8 + bias5 + 1)
    return E | B << 18 | G << 9 | R;
}

float3 UnpackRGBE(uint p)
{
    // 12 VOPs (11 VOPs when ldexp() is backed by a single instruction)
    float R = p & 0x1FF; // 2
    float G = (p >> 9) & 0x1FF; // 2
    float B = (p >> 18) & 0x1FF;  // 2
    return ldexp(float3(R, G, B), (p >> 27) + 103); // 5
    //return float3(R, G, B) * asfloat(((p >> 27) + 103) << 23); // 6
}

uint PackRGBE_sRGB(float3 rgb)
{
    // The stored exponent will be +1 of the maximum exponent, so [-15, +16].  (There are
    // no floating point specials, so we get all 32 exponents.)
    const float kMaxVal = asfloat(0x477FFFFF); // 1.FFFFFE x 2^+15
    const float kMinVal = asfloat(0x37800000); // 1.000000 x 2^-16

    rgb = clamp(rgb, 0, kMaxVal);

    float MaxChannel = max(max(kMinVal, rgb.r), max(rgb.g, rgb.b));

    // Always uses precise rounding
    uint Exp = asuint(MaxChannel) & 0x7F800000;
    float ScaleR = asfloat(0x7E800000 - Exp);	// 2^(-exp - 1)   (negate exponent and sub 1)
    rgb = ApplySRGBCurve(rgb * ScaleR) * 511.0 + 0.5;
    uint R = rgb.r;
    uint G = rgb.g;
    uint B = rgb.b;
    uint E = (Exp - 0x37800000) << 4;	// E = exp - 127 + 15 + 1  (exponent - bias8 + bias5 + 1)
    return E | B << 18 | G << 9 | R;
}

float3 UnpackRGBE_sRGB(uint p)
{
    float3 rgb = (uint3(p, p >> 9, p >> 18) & 0x1FF) / 511.0;
    return RemoveSRGBCurve(rgb) * asfloat(((p >> 27) + 112) << 23);
}

uint PackRGBE_sqrt(float3 rgb)
{
    // The stored exponent will be +1 of the maximum exponent, so [-15, +16].  (There are
    // no floating point specials, so we get all 32 exponents.)
    const float kMaxVal = asfloat(0x477FFFFF); // 1.FFFFFE x 2^+15
    const float kMinVal = asfloat(0x37800000); // 1.000000 x 2^-16

    rgb = clamp(rgb, 0, kMaxVal);

    float MaxChannel = max(max(kMinVal, rgb.r), max(rgb.g, rgb.b));

    // Always uses precise rounding
    uint Exp = asuint(MaxChannel) & 0x7F800000;
    float ScaleR = asfloat(0x7E800000 - Exp);	// 2^(-exp - 1)   (negate exponent and sub 1)
    rgb = sqrt(rgb * ScaleR) * 511.0 + 0.5;
    uint R = rgb.r;
    uint G = rgb.g;
    uint B = rgb.b;
    uint E = (Exp - 0x37800000) << 4;	// E = exp - 127 + 15 + 1  (exponent - bias8 + bias5 + 1)
    return E | B << 18 | G << 9 | R;
}

float3 UnpackRGBE_sqrt(uint p)
{
    float3 rgb = (uint3(p, p >> 9, p >> 18) & 0x1FF) / 511.0;
    return rgb * rgb * asfloat(((p >> 27) + 112) << 23);
}

#endif // __PIXEL_PACKING_RGBE_HLSLI__
