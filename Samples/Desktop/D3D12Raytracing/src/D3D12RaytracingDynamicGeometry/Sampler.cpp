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
using namespace DirectX;
using namespace Samplers;

Sampler::Sampler() : 
    m_numSamples(0), 
    m_numSampleSets(0), 
    m_index(0) 
{
}

// Get a valid index from <0, m_numSapleSets * m_numSamples>.
// The index increases by 1 on each call, but on a first 
// access of a next sample set, the:
// - sample set is randomly picked
// - sample set is indexed from a random starting index within a set.
// In addition the order of indices is retrieved from shuffledIndices.
UINT Sampler::GetSampleIndex()
{
    // Initialize sample and set jumps.
    if (m_index % m_numSamples == 0)
    {
        // Pick a random index jump within a set.
        m_jump = GetRandomJump();
        
        // Pick a random set index jump.
        m_setJump = GetRandomSetJump() * m_numSamples;
    }
    return m_index++ % (m_numSamples * m_numSampleSets);
    return m_setJump + m_shuffledIndices[(m_index++ + m_jump) % m_numSamples];
}

// Resets the sampler with newly randomly generated samples
void Sampler::Reset(UINT numSamples, UINT numSampleSets, HemisphereDistribution::Enum hemisphereDistribution)
{
    m_index = 0;
    m_numSamples = numSamples;
    m_numSampleSets = numSampleSets;
    m_samples.resize(m_numSamples * m_numSampleSets, UnitSquareSample2D(FLT_MAX, FLT_MAX));
    m_shuffledIndices.resize(m_numSamples);
    m_hemisphereSamples.resize(m_numSamples, HemisphereSample3D(FLT_MAX, FLT_MAX, FLT_MAX));
    iota(begin(m_shuffledIndices), end(m_shuffledIndices), 0u);     // Fill with 0, 1, ..., m_numSamples - 1

    // Reset generator and initialize distributions.
    {
        // Initialize to the same seed for determinism.
        m_generatorURNG.seed(s_seed);

        uniform_int_distribution<UINT> jumpDistribution(0, m_numSamples - 1);
        uniform_int_distribution<UINT> jumpSetDistribution(0, m_numSampleSets - 1);

        // Specify the next representable value for the end range, since
        // uniform_real_distribution constructs excluding the end value [being, end).
        uniform_real_distribution<float> unitSquareDistribution(0.f, nextafter(1.0f, FLT_MAX));

        GetRandomJump = bind(jumpDistribution, ref(m_generatorURNG));
        GetRandomSetJump = bind(jumpSetDistribution, ref(m_generatorURNG));
        GetRandomFloat01 = bind(unitSquareDistribution, ref(m_generatorURNG));
    }

    // Generate random samples.
    {
        GenerateSamples2D();

        switch (hemisphereDistribution)
        {
        case HemisphereDistribution::Uniform: InitializeUniformHemisphereSamples(); break;
        case HemisphereDistribution::Cosine: InitializeCosineHemisphereSamples(); break;
        }

        //shuffle(begin(m_shuffledIndices), end(m_shuffledIndices), m_generatorURNG);
    }
};

UnitSquareSample2D Sampler::GenerateRandomUnitSquareSample2D()
{
    return XMFLOAT2(GetRandomFloat01(), GetRandomFloat01());
}

UnitSquareSample2D Sampler::GetSample2D()
{
    return m_samples[GetSampleIndex()];
}

HemisphereSample3D Sampler::GetHemisphereSample3D()
{
    return m_hemisphereSamples[GetSampleIndex()];
}

void Sampler::InitializeUniformHemisphereSamples()
{

}

void Sampler::InitializeCosineHemisphereSamples()
{

}

// Generate multi-jittered sample patterns on unit square.
// Ref: Section 5.3.4 in Ray Tracing from the Ground Up
// The distribution has good random sampling distributions
// with somewhat uniform distributions in both:
// - 2D
// - 1D projections of each axes.
// Multi-jittered is a combination of two sample distributions:
// - Jittered: samples are distributed on a NxN grid, 
//             with each sample being random within its cell.
// - N-rooks/Linear hypercube sampling: samples have uniform
//             distribution in 1D projections of each axes.
void MultiJittered::GenerateSamples2D()
{
    for (UINT s = 0; s < NumSampleSets(); s++)
    {
        // Generate samples on 2 level grid, with one sample per each (x,y)
        UINT sampleSetStartID = s * NumSamples();

        const UINT T = NumSamples();
        const UINT N = static_cast<UINT>(sqrt(T));

        #define SAMPLE(i) m_samples[sampleSetStartID + i]

        // Generate random samples
        for (UINT y = 0, i = 0; y < N; y++)
            for (UINT x = 0; x < N; x++, i++)
            {
                XMFLOAT2 gridID(static_cast<float>(x), static_cast<float>(y));
                XMFLOAT2 subID(static_cast<float>(y), static_cast<float>(x));
                UnitSquareSample2D randomValue01 = GenerateRandomUnitSquareSample2D();

                SAMPLE(i).x = (randomValue01.x + subID.x) / T + gridID.x / N;
                SAMPLE(i).y = (randomValue01.y + subID.y) / T + gridID.y / N;
            }

        // Shuffle sample axes such that there's a sample in each stratum 
        // and n-rooks is maintained.

        for (UINT i = 0; i < N; i++)
            for (UINT j = 0; j < N; j++)
            {
                uniform_int_distribution<UINT> dis(j, N - 1);
                UINT k = dis(m_generatorURNG);
                swap(SAMPLE(i*N + j).x, SAMPLE(i*N + k).x);
            }
        
        for (UINT i = 0; i < N; i++)
            for (UINT j = 0; j < N; j++)
            {
                uniform_int_distribution<UINT> dis(j, N - 1);
                UINT k = dis(m_generatorURNG);
               // if (i * N + j >= 1) break;
                swap(SAMPLE(j*N + i).y, SAMPLE(k*N + i).y);
            }
    }
}

// Generate random sample patterns on unit square.
void Random::GenerateSamples2D()
{
    for (auto& sample : m_samples)
    {
        sample = GenerateRandomUnitSquareSample2D();
    }
}

