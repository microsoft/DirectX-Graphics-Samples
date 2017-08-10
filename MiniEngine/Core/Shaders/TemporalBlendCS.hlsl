//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "TemporalRS.hlsli"
#include "ShaderUtility.hlsli"
#include "PixelPacking_Velocity.hlsli"

static const uint kLdsPitch = 18;
static const uint kLdsRows = 10;

RWTexture2D<float4> OutTemporal : register(u0);

Texture2D<packed_velocity_t> VelocityBuffer : register(t0);
Texture2D<float3> InColor : register(t1);
Texture2D<float4> InTemporal : register(t2);
Texture2D<float> CurDepth : register(t3);
Texture2D<float> PreDepth : register(t4);

SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1);

groupshared float ldsDepth[kLdsPitch * kLdsRows];
groupshared float ldsR[kLdsPitch * kLdsRows];
groupshared float ldsG[kLdsPitch * kLdsRows];
groupshared float ldsB[kLdsPitch * kLdsRows];


cbuffer CB1 : register(b1)
{
    float2 RcpBufferDim;	// 1 / width, 1 / height
    float TemporalBlendFactor;
    float RcpSpeedLimiter;
    float2 ViewportJitter;
}

void StoreRGB(uint ldsIdx, float3 RGB)
{
    ldsR[ldsIdx] = RGB.r;
    ldsG[ldsIdx] = RGB.g;
    ldsB[ldsIdx] = RGB.b;
}

float3 LoadRGB(uint ldsIdx)
{
    return float3(ldsR[ldsIdx], ldsG[ldsIdx], ldsB[ldsIdx]);
}

float2 STtoUV(float2 ST)
{
    return (ST + 0.5) * RcpBufferDim;
}

float3 ClipColor(float3 Color, float3 BoxMin, float3 BoxMax, float Dilation = 1.0)
{
    float3 BoxCenter = (BoxMax + BoxMin) * 0.5;
    float3 HalfDim = (BoxMax - BoxMin) * 0.5 * Dilation + 0.001;
    float3 Displacement = Color - BoxCenter;
    float3 Units = abs(Displacement / HalfDim);
    float MaxUnit = max(max(Units.x, Units.y), max(Units.z, 1.0));
    return BoxCenter + Displacement / MaxUnit;
}

void GetBBoxForPair(uint fillIdx, uint holeIdx, out float3 boxMin, out float3 boxMax)
{
    boxMin = boxMax = LoadRGB(fillIdx);
    float3 a = LoadRGB(fillIdx - kLdsPitch - 1);
    float3 b = LoadRGB(fillIdx - kLdsPitch + 1);
    boxMin = min(boxMin, min(a, b));
    boxMax = max(boxMax, max(a, b));
    a = LoadRGB(fillIdx + kLdsPitch - 1);
    b = LoadRGB(fillIdx + kLdsPitch + 1);
    boxMin = min(boxMin, min(a, b));
    boxMax = max(boxMax, max(a, b));
    a = LoadRGB(holeIdx);
    b = LoadRGB(holeIdx - fillIdx + holeIdx);
    boxMin = min(boxMin, min(a, b));
    boxMax = max(boxMax, max(a, b));
}

float MaxOf(float4 Depths) { return max(max(Depths.x, Depths.y), max(Depths.z, Depths.w)); }

int2 GetClosestPixel(uint Idx, out float ClosestDepth)
{
    float DepthO = ldsDepth[Idx];
    float DepthW = ldsDepth[Idx - 1];
    float DepthE = ldsDepth[Idx + 1];
    float DepthN = ldsDepth[Idx - kLdsPitch];
    float DepthS = ldsDepth[Idx + kLdsPitch];

    ClosestDepth = min(DepthO, min(min(DepthW, DepthE), min(DepthN, DepthS)));

    if (DepthN == ClosestDepth)
        return int2(0, -1);
    else if (DepthS == ClosestDepth)
        return int2(0, +1);
    else if (DepthW == ClosestDepth)
        return int2(-1, 0);
    else if (DepthE == ClosestDepth)
        return int2(+1, 0);

    return int2(0, 0);
}

void ApplyTemporalBlend(uint2 ST, uint ldsIdx, float3 BoxMin, float3 BoxMax)
{
    float3 CurrentColor = LoadRGB(ldsIdx);

    float CompareDepth;

    // Get the velocity of the closest pixel in the '+' formation
    float3 Velocity = UnpackVelocity(VelocityBuffer[ST + GetClosestPixel(ldsIdx, CompareDepth)]);

    CompareDepth += Velocity.z;

    // The temporal depth is the actual depth of the pixel found at the same reprojected location.
    float TemporalDepth = MaxOf(PreDepth.Gather(LinearSampler, STtoUV(ST + Velocity.xy + ViewportJitter))) + 1e-3;

    // Fast-moving pixels cause motion blur and probably don't need TAA
    float SpeedFactor = saturate(1.0 - length(Velocity.xy) * RcpSpeedLimiter);

    // Fetch temporal color.  Its "confidence" weight is stored in alpha.
    float4 Temp = InTemporal.SampleLevel(LinearSampler, STtoUV(ST + Velocity.xy), 0);
    float3 TemporalColor = Temp.rgb;
    float TemporalWeight = Temp.w;

    // Pixel colors are pre-multiplied by their weight to enable bilinear filtering.  Divide by weight to recover color.
    TemporalColor /= max(TemporalWeight, 1e-6);

    // Clip the temporal color to the current neighborhood's bounding box.  Increase the size of the bounding box for
    // stationary pixels to avoid rejecting noisy specular highlights.
    TemporalColor = ClipColor(TemporalColor, BoxMin, BoxMax, lerp(1.0, 4.0, SpeedFactor * SpeedFactor));

    // Update the confidence term based on speed and disocclusion
    TemporalWeight *= SpeedFactor * step(CompareDepth, TemporalDepth);

    // Blend previous color with new color based on confidence.  Confidence steadily grows with each iteration
    // until it is broken by movement such as through disocclusion, color changes, or moving beyond the resolution
    // of the velocity buffer.
    TemporalColor = ITM(lerp(TM(CurrentColor), TM(TemporalColor), TemporalWeight));

    // Update weight
    TemporalWeight = saturate(rcp(2.0 - TemporalWeight));

    // Quantize weight to what is representable
    TemporalWeight = f16tof32(f32tof16(TemporalWeight));

    // Breaking this up into two buffers means it can be 40 bits instead of 64.
    OutTemporal[ST] = float4(TemporalColor, 1) * TemporalWeight;
}

[RootSignature(Temporal_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
    const uint ldsHalfPitch = kLdsPitch / 2;

    // Prefetch an 16x8 tile of pixels (8x8 colors) including a 1 pixel border
    // 10x18 IDs with 4 IDs per thread = 45 threads
    for (uint i = GI; i < 45; i += 64)
    {
        uint X = (i % ldsHalfPitch) * 2;
        uint Y = (i / ldsHalfPitch) * 2;
        uint TopLeftIdx = X + Y * kLdsPitch;
        int2 TopLeftST = Gid.xy * uint2(8, 8) - 1 + uint2(X / 2, Y);
        float2 UV = RcpBufferDim * (TopLeftST * float2(2, 1) + float2(2, 1));

        float4 Depths = CurDepth.Gather(LinearSampler, UV);
        ldsDepth[TopLeftIdx + 0] = Depths.w;
        ldsDepth[TopLeftIdx + 1] = Depths.z;
        ldsDepth[TopLeftIdx + kLdsPitch] = Depths.x;
        ldsDepth[TopLeftIdx + 1 + kLdsPitch] = Depths.y;

        float4 R4 = InColor.GatherRed(LinearSampler, UV);
        float4 G4 = InColor.GatherGreen(LinearSampler, UV);
        float4 B4 = InColor.GatherBlue(LinearSampler, UV);
        StoreRGB(TopLeftIdx, float3(R4.w, G4.w, B4.w));
        StoreRGB(TopLeftIdx + 1, float3(R4.z, G4.z, B4.z));
        StoreRGB(TopLeftIdx + kLdsPitch, float3(R4.x, G4.x, B4.x));
        StoreRGB(TopLeftIdx + 1 + kLdsPitch, float3(R4.y, G4.y, B4.y));
    }

    GroupMemoryBarrierWithGroupSync();

    uint Idx0 = GTid.x * 2 + GTid.y * kLdsPitch + kLdsPitch + 1;
    uint Idx1 = Idx0 + 1;

    GroupMemoryBarrierWithGroupSync();

    float3 BoxMin, BoxMax;
    GetBBoxForPair(Idx0, Idx1, BoxMin, BoxMax);

    uint2 ST0 = DTid.xy * uint2(2, 1);
    ApplyTemporalBlend(ST0, Idx0, BoxMin, BoxMax);

    uint2 ST1 = ST0 + uint2(1, 0);
    ApplyTemporalBlend(ST1, Idx1, BoxMin, BoxMax);
}
