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
// Author(s):  Julia Careaga
//             James Stanard

#pragma once
#include "pch.h"
#include "GpuBuffer.h"
#include "ParticleEffectProperties.h"
#include "ParticleShaderStructs.h"

class ParticleEffect 
{
public:
    ParticleEffect(ParticleEffectProperties& effectProperties);
    void LoadDeviceResources(ID3D12Device* device);
    void Update(ComputeContext& CompContext, float timeDelta);
    float GetLifetime(){ return m_EffectProperties.TotalActiveLifetime; }
    float GetElapsedTime(){ return m_ElapsedTime; }
    void Reset();

private:

    StructuredBuffer m_StateBuffers[2];
    uint32_t m_CurrentStateBuffer;
    StructuredBuffer m_RandomStateBuffer; 
    IndirectArgsBuffer m_DispatchIndirectArgs;
    IndirectArgsBuffer m_DrawIndirectArgs;

    ParticleEffectProperties m_EffectProperties;
    ParticleEffectProperties m_OriginalEffectProperties;
    float m_ElapsedTime;
    UINT m_effectID;
    

};
