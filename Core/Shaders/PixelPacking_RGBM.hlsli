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

#ifndef __PIXEL_PACKING_RGBM_HLSLI__
#define __PIXEL_PACKING_RGBM_HLSLI__

#include "ColorSpaceUtility.hlsli"

float4 ToRGBM( float3 rgb, float PeakValue = 255.0 / 16.0 )
{
    rgb = saturate(rgb / PeakValue);
    float maxVal = max(max(1e-6, rgb.x), max(rgb.y, rgb.z));
    maxVal = ceil(maxVal * 255.0) / 255.0;
    return float4(rgb / maxVal, maxVal);
}

float3 FromRGBM(float4 rgbm, float PeakValue = 255.0 / 16.0 )
{
    return rgbm.rgb * rgbm.a * PeakValue;
}

// RGBM is a good way to pack HDR values into R8G8B8A8_UNORM
uint PackRGBM( float4 rgbm, bool sRGB = true )
{
    if (sRGB)
        rgbm.rgb = ApplySRGBCurve(rgbm.rgb);
    rgbm = rgbm * 255.0 + 0.5;
    return (uint)rgbm.a << 24 | (uint)rgbm.b << 16 | (uint)rgbm.g << 8 | (uint)rgbm.r;
}

float4 UnpackRGBM( uint p, bool sRGB = true )
{
    float4 rgbm = float4(uint4(p, p >> 8, p >> 16, p >> 24) & 0xFF);
    rgbm /= 255.0;
    if (sRGB)
        rgbm.rgb = RemoveSRGBCurve(rgbm.rgb);
    return rgbm;
}

#endif // __PIXEL_PACKING_RGBM_HLSLI__
