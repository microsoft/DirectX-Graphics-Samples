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

#ifndef SIGNEDDISTANCEFRACTALS_H
#define SIGNEDDISTANCEFRACTALS_H


#include "RaytracingShaderHelper.h"
#include "SignedDistancePrimitives.h"

enum SD_FRACTALS
{
    Tetrahedron = 0,
};

//------------------------------------------------------------------

// Returns a signed distance to a recursive tetrahedron fractal.
// h = { sin a, cos a, height of a pyramid}
// a = pyramid's inner angle between its side plane and a ground plane.
// Pyramid position - sitting on a ground plane 
// Pyramid span: {<-a,0,-a>, <a,h.z,a>}, where a = width of base = h.z * h.y / h.x
// Ref: http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-iii-folding-space/
float sdFractalPyramid(in float3 position, float3 h, in UINT Iterations, in float Scale = 2.0f)
{
    // Set pyramid vertices to AABB's extremities.
    float a = h.z * h.y / h.x;
    float3 v1 = float3(0, h.z, 0);
    float3 v2 = float3(-a, 0, a);
    float3 v3 = float3(a, 0, -a);
    float3 v4 = float3(a, 0, a);
    float3 v5 = float3(-a, 0, -a);

    float3 v;
    int n = 0;
    float dist, d;
    [unroll]
    for (n = 0; n < 4; n++)
    //while (n < Iterations) 
    {
        // ToDo use squared dist
        // Find the closest vertex.
        v = v1; dist = length(position - v1); 
        d = length(position - v2); if (d < dist) { v = v2; dist = d; }
        d = length(position - v3); if (d < dist) { v = v3; dist = d; }
        d = length(position - v4); if (d < dist) { v = v4; dist = d; }
        d = length(position - v5); if (d < dist) { v = v5; dist = d; }

        // Find a relative position in the next fractal interation.
        position = Scale * position - v * (Scale - 1.0);

    }
    float distance = sdPyramid(position, h);
    return distance * pow(Scale, float(-n));
}
float sdFractalTetrahedron2(in float3 position, in UINT Iterations, in float Scale = 2.0f)
{
    // Set tetrahedron vertices close to the AABB's <-1,1> extremities.
#if 0
    float3 v1 = float3(-0.999, -0.999, -0.999);
    float3 v2 = float3(0.999, -0.999, -0.999);
    float3 v3 = float3(0, -0.999, 0.999);
    float3 v4 = float3(0, 0.999, 0);
#else
    float3 v1 = float3(1, 1, 1);
    float3 v2 = float3(-1, -1, 1);
    float3 v3 = float3(1, -1, -1);
    float3 v4 = float3(-1, 1, -1);
#endif
    float3 v;
    int n = 0;
    float dist2, d2;
    while (n < Iterations)
    {
        // ToDo use squared dist
        // Find the closest vertex.
        v = v1; d2 = length_toPow2(position - v1);
        d2 = length_toPow2(position - v2); if (d2 < dist2) { v = v2; dist2 = d2; }
        d2 = length_toPow2(position - v3); if (d2 < dist2) { v = v3; dist2 = d2; }
        d2 = length_toPow2(position - v4); if (d2 < dist2) { v = v4; dist2 = d2; }

        // Find a relative position in the next fractal interation.
        position = Scale * position - v * (Scale - 1.0);
        n++;
    }
#if 0
    //position = position * 2 - 1;
    float distance = opS(sdPyramid4(position, float3(0.8, 0.6, 0.70)));// ,
     //   sdSphere(position, 0.85));
    return distance * pow(Scale, float(-n));
#else
    return length(position) * pow(Scale, float(-n));
#endif
}


float sdFractalPyramid(in float3 position, in UINT Iterations, in float Scale = 2.0f)
{
    // Set tetrahedron vertices close to the AABB's <-1,1> extremities.
#if 1
    float3 v1 = float3(-1, -1, -1);
    float3 v2 = float3(1, -1, -1);
    float3 v3 = float3(-1, -1, 1);
    float3 v4 = float3(1, -1, 1);
    float3 v5 = float3(0, 1, 0);
#else
    float3 v1 = float3(1, 1, 1);
    float3 v2 = float3(-1, -1, 1);
    float3 v3 = float3(1, -1, -1);
    float3 v4 = float3(-1, 1, -1);
#endif
    float3 v;
    int n = 0;
    float dist, d;
    [unroll]
    for (n = 0; n < 4; n++)
    //while (n < Iterations) 
    {
        // ToDo use squared dist
        // Find the closest vertex.
        v = v1; dist = length(position - v1); 
        d = length(position - v2); if (d < dist) { v = v2; dist = d; }
        d = length(position - v3); if (d < dist) { v = v3; dist = d; }
        d = length(position - v4); if (d < dist) { v = v4; dist = d; }
        d = length(position - v5); if (d < dist) { v = v5; dist = d; }

        // Find a relative position in the next fractal interation.
        position = Scale * position - v * (Scale - 1.0);

    }
    float distance = sdPyramid(position, h);
    return distance * pow(Scale, float(-n));
}

#endif // SIGNEDDISTANCEFRACTALS_H