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
RaytracingAccelerationStructure MyAccelerationStructure : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);

cbuffer Viewport : register(b0)
{
    float2 TopLeft;
    float2 BottomRight;
    int2 Dim;
    uint TestRayFlags;
    uint InstanceInclusionMask;
}

struct MyPayload
{
    int val;
};

[shader("raygeneration")]
void RayGen()
{
    float2 lerpValues = (DispatchRaysIndex().xy + 0.5) / DispatchRaysDimensions().xy;

    float3 rayDir = float3(0.0, 0.0, 1);
    float3 origin = float3(
        lerp(TopLeft.x, BottomRight.x, lerpValues.x),
        lerp(TopLeft.y, BottomRight.y, lerpValues.y),
        0.0f);

    RayDesc myRay = { origin,
        0.0f,
        rayDir,
        10000.0f };
    MyPayload payload = { 0 };
    TraceRay(MyAccelerationStructure, TestRayFlags, InstanceInclusionMask, 0, 1, 0, myRay, payload);
}

[shader("miss")]
void Miss(inout MyPayload payload)
{
    RenderTarget[DispatchRaysIndex().xy] = float4(1, 0, 0, 1);
}

[shader("closesthit")]
void Hit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    RenderTarget[DispatchRaysIndex().xy] = float4(1, 0, 1, 1);
}
