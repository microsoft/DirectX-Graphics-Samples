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

#define N_METABALLS 5

// ToDo cleanup - test quintic vs smooth + lipschitz
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
        float derivativeCoef = 6 * sqrt(distance) / (blob.radius * blob.radius * blob.radius) - 6 / (blob.radius * blob.radius);
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
        // ToDo compare perf
#if 1
        d = blob.radius - d;
        return 6 * (d*d*d*d*d) / pow(blob.radius,5)
            - 15 * (d*d*d*d) / pow(blob.radius,4)
            + 10 * (d*d*d) / pow(blob.radius,3);
#else
        float dR = distance * blob.invRadius;
        return (2 * dR - 3) * (dR * dR) + 1;
#endif
    }
    return 0;
}

float3 CalculateMetaballNormalQuintic(in float3 position, in Metaball blob)
{
    float3 distanceVector = position - blob.center;
    float d = length(distanceVector);
    
    if (d <= blob.radius)
    {
        d = blob.radius - d;
        float derivativeCoef = 30 * (d*d*d*d) / pow(blob.radius,5) 
                             - 60 * (d*d*d) / (blob.radius, 4)
                             + 30 * (d*d) / pow (blob.radius,3);
        return derivativeCoef * distanceVector;
    }
    else
        return float3(0, 0, 0);
}


float3 CalculateMetaballsNormal(in float3 position, in Metaball blobs[N_METABALLS], in float fieldPotentials[N_METABALLS], in float fieldPotential)
{
    float dummy;
    float2 e = float2(1.0, -1.0) * 0.5773 * 0.0001;
    float3 normal = float3(0, 0, 0);

    for (UINT i = 0; i < N_METABALLS; i++)
    {
        normal += fieldPotentials[i]* CalculateMetaballNormal(position, blobs[i]);
    }
    return normalize(normal/fieldPotential);
}

float CalculateMetaballsPotential(in float3 position, in Metaball blobs[N_METABALLS])
{
    float sumFieldPotential = 0;
    for (UINT j = 0; j < N_METABALLS; j++)
    {
        float dummy;
        sumFieldPotential += CalculateMetaballPotentialQuintic(position, blobs[j], dummy);
    }
    return sumFieldPotential;
}

float3 CalculateMetaballGradient(in float3 position, in Metaball blobs[N_METABALLS])
{
    float e = 0.5773 * 0.0001;
    return float3(
        CalculateMetaballsPotential(position + float3(-e, 0, 0), blobs) -
        CalculateMetaballsPotential(position + float3(e, 0, 0), blobs),
        CalculateMetaballsPotential(position + float3(0, -e, 0), blobs) -
        CalculateMetaballsPotential(position + float3(0, e, 0), blobs),
        CalculateMetaballsPotential(position + float3(0, 0, -e), blobs) -
        CalculateMetaballsPotential(position + float3(0, 0, e), blobs));
}


void InitializeMetaballs(out Metaball blobs[N_METABALLS], in float elapsedTime, in float cycleDuration)
{
    // ToDo Pass in from the app?
    // Metaball centers at t0 and t1 key frames.
#if 1
    float3 keyFrameCenters[N_METABALLS][2] =
    {
        { float3(-0.7, 0, 0),float3(0.7,0, 0) },
        { float3(0.7 , 0, 0), float3(-0.7, 0, 0) },
        { float3(0, -0.7, 0),float3(0, 0.7, 0) },
        { float3(0, 0.7, 0), float3(0, -0.7, 0) },
        { float3(0, 0, 0),   float3(0, 0, 0) }
    };
#else
    float3 keyFrameCenters[N_METABALLS][2] =
    {
        { float3(-0.5, -0.3, -0.4),float3(0.5,-0.3,-0.0) },
        { float3(0.0, -0.4, 0.5), float3(0.0, 0.4, 0.5) },
        { float3(0.5,0.5, 0.4), float3(-0.5, 0.2, -0.4) }
    };
#endif
    // Metaball field radii of max influence
    // ToDo Compare perf with precomputed invRadius
    float radii[N_METABALLS] = { 0.35, 0.35, 0.35, 0.35, 0.25 };    

    // Calculate animated metaball center positions.
    float  tAnimate = CalculateAnimationInterpolant(elapsedTime, cycleDuration);
    for (UINT j = 0; j < N_METABALLS; j++)
    {
        blobs[j].center = lerp(keyFrameCenters[j][0], keyFrameCenters[j][1], tAnimate);
        blobs[j].radius = radii[j];
    }

}
// Test if a ray with RayFlags and segment <RayTMin(), RayTCurrent()> intersect a metaball field.
// The test sphere traces through the metaball field until it hits a threshold isosurface. 
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies
bool RayMetaballsIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr, in float elapsedTime)
{
    // Set ray march bounds to the min and max of all metaball intersections.
    float tmin, tmax;
    tmin = 10000; //INFINITY;
    tmax = -10000;// INFINITY;

    Metaball blobs[N_METABALLS];
    InitializeMetaballs(blobs, elapsedTime, 12.0f);

#if 0
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
    for (UINT i = 0; i < N_METABALLS; i++)
    {
        float _thit, _tmax;
        if (RaySolidSphereIntersectionTest(ray, _thit, _tmax, blobs[i].center, blobs[i].radius))
        {
            tmin = min(_thit, tmin);
            tmax = max(_tmax, tmax);
        }
    }
#endif
    tmin = max(tmin, RayTMin());
    tmax = min(tmax, RayTCurrent());

    UINT MAX_STEPS = 128;
#if 0
        thit = RayTCurrent();
        bool hitFound = false;

        // test against all spheres
        for (int i = 0; i < N_METABALLS; i++)
        {
            float _thit;
            float _tmax;
            ProceduralPrimitiveAttributes _attr;
            if (RaySphereIntersectionTest(ray, _thit, _tmax, _attr, blobs[i].center, blobs[i].radius))
            {
                if (_thit < thit)
                {
                    thit = _thit;
                    attr = _attr;
                    hitFound = true;
                }
            }
        }
        return hitFound;
#endif

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
        for (UINT j = 0; j < N_METABALLS; j++)
        {
            float distance;
            fieldPotentials[j] = CalculateMetaballPotentialQuintic(position, blobs[j], distance);
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
#if 0
            // Calculate normal as a weighted average of sphere normals from contributing metaballs.
            float3 normal = float3(0, 0, 0);
            for (UINT j = 0; j < N_METABALLS; j++)
            {
                //if (fieldPotentials[j] > 0.001)
                normal += fieldPotentials[j] * CalculateNormalForARaySphereHit(ray, t, blobs[j].center);
            }
            normal = normalize(normal / sumFieldPotential);
#else
            float3 normal = normalize(CalculateMetaballGradient(position, blobs));
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
        //t += minTStep;
        t += max(minTStep, max(signedMinDistanceToABlob, distanceUnderestimate));
    }

    return false;
}


#endif // VOLUMETRICPRIMITIVESLIBRARY_H