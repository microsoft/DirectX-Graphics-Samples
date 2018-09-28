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

#include "stdafx.h"
#include "Sampler.h"

using namespace std;


UINT Sampler::GetSampleIndex()
{
    mt19937 generator(1729); 
    uniform_int_distribution<UINT> jumpDistribution(0, m_numSamples - 1);
    uniform_int_distribution<UINT> setJumpDistribution(0, m_numSampleSets - 1);

    // Initialize sample and set jumps.
    if (m_index % m_numSample == 0)
    {
        // pick a random index jump within a set
        m_jump = jumpDistribution(generator);
        
        // pick a random set index jump
        m_setJump = setJumpDistribution(generator) * m_numSamples;
    }
    return m_setJump + m_shuffledIndices[(m_index++ + m_jump) % m_numSample];
}

UnitSample2D Sampler::GetSample2D()
{
    return m_samples[GetSampleIndex()];
}

HemisphereSample3D Sampler::GetHemisphereSample3D()
{
    return m_hemisphereSamples[GetSampleIndex()];
}

void ShuffleIndices();
virtual void GenerateSamples() = 0;