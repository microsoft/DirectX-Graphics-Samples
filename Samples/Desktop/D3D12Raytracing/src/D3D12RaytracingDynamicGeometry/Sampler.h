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

typedef DirectXMath::XMFLOAT2 UnitSample2D;        // unit square sample with a valid range of <0,1>
typedef DirectXMath::XMFLOAT3 HemisphereSample3D;  // hemisphere sample <0,1>

class Sampler
{
public:
    // Constructor, desctructor
    Sampler() : m_numSamples(0), m_numSampleSets(0), m_index(0) {};
    ~Sampler() {};

    // Member functions
    UnitSample2D GetSample2D();
    HemisphereSample3D GetHemisphereSample3D();

    void ShuffleIndices();
    virtual void GenerateSamples() = 0;
private:
    UINT GetSampleIndex();

protected:
    UINT m_numSamples;      // number of samples in a set (pattern).
    UINT m_numSampleSets;   // number of sample sets.
    std::vector<UnitSample2D> m_samples; // samples on a unit square.
    std::vector<HemisphereSample3D> m_hemisphereSamples; // unit square samples on a hemisphere

    UINT m_index;         // next sample index.
    

    // Sample correlations and access pattern repets can
    // lead to aliasing. Therefore selection of sets, 
    // starting sample index within a set, as well as 
    // index order is randomized to reduce correlations.

    // Shuffled indices to the samples.
    // Shuffling indices helps reduces spatial correlation
    // among sample sets. Accessing spattialy correlated 
    // samples for subsequent effects for a ray
    // can lead to aliasing.
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