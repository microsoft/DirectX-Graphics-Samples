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
    uniform_int_distribution<UINT> jumpDistribution(0, m_numSamples - 1);
    uniform_int_distribution<UINT> jumpSetDistribution(0, m_numSampleSets - 1);

    // Specify the next representable value for the end range, since
    // uniform_real_distribution constructs excluding the end value [being, end).
    uniform_real_distribution<float> unitSquareDistribution(0.f, nextafter(1.0f, FLT_MAX));

    GetRandomJump = bind(jumpDistribution, std::ref(m_generatorURNG));
    GetRandomSetJump = bind(jumpSetDistribution, std::ref(m_generatorURNG));
    GetRandomFloat01 = bind(unitSquareDistribution, std::ref(m_generatorURNG));
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
    return m_setJump + m_shuffledIndices[(m_index++ + m_jump) % m_numSamples];
}

// Resets the sampler with newly randomly generated samples
void Sampler::Reset(UINT numSamples, UINT numSampleSets, HemisphereDistribution::Enum hemisphereDistribution)
{
    m_index = 0;

    // Reset the generator for determinism.
    m_generatorURNG.seed(s_seed);

    m_samples.resize(numSamples, UnitSquareSample2D(FLT_MAX, FLT_MAX));
    m_hemisphereSamples.resize(numSamples, HemisphereSample3D(FLT_MAX, FLT_MAX, FLT_MAX));
    m_numSamples = numSamples;
    m_numSampleSets = numSampleSets;

    GenerateSamples2D();

    switch (hemisphereDistribution)
    {
        HemisphereDistribution::Uniform: InitializeUniformHemisphereSamples(); break;
        HemisphereDistribution::Cosine: InitializeCosineHemisphereSamples(); break;
    }

    // Fill with 0, 1, ..., m_numSamples - 1
    iota(begin(m_shuffledIndices), end(m_shuffledIndices), 0u); // Fill with 0, 1, ..., 99.
    shuffle(m_shuffledIndices, m_shuffledIndices, m_generatorURNG);
    
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

// Generate multi-jittered sample patterns on unit square
// ...
void Random::GenerateSamples2D()
{

}

// Generate multi-jittered sample patterns on unit square
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

}

// Generate random sample patterns on unit square.
void Random::GenerateSamples2D()
{
    for (auto& sample : m_samples)
    {
        sample = GenerateRandomUnitSquareSample2D();
    }
}

