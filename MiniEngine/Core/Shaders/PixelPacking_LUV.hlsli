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

#ifndef __PIXEL_PACKING_LUV_HLSLI__
#define __PIXEL_PACKING_LUV_HLSLI__

// This is the CIELUV color space which separates luminance from chrominance (like xyY) and rotates
// chroma to be more perceptually uniform.  The intention is to be able to pack this triplet into a
// custom 32-bit encoding that maximizes luminance precision (better than 12-bit PQ) while leaving
// enough precision for chrominance to express all visible colors without (negligible) banding.

/*
// This describes the process of converting RGB to LUV.
    
// Start with the right RGBtoXYZ matrix for your color space (this one is sRGB D65)
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

static const float3x3 RGBtoXYZ =
{
    0.412387, 0.357591, 0.180450,
    0.212637, 0.715183, 0.072180,  <--The luminance dot product
    0.019331, 0.119197, 0.950373
};

// Compute u' and v'.  UV is a two dimensional term describing the pixel's
// chrominance (hue & saturation without brightness).  In this space (CIELUV),
// chrominance is fairly perceptually uniform.
// u' = 4X / (X + 15Y + 3Z)
// v' = 9Y / (X + 15Y + 3Z)

// Because all visible colors will be within the bounds of u':[0.00, 0.62], v':[0.01, 0.59],
// we can normalize the values (for unorm representation).
// U = u' / 0.62
// V = v' / 0.59

// If we compute these two values...
// A = 4 / 9 * 0.59 / 0.62 * X
// B = (X + 15 * Y + 3 * Z) * 0.59 / 9

// ...we can derive our final LUV from A, Y, and B
// L = Y
// U = A / B
// V = Y / B

// We can compute (A, Y, B) by multiplying XYZ by this matrix
static const float3x3 FixupMatrix =
{
    4.0 / 9.0 * 0.59 / 0.62, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.59 / 9.0, 15.0 * 0.59 / 9.0, 3.0 * 0.59 / 9.0
};

// But we should just concatenate the two matrices...
static const float3x3 EncodeMatrix = mul(FixupMatrix, RGBtoXYZ);
*/

float3 Rec709toLUV(float3 RGB)
{
    static const float3x3 EncodeMatrix = 
    {
        0.174414,  0.151239,  0.076320,
        0.212637,  0.715183,  0.072180,
        0.239929,  0.750147,  0.269713 
    };

    // Returns [A, Y, B], from which we can easily compress to LUV32
    return mul(EncodeMatrix, RGB);
}

float3 LUVtoRec709(float3 AYB)
{
    static const float3x3 DecodeMatrix = 
    {
         8.056027,  0.955680, -2.535335,
        -2.324391,  1.668159,  0.211293,
        -0.701623, -5.489756,  5.375334 
    };

    return mul(DecodeMatrix, AYB);
}

float3 Rec2020toLUV(float3 RGB)
{
    static const float3x3 EncodeMatrix = 
    {
        0.269393,  0.061165,  0.071416,
        0.262698,  0.678009,  0.059293,
        0.300076,  0.681710,  0.278003 
    };

    // Returns [A, Y, B], from which we can easily compress to LUV32
    return mul(EncodeMatrix, RGB);
}

float3 LUVtoRec2020(float3 AYB)
{
    static const float3x3 DecodeMatrix = 
    {
         4.258579,  0.911167, -1.288312,
        -1.588716,  1.537614,  0.080178,
        -0.700901, -4.753993,  4.791068 
    };

    return mul(DecodeMatrix, AYB);
}

uint PackLUV(float3 AYB)
{
    if (AYB.y < 0.00005)
        return 0;

    uint L = (f32tof16(AYB.y) + 1) >> 1;
    uint2 UV = saturate(AYB.xy / AYB.z) * 511.0 + 0.5;
    return L | UV.x << 14 | UV.y << 23;
}

float3 UnpackLUV(uint LUV)
{
    float L = f16tof32((LUV << 1) & 0x7FFE);
    float2 UV = (uint2(LUV >> 14, LUV >> 23) & 0x1FF) / 511.0;
    float B = L / max(UV.y, 1e-6);
    return float3(UV.x * B, L, B);
}

#endif // __PIXEL_PACKING_LUV_HLSLI__
