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

#include "RaytracingHlslCompat.h"
using namespace dx; // dx::HitObject and dx::MaybeReorderThread

//*********************************************************
// Configuration options
//*********************************************************

// TraceRay the old fashioned way
//#define USE_ORIGINAL_TRACERAY_NO_SER

// Call MaybeReorderThread(sortKey,1), sortKey is 1 bit 
// indicating if the thread has dummy work
#define REQUEST_REORDER

// Don't invoke ClosestHit or Miss shaders, use hitObject 
// properties in RayGen to shade
// #define SKIP_INVOKE_INSTEAD_SHADE_IN_RAYGEN

// Rays do loop a of artificial work in the 
// Closest Hit shader.  This setting makes 
// some rays looping more than others (a sort candidate):
#define USE_VARYING_ARTIFICIAL_WORK

// Number of iterations in the heavy artificial work loop
#define WORK_LOOP_ITERATIONS_HEAVY 5000

// Number of iterations in the light artificial work loop
#define WORK_LOOP_ITERATIONS_LIGHT 1000

// N, where 1/N is the proportion of rays that do the 
// heavy artificial work load
#define RAYS_WITH_HEAVY_WORK_FRACTION 4

// Put all the rays with dummy work on the left side
// #define SPATIALLY_SORTED

//*********************************************************

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

struct [raypayload] RayPayload
{
    float4 color : write(caller, closesthit, miss) : read(caller);
    uint iterations : write(caller) : read(closesthit);
};

float4 ClosestHitWorker(MyAttributes attr, uint iterations)
{
    float3 barycentrics = float3(
        1 - attr.barycentrics.x - attr.barycentrics.y, 
        attr.barycentrics.x, 
        attr.barycentrics.y);

    #ifdef USE_VARYING_ARTIFICIAL_WORK
        for(uint i = 0; i < iterations; i++)
        {
            if(i%2) barycentrics += 1.175494e-38; // FLT_MIN
            else barycentrics -= 1.175494e-38; // FLT_MIN
        }
        if(iterations != WORK_LOOP_ITERATIONS_LIGHT) barycentrics += 1; // make artificial work pixels white
    #endif

    return float4(barycentrics, 1);
}

float4 MissWorker()
{
    return float4(0, 0, 0, 1);
}

RayDesc SetupRay(uint3 index, uint3 dimensions)
{
    float2 lerpValues = (float2)index / (float2)dimensions;

    // Orthographic projection since we're raytracing in screen space.
    float3 rayDir = float3(0, 0, 1);
    float3 origin = float3(
        lerp(g_rayGenCB.viewport.left, g_rayGenCB.viewport.right, lerpValues.x),
        lerp(g_rayGenCB.viewport.top, g_rayGenCB.viewport.bottom, lerpValues.y),
        0.0f);

    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    return ray;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    RayDesc ray = 
        SetupRay(DispatchRaysIndex(), DispatchRaysDimensions()); 

    uint iterations = WORK_LOOP_ITERATIONS_LIGHT;

    #ifdef USE_VARYING_ARTIFICIAL_WORK

        #ifdef SPATIALLY_SORTED
            // Extra work is all on left side of screen
            if((ray.Origin.x + 1)/2.f <= 1.f/RAYS_WITH_HEAVY_WORK_FRACTION)
            {
                iterations = WORK_LOOP_ITERATIONS_HEAVY; 
            }
        #else
            // Extra work distributed in vertical bands
            if( (DispatchRaysIndex().x) % RAYS_WITH_HEAVY_WORK_FRACTION == 0 )
            {
                iterations = WORK_LOOP_ITERATIONS_HEAVY; 
            }
        #endif

    #endif

    RayPayload payload = { float4(0, 0, 0, 0), iterations };
    float4 color = float4(1,1,1,1);

    #ifdef USE_ORIGINAL_TRACERAY_NO_SER
        TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);
        color = payload.color;
    #else

        // Trace without closest hit or miss
        HitObject hit = 
            HitObject::TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, 
                                ray, payload);

        #ifdef REQUEST_REORDER
            int sortKey = iterations != WORK_LOOP_ITERATIONS_LIGHT ? 1:0;
            dx::MaybeReorderThread(sortKey, 1);

            // There's currently a DXC bug that causes "using namespace dx;" 
            // (see further above) to generate bad DXIL for MaybeReorderThread, 
            // so it's explicitly scoped here. The namespace works fine for 
            // HitObject
        #endif

        #ifdef SKIP_INVOKE_INSTEAD_SHADE_IN_RAYGEN
            if(hit.IsHit())
            {
                MyAttributes attr = hit.GetAttributes<MyAttributes>();
                color = ClosestHitWorker(attr,iterations);
            }
            else
            {
                color = MissWorker();
            }

        #else
            // Run closest hit / miss
            HitObject::Invoke(hit, payload);
            color = payload.color;
        #endif

    #endif

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = color;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = ClosestHitWorker(attr,payload.iterations);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = MissWorker();
}
