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

#pragma once

namespace Samplers {

    typedef DirectX::XMFLOAT2 UnitSquareSample2D;  // unit square sample with a valid range of <0,1>
    typedef DirectX::XMFLOAT3 HemisphereSample3D;  // hemisphere sample <0,1>

    namespace HemisphereDistribution {
        enum Enum {
            Uniform,
            Cosine
        };
    }
    class Sampler
    {
        static const UINT s_seed = 1729;
        std::function<UINT()> GetRandomJump;        // Generates a random uniform index within [0, m_numSamples - 1]
        std::function<UINT()> GetRandomSetJump;     // Generates a random uniform index within [0, m_numSampleSets - 1]
        std::function<float()> GetRandomFloat01;    // Generates a random uniform float within [0,1)
        std::function<float()> GetRandomFloat01inclusive; // Generates a random uniform float within [0,1]
    public:
        // Constructor, desctructor
        Sampler();
        ~Sampler() {}
        
        // Accessors
        UINT NumSamples() { return m_numSamples; }
        UINT NumSampleSets() { return m_numSampleSets; }

        // Member functions
        UnitSquareSample2D GetSample2D();
        HemisphereSample3D GetHemisphereSample3D();
        void Reset(UINT numSamples, UINT numSampleSets, HemisphereDistribution::Enum useConsineHemisphereDistribution);

    private:
        UINT GetSampleIndex();
        void InitializeHemisphereSamples(float cosDensityPower);

    protected:
        virtual void GenerateSamples2D() = 0; // Generate sample patterns in a unit square.
        UnitSquareSample2D RandomFloat01_2D();
        UINT GetRandomNumber(UINT min, UINT max);

        
        std::mt19937 m_generatorURNG;  // Uniform random number generator
        UINT m_numSamples;      // number of samples in a set (pattern).
        UINT m_numSampleSets;   // number of sample sets.
        std::vector<UnitSquareSample2D> m_samples; // samples on a unit square.
        std::vector<HemisphereSample3D> m_hemisphereSamples; // unit square samples on a hemisphere

        UINT m_index;           // next sample index.


        // Sample correlations and access pattern repeats can
        // lead to aliasing. Therefore selection of a set, 
        // starting sample index within a set, as well as 
        // index order is randomized to reduce correlations.

        // Shuffled indices to the samples.
        // Shuffling indices helps reduces spatial correlation
        // among sample sets. Accessing spattialy correlated 
        // samples for subsequent effects for a ray
        // can lead to aliasing. This is a case when the initial
        // samples are generated always in the same spatial order,
        // such as is the case in multi-jittered sampler.
        std::vector<UINT> m_shuffledIndices;

        // Index jump.
        // Accessing samples randomly within a set
        // helps reduce correlation among repeated sample 
        // sets, particularly when sampling ray hits close to 
        // each other. 
        UINT m_jump;

        // Set jump
        // Accessing sets randomly helps reduce correlation
        // and break repeating pattern access to sets among  
        // neighboring rays.
        UINT m_setJump;
    };

    // Multi-jittered sample patterns on unit square
    // Ref: Section 5.3.4 in [Ray Tracing from the Ground Up]
    // The distribution has good random sampling distributions
    // with somewhat uniform distributions in both:
    // - 2D
    // - 1D projections of each axes.
    // Multi-jittered is a combination of two sample distributions:
    // - Jittered: samples are distributed on a NxN grid, 
    //             with each sample being random within its cell.
    // - N-rooks/Linear hypercube sampling: samples have uniform
    //             distribution in 1D projections of each axes.
    class MultiJittered : public Sampler
    {
    private:
        void GenerateSamples2D();
    };

    // Random sample patterns on unit square
    class Random : public Sampler
    {
    private:
        void GenerateSamples2D();
    };

} // namespace Samplers