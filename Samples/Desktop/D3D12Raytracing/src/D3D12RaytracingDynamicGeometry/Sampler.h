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

typedef DirectX::XMFLOAT2 UnitSquareSample2D;        // unit square sample with a valid range of <0,1>
typedef DirectX::XMFLOAT3 HemisphereSample3D;  // hemisphere sample <0,1>

namespace Samplers {

    namespace HemisphereDistribution {
        enum Enum {
            Uniform,
            Cosine
        };
    }
    class Sampler
    {
        static const UINT s_seed = 1729;
        std::mt19937 m_generatorURNG;               // Uniform random number generator
        std::function<UINT()> GetRandomJump;        // Generates a random uniform index within [0, m_numSamples - 1]
        std::function<UINT()> GetRandomSetJump;     // Generates a random uniform index within [0, m_numSampleSets - 1]
        std::function<float()> GetRandomFloat01;    // Generates a random uniform float within [0,1]
    public:
        // Constructor, desctructor
        Sampler();
        ~Sampler() {}

        // Member functions
        UnitSquareSample2D GetSample2D();
        HemisphereSample3D GetHemisphereSample3D();
        void Reset(UINT numSamples, UINT numSampleSets, HemisphereDistribution::Enum useConsineHemisphereDistribution);

    private:
        UINT GetSampleIndex();
        void InitializeUniformHemisphereSamples();
        void InitializeCosineHemisphereSamples();

    protected:
        virtual void GenerateSamples2D() = 0; // Generate sample patterns in a unit square.
        UnitSquareSample2D GenerateRandomUnitSquareSample2D();
        
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


    class MultiJittered : public Sampler
    {
    private:
        void GenerateSamples2D();
    };

    class Random : public Sampler
    {
    private:
        void GenerateSamples2D();
    };

} // namespace Samplers