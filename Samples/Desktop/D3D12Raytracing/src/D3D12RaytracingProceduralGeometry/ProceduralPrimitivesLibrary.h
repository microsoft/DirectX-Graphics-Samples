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

#ifndef RAYTRACINGPRIMITIVESLIBRARY_H
#define RAYTRACINGPRIMITIVESLIBRARY_H

#include "RaytracingShaderHelper.h"
#include "SignedDistancePrimitives.h"
#include "SignedDistanceFractals.h"


// ToDo revise inout specifiers
// ToDo pass raytracing intrinsics as parameters?

struct ProceduralPrimitiveAttributes
{
    float3 normal;
};

void swap(inout float a, inout float b)
{
    float temp = a;
    a = b;
    b = temp;
}

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool SolveQuadraticEqn(float a, float b, float c, out float x0, out float x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = -0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) swap(x0, x1);

    return true;
}


// Test if a hit is valid based on specified RayFlags
// ToDo explain why check for thit being in range here instead of depend on ReportHit
// ToDo separate tests that are final using IsAValid hit and intermediate ones
bool IsAValidHit(in Ray ray, in float thit, in float3 hitSurfaceNormal)
{
    float rayDirectionNormalDot = dot(ray.direction, hitSurfaceNormal);
    return
        // Is thit within <tmin, tmax> range.
        IsInRange(thit, RayTMin(), RayTCurrent())
        &&
        // Is the hit not culled.
        (   !(RayFlags() & (RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_CULL_FRONT_FACING_TRIANGLES)) 
            || ((RayFlags() & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (rayDirectionNormalDot < 0))
            || ((RayFlags() & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) && (rayDirectionNormalDot > 0)));
}

// Calculates a normal for a hit point on a sphere 
float3 CalculateNormalForARaySphereHit(in Ray ray, in float thit, float3 center)
{
    float3 hitPosition = ray.origin + thit * ray.direction;
    // Get the normal in world space
    return normalize(hitPosition - center);
}

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool RaySphereIntersectionTest(in Ray ray, out float thit, in ProceduralPrimitiveAttributes attr, in float3 center = float3(0, 0, 0), in float radius = 1)
{
    float t0, t1; // solutions for t if the ray intersects 
    float radius2 = pow(radius, 2);

    // Analytic solution
    float3 L = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, L);
    float c = dot(L, L) - radius2;
    if (!SolveQuadraticEqn(a, b, c, t0, t1)) return false;

    if (t0 > t1) swap(t0, t1);

    if (t0 < RayTMin())
    {
        // t0 is before tmin, let's use t1 instead 
        if (t1 < RayTMin()) return false; // both t0 and t1 are before tmin

        attr.normal = CalculateNormalForARaySphereHit(ray, t1, center);
        if (IsAValidHit(ray, t1, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    else
    {
        attr.normal = CalculateNormalForARaySphereHit(ray, t0, center);
        if (IsAValidHit(ray, t0, attr.normal))
        {
            thit = t0;
            return true;
        }

        attr.normal = CalculateNormalForARaySphereHit(ray, t1, center);
        if (IsAValidHit(ray, t1, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    return false;
}

bool RaySpheresIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr)
{
    const int N = 3;
    float3 centers[N] =
    {
        float3(-0.3, -0.3, -0.3),
        float3(0.0, 0.0, 0.4),
        float3(0.3,0.3, 0.0)
    };
    float  radii[N] = { 0.6, 0.3, 0.15 };
    bool hitFound = false;
#if DO_NOT_USE_DYNAMIC_INDEXING
    float _thit;
    ProceduralPrimitiveAttributes _attr;

    //
    // Test for intersection against all spheres and take the closest hit.
    //
    thit = RayTCurrent();
    if (RaySphereIntersectionTest(ray, _thit, _attr, centers[0], radii[0]))
    {
        if (_thit < thit)
        {
            thit = _thit;
            attr = _attr;
            hitFound = true;
        }
    }
    if (RaySphereIntersectionTest(ray, _thit, _attr, centers[1], radii[1]))
    {
        if (_thit < thit)
        {
            thit = _thit;
            attr = _attr;
            hitFound = true;
        }
    }
    if (RaySphereIntersectionTest(ray, _thit, _attr, centers[2], radii[2]))
    {
        if (_thit < thit)
        {
            thit = _thit;
            attr = _attr;
            hitFound = true;
        }
    }
    
    return hitFound;
#else
    ToDo
    // test against all spheres
    //[unroll]
    for (int i = 0; i < N; i++)
    {
        float _thit;
        ProceduralPrimitiveAttributes _attr;
        if (intersectSphere(ray.origin, ray.direction, _thit, _attr, centers[i], radii[i]))
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
}

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool RayAABBIntersectionTest(Ray ray, out float tmin, out float tmax)
{
    float3 bounds[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };
    float tymin, tymax, tzmin, tzmax;
    tmin = (bounds[0].x - ray.origin.x) / ray.direction.x;
    tmax = (bounds[1].x - ray.origin.x) / ray.direction.x;
    if (ray.direction.x < 0) swap(tmin, tmax);
    tymin = (bounds[0].y - ray.origin.y) / ray.direction.y;
    tymax = (bounds[1].y - ray.origin.y) / ray.direction.y;
    if (ray.direction.y < 0) swap(tymin, tymax);
    tzmin = (bounds[0].z - ray.origin.z) / ray.direction.z;
    tzmax = (bounds[1].z - ray.origin.z) / ray.direction.z;
    if (ray.direction.z < 0) swap(tzmin, tzmax);
    tmin = max(max(tmin, tymin), tzmin);
    tmax = min(min(tmax, tymax), tzmax);

    return tmax > tmin;
}

// ToDo add IsAValidHit to all intersection shaders
bool RayAABBIntersectionTest(Ray ray, out float thit, out ProceduralPrimitiveAttributes attr)
{
    float tmin, tmax;
    if (RayAABBIntersectionTest(ray, tmin, tmax))
    {
        thit = tmin;
        // Calculate cube face normal
        float3 center = float3(0, 0, 0);
        float3 hitPosition = ray.origin + thit * ray.direction;
        float3 sphereNormal = normalize(hitPosition - center);
        // take the largest dimension and normalize it
        // ToDo why
        if (abs(sphereNormal.x) > abs(sphereNormal.y))
        {
            if (abs(sphereNormal.x) > abs(sphereNormal.z))
            {
                attr.normal = float3(sphereNormal.x, 0, 0);
            }
            else
            {
                attr.normal = float3(0, 0, sphereNormal.z);
            }
        }
        else
        {
            if (abs(sphereNormal.y) > abs(sphereNormal.z))
            {
                attr.normal = float3(0, sphereNormal.y, 0);
            }
            else
            {
                attr.normal = float3(0, 0, sphereNormal.z);
            }
        }

        // Get the normal in world space
        attr.normal = mul((float3x3)ObjectToWorld(), normalize(attr.normal));

        return true;
    }
    return false;
}


#define METABALL_POTENTIAL_SAP 1
#if METABALL_POTENTIAL_SAP

// Calculate a magnitude of an influence from a Metaball charge.
// Ref: http://www.geisswerks.com/ryan/BLOBS/blobs.html
// mbRadius - largest possible area of metaball contribution - AKA its bounding sphere.
// invMbRadiusSquared ~ 1/mbRadius^2. 
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies
float CalculateMetaballPotential(in float3 position, in float3 mbCenter, in float mbRadius, in float invMbRadiusSquared)
{
    float d = length(position - mbCenter);
    if (d <= mbRadius) 
    {
        // This can be factored for speed if you want.
        return   2 * (d * d * d) / (mbRadius* mbRadius * mbRadius) 
               - 3 * (d * d) / (mbRadius * mbRadius) 
               + 1;
    }
    return 0;
}


// Ref: http://www.geisswerks.com/ryan/BLOBS/blobs.html
bool RayMetaballsIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr)
{
    const int N = 3;
    float3 centers[N] =
    {
        float3(-0.3, -0.3, -0.3),
        float3(0.0, 0.0, 0.5),
        float3(0.3,0.5, 0.0)
    };
    float fieldPotentials[N];
    // Metaball field radii of max influence
    float radii[N] = { 1.2, 1, 1.1 };
    float  invRadiiSq[N] = { 1 / (radii[0] * radii[0]), 1 / (radii[1] * radii[1]), 1 / (radii[2] * radii[2]) };

    // Calculate step size based on the ray AABB intersection segment
    UINT MAX_STEPS = 1024;
    float tmin, tmax;

    if (!RayAABBIntersectionTest(ray, tmin, tmax))
    {
        return false;
    }
    //if (RaySphereIntersectionTest(ray, thit, attr, centers[0], 0.6))
    //{
    //    if (thit <= RayTCurrent())
    //    {
    //        return true;
    //    }
    //}

    tmin = max(tmin, RayTMin());
    tmax = min(tmax, RayTCurrent());
    float tstep = (tmax - tmin) / (MAX_STEPS - 1);

    // Step along the ray calculating field potentials from all metaballs.
    for (UINT i = 0; i < MAX_STEPS; i++)
    {
        float t = tmin + i * tstep;
        float3 position = ray.origin + t * ray.direction;
        fieldPotentials[0] = CalculateMetaballPotential(position, centers[0], radii[0], invRadiiSq[0]);
        fieldPotentials[1] = CalculateMetaballPotential(position, centers[1], radii[0], invRadiiSq[1]);
        fieldPotentials[2] = CalculateMetaballPotential(position, centers[2], radii[0], invRadiiSq[2]);


        float fieldPotential = fieldPotentials[0] + fieldPotentials[1] + fieldPotentials[2];
        // Threshold - valid range is (0, 0.25>, the larger the threshold the smaller the blob.
        float threshold = 0.15f;
        if (fieldPotential >= threshold)
        {
            // Calculate normal as a weighted average of sphere normals from contributing metaballs.
            float3 normal = float3(0, 0, 0);
            normal += fieldPotentials[0] * CalculateNormalForARaySphereHit(ray, t, centers[0]);
            normal += fieldPotentials[1] * CalculateNormalForARaySphereHit(ray, t, centers[1]);
            normal += fieldPotentials[2] * CalculateNormalForARaySphereHit(ray, t, centers[2]);
            attr.normal = normalize(normal / fieldPotential);
            thit = t;
            return true;
        }
    }

    return false;
}

#else
// Calculate a magnitude of an influence from a Metaball charge.
// Ref: http://www.geisswerks.com/ryan/BLOBS/blobs.html
// invMbRadiusSquared ~ 1/mbRadius^2. 
// mbRadius - largest possible area of metaball contribution - AKA its bounding sphere.

float CalculateMetaballPotential(in float3 position, in float3 mbCenter, in float invMbRadiusSquared)
{
    float3 distanceVector = position - mbCenter;

    // Squared distance.
    float d2 = dot(distanceVector, distanceVector);

    // Enable this line if your blobs are of varying sizes.
    d2 *= invMbRadiusSquared;

    // f(d) is valid when d2 is in the range [0-.5], or [0-.707] for d
    if (d2 < 0.5f)   
    {
        // Calculate field po
        return 0.25 - d2 + d2*d2;
    }
    return 0;
}

// Ref: http://www.geisswerks.com/ryan/BLOBS/blobs.html
bool RayMetaballsIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr)
{
    const int N = 3;
    float3 centers[N] =
    {
        float3(-0.3, -0.3, -0.3),
        float3(0.0, 0.0, 0.5),
        float3(0.3,0.5, 0.0)
    };
    float fieldPotentials[N];
    // Metaball field radii of max influence
    float radii[N] = { 1.2, 1, 1.1 };
    float  invRadiiSq[N] = { 1/(radii[0]* radii[0]), 1/(radii[1]*radii[1]), 1/(radii[2]*radii[2])};

    // Calculate step size based on the ray AABB intersection segment
    UINT MAX_STEPS = 1024;
    float tmin, tmax;

    if (!RayAABBIntersectionTest(ray, tmin, tmax))
    {
        return false;
    }
    //if (RaySphereIntersectionTest(ray, thit, attr, centers[0], 0.6))
    //{
    //    if (thit <= RayTCurrent())
    //    {
    //        return true;
    //    }
    //}
 
    tmin = max(tmin, RayTMin());
    tmax = min(tmax, RayTCurrent());
    float tstep = (tmax - tmin) / (MAX_STEPS - 1);
    
    // Step along the ray calculating field potentials from all metaballs.
    for (UINT i = 0; i < MAX_STEPS; i++)
    {
        float t = tmin + i * tstep;
        float3 position = ray.origin + t * ray.direction;
        fieldPotentials[0] = CalculateMetaballPotential(position, centers[0], invRadiiSq[0]);
        fieldPotentials[1] = CalculateMetaballPotential(position, centers[1], invRadiiSq[1]);
        fieldPotentials[2] = CalculateMetaballPotential(position, centers[2], invRadiiSq[2]);


        float fieldPotential = fieldPotentials[0] + fieldPotentials[1] + fieldPotentials[2];
        // Threshold - valid range is (0, 0.25>, the larger the threshold the smaller the blob.
        float threshold = 0.15f;
        if (fieldPotential >= threshold)
        {
            // Calculate normal as a weighted average of sphere normals from contributing metaballs.
            float3 normal = float3(0, 0, 0);
            normal += fieldPotentials[0] * CalculateNormalForARaySphereHit(ray, t, centers[0]);
            normal += fieldPotentials[1] * CalculateNormalForARaySphereHit(ray, t, centers[1]);
            normal += fieldPotentials[2] * CalculateNormalForARaySphereHit(ray, t, centers[2]);
            attr.normal = normalize(normal / fieldPotential);
            thit = t;
            return true;
        }
    }

    return false;
}
#endif


float GetDistanceFromSignedDistancePrimitive(in float3 position, in SDPrimitive sdPrimitive)
{
    switch (sdPrimitive)
    {
    case Cone: return sdCone(position, float3(0.8, 0.6, 0.3));
    case Torus: return sdTorus(position, float2(0.20, 0.05));
    case Pyramid: return sdPyramid4(position, float3(0.8, 0.6, 0.25));
    case FractalTetrahedron: return sdFractalTetrahedron(position, 5);
    }
    return 0;
}

// Test ray against a signed distance primitive.
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer
bool RaySignedDistancePrimitiveTest(in Ray ray, in SDPrimitive sdPrimitive, out float thit, out ProceduralPrimitiveAttributes attr)
{
    float tmin, tmax;
    RayAABBIntersectionTest(ray, tmin, tmax);
    tmin = max(tmin, RayTMin());
    tmax = min(tmax, RayTCurrent());
    
    const float threshold = 10e-6;
    float t = tmin;

    // Do sphere tracing through the AABB.
    while (t < tmax) 
    {
        float3 position = ray.origin + t * ray.direction;
        float distance = GetDistanceFromSignedDistancePrimitive(position, sdPrimitive);
  
        // Did we intersect the shape?
        if (distance <= threshold * t)
        {
            thit = t;
            attr.normal = float3(0, 1, 0);
            return true;
        }

        // Since distance is the minimum distance to the primitive, 
        // we can safely jump by that without intersecting the primitive.
        t += distance;
    }
    return false;
}


#endif // RAYTRACINGPRIMITIVESLIBRARY_H