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
#include "HLSLRaytracingInternalPrototypes.h"

#define HLSL

#include "UberShaderBindings.h"
#include "DebugLog.h"
#include "RayTracingHelper.hlsli"
#include "EmulatedPointerIntrinsics.hlsli"
#include "TraverseFunction.hlsli"

SHADER_internal
export void Fallback_TraceRay(
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
    LogTraceRayStart();
    uint oldPayloadOffset = Fallback_TraceRayBegin(rayFlags, float3(originX, originY, originZ), tMin, float3(directionX, directionY, directionZ), tMax, payloadOffset);
    
    bool hit = Traverse(
        instanceInclusionMask,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex
    );

    uint stateID;
    if (hit)
    {
      if (RayFlags() & RAY_FLAG_SKIP_CLOSEST_HIT_SHADER)
      {
        stateID = 0;
      }
      else
      {
        stateID = HitGroupShaderTable.Load(Fallback_ShaderRecordOffset());
      }
    }
    else
    {
      int missShaderRecordOffset = missShaderIndex * MissShaderRecordStride;
      Fallback_SetShaderRecordOffset(missShaderRecordOffset);
      stateID = MissShaderTable.Load(missShaderRecordOffset);
    }

    if (stateID != 0)
    {
        Fallback_CallIndirect(stateID);
    }

    Fallback_TraceRayEnd(oldPayloadOffset);
    LogTraceRayEnd();
}
