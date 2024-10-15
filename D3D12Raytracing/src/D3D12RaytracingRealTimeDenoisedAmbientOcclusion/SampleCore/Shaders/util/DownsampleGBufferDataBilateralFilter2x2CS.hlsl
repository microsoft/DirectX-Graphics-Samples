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

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"

Texture2D<float4> g_inNormalDepth : register(t1);
Texture2D<float4> g_inHitPosition : register(t2);
Texture2D<float2> g_inPartialDistanceDerivatives : register(t4);
Texture2D<float> g_inDepth : register(t5); 
Texture2D<float2> g_inMotionVector : register(t6);
Texture2D<NormalDepthTexFormat> g_inReprojectedNormalDepth : register(t7);
Texture2D<float4> g_inSurfaceAlbedo : register(t8);
RWTexture2D<float4> g_outNormalDepth : register(u1);
RWTexture2D<float4> g_outHitPosition : register(u2);
RWTexture2D<float2> g_outPartialDistanceDerivatives : register(u4);
RWTexture2D<float> g_outDepth : register(u5);   
RWTexture2D<float2> g_outMotionVector : register(u6);
RWTexture2D<NormalDepthTexFormat> g_outReprojectedNormalDepth : register(u7);
RWTexture2D<float4> g_outSurfaceAlbedo : register(u8);

SamplerState ClampSampler : register(s0);

ConstantBuffer<TextureDimConstantBuffer> cb : register(b0);

void LoadDepthAndEncodedNormal(in uint2 texIndex, out float4 encodedNormalDepth, out float depth)
{
    encodedNormalDepth = g_inNormalDepth[texIndex];
    depth = encodedNormalDepth.z;
}

// Returns a selected depth index when bilateral downsapling.
uint GetIndexFromDepthAwareBilateralDownsample2x2(in float4 vDepths, in uint2 DTid)
{
    // Alternate between min max depth sample in a checkerboard 2x2 pattern to improve 
    // depth matching for bilateral 2x2 upsampling in a later pass.
    // Ref: http://c0de517e.blogspot.com/2016/02/downsampled-effects-with-depth-aware.html
    bool checkerboardTakeMin = ((DTid.x + DTid.y) & 1) == 0;

    float lowResDepth = checkerboardTakeMin ? min4(vDepths) : max4(vDepths);

    // Find the corresponding sample index to the the selected sample depth.
    return GetIndexOfValueClosestToTheReference(lowResDepth, vDepths);
}

[numthreads(DefaultComputeShaderParams::ThreadGroup::Width, DefaultComputeShaderParams::ThreadGroup::Height, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    uint2 topLeftSrcIndex = DTid << 1;
    
    float2 centerTexCoord = (topLeftSrcIndex + 0.5) * cb.invTextureDim;
    float4 vDepths = g_inDepth.Gather(ClampSampler, centerTexCoord).wzxy;

    uint selectedOffset = GetIndexFromDepthAwareBilateralDownsample2x2(vDepths, DTid);
    uint2 selectedDTid = topLeftSrcIndex + Get2DQuadIndexOffset(selectedOffset);

    g_outDepth[DTid] = vDepths[selectedOffset];
    g_outNormalDepth[DTid] = g_inNormalDepth[selectedDTid];

    // Since we're reducing the resolution by 2, multiple the partial derivatives by 2. 
    // Either that or the multiplier should be applied when calculating weights.
    // TODO: use perspective correct ddxy interpolation? Or apply the scaling on use?
    g_outPartialDistanceDerivatives[DTid] = 2 * g_inPartialDistanceDerivatives[selectedDTid];

    g_outMotionVector[DTid] = g_inMotionVector[selectedDTid];
    g_outReprojectedNormalDepth[DTid] = g_inReprojectedNormalDepth[selectedDTid];
    g_outHitPosition[DTid] = g_inHitPosition[selectedDTid];
    g_outSurfaceAlbedo[DTid] = g_inSurfaceAlbedo[selectedDTid];
}