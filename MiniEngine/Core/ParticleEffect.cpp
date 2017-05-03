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
//

#include "pch.h"
#include "ParticleEffect.h"
#include "CommandContext.h"
#include "GraphicsCore.h"
#include "BufferManager.h"
#include "ParticleEffectManager.h"
#include "GameInput.h"
#include "Math/Random.h"

using namespace Math;
using namespace ParticleEffects;

namespace ParticleEffects
{
    extern ComputePSO s_ParticleSpawnCS;
    extern ComputePSO s_ParticleUpdateCS;
    extern ComputePSO s_ParticleDispatchIndirectArgsCS;
    extern StructuredBuffer SpriteVertexBuffer;
    extern RandomNumberGenerator s_RNG;
}

ParticleEffect::ParticleEffect(ParticleEffectProperties& effectProperties)
{
    m_ElapsedTime = 0.0;
    m_EffectProperties = effectProperties;
}

inline static Color RandColor( Color c0, Color c1 )
{
    // We might want to find min and max of each channel rather than assuming c0 <= c1
    return Color(
        s_RNG.NextFloat( c0.R(), c1.R()),
        s_RNG.NextFloat( c0.G(), c1.G()),
        s_RNG.NextFloat( c0.B(), c1.B()),
        s_RNG.NextFloat( c0.A(), c1.A())
        );
}

inline static XMFLOAT3 RandSpread( const XMFLOAT3& s )
{
    // We might want to find min and max of each channel rather than assuming c0 <= c1
    return XMFLOAT3(
        s_RNG.NextFloat(-s.x, s.x),
        s_RNG.NextFloat(-s.y, s.y), 
        s_RNG.NextFloat(-s.z, s.z)
        );
}

void ParticleEffect::LoadDeviceResources(ID3D12Device* device)
{
    (device); // Currently unused.  May be useful with multi-adapter support.

    m_OriginalEffectProperties = m_EffectProperties; //In case we want to reset
    
    //Fill particle spawn data buffer
    ParticleSpawnData* pSpawnData = (ParticleSpawnData*)_malloca(m_EffectProperties.EmitProperties.MaxParticles * sizeof(ParticleSpawnData));
    
    for (UINT i = 0; i < m_EffectProperties.EmitProperties.MaxParticles; i++)
    {
        ParticleSpawnData& SpawnData = pSpawnData[i];
        SpawnData.AgeRate = 1.0f / s_RNG.NextFloat( m_EffectProperties.LifeMinMax.x, m_EffectProperties.LifeMinMax.y );
        float horizontalAngle = s_RNG.NextFloat(XM_2PI);
        float horizontalVelocity = s_RNG.NextFloat( m_EffectProperties.Velocity.GetX(), m_EffectProperties.Velocity.GetY() );
        SpawnData.Velocity.x = horizontalVelocity * cos(horizontalAngle);
        SpawnData.Velocity.y = s_RNG.NextFloat( m_EffectProperties.Velocity.GetZ(), m_EffectProperties.Velocity.GetW() );
        SpawnData.Velocity.z = horizontalVelocity * sin(horizontalAngle);

        SpawnData.SpreadOffset = RandSpread(m_EffectProperties.Spread ) ;

        SpawnData.StartSize = s_RNG.NextFloat( m_EffectProperties.Size.GetX(), m_EffectProperties.Size.GetY() );
        SpawnData.EndSize = s_RNG.NextFloat( m_EffectProperties.Size.GetZ(), m_EffectProperties.Size.GetW() );
        SpawnData.StartColor = RandColor( m_EffectProperties.MinStartColor, m_EffectProperties.MaxStartColor );
        SpawnData.EndColor = RandColor( m_EffectProperties.MinEndColor, m_EffectProperties.MaxEndColor );
        SpawnData.Mass = s_RNG.NextFloat( m_EffectProperties.MassMinMax.x, m_EffectProperties.MassMinMax.y );
        SpawnData.RotationSpeed = s_RNG.NextFloat(); //todo
        SpawnData.Random = s_RNG.NextFloat();
    }
    
    m_RandomStateBuffer.Create(L"ParticleSystem::SpawnDataBuffer", m_EffectProperties.EmitProperties.MaxParticles, sizeof(ParticleSpawnData), pSpawnData);
    _freea(pSpawnData);

    m_StateBuffers[0].Create(L"ParticleSystem::Buffer0", m_EffectProperties.EmitProperties.MaxParticles, sizeof(ParticleMotion));
    m_StateBuffers[1].Create(L"ParticleSystem::Buffer1", m_EffectProperties.EmitProperties.MaxParticles, sizeof(ParticleMotion));
    m_CurrentStateBuffer = 0;

    //DispatchIndirect args buffer / number of thread groups
    __declspec(align(16)) UINT DispatchIndirectData[3] = { 0, 1, 1 };
    m_DispatchIndirectArgs.Create(L"ParticleSystem::DispatchIndirectArgs", 1, sizeof(D3D12_DISPATCH_ARGUMENTS), DispatchIndirectData);

}

void ParticleEffect::Update(ComputeContext& CompContext,  float timeDelta)
{

    m_ElapsedTime += timeDelta;
    m_EffectProperties.EmitProperties.LastEmitPosW = m_EffectProperties.EmitProperties.EmitPosW;
    
    //m_EffectProperties.EmitProperties.EmitPosW = XMFLOAT3(ComputeConstants.EmitPosW.x + 1.0f * float(GameInput::IsPressed(GameInput::kBButton)), ComputeConstants.EmitPosW.y + 1.0f * float(GameInput::IsPressed(GameInput::kYButton)), ComputeConstants.EmitPosW.z - 1.0f * float(GameInput::IsPressed(GameInput::kAButton)));//
    //m_EffectProperties.EmitProperties.EmitPosW.x += m_EffectProperties.DirectionIncrement.x;
    //m_EffectProperties.EmitProperties.EmitPosW.y += m_EffectProperties.DirectionIncrement.y;
    //m_EffectProperties.EmitProperties.EmitPosW.z += m_EffectProperties.DirectionIncrement.z;


    //CPU side random num gen
    for (uint32_t i = 0; i < 64; i++)
    {
        UINT random = (UINT)s_RNG.NextInt(m_EffectProperties.EmitProperties.MaxParticles - 1);
        m_EffectProperties.EmitProperties.RandIndex[i].x = random;
    }
    CompContext.SetDynamicConstantBufferView(2, sizeof(EmissionProperties), &m_EffectProperties.EmitProperties);	

    CompContext.TransitionResource(m_StateBuffers[m_CurrentStateBuffer], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    CompContext.SetDynamicDescriptor(4, 0, m_RandomStateBuffer.GetSRV());
    CompContext.SetDynamicDescriptor(4, 1, m_StateBuffers[m_CurrentStateBuffer].GetSRV());

    m_CurrentStateBuffer ^= 1;

    CompContext.ResetCounter(m_StateBuffers[m_CurrentStateBuffer]);

    CompContext.SetPipelineState(s_ParticleUpdateCS);
    CompContext.TransitionResource(m_StateBuffers[m_CurrentStateBuffer], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    CompContext.TransitionResource(m_DispatchIndirectArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    CompContext.SetDynamicDescriptor(3, 2, m_StateBuffers[m_CurrentStateBuffer].GetUAV());
    CompContext.DispatchIndirect(m_DispatchIndirectArgs, 0);

    // Why need a barrier here so long as we are artificially clamping particle count.  This allows living
    // particles to take precedence over new particles.  The current system always spawns a multiple of 64
    // particles (To Be Fixed) until the total particle count reaches maximum.
    CompContext.InsertUAVBarrier(m_StateBuffers[m_CurrentStateBuffer]);
    
    // Spawn to replace dead ones 
    CompContext.SetPipelineState(s_ParticleSpawnCS);
    CompContext.SetDynamicDescriptor(4, 0, m_RandomStateBuffer.GetSRV());
    UINT NumSpawnThreads = (UINT)(m_EffectProperties.EmitRate * timeDelta);
    CompContext.Dispatch((NumSpawnThreads + 63) / 64, 1, 1);

    // Output number of thread groups into m_DispatchIndirectArgs	
    CompContext.SetPipelineState(s_ParticleDispatchIndirectArgsCS);
    CompContext.TransitionResource(m_DispatchIndirectArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    CompContext.TransitionResource(m_StateBuffers[m_CurrentStateBuffer], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    CompContext.SetDynamicDescriptor(4, 0, m_StateBuffers[m_CurrentStateBuffer].GetCounterSRV(CompContext));
    CompContext.SetDynamicDescriptor(3, 1, m_DispatchIndirectArgs.GetUAV());
    CompContext.Dispatch(1, 1, 1);
}


void ParticleEffect::Reset()
{
    m_EffectProperties = m_OriginalEffectProperties;
}
