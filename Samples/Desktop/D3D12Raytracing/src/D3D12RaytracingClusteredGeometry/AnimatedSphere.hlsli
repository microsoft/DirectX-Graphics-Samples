// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).

#ifndef ANIMATED_SPHERE_HLSLI
#define ANIMATED_SPHERE_HLSLI

#include "RaytracingHlslCompat.h"

float3 AnimatedSpherePhase(float3 p, float time)
{
    return ANIMATED_SPHERE_WOBBLE_FREQUENCY * p + time * 3.0
        + float3(0.0, 1.7, 3.4);
}

float3 AnimatedSpherePosition(float3 p, float time)
{
    float3 waves = sin(AnimatedSpherePhase(p, time));
    float scale = 1.0 + ANIMATED_SPHERE_WOBBLE_AMPLITUDE
        * (waves.x + waves.y + waves.z) * (1.0 / 3.0);
    return p * scale;
}

float3 AnimatedSphereNormal(float3 p, float time)
{
    float3 waves, slopes;
    sincos(AnimatedSpherePhase(p, time), waves, slopes);
    float scale = 1.0 + ANIMATED_SPHERE_WOBBLE_AMPLITUDE
        * (waves.x + waves.y + waves.z) * (1.0 / 3.0);
    float3 gradient = (ANIMATED_SPHERE_WOBBLE_AMPLITUDE
        * ANIMATED_SPHERE_WOBBLE_FREQUENCY / 3.0) * slopes;
    // Transform the sphere normal by the radial deformation's inverse Jacobian.
    return normalize((scale + dot(p, gradient)) * p - dot(p, p) * gradient);
}

#endif
