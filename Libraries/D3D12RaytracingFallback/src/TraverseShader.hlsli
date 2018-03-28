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

int Traverse(
    uint instanceInclusionMask,
    uint rayContributionToHitGroupIndex,
    uint multiplierForGeometryContributionToHitGroupIndex,
    uint missShaderIndex
)
{
    LogTraceRayStart();
    bool hit = Traverse(
        instanceInclusionMask,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex
    );
    LogTraceRayEnd();


    // TODO: Need to be loading from table using the ContributionToHitGroupIndex
    uint stateID;
    if (hit)
    {
        if (RayFlags() & RAY_FLAG_SKIP_CLOSEST_HIT_SHADER)
        {
            stateID = 0;
        }
        else
        {
            stateID = HitGroupShaderTable.Load(Fallback_GeometryIndex() * HitGroupShaderRecordStride);
        }
    }
    else
    {
        stateID = MissShaderTable.Load(missShaderIndex * MissShaderRecordStride);
    }

    return stateID;
}

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
    uint oldPayloadOffset = Fallback_SetPayloadOffset(payloadOffset);
    Fallback_SetRayFlags(rayFlags);
    Fallback_TraceRayBegin(float3(originX, originY, originZ), tMin, float3(directionX, directionY, directionZ), tMax);
    int stateId = Traverse(
        instanceInclusionMask,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex,
        missShaderIndex
    );
    if (stateId != 0)
    {
        Fallback_CallIndirect(stateId);
    }
    Fallback_SetPayloadOffset(oldPayloadOffset);
}
