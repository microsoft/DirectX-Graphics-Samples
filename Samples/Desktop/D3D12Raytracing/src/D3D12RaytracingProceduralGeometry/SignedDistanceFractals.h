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
// ToDo cleanup
#ifndef SIGNEDDISTANCEFRACTALS_H
#define SIGNEDDISTANCEFRACTALS_H

#include "RaytracingShaderHelper.h"
#include "SignedDistancePrimitives.h"

//------------------------------------------------------------------

// Returns a signed distance to a recursive pyramid fractal.
// h = { sin a, cos a, height of a pyramid}.
// a = pyramid's inner angle between its side plane and a ground plane.
// Pyramid position - sitting on a ground plane.
// Pyramid span: {<-a,0,-a>, <a,h.z,a>}, where a = width of base = h.z * h.y / h.x.
// Ref: http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-iii-folding-space/
float sdFractalPyramid(in float3 position, float3 h, in float Scale = 2.0f)
{
    // Set pyramid vertices to AABB's extremities.
    float a = h.z * h.y / h.x;
    float3 v1 = float3(0, h.z, 0);
    float3 v2 = float3(-a, 0, a);
    float3 v3 = float3(a, 0, -a);
    float3 v4 = float3(a, 0, a);
    float3 v5 = float3(-a, 0, -a);

    int n = 0;
#if USE_EXPLICIT_UNROLL
    [unroll]
#endif
    for (n = 0; n < N_FRACTAL_ITERATIONS; n++)
    {
        // Find the closest vertex.
        float dist, d;
        float3 v;
        v = v1; dist = length_toPow2(position - v1); 
        d = length_toPow2(position - v2); if (d < dist) { v = v2; dist = d; }
        d = length_toPow2(position - v3); if (d < dist) { v = v3; dist = d; }
        d = length_toPow2(position - v4); if (d < dist) { v = v4; dist = d; }
        d = length_toPow2(position - v5); if (d < dist) { v = v5; dist = d; }

        // Find a relative position in the next fractal interation.
        position = Scale * position - v * (Scale - 1.0);
    }
    float distance = sdPyramid(position, h);
    return distance * pow(Scale, float(-n));
}
#endif // SIGNEDDISTANCEFRACTALS_H