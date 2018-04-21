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

enum SD_FRACTALS
{
    Tetrahedron = 0,
};

//------------------------------------------------------------------

// Returns a signed distance to a recursive tetrahedron fractal.
// Ref: http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-iii-folding-space/
float sdFractalTetrahedron(in float3 position, in UINT Iterations, in float Scale = 2.0f)
{
    // Set tetrahedron vertices close to the AABB's <-1,1> extremities.
    float3 v1 = float3(-0.999, -0.999, -0.999);
    float3 v2 = float3(0.999, -0.999, -0.999);
    float3 v3 = float3(0, -0.999, 0.999);
    float3 v4 = float3(0, 0.999, 0);
    float3 v;
    UINT n = 0;
    float dist, d;
    while (n < Iterations) 
    {
        // Find the closest vertex.
        v = v1; dist = length(position - v1);
        d = length(position - v2); if (d < dist) { v = v2; dist = d; }
        d = length(position - v3); if (d < dist) { v = v3; dist = d; }
        d = length(position - v4); if (d < dist) { v = v4; dist = d; }
        
        // Find a relative position in the next fractal interation.
        position = Scale * position - v * (Scale - 1.0);
        n++;
    }
    return length(position) * pow(Scale, float(-n));
}

#endif // SIGNEDDISTANCEFRACTALS_H