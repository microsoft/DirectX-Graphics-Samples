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

#ifndef VOLUMETRICPRIMITIVESLIBRARY_H
#define VOLUMETRICPRIMITIVESLIBRARY_H

#include "RaytracingShaderHelper.h"

// Calculate a magnitude of an influence from a Metaball charge.
// mbRadius - largest possible area of metaball contribution - AKA its bounding sphere.
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies
float CalculateMetaballPotential(in float3 position, in float3 mbCenter, in float mbRadius)
{
    float d = length(position - mbCenter);

    if (d <= mbRadius)
    {
        //return 2 * (d * d * d) / (mbRadius * mbRadius * mbRadius)
        //    - 3 * (d * d) / (mbRadius * mbRadius)
        //    + 1;
        float dR = d * mbRadius;
        return (2 * dR - 3) * (dR * dR) + 1;
    }
    return 0;
}

// Test if a ray with RayFlags and segment <RayTMin(), RayTCurrent()> intersect a metaball field.
// Ref: http://www.geisswerks.com/ryan/BLOBS/blobs.html
bool RayMetaballsIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr, in float totalTime)
{
    const int N = 3;
    // ToDo Pass in from the app?
    // Metaball centers at t0 and t1 key frames.
    float3 keyFrameCenters[N][2] =
    {
        { float3(-0.5, -0.3, -0.4),float3(0.5,-0.3,-0.0) },
        { float3(0.0, -0.4, 0.5), float3(0.0, 0.4, 0.5) },
        { float3(0.5,0.5, 0.4), float3(-0.5, 0.2, -0.4) }
    };

    // Calculate animated metaball center positions.
    float  tAnimate = CalculateAnimationInterpolant(totalTime, 8.0f);
    float3 centers[N];
    centers[0] = lerp(keyFrameCenters[0][0], keyFrameCenters[0][1], tAnimate);
    centers[1] = lerp(keyFrameCenters[1][0], keyFrameCenters[1][1], tAnimate);
    centers[2] = lerp(keyFrameCenters[2][0], keyFrameCenters[2][1], tAnimate);

    // Metaball field radii of max influence
    float radii[N] = { 0.50, 0.65, 0.50 };
    
    // Set bounds for ray march to in and out intersection 
    // against max influence of all metaballs. 
    float tmin, tmax;
    tmin = RayTCurrent();
    tmax = RayTMin();
    for (UINT j = 0; j < N; j++)
    {
        float _thit, _tmax;
        ProceduralPrimitiveAttributes _attr;
        if (RaySphereIntersectionTest(ray, _thit, _tmax, _attr, centers[j], radii[j]))
        {
            tmin = min(_thit, tmin);
            tmax = max(_tmax, tmax);
        }
    }

    UINT MAX_STEPS = 128;
    float tstep = (tmax - tmin) / (MAX_STEPS - 1);

    // ToDo lipchshitz ray marcher

    // Step along the ray calculating field potentials from all metaballs.
    for (UINT i = 0; i < MAX_STEPS; i++)
    {
        float t = tmin + i * tstep;
        float3 position = ray.origin + t * ray.direction;
        float fieldPotentials[N];
        fieldPotentials[0] = CalculateMetaballPotential(position, centers[0], radii[0]);
        fieldPotentials[1] = CalculateMetaballPotential(position, centers[1], radii[1]);
        fieldPotentials[2] = CalculateMetaballPotential(position, centers[2], radii[2]);
        float fieldPotential = fieldPotentials[0] + fieldPotentials[1] + fieldPotentials[2];

        // ToDo revise threshold range
        // ToDo pass threshold from app
        // Threshold - valid range is (0, 0.25>, the larger the threshold the smaller the blob.
        float threshold = 0.15f;
        if (fieldPotential >= threshold)
        {
            // Calculate normal as a weighted average of sphere normals from contributing metaballs.
            float3 normal = float3(0, 0, 0);
            normal += fieldPotentials[0] * CalculateNormalForARaySphereHit(ray, t, centers[0]);
            normal += fieldPotentials[1] * CalculateNormalForARaySphereHit(ray, t, centers[1]);
            normal += fieldPotentials[2] * CalculateNormalForARaySphereHit(ray, t, centers[2]);
            if (IsAValidHit(ray, t, normal))
            {
                thit = t;
                attr.normal = normalize(normal / fieldPotential);
                return true;
            }
        }
    }

    return false;
}


#endif // VOLUMETRICPRIMITIVESLIBRARY_H