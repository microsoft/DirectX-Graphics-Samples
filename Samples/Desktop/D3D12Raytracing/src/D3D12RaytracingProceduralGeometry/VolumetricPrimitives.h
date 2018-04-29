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


// ToDo cleanup - test quintic vs smooth + lipschitz
struct Metaball
{
    float3 center;
    float  radius;
    float  invRadius;
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
        // f(0) = 1, f(radius) = 0
        float d = distance;
        float r = blob.radius;
        return 2*(d*d*d)/(r*r*r) - 3*(d*d)/(r*r) + 1;
    }
    return 0;
}

float3 CalculateMetaballGradient(in float3 position, in Metaball blob)
{
    float3 distanceVector = position - blob.center;
    float distance = length(distanceVector);

    if (distance <= blob.radius)
    {
        float r = blob.radius;
        float derivativeCoef = 6 * sqrt(distance) / (r*r*r) - 6 / (r*r);
        return derivativeCoef * distanceVector;
    }
    else
        return float3(0, 0, 0);
}

float CalculateMetaballPotentialQuintic(in float3 position, in Metaball blob, out float distance)
{
    distance = length(position - blob.center);
    
    if (distance <= blob.radius)
    {
        float d = distance;
        // f(0) = 0, f(radius) = 1 => invert it.
        d = blob.radius - d;
        float r = blob.radius;
        return 6 * (d*d*d*d*d) / (r*r*r*r*r)
            - 15 * (d*d*d*d) / (r*r*r*r)
            + 10 * (d*d*d) / (r*r*r);
    }
    return 0;
}

// Calculate a gradient of a quintic equation.
float3 CalculateMetaballGradientQuintic(in float3 position, in Metaball blob)
{
    float3 distanceVector = position - blob.center;
    float d = length(distanceVector);
    
    if (d <= blob.radius)
    {
        d = blob.radius - d;
        float r = blob.radius;
        float derivativeCoef = 30 * (d*d*d*d) / (r*r*r*r*r) 
                             - 60 * (d*d*d) / (r*r*r*r)
                             + 30 * (d*d) / (r*r*r);
        return derivativeCoef * distanceVector;
    }
    else
        return float3(0, 0, 0);
}

float3 CalculateMetaballsNormal(in float3 position, in Metaball blobs[N_METABALLS], in UINT nActiveMetaballs, in float fieldPotentials[N_METABALLS])
{
    float dummy;
    float3 normal = float3(0, 0, 0);
#if USE_DYNAMIC_LOOPS
    for (UINT i = 0; i < nActiveMetaballs; i++)
#else
    for (UINT i = 0; i < N_METABALLS; i++)
#endif
    {
#if METABALL_QUINTIC_EQN
        normal += fieldPotentials[i] * CalculateMetaballGradientQuintic(position, blobs[i]);
#else
        normal += fieldPotentials[i]* CalculateMetaballGradient(position, blobs[i]);
#endif
    }
    return normalize(normal);
}

float CalculateMetaballsPotential(in float3 position, in Metaball blobs[N_METABALLS], in UINT nActiveMetaballs)
{
    float sumFieldPotential = 0;
#if USE_DYNAMIC_LOOPS 
    for (UINT j = 0; j < nActiveMetaballs; j++)
#else
    for (UINT j = 0; j < N_METABALLS; j++)
#endif
    {
        float dummy;
#if METABALL_QUINTIC_EQN
        sumFieldPotential += CalculateMetaballPotentialQuintic(position, blobs[j], dummy);
#else
        sumFieldPotential += CalculateMetaballPotential(position, blobs[j], dummy);
#endif
    }
    return sumFieldPotential;
}

float3 CalculateMetaballsNormalSampled(in float3 position, in Metaball blobs[N_METABALLS], in UINT nActiveMetaballs)
{
    float e = 0.5773 * 0.00001;
    return normalize(float3(
        CalculateMetaballsPotential(position + float3(-e, 0, 0), blobs, nActiveMetaballs) -
        CalculateMetaballsPotential(position + float3(e, 0, 0), blobs, nActiveMetaballs),
        CalculateMetaballsPotential(position + float3(0, -e, 0), blobs, nActiveMetaballs) -
        CalculateMetaballsPotential(position + float3(0, e, 0), blobs, nActiveMetaballs),
        CalculateMetaballsPotential(position + float3(0, 0, -e), blobs, nActiveMetaballs) -
        CalculateMetaballsPotential(position + float3(0, 0, e), blobs, nActiveMetaballs)));
}


void InitializeMetaballs(out Metaball blobs[N_METABALLS], in float elapsedTime, in float cycleDuration)
{
    // ToDo Compare perf with precomputed invRadius
    // ToDo Pass in from the app?
    // Metaball centers at t0 and t1 key frames.
#if N_METABALLS == 5
    float3 keyFrameCenters[N_METABALLS][2] =
    {
        { float3(-0.7, 0, 0),float3(0.7,0, 0) },
        { float3(0.7 , 0, 0), float3(-0.7, 0, 0) },
        { float3(0, -0.7, 0),float3(0, 0.7, 0) },
        { float3(0, 0.7, 0), float3(0, -0.7, 0) },
        { float3(0, 0, 0),   float3(0, 0, 0) }
    };
    // Metaball field radii of max influence
    float radii[N_METABALLS] = { 0.35, 0.35, 0.35, 0.35, 0.25 };
#elif 1
    float3 keyFrameCenters[N_METABALLS][2] =
    {
        { float3(0, -0.7, 0),float3(0, 0.7, 0) },
        { float3(0, 0.7, 0), float3(0, -0.7, 0) },
        { float3(0, 0, 0),   float3(0, 0, 0) }
    };
    // Metaball field radii of max influence
    float radii[N_METABALLS] = { 0.45, 0.45, 0.3 };
#else
    float3 keyFrameCenters[N_METABALLS][2] =
    {
        { float3(-0.5, -0.3, -0.4),float3(0.5,-0.3,-0.0) },
        { float3(0.0, -0.4, 0.5), float3(0.0, 0.4, 0.5) },
        { float3(0.5,0.5, 0.4), float3(-0.5, 0.2, -0.4) }
    };
    // Metaball field radii of max influence
    float radii[N_METABALLS] = { 0.45, 0.55, 0.45 };
#endif

    // Calculate animated metaball center positions.
    float  tAnimate = CalculateAnimationInterpolant(elapsedTime, cycleDuration);
    for (UINT j = 0; j < N_METABALLS; j++)
    {
        blobs[j].center = lerp(keyFrameCenters[j][0], keyFrameCenters[j][1], tAnimate);
        blobs[j].radius = radii[j];
        blobs[j].invRadius = 1 / radii[j];
    }
}

// Find all metaballs that ray intersects.
// The passed in array is sorted to first nActiveMetaballs.Me
void FindIntersectingMetaballs(in Ray ray, out float tmin, out float tmax, inout Metaball blobs[N_METABALLS], out UINT nActiveMetaballs)
{
    // Find the entry and exit points for all metaball bounding spheres combined.
    tmin = INFINITY;
    tmax = -INFINITY;
#if 0
    float3 aabb[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };

    if (!RayAABBIntersectionTest(ray, aabb, tmin, tmax))
    {
        return false;
    }
    nActiveMetaballs = N_METABALLS;
#else
    nActiveMetaballs = 0;
    for (UINT i = 0; i < N_METABALLS; i++)
    {
        float _thit, _tmax;
        if (RaySolidSphereIntersectionTest(ray, _thit, _tmax, blobs[i].center, blobs[i].radius))
        {
            tmin = min(_thit, tmin);
            tmax = max(_tmax, tmax);
#if LIMIT_TO_ACTIVE_METABALLS
            blobs[nActiveMetaballs++] = blobs[i];
#else
            nActiveMetaballs = N_METABALLS;
#endif
        }
    }
#endif
    tmin = max(tmin, RayTMin());
    tmax = min(tmax, RayTCurrent());
}

// Test if a ray with RayFlags and segment <RayTMin(), RayTCurrent()> intersect a metaball field.
// The test sphere traces through the metaball field until it hits a threshold isosurface. 
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies
bool RayMetaballsIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr, in float elapsedTime)
{
    Metaball blobs[N_METABALLS];
    InitializeMetaballs(blobs, elapsedTime, 12.0f);
    
    float tmin, tmax;   // Ray extents to first and last metaball intersections.
    UINT nActiveMetaballs = 0;  // Number of metaballs's that the ray intersects.
    FindIntersectingMetaballs(ray, tmin, tmax, blobs, nActiveMetaballs);


    UINT MAX_STEPS = 128;
    float t = tmin;
    float minTStep = (tmax - tmin) / (MAX_STEPS / 1);
    UINT iStep = 0;
    //for (UINT iStep = 0; iStep < MAX_STEPS; iStep++)
    while (iStep++ < MAX_STEPS && t < RayTCurrent())
    {
        float3 position = ray.origin + t * ray.direction;
        float fieldPotentials[N_METABALLS];              // Field potentials for each metaball.
        float sumFieldPotential = 0;           // Sum of all metaball field potentials.
        float signedMinDistanceToABlob = 1000;//INFINITY   // Signed minimum distance to blobs' bounding spheres.
        
        // Lipschitz coefficient
        // Gives an upper bound for distance underestimate of our implicit function (DUF).
        // Ref: Hart96 - Sphere tracing: a geometric method for the antialiased ray tracing of implicit surfaces.
        // For our field potential, it equals to 3 / (2*Radius) for each Metaball, 
        // and combined as a product for all metaballs combined.
        float inverseLipschitzCoef = 1;
      
        // Calculate field potentials from all metaballs.
#if USE_DYNAMIC_LOOPS
        for (UINT j = 0; j < nActiveMetaballs; j++)
#else
        for (UINT j = 0; j < N_METABALLS; j++)
#endif
        {
            float distance;
#if METABALL_QUINTIC_EQN
            fieldPotentials[j] = CalculateMetaballPotentialQuintic(position, blobs[j], distance);
#else
            fieldPotentials[j] = CalculateMetaballPotential(position, blobs[j], distance);
#endif
            sumFieldPotential += fieldPotentials[j];
            signedMinDistanceToABlob = min(signedMinDistanceToABlob, distance - blobs[j].radius);
            inverseLipschitzCoef *= 2 / 3 * blobs[j].radius;
        }

        // Have we crossed the implicit surface.
        // ToDo pass threshold from app
        // Threshold - valid range is (0, 0.1>, the larger the threshold the smaller the blob.
        const float Threshold = 0.25f;
        if (sumFieldPotential >= Threshold)// && sumFieldPotential <= 0.37f)
        {
#if NORMAL_AS_SAMPLED_GRADIENT
            float3 normal = CalculateMetaballsNormalSampled(position, blobs, nActiveMetaballs);
#else
            float3 normal = CalculateMetaballsNormal(position, blobs, nActiveMetaballs, fieldPotentials);
#endif
            if (IsAValidHit(ray, t, normal))
            {
                thit = t;
                attr.normal = normal;
                return true;
            }
        }

        // Calculate a distance underestimate which gives us an upper 
        // bound distance we can safely step by without intersecting
        // the implicit function.
        float distanceUnderestimate = inverseLipschitzCoef * (Threshold - sumFieldPotential);
        
        // ToDo test how much perf it gives
        // Step by a minimum distance to a blob bounding sphere 
        // or distance underestimate, whichever is greater.
        //t += (tmax - tmin) / (MAX_STEPS - 1); //
        t += minTStep;
        //t += max(minTStep, max(signedMinDistanceToABlob, distanceUnderestimate));
    }

    return false;
}


#endif // VOLUMETRICPRIMITIVESLIBRARY_H