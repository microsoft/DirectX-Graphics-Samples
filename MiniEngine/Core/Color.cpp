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

#include "pch.h"
#include "Color.h"

using DirectX::XMVECTORU32;

uint32_t Color::R11G11B10F(bool RoundToEven) const
{
    static XMVECTORU32 Scale = { 0x07800000, 0x07800000, 0x07800000, 0 }; // 2^-112
    static XMVECTORU32 Round1 = { 0x00010000, 0x00010000, 0x00020000, 0 };
    static XMVECTORU32 Round2 = { 0x0000FFFF, 0x0000FFFF, 0x0001FFFF, 0 };
    static XMVECTORU32 Mask = { 0x0FFE0000, 0x0FFE0000, 0x0FFC0000, 0 };

    // Treat the values like integers as we clamp to [0, +Inf].  This translates 32-bit specials
    // to 16-bit specials (while also turning anything greater than MAX_HALF into +INF).
    __m128i ti = _mm_max_epi32(_mm_castps_si128(m_value), _mm_setzero_si128());
    ti = _mm_min_epi32(ti, _mm_set1_epi32(0x47800000)); // 2^16 = 65536.0f = INF

    // Bias the exponent by -112 (-127 + 15) to denormalize values < 2^-14
    ti = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(ti), Scale));

    if (RoundToEven)
    {
        // Add 0x10000 when odd, 0x0FFFF when even (before truncating bits)
        ti = _mm_add_epi32(ti, _mm_max_epi32(_mm_and_si128(_mm_srli_epi32(ti, 1), Round1), Round2));
    }
    else //if (RoundToNearest)
    {
        ti = _mm_add_epi32(ti, Round1);
    }

    XMVECTORU32 ret;
    ret.v = _mm_castsi128_ps(_mm_and_si128(ti, Mask));
    return ret.u[0] >> 17 | ret.u[1] >> 6 | ret.u[2] << 4;
}
