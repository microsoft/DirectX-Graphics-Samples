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
#include "HLSLRayTracingInternalPrototypes.h"

#define HLSL

#include "UberShaderBindings.h"
#include "RayTracingHelper.hlsli"
#include "EmulatedPointerIntrinsics.hlsli"

Declare_Fallback_SetPendingAttr(BuiltInTriangleIntersectionAttributes);

SHADER_internal
void Fallback_TraceRay(
  uint rayFlags,
  uint instanceInclusionMask,
  uint rayContributionToHitGroupIndex,
  uint multiplierForGeometryContributionToHitGroupIndex,
  uint missShaderIndex,
  float originX, 
  float originY,
  float originZ,
  float tMin,
  float directionX,
  float directionY,
  float directionZ,
  float tMax,
  uint payloadOffset)
{
    uint oldPayloadOffset = Fallback_TraceRayBegin(rayFlags, float3(originX,originY,originZ), tMin, float3(directionX,directionY,directionZ), tMax, payloadOffset);

    BuiltInTriangleIntersectionAttributes attr;
    attr.barycentrics = float2(0.0, 0.0);
    Fallback_SetPendingAttr(attr);
    Fallback_SetPendingTriVals(0, 0, 0, 0, 0.5, HIT_KIND_TRIANGLE_FRONT_FACE);
    Fallback_CommitHit();

    uint stateID = HitGroupShaderTable.Load(0);
    if (stateID != 0)
    {
        Fallback_CallIndirect(stateID);
    }

    Fallback_TraceRayEnd(oldPayloadOffset);
}
