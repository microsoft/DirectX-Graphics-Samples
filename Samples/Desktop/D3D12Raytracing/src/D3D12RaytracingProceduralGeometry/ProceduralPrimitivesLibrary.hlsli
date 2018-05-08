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

//**********************************************************************************************
//
// ProceduralPrimitivesLibrary.hlsli
//
// An interface to call per geometry intersection tests based on as primitive type.
//
//**********************************************************************************************

#ifndef PROCEDURALPRIMITIVESLIBRARY_H
#define PROCEDURALPRIMITIVESLIBRARY_H

#include "RaytracingShaderHelper.hlsli"

#include "AnalyticPrimitives.hlsli"
#include "VolumetricPrimitives.hlsli"
#include "SignedDistancePrimitives.hlsli"
#include "SignedDistanceFractals.hlsli"

// Analytic geometry intersection test.
// AABB local space dimensions: <-1,1>.
bool RayAnalyticGeometryIntersectionTest(in Ray ray, in AnalyticPrimitive::Enum analyticPrimitive, out float thit, out ProceduralPrimitiveAttributes attr)
{
    float3 aabb[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };
    float tmax;

    switch (analyticPrimitive)
    {
    case AnalyticPrimitive::AABB: return RayAABBIntersectionTest(ray, aabb, thit, attr);
    case AnalyticPrimitive::Spheres: return RaySpheresIntersectionTest(ray, thit, attr);
    default: return false;
    }
}

// Analytic geometry intersection test.
// AABB local space dimensions: <-1,1>.
bool RayVolumetricGeometryIntersectionTest(in Ray ray, in VolumetricPrimitive::Enum volumetricPrimitive, out float thit, out ProceduralPrimitiveAttributes attr, in float elapsedTime)
{
    switch (volumetricPrimitive)
    {
    case VolumetricPrimitive::Metaballs: return RayMetaballsIntersectionTest(ray, thit, attr, elapsedTime);
    default: return false;
    }
}

// Signed distance functions use a shared ray signed distance test.
// The test, instead, calls into this function to retrieve a distance for a primitive.
// AABB local space dimensions: <-1,1>.
// Ref: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float GetDistanceFromSignedDistancePrimitive(in float3 position, in SignedDistancePrimitive::Enum signedDistancePrimitive)
{
    switch (signedDistancePrimitive)
    {
    case SignedDistancePrimitive::MiniSpheres:
        return opI(sdSphere(opRep(position + 1, (float3) 2/4), 0.65 / 4), sdBox(position, (float3)1));

    case SignedDistancePrimitive::IntersectedRoundCube:
        return opS(opS(udRoundBox(position, (float3) 0.75, 0.2), sdSphere(position, 1.20)), -sdSphere(position, 1.32));

    case SignedDistancePrimitive::SquareTorus: 
        return sdTorus82(position, float2(0.75, 0.15));
    
    case SignedDistancePrimitive::TwistedTorus: 
        return sdTorus(opTwist(position), float2(0.6, 0.2));
        
    case SignedDistancePrimitive::Cog:
        return opS( sdTorus82(position, float2(0.60, 0.3)),
                    sdCylinder(opRep(float3(atan2(position.z, position.x) / 6.2831, 
                                            1, 
                                            0.015 + 0.25 * length(position)) + 1,
                                     float3(0.05, 1, 0.075)),
                               float2(0.02, 0.8)));
    
    case SignedDistancePrimitive::Cylinder: 
        return opI(sdCylinder(opRep(position + float3(1, 1, 1), float3(1, 2, 1)), float2(0.3, 2)),
                   sdBox(position + float3(1, 1, 1), float3(2, 2, 2)));
    
    case SignedDistancePrimitive::FractalPyramid: 
         // Let pyramid have a base at y == -1 of AABB => position + float3(0,1,0) 
         // Pyramid: 63.435 degrees at base, height 2
         return sdFractalPyramid(position + float3(0, 1, 0), float3(0.894, 0.447, 2.0), 2.0f);
    
    default: return 0;
    }
}

#endif // PROCEDURALPRIMITIVESLIBRARY_H