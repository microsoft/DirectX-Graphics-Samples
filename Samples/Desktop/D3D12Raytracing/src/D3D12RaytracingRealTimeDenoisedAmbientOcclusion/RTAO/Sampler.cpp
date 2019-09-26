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

// Get a valid index from <0, m_numSampleSets * m_numSamples>.
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
    return m_setJump + m_shuffledIndices[(m_index++ + m_jump) % m_numSamples];
}

// Resets the sampler with newly randomly generated samples
void Sampler::Reset(UINT numSamples, UINT numSampleSets, HemisphereDistribution::Enum hemisphereDistribution)
{
    m_index = 0;
    m_numSamples = numSamples;
    m_numSampleSets = numSampleSets;
    m_samples.resize(m_numSamples * m_numSampleSets, UnitSquareSample2D(FLT_MAX, FLT_MAX));
    m_shuffledIndices.resize(m_numSamples * m_numSampleSets);
    m_hemisphereSamples.resize(m_numSamples * m_numSampleSets, HemisphereSample3D(FLT_MAX, FLT_MAX, FLT_MAX));
    
    // Reset generator and initialize distributions.
    {
        // Initialize to the same seed for determinism.
        m_generatorURNG.seed(s_seed);

        uniform_int_distribution<UINT> jumpDistribution(0, m_numSamples - 1);
        uniform_int_distribution<UINT> jumpSetDistribution(0, m_numSampleSets - 1);

        uniform_real_distribution<float> unitSquareDistribution(0.f, 1.f);

        // Specify the next representable value for the end range, since
        // uniform_real_distribution constructs excluding the end value [being, end).
        uniform_real_distribution<float> unitSquareDistributionInclusive(0.f, nextafter(1.f, FLT_MAX));

        GetRandomJump = bind(jumpDistribution, ref(m_generatorURNG));
        GetRandomSetJump = bind(jumpSetDistribution, ref(m_generatorURNG));
        GetRandomFloat01 = bind(unitSquareDistribution, ref(m_generatorURNG));
        GetRandomFloat01inclusive = bind(unitSquareDistributionInclusive, ref(m_generatorURNG));
    }

    // Generate random samples.
    {
        GenerateSamples2D();

        switch (hemisphereDistribution)
        {
        case HemisphereDistribution::Uniform: InitializeHemisphereSamples(0.f); break;
        case HemisphereDistribution::Cosine: InitializeHemisphereSamples(1.f); break;
        }

        for (UINT i = 0; i < m_numSampleSets; i++)
        {
            auto first = begin(m_shuffledIndices) + i * m_numSamples;
            auto last = first + m_numSamples;
            
            iota(first, last, 0u); // Fill with 0, 1, ..., m_numSamples - 1 
            shuffle(first, last, m_generatorURNG);
        }
    }
};

UnitSquareSample2D Sampler::RandomFloat01_2D()
{
    return XMFLOAT2(GetRandomFloat01(), GetRandomFloat01());
}

UINT Sampler::GetRandomNumber(UINT min, UINT max)
{
    uniform_int_distribution<UINT> distribution(min, max);
    return distribution(m_generatorURNG);
}
UnitSquareSample2D Sampler::GetSample2D()
{
    return m_samples[GetSampleIndex()];
}

HemisphereSample3D Sampler::GetHemisphereSample3D()
{
    return m_hemisphereSamples[GetSampleIndex()];
}

// Initialize samples on a 3D hemisphere from 2D unit square samples
// cosDensityPower - cosine density power {0, 1, ...}. 0:uniform, 1:cosine,...
void Sampler::InitializeHemisphereSamples(float cosDensityPower)
{
    for (UINT i = 0; i < m_samples.size(); i++)
    {
        // Compute azimuth (phi) and polar angle (theta)
        /*
        float phi = XM_2PI * m_samples[i].x;   
        float theta = acos(powf((1.f - m_samples[i].y), 1.f / (cosDensityPower + 1)));

        // Convert the polar angles to a 3D point in local orthornomal 
        // basis with orthogonal unit vectors along x, y, z.
        m_hemisphereSamples[i].x = sinf(theta) * cosf(phi);
        m_hemisphereSamples[i].y = sinf(theta) * sinf(phi);
        m_hemisphereSamples[i].z = cosf(theta);
        */ 
        // Optimized version using trigonometry equations.
        float cosTheta = powf((1.f - m_samples[i].y), 1.f / (cosDensityPower + 1));
        float sinTheta = sqrtf(1.f - cosTheta * cosTheta);
        m_hemisphereSamples[i].x = sinTheta * cosf(XM_2PI * m_samples[i].x);
        m_hemisphereSamples[i].y = sinTheta * sinf(XM_2PI * m_samples[i].x);
        m_hemisphereSamples[i].z = cosTheta;

    }
}

// Generate multi-jittered sample patterns on a unit square [0,1].
// Ref: Section 5.3.4 in Ray Tracing from the Ground Up.
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
        for (UINT col = 0, i = 0; col < N; col++)
            for (UINT row = 0; row < N; row++, i++)
            {
                XMFLOAT2 stratum(static_cast<float>(row), static_cast<float>(col));
                XMFLOAT2 cell(static_cast<float>(col), static_cast<float>(row));
                UnitSquareSample2D randomValue01 = RandomFloat01_2D();

                SAMPLE(i).x = (randomValue01.x + cell.x) / T + stratum.x / N;
                SAMPLE(i).y = (randomValue01.y + cell.y) / T + stratum.y / N;
            }

        // Shuffle sample axes such that there's a sample in each stratum 
        // and n-rooks is maintained.

        // Shuffle x coordinate across rows within a column
        for (UINT row = 0; row < N - 1; row++)
            for (UINT col = 0; col < N; col++)
            {
                UINT k = GetRandomNumber(row + 1, N - 1);
                swap(SAMPLE(row*N + col).x, SAMPLE(k*N + col).x);
            }

        // Shuffle y coordinate across columns within a row
        for (UINT row = 0; row < N; row++)
            for (UINT col = 0; col < N - 1; col++)
            {
                UINT k = GetRandomNumber(col + 1, N - 1);
                swap(SAMPLE(row*N + col).y, SAMPLE(row*N + k).y);
            }
        
    }
}

// Generate random sample patterns on unit square.
void Random::GenerateSamples2D()
{
    for (auto& sample : m_samples)
    {
        sample = RandomFloat01_2D();
    }
}

