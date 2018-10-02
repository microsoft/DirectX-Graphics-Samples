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
#include "Validate.hlsli"

RaytracingAccelerationStructure AS : register(t0);

[shader("raygeneration")]
void raygen()
{
    EmptyPayload payload = { 0 };
    RayDesc ray = { 
        float3(1, 0, 0),
        0.0f,
        float3(1, 0, 0),
        0.0f };
    TraceRay(AS, 0, ~0, 0, 1, 0, ray, payload);
}