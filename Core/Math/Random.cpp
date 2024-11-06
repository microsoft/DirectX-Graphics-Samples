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
#include "Random.h"

namespace Math
{
    RandomNumberGenerator g_RNG;
}

#ifndef USE_STL

inline uint32_t Twiddle(uint32_t u, uint32_t v)
{
    return (((u & 0x80000000ul) | (v & 0x7FFFFFFFul)) >> 1) ^ ((v & 1ul) * 0x9908B0DFul);
}

uint32_t Math::RandomNumberGenerator::Rand()
{
    if (m_Pos == n)
    {
        for (uint32_t i = 0; i < n - m; ++i)
            m_State[i] = m_State[i + m] ^ Twiddle(m_State[i], m_State[i + 1]);
        for (uint32_t i = n - m; i < n - 1; ++i)
            m_State[i] = m_State[i + m - n] ^ Twiddle(m_State[i], m_State[i + 1]);
        m_State[n - 1] = m_State[m - 1] ^ Twiddle(m_State[n - 1], m_State[0]);

        m_Pos = 0;
    }

    uint32_t x = m_State[m_Pos++];
    x ^= (x >> 11);
    x ^= (x << 7) & 0x9D2C5680ul;
    x ^= (x << 15) & 0xEFC60000ul;
    return x ^ (x >> 18);
}

void Math::RandomNumberGenerator::SetSeed(uint32_t s)
{
    m_State[0] = s;

    for (int i = 1; i < n; ++i)
        m_State[i] = 1812433253ul * (m_State[i - 1] ^ (m_State[i - 1] >> 30)) + i;

    m_Pos = n;
}

#endif
