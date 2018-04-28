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


struct Metaball
{
    float3 center;
    float  radius;
    //float  invRadius;
};

// Calculate a magnitude of an influence from a Metaball charge.
// Return metaball potential range: <0,1>
// mbRadius - largest possible area of metaball contribution - AKA its bounding sphere.
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies
float CalculateMetaballPotential(in float3 position, in Metaball blob, out float distance)
{
    distance = length(position - blob.center);

    if (distance <= blob.radius)
    {
        // ToDo compare perf
#if 1
        return 2 * (distance * distance * distance) / (blob.radius * blob.radius * blob.radius)
            - 3 * (distance * distance) / (blob.radius * blob.radius)
            + 1;
#else
        float dR = distance * blob.invRadius;
        return (2 * dR - 3) * (dR * dR) + 1;
#endif
    }
    return 0;
}

float3 CalculateMetaballNormal(in float3 position, in Metaball blob)
{
    float3 distanceVector = position - blob.center;
    float distance = length(distanceVector);

    if (distance <= blob.radius)
    {
        float derivativeCoef = 6 * sqrt(distance) / (blob.radius * blob.radius * blob.radius) + 6 / (blob.radius * blob.radius);
        return derivativeCoef * distanceVector;
    }
    else
        return float3(0, 0, 0);
}

float3 CalculateMetaballsNormal(in float3 position, in Metaball blobs[3], in float fieldPotentials[3], in float fieldPotential)
{
    float dummy;
    float2 e = float2(1.0, -1.0) * 0.5773 * 0.0001;
    float3 normal = float3(0, 0, 0);

    for (UINT i = 0; i < 3; i++)
    {
        normal += fieldPotentials[i]*CalculateMetaballNormal(position, blobs[i]);
    }
    return normalize(normal/fieldPotential);
}


// Test if a ray with RayFlags and segment <RayTMin(), RayTCurrent()> intersect a metaball field.
// The test sphere traces through the metaball field until it hits a threshold isosurface. 
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies
bool RayMetaballsIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr, in float totalTime)
{
    const UINT N = 3;
    // ToDo Pass in from the app?
    // Metaball centers at t0 and t1 key frames.
    float3 keyFrameCenters[N][2] =
    {
        { float3(-0.5, -0.3, -0.4),float3(0.5,-0.3,-0.0) },
        { float3(0.0, -0.4, 0.5), float3(0.0, 0.4, 0.5) },
        { float3(0.5,0.5, 0.4), float3(-0.5, 0.2, -0.4) }
    };
    // Metaball field radii of max influence
    float radii[N] = { 0.50, 0.65, 0.50 };    // ToDo Compare perf with precomputed invRadius

    // Calculate animated metaball center positions.
    float  tAnimate = CalculateAnimationInterpolant(totalTime, 20.0f);
    Metaball blobs[N];
    for (UINT j = 0; j < N; j++)
    {
        blobs[j].center = lerp(keyFrameCenters[j][0], keyFrameCenters[j][1], tAnimate);
        blobs[j].radius = radii[j];
    }

    // Set ray march bounds to the min and max of all metaball intersections.
    float tmin, tmax;
    tmin = INFINITY;
    tmax = -INFINITY;

#if 1
    float3 aabb[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };

    if (!RayAABBIntersectionTest(ray, aabb, tmin, tmax))
    {
        return false;
    }
#else
    // Find the entry and exit points for all metaball bounding spheres combined.
    for (UINT j = 0; j < N; j++)
    {
        float _thit, _tmax;
        if (RaySolidSphereIntersectionTest(ray, _thit, _tmax, blob[j].center, blob[j].radius))
        {
            tmin = min(_thit, tmin);
            tmax = max(_tmax, tmax);
        }
    }
#endif
    tmin = max(tmin, RayTMin());
    tmax = min(tmax, RayTCurrent());

    UINT MAX_STEPS = 1024;
    const float minTStep = 0.0001;
    float t = tmin;
    UINT iStep = 0;
    // Step along the ray calculating field potentials from all metaballs
    // until we hit a isusurface equal to Threshold field potential.
    while (iStep++ < MAX_STEPS && t <= RayTCurrent())
    {
        float3 position = ray.origin + t * ray.direction;
        float fieldPotentials[N];              // Field potentials for each metaball.
        float sumFieldPotential = 0;           // Sum of all metaball field potentials.
        float signedMinDistanceToABlob = 1000;//INFINITY   // Signed minimum distance to blobs' bounding spheres.
        float sumRi = 0;                       // Sum of all contributing metaball radii.
      
        // Calculate field potentials from all metaballs.
        for (UINT j = 0; j < N; j++)
        {
            float distance;
            fieldPotentials[j] = CalculateMetaballPotential(position, blobs[j], distance);
            sumFieldPotential += fieldPotentials[j];

            signedMinDistanceToABlob = min(signedMinDistanceToABlob, distance - radii[j]);
            sumRi += (fieldPotentials[j] > 0) ? radii[j] : 0;
        }

        // Have we crossed the implicit surface.
        // ToDo pass threshold from app
        // Threshold - valid range is (0, 0.1>, the larger the threshold the smaller the blob.
        const float Threshold = 0.35f;
        if (sumFieldPotential >= Threshold)// && sumFieldPotential <= 0.37f)
        {
            // Calculate normal as a weighted average of sphere normals from contributing metaballs.
            float3 normal = float3(0, 0, 0);

            for (UINT j = 0; j < N; j++)
            {
                normal += fieldPotentials[j] * CalculateNormalForARaySphereHit(ray, t, blobs[j].center);
            }
           // normal = CalculateMetaballsNormal(position, blobs, fieldPotentials, sumFieldPotential);

            normal = normalize(normal / sumFieldPotential);
            if (IsAValidHit(ray, t, normal))
            {
                thit = t;
                attr.normal = normal;
                //attr.normal = CalculateMetaballsNormal(position, blobs, fieldPotentials, sumFieldPotential);
                return true;
            }
        }

        // Inverse lipschitz coefficient for the used field potential equation.
        // The Lipschitz constant of a sum is bounded by the sum of the Lipschitz constants.
        const float inverseLipschitzCoef = (2.0 / 3.0) * sumRi;     

        // Calculate a distance underestimate which gives us an upper 
        // bound distance we can safely step by without intersecting
        // the implicit function.
        float distanceUnderestimate = inverseLipschitzCoef * (Threshold - sumFieldPotential);
        
        // ToDo test how much perf it gives
        // Step by a minimum distance to a blob bounding sphere 
        // or distance underestimate, whichever is greater.
        t += max(minTStep, max(signedMinDistanceToABlob, distanceUnderestimate));
    }

    return false;
}


#endif // VOLUMETRICPRIMITIVESLIBRARY_H