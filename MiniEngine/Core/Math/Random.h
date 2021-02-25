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

#define USE_STL

#include "Common.h"
#ifdef USE_STL
    #include <random>
#endif

namespace Math
{
#ifdef USE_STL
    class RandomNumberGenerator
    {
    public:
        RandomNumberGenerator(uint32_t s = 0) : m_gen(m_rd())
        {
            if (s != 0)
                SetSeed(s);
        }

        // Default int range is [MIN_INT, MAX_INT].  Max value is included.
        int32_t NextInt(void)
        {
            return std::uniform_int_distribution<int32_t>(0x80000000, 0x7FFFFFFF)(m_gen);
        }

        int32_t NextInt(int32_t MaxVal)
        {
            return std::uniform_int_distribution<int32_t>(0, MaxVal)(m_gen);
        }

        int32_t NextInt(int32_t MinVal, int32_t MaxVal)
        {
            return std::uniform_int_distribution<int32_t>(MinVal, MaxVal)(m_gen);
        }

        // Default float range is [0.0f, 1.0f).  Max value is excluded.
        float NextFloat(float MaxVal = 1.0f)
        {
            return std::uniform_real_distribution<float>(0.0f, MaxVal)(m_gen);
        }

        float NextFloat(float MinVal, float MaxVal)
        {
            return std::uniform_real_distribution<float>(MinVal, MaxVal)(m_gen);
        }

        void SetSeed(uint32_t s)
        {
            m_gen.seed(s);
        }

    private:

        std::random_device m_rd;
        std::minstd_rand m_gen;
    };
#else
    class RandomNumberGenerator
    {
    public:
        RandomNumberGenerator( uint32_t s = 0xFEA4BEE5 )
        {
            SetSeed(s);
        }

        // Default int range is [MIN_INT, MAX_INT].  Max value is included.
        int32_t NextInt( void )
        {
            return (int32_t)Rand();
        }

        uint32_t NextInt( uint32_t MaxVal )
        {
            return Rand() % (MaxVal + 1);
        }

        int32_t NextInt( int32_t MinVal, int32_t MaxVal )
        {
            return MinVal + NextInt(MaxVal - MinVal);
        }

        // Default float range is [0.0f, 1.0f).  Max value is excluded.
        float NextFloat( float MaxVal = 1.0f )
        {
            union { uint32_t intRep; float fltRep; };
            intRep = 0x3f800000 | (NextInt() & 0x7FFFFF);
            return (fltRep - 1.0f) * MaxVal;
        }

        float NextFloat( float MinVal, float MaxVal )
        {
            return MinVal + NextFloat(MaxVal - MinVal);
        }

        void SetSeed( uint32_t s );

    private:

        static const int n = 624, m = 397;

        uint32_t m_State[n];
        uint32_t m_Pos;

        uint32_t Rand();
    };
#endif

    extern RandomNumberGenerator g_RNG;
} // namespace Math
