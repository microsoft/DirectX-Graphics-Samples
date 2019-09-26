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

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "RandomNumberGenerator.hlsli"
#include "Ray Sorting/RaySorting.hlsli"
#include "RTAO.hlsli"

RaytracingAccelerationStructure g_scene : register(t0);
Texture2D<float4> g_texRayOriginPosition : register(t7);
Texture2D<NormalDepthTexFormat> g_texRayOriginSurfaceNormalDepth : register(t8);
Texture2D<NormalDepthTexFormat> g_texAORaysDirectionOriginDepth : register(t22);
Texture2D<uint2> g_texAOSortedToSourceRayIndexOffset : register(t23);
Texture2D<float4> g_texAOSurfaceAlbedo : register(t24);

RWTexture2D<float> g_outAOAmbientCoefficient : register(u10);
RWTexture2D<float> g_outAORayHitDistance : register(u15);

ConstantBuffer<RTAOConstantBuffer> cb : register(b0);
StructuredBuffer<AlignedHemisphereSample3D> g_sampleSets : register(t4);

// Delay the include so that resource references resolve.
#include "RayGen.hlsli"

//***************************************************************************
//*********************------ TraceRay wrappers. -------*********************
//***************************************************************************

// Trace an AO ray and return true if it hits any geometry.
bool TraceAORayAndReportIfHit(out float tHit, in Ray ray, in float TMax, in float3 surfaceNormal)
{
    RayDesc rayDesc;

    // Nudge the origin along the surface normal a bit to avoid 
    // starting from behind the surface
    // due to float calculations imprecision.
    rayDesc.Origin = ray.origin + 0.001 * surfaceNormal;
    rayDesc.Direction = ray.direction;

    // Set the ray's extents.
    rayDesc.TMin = 0.0;
	rayDesc.TMax = TMax;

    // Initialize shadow ray payload.
    // Set the initial value to a hit at TMax. 
    // This way closest and any hit shaders can be skipped if true tHit is not needed. 
    ShadowRayPayload shadowPayload = { TMax };

    UINT rayFlags =
        // Ignore transparent surfaces for occlusion testing.
        RAY_FLAG_CULL_NON_OPAQUE;        

    TraceRay(g_scene,
        rayFlags,
        RTAOTraceRayParameters::InstanceMask,
        RTAOTraceRayParameters::HitGroup::Offset[RTAORayType::AO],
        RTAOTraceRayParameters::HitGroup::GeometryStride,
        RTAOTraceRayParameters::MissShader::Offset[RTAORayType::AO],
        rayDesc, shadowPayload);
    
    tHit = shadowPayload.tHit;

    // Report a hit if Miss Shader didn't set the value to HitDistanceOnMiss.
    return RTAO::HasAORayHitAnyGeometry(tHit);
}

// Traces a given AO ray. 
// Returns its tHit and a calculated ambient coefficient.
float CalculateAO(out float tHit, in uint2 srcPixelIndex, in Ray AOray, in float3 surfaceNormal)
{
    float ambientCoef = 1;
    const float tMax = cb.maxAORayHitTime; 
    if (TraceAORayAndReportIfHit(tHit, AOray, tMax, surfaceNormal))
    {
        float occlusionCoef = 1;
        if (cb.applyExponentialFalloff)
        {
            float theoreticalTMax = cb.maxTheoreticalAORayHitTime;
            float t = tHit / theoreticalTMax;
            float lambda = cb.exponentialFalloffDecayConstant;
            occlusionCoef = exp(-lambda * t * t);
        }
        ambientCoef = 1 - (1 - cb.minimumAmbientIllumination) * occlusionCoef;

        // Approximate interreflections of light from blocking surfaces which are generally not completely dark and tend to have similar radiance.
        // Ref: Ch 11.3.3 Accounting for Interreflections, Real-Time Rendering (4th edition).
        // The approximation assumes:
        //      o All surfaces' incoming and outgoing radiance is the same 
        //      o Current surface color is the same as that of the occluders
        // Since this sample uses scalar ambient coefficient, it usse the scalar luminance of the surface color.
        // This will generally brighten the AO making it closer to the result of full Global Illumination, including interreflections.
        if (cb.approximateInterreflections)
        {
            float3 surfaceAlbedo = g_texAOSurfaceAlbedo[srcPixelIndex].xyz;

            float kA = ambientCoef;
            float rho = cb.diffuseReflectanceScale * RGBtoLuminance(surfaceAlbedo);

            ambientCoef = kA / (1 - rho * (1 - kA));
        }
    }

    return ambientCoef;
}

//***************************************************************************
//********************------ Ray gen shader.. -------************************
//***************************************************************************

[shader("raygeneration")]
void RayGenShader()
{
    uint2 srcRayIndex = DispatchRaysIndex().xy;

    float3 surfaceNormal;
    float depth;
    DecodeNormalDepth(g_texRayOriginSurfaceNormalDepth[srcRayIndex], surfaceNormal, depth);
	bool isValidHit = depth != HitDistanceOnMiss;
    float tHit = RTAO::RayHitDistanceOnMiss;
    float ambientCoef = RTAO::InvalidAOCoefficientValue;
	if (isValidHit)
	{
        float3 hitPosition = g_texRayOriginPosition[srcRayIndex].xyz;
        ambientCoef = 0;
        for (uint r = 0; r < cb.spp; r++)
        {
            float3 rayDirection = GetRandomRayDirection(srcRayIndex, surfaceNormal, cb.raytracingDim, r);
            Ray AORay = { hitPosition, rayDirection };
            ambientCoef += CalculateAO(tHit, srcRayIndex, AORay, surfaceNormal);
        }
        ambientCoef /= cb.spp;
    }

    g_outAOAmbientCoefficient[srcRayIndex] = ambientCoef;
    g_outAORayHitDistance[srcRayIndex] = RTAO::HasAORayHitAnyGeometry(tHit) ? tHit : cb.maxTheoreticalAORayHitTime;
}

// Retrieves 2D source and sorted ray indices from a 1D ray index where
// - every valid (i.e. is within ray tracing buffer dimensions) 1D index maps to a valid 2D index.
// - pixels are row major within a ray group.
// - ray groups are row major within the raytracing buffer dimensions.
// - rays are sorted per ray group.
// Overflowing ray group dimensions on the borders are clipped to valid raytracing dimnesions.
// Returns whether the retrieved ray is active.
bool Get2DRayIndices(out uint2 sortedRayIndex2D, out uint2 srcRayIndex2D, in uint index1D)
{
    uint2 rayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);

    // Find the ray group row index.
    uint numValidPixelsInRow = cb.raytracingDim.x;
    uint rowOfRayGroupSize = rayGroupDim.y * numValidPixelsInRow;
    uint rayGroupRowIndex = index1D / rowOfRayGroupSize;

    // Find the ray group column index.
    uint numValidPixelsInColumn = cb.raytracingDim.y;
    uint numRowsInCurrentRayGroup = min((rayGroupRowIndex + 1) * rayGroupDim.y, numValidPixelsInColumn) - rayGroupRowIndex * rayGroupDim.y;
    uint currentRow_RayGroupSize = numRowsInCurrentRayGroup * rayGroupDim.x;
    uint index1DWithinRayGroupRow = index1D - rayGroupRowIndex * rowOfRayGroupSize;
    uint rayGroupColumnIndex = index1DWithinRayGroupRow / currentRow_RayGroupSize;
    uint2 rayGroupIndex = uint2(rayGroupColumnIndex, rayGroupRowIndex);

    // Find the thread offset index within the ray group.
    uint currentRayGroup_index1D = index1DWithinRayGroupRow - rayGroupIndex.x * currentRow_RayGroupSize;
    uint currentRayGroupWidth = min((rayGroupIndex.x + 1) * rayGroupDim.x, numValidPixelsInRow) - rayGroupIndex.x * rayGroupDim.x;
    uint rayThreadRowIndex = currentRayGroup_index1D / currentRayGroupWidth;
    uint rayThreadColumnIndex = currentRayGroup_index1D - rayThreadRowIndex * currentRayGroupWidth;
    uint2 rayThreadIndex = uint2(rayThreadColumnIndex, rayThreadRowIndex);

    // Get the corresponding source index
    sortedRayIndex2D = rayGroupIndex * rayGroupDim + rayThreadIndex;
    uint2 rayGroupBase = rayGroupIndex * rayGroupDim;
    uint2 rayGroupRayIndexOffset = g_texAOSortedToSourceRayIndexOffset[sortedRayIndex2D];
    srcRayIndex2D = rayGroupBase + GetRawRayIndexOffset(rayGroupRayIndexOffset);

    return IsActiveRay(rayGroupRayIndexOffset);
}

[shader("raygeneration")]
void RayGenShader_sortedRays()
{
    uint DTid_1D = DispatchRaysIndex().x; 
    uint2 srcRayIndex;
    uint2 sortedRayIndex;
    bool isActiveRay = Get2DRayIndices(sortedRayIndex, srcRayIndex, DTid_1D);

    uint2 srcRayIndexFullRes = srcRayIndex;
    if (cb.doCheckerboardSampling)
    {
        UINT pixelStepX = 2;
        bool isEvenPixelY = (srcRayIndex.y & 1) == 0;
        UINT pixelOffsetX = isEvenPixelY != cb.areEvenPixelsActive;
        srcRayIndexFullRes.x = srcRayIndex.x * pixelStepX + pixelOffsetX;
    }

    float tHit = RTAO::RayHitDistanceOnMiss;
    float ambientCoef = RTAO::InvalidAOCoefficientValue;
    if (isActiveRay)
    {
        float dummy;
        float3 rayDirection;
        DecodeNormalDepth(g_texAORaysDirectionOriginDepth[srcRayIndex], rayDirection, dummy);
        float3 hitPosition = g_texRayOriginPosition[srcRayIndexFullRes].xyz;

        float3 surfaceNormal;
        float depth;
        DecodeNormalDepth(g_texRayOriginSurfaceNormalDepth[srcRayIndexFullRes], surfaceNormal, depth);

        Ray AORay = { hitPosition, rayDirection };
        ambientCoef = CalculateAO(tHit, srcRayIndexFullRes, AORay, surfaceNormal);
    }

    uint2 outPixel = srcRayIndexFullRes;

    g_outAOAmbientCoefficient[outPixel] = ambientCoef;
    g_outAORayHitDistance[outPixel] = RTAO::HasAORayHitAnyGeometry(tHit) ? tHit : cb.maxTheoreticalAORayHitTime;
}

//***************************************************************************
//******************------ Closest hit shaders -------***********************
//***************************************************************************

[shader("closesthit")]
void ClosestHitShader(inout ShadowRayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    rayPayload.tHit = RayTCurrent();
}

//***************************************************************************
//**********************------ Miss shaders -------**************************
//***************************************************************************

[shader("miss")]
void MissShader(inout ShadowRayPayload rayPayload)
{
    rayPayload.tHit = RTAO::RayHitDistanceOnMiss;
}



#endif // RAYTRACING_HLSL