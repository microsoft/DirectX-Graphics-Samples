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

#pragma once

#include "Common.h"
#include <random>

namespace Math
{
    class RandomNumberGenerator
    {
    public:
        RandomNumberGenerator() : m_gen(m_rd())
        {
        }

        // Default int range is [MIN_INT, MAX_INT].  Max value is included.
        int32_t NextInt( void )
        {
            return std::uniform_int_distribution<int32_t>(0x80000000, 0x7FFFFFFF)(m_gen);
        }

        int32_t NextInt( int32_t MaxVal )
        {
            return std::uniform_int_distribution<int32_t>(0, MaxVal)(m_gen);
        }

        int32_t NextInt( int32_t MinVal, int32_t MaxVal )
        {
            return std::uniform_int_distribution<int32_t>(MinVal, MaxVal)(m_gen);
        }

        // Default float range is [0.0f, 1.0f).  Max value is excluded.
        float NextFloat( float MaxVal = 1.0f )
        {
            return std::uniform_real_distribution<float>(0.0f, MaxVal)(m_gen);
        }

        float NextFloat( float MinVal, float MaxVal )
        {
            return std::uniform_real_distribution<float>(MinVal, MaxVal)(m_gen);
        }

        void SetSeed( UINT s )
        {
            m_gen.seed(s);
        }

    private:

        std::random_device m_rd;
        std::minstd_rand m_gen;
    };

    extern RandomNumberGenerator g_RNG;
};
