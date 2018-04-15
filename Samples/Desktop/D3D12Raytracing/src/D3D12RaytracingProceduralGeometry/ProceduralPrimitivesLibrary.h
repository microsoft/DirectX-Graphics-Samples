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

#include "RaytracingShaderHelper.h"

// ToDo revise inout specifiers

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

// ToDo - take ray flags into consideration
// if ((RayFlags() & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (dot(localRay.direction, attr.normal) < 0))
// 

// Test if a hit is valid based on specified RayFlags
bool IsAValidHit(RAY_FLAG rayFlags, in Ray ray, in float3 surfaceNormal)
{
    float dirNormalDot = dot(ray.direction, surfaceNormal);
    return (
        !(rayFlags & (RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_CULL_FRONT_FACING_TRIANGLES)) ||
        ((rayFlags & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (dirNormalDot < 0)) ||
        ((rayFlags & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) && (dirNormalDot > 0)));
}

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool RaySphereIntersectionTest(Ray ray, out float thit, inout ProceduralPrimitiveAttributes attr, float3 center = float3(0, 0, 0), float radius = 1)
{
    float t0, t1; // solutions for t if the ray intersects 
    float radius2 = pow(radius, 2);

    // analytic solution
    float3 L = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, L);
    float c = dot(L, L) - radius2;
    if (!SolveQuadraticEqn(a, b, c, t0, t1)) return false;

    if (t0 > t1) swap(t0, t1);

    if (t0 < 0)
    {
        // if t0 is negative, let's use t1 instead 
        if (t1 < 0) return false; // both t0 and t1 are negative 

        float3 hitPosition = ray.origin + t1 * ray.direction;
        // Transform by a row-major object to world matrix
        // ToDo do not use semantics directnly in the tests
        attr.normal = mul(normalize(hitPosition - center), ObjectToWorld()).xyz;

        if (IsAValidHit(RayFlags(), ray, attr.normal))
        {
            thit = t0;
            return true;
        }
    }
    else
    {
        float3 hitPosition = ray.origin + t0 * ray.direction;
        // Transform by a row-major object to world matrix
        // ToDo do not use semantics directnly in the tests
        attr.normal = mul(normalize(hitPosition - center), ObjectToWorld()).xyz;

        if (IsAValidHit(RayFlags(), ray, attr.normal))
        {
            thit = t0;
            return true;
        }

        hitPosition = ray.origin + t1 * ray.direction;
        // Transform by a row-major object to world matrix
        // ToDo do not use semantics directnly in the tests
        attr.normal = mul(normalize(hitPosition - center), ObjectToWorld()).xyz;

        if (IsAValidHit(RayFlags(), ray, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    return false;
}

bool RaySpheresIntersectionTest(Ray ray, out float thit, in float tmin, in float tmax, inout ProceduralPrimitiveAttributes attr)
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
#if 0
    return RaySphereIntersectionTest(ray.origin, ray.direction, thit, attr, centers[2], radii[2]);
#elif 1
    // Workaround for dynamic indexing issue in DXR shaders
    float _thit;
    thit = tmax;
    ProceduralPrimitiveAttributes _attr;
    if (RaySphereIntersectionTest(ray, _thit, _attr, centers[0], radii[0]))
    {
        if (IsInRange(_thit, tmin, thit))
        {
            thit = _thit;
            attr = _attr;
            hitFound = true;
        }
    }
    if (RaySphereIntersectionTest(ray, _thit, _attr, centers[1], radii[1]))
    {
        if (IsInRange(_thit, tmin, thit))
        {
            thit = _thit;
            attr = _attr;
            hitFound = true;
        }
    }
    if (RaySphereIntersectionTest(ray, _thit, _attr, centers[2], radii[2]))
    {
        if (IsInRange(_thit, tmin, thit))
        {
            thit = _thit;
            attr = _attr;
            hitFound = true;
        }
    }
    
    return hitFound;
#else
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
bool RayAABBIntersectionTest(Ray ray, out float thit, inout ProceduralPrimitiveAttributes attr)
{
    float3 bounds[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
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
    thit = tmin;

    // Calculate cube face normal
    float3 center = float3(0, 0, 0);
    float3 hitPosition = ray.origin + thit * ray.direction;
    float3 sphereNormal = normalize(hitPosition - center);
    // take the largest dimension and normalize it
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
    // Transform by a row-major object to world matrix
    attr.normal = mul(normalize(attr.normal), ObjectToWorld()).xyz;

    return tmax > tmin;
}