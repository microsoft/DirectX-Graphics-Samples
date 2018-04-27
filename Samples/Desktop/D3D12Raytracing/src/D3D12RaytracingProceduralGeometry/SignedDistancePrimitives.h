//*********************************************************************************
//
// This file is based on or incorporates material from the projects listed below 
// (Third Party OSS). The original copyright notice and the license under which 
// Microsoft received such Third Party OSS, are set forth below. Such licenses 
// and notices are provided for informational purposes only. Microsoft licenses 
// the Third Party OSS to you under the licensing terms for the Microsoft product 
// or service. Microsoft reserves all other rights not expressly granted under 
// this agreement, whether by implication, estoppel or otherwise.
//
// MIT License
// Copyright(c) 2013 Inigo Quilez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files(the Software), to deal 
// in the Software without restriction, including without limitation the rights 
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is furnished 
// to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
// IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//*********************************************************************************

#ifndef SIGNEDDISTANCEPRIMITIVES_H
#define SIGNEDDISTANCEPRIMITIVES_H

//**********************************************************************************************
//
// SignedDistanceFieldLibrary.h
// Ref: https://www.shadertoy.com/view/Xds3zN 
// A list of useful distance function to simple primitives, and an example on how to 
// do some interesting boolean operations, repetition and displacement.
//
// More info here: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//
//**********************************************************************************************


#include "RaytracingShaderHelper.h"


//------------------------------------------------------------------
float GetDistanceFromSignedDistancePrimitive(in float3 position, in SignedDistancePrimitive::Enum sdPrimitive);

//------------------------------------------------------------------

// Subtract: Obj1 - Obj2
float opS(float d1, float d2)
{
    return max(d1, -d2);
}

// Union: Obj1 + Obj2
float opU(float d1, float d2)
{
    return min(d1, d2);
}

// Intersection: Obj1 & Obj2
float opI(float d1, float d2)
{
    return max(d1, d2);
}

// Repetitions
float3 opRep(float3 p, float3 c)
{
    return fmod(p, c) - 0.5 * c;
}

//------------------------------------------------------------------

float sdPlane(float3 p)
{
    return p.y;
}

float sdSphere(float3 p, float s)
{
    return length(p) - s;
}

// Box extents: <-b,b>
float sdBox(float3 p, float3 b)
{
    float3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float sdEllipsoid(in float3 p, in float3 r)
{
    return (length(p / r) - 1.0) * min(min(r.x, r.y), r.z);
}

float udRoundBox(float3 p, float3 b, float r)
{
    return length(max(abs(p) - b, 0.0)) - r;
}

// t: {radius, tube radius}
float sdTorus(float3 p, float2 t)
{
    float2 q = float2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float sdHexPrism(float3 p, float2 h)
{
    float3 q = abs(p);
    float d1 = q.z - h.y;
    float d2 = max((q.x * 0.866025 + q.y * 0.5), q.y) - h.x;
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sdCapsule(float3 p, float3 a, float3 b, float r)
{
    float3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sdEquilateralTriangle(in float2 p)
{
    const float k = 1.73205;    //sqrt(3.0);
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0 / k;
    if (p.x + k * p.y > 0.0) p = float2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    p.x += 2.0 - 2.0 * clamp((p.x + 2.0) / 2.0, 0.0, 1.0);
    return -length(p) * sign(p.y);
}

float sdTriPrism(float3 p, float2 h)
{
    float3 q = abs(p);
    float d1 = q.z - h.y;
#if 1
    // distance bound
    float d2 = max(q.x * 0.866025 + p.y * 0.5, -p.y) - h.x * 0.5;
#else
    // correct distance
    h.x *= 0.866025;
    float d2 = sdEquilateralTriangle(p.xy / h.x) * h.x;
#endif
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sdCylinder(float3 p, float2 h)
{
    float2 d = abs(float2(length(p.xz), p.y)) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdCone(in float3 p, in float3 c)
{
    float2 q = float2(length(p.xz), p.y);
    float d1 = -q.y - c.z;
    float d2 = max(dot(q, c.xy), q.y);
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sdConeSection(in float3 p, in float h, in float r1, in float r2)
{
    float d1 = -p.y - h;
    float q = p.y - h;
    float si = 0.5 * (r1 - r2) / h;
    float d2 = max(sqrt(dot(p.xz, p.xz) * (1.0 - si * si)) + q * si - r2, q);
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}


// h = { sin a, cos a, height of a pyramid }
// a = pyramid's inner angle between its side plane and a ground plane.
// Octahedron position - ground plane intersecting in the middle.
float sdOctahedron(float3 p, float3 h)
{
    float d = 0.0;

    // Test: d = p.x * sin a + p.y * cos a
    // d = max(d, dot(abs(p), float3(h.x, h.y, 0)));
    // d = max(d, dot(abs(p), float3(0, h.y, h.x)));
    d = dot(float2(max(abs(p.x), abs(p.z)), abs(p.y)), 
            float2(h.x, h.y));
    
    // Move the ground plane down by height
    return d - h.y * h.z;
}

// h = { sin a, cos a, height of a pyramid}
// a = pyramid's inner angle between its side plane and a ground plane.
// Pyramid position - sitting on a ground plane.
float sdPyramid(float3 p, float3 h) // h = { sin a, cos a, height }
{
    float octa = sdOctahedron(p, h);

    // Subtract bottom half
    return opS(octa, p.y);
}

float length_toPowNegative2(float2 p)
{
    return sqrt(p.x * p.x + p.y * p.y);
}

float length_toPowNegative6(float2 p)
{
    p = p * p * p; 
    p = p * p;
    return pow(p.x + p.y, 1.0 / 6.0);
}

float length_toPowNegative8(float2 p)
{
    p = p * p; p = p * p; p = p * p;
    return pow(p.x + p.y, 1.0 / 8.0);
}

float sdTorus82(float3 p, float2 t)
{
    float2 q = float2(length_toPowNegative2(p.xz) - t.x, p.y);
    return length_toPowNegative8(q) - t.y;
}

float sdTorus88(float3 p, float2 t)
{
    float2 q = float2(length_toPowNegative8(p.xz) - t.x, p.y);
    return length_toPowNegative8(q) - t.y;
}

float sdCylinder6(float3 p, float2 h)
{
    return max(length_toPowNegative6(p.xz) - h.x, abs(p.y) - h.y);
}

float3 opTwist(float3 p)
{
    float c = cos(3.0 * p.y);
    float s = sin(3.0 * p.y);
    float2x2 m = float2x2(c, -s, s, c);
    return float3(mul(m, p.xz), p.y);
}

float3 sdCalculateNormal(in float3 pos, in SignedDistancePrimitive::Enum sdPrimitive)
{
    float2 e = float2(1.0, -1.0) * 0.5773 * 0.0001;
    return normalize(
        e.xyy * GetDistanceFromSignedDistancePrimitive(pos + e.xyy, sdPrimitive) +
        e.yyx * GetDistanceFromSignedDistancePrimitive(pos + e.yyx, sdPrimitive) +
        e.yxy * GetDistanceFromSignedDistancePrimitive(pos + e.yxy, sdPrimitive) +
        e.xxx * GetDistanceFromSignedDistancePrimitive(pos + e.xxx, sdPrimitive));
}

// Test ray against a signed distance primitive.
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer
bool RaySignedDistancePrimitiveTest(in Ray ray, in SignedDistancePrimitive::Enum sdPrimitive, out float thit, out ProceduralPrimitiveAttributes attr, in float stepScale = 1.0f)
{
    const float threshold = 0.0001;
    float t = RayTMin();
    const UINT MaxSteps = 512;

    // Do sphere tracing through the AABB.
#if 0 
    for (UINT i = 0; i < MaxSteps; i++)
    {
        if (t > RayTCurrent())
        {
            return false;
        }
#else
    UINT i = 0;
    while (i++ < MaxSteps && t <= RayTCurrent())
    {
#endif  
        float3 position = ray.origin + t * ray.direction;
        float distance = GetDistanceFromSignedDistancePrimitive(position, sdPrimitive);

        // Has the ray intersected the primitive? 
        if (distance <= threshold * t)
        {
            float3 hitSurfaceNormal = sdCalculateNormal(position, sdPrimitive);
            if (IsAValidHit(ray, t, hitSurfaceNormal))
            {
                thit = t;
                attr.normal = hitSurfaceNormal;
                return true;
            }
        }

        // Since distance is the minimum distance to the primitive, 
        // we can safely jump by that amount without intersecting the primitive.
        // We allow for scaling of steps per primitive type due to their transformations 
        // that don't preserve true distances.
        t += stepScale * distance;
    }
    return false;
}


#endif // SIGNEDDISTANCEPRIMITIVES_H