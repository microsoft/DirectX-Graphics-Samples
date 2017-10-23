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

#pragma once

#include "ParticleEffectProperties.h"
#include "ParticleEffect.h"
#include "CommandContext.h"
#include "Math/Random.h"

namespace Math
{
    class Camera;
}

namespace ParticleEffects
{
    void Initialize( uint32_t MaxDisplayWidth, uint32_t MaxDisplayHeight );
    void Shutdown();
    void ClearAll();
    typedef uint32_t EffectHandle;
    EffectHandle PreLoadEffectResources( ParticleEffectProperties& effectProperties );
    EffectHandle InstantiateEffect( EffectHandle effectHandle );
    EffectHandle InstantiateEffect( ParticleEffectProperties& effectProperties );
    void Update(ComputeContext& Context, float timeDelta );
    void Render(CommandContext& Context, const Camera& Camera, ColorBuffer& ColorTarget, DepthBuffer& DepthTarget, ColorBuffer& LinearDepth);
    void ResetEffect(EffectHandle EffectID);
    float GetCurrentLife(EffectHandle EffectID);

    extern BoolVar Enable;
    extern BoolVar PauseSim;
    extern BoolVar EnableTiledRendering;
    extern bool Reproducible; //If you want to repro set to true. When true, effect uses the same set of random numbers each run
    extern UINT ReproFrame;
} // namespace ParticleEffects
