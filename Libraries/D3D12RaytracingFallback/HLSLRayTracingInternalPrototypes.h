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
#ifndef HLSL_RAYTRACING_INTERNAL_PROTOTYPES
#define HLSL_RAYTRACING_INTERNAL_PROTOTYPES

#define INSTANCE_FLAG_NONE                              0x0
#define INSTANCE_FLAG_TRIANGLE_CULL_DISABLE             0x1
#define INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE   0x2
#define INSTANCE_FLAG_FORCE_OPAQUE                      0x4
#define INSTANCE_FLAG_FORCE_NON_OPAQUE                  0x8

// Declare Fallback_SetPendingAttr overload for given attribute structure
#define Declare_Fallback_SetPendingAttr(attr_t) \
    void Fallback_SetPendingAttr(attr_t);

uint Fallback_GroupIndex();
uint Fallback_GeometryIndex();

void Fallback_SetWorldRayOrigin(float3 val);
void Fallback_SetWorldRayDirection(float3 val);
void Fallback_SetRayTMin(float val);
void Fallback_SetRayTCurrent(float val);
void Fallback_SetRayFlags(uint rayFlags);
void Fallback_SetPrimitiveIndex(uint val);
void Fallback_SetInstanceIndex(uint val);
void Fallback_SetInstanceID(uint val);
void Fallback_SetObjectRayOrigin(float3 val);
void Fallback_SetObjectRayDirection(float3 val);
void Fallback_SetObjectToWorld(row_major float3x4 val);
void Fallback_SetWorldToObject(row_major float3x4 val);
void Fallback_SetHitKind(uint val);
void Fallback_SetPendingRayTCurrent(float t);
void Fallback_SetPendingHitKind(uint hitKind);
void Fallback_SetPendingTriVals(float t, uint primitiveIndex, uint geometryIndex, uint instanceIndex, uint instanceID, uint hitKind);
void Fallback_SetPendingCustomVals(uint primitiveIndex, uint geometryIndex, uint instanceIndex, uint instanceID);
uint Fallback_SetPayloadOffset(uint payloadOffset);

void Fallback_TraceRayBegin(float3 origin, float tmin, float3 dir, float tmax);
int  Fallback_AnyHitResult();
void Fallback_SetAnyHitResult(int result);
int  Fallback_AnyHitStateId();
void Fallback_SetAnyHitStateId(int stateId);
void Fallback_CommitHit();
void Fallback_CallIndirect(int stateId);
void Fallback_Scheduler(int initialStateId, uint dimx, uint dimy);

#define SHADER_internal [experimental("shader", "internal")]

SHADER_internal
void Fallback_TraceRay(RayDesc rd);

#endif // HLSL_RAYTRACING_INTERNAL_PROTOTYPES
