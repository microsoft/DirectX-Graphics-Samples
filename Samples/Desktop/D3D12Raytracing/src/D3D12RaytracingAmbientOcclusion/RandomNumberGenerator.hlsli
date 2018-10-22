//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RANDOMNUMBERGENERATOR_H
#define RANDOMNUMBERGENERATOR_H

// Ref: http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
namespace RNG
{
    // Create an initial random number for a thread.
    uint SeedThread(uint seed)
    {
        // Thomas Wang hash 
        // Ref: http://www.burtleburtle.net/bob/hash/integer.html
        seed = (seed ^ 61) ^ (seed >> 16);
        seed *= 9;
        seed = seed ^ (seed >> 4);
        seed *= 0x27d4eb2d;
        seed = seed ^ (seed >> 15);
        return seed;
    }

    // Generate a random number [0, UINT_MAX].
    uint Random(inout uint state)
    {
        // Xorshift algorithm from George Marsaglia's paper.
        state ^= (state << 13);
        state ^= (state >> 17);
        state ^= (state << 5);
        return state;
    }

    // Generate a random number [0,1].
    float Random01(inout uint state)
    {
        uint UintMax = 0xffffffff;
        return Random(state) / float(UintMax);
    }

    // Generate a random number [0,1).
    float Random01ex(inout uint state)
    {
        uint UintMax = 0xffffffff;
        return Random(state) / (float(UintMax) + 1.0f);
    }

    // Generate a random number [_min,_max].
    uint Random(inout uint state, uint _min, uint _max)
    {
        // min is here just for safety so as not to return _max + 1 due float imprecision.
        return min(_min + floor((_max - _min + 1) * Random01ex(state)), _max);
    }
}
#endif // RANDOMNUMBERGENERATOR_H