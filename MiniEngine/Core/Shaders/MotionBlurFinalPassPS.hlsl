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

#include "MotionBlurRS.hlsli"

#define MAX_SAMPLE_COUNT  10
#define STEP_SIZE         3.0

Texture2D<float2> MotionBuffer : register(t0);		// full resolution motion vectors
Texture2D<float4> PrepBuffer : register(t1);		// 1/4 resolution pre-weighted blurred color samples
SamplerState LinearSampler : register(s0);

cbuffer c0 : register(b0)
{
	float2 RcpBufferDim;	// 1 / width, 1 / height
}

[RootSignature(MotionBlur_RootSig)]
float4 main( float4 position : SV_Position ) : SV_Target0
{
	uint2 st = uint2(position.xy);
	float2 uv = position.xy * RcpBufferDim;

	float2 motionVec = MotionBuffer[st] * 32;

	// Computing speed in this way will set the step size to two-pixel increments in the dominant
	// direction.
	float speed = length(motionVec);

	if (speed < 4)
		discard;

	float4 accum = 0;

	// Half of the speed goes in each direction
	float halfSampleCount = min(MAX_SAMPLE_COUNT * 0.5, speed * 0.5 / STEP_SIZE);

	// Accumulate low-res, pre-weighted samples, summing their weights in alpha.
	// The center sample is skipped because we are alpha blending onto it in the
	// destination buffer.  Only its weight is considered.  Accumulating low-res
	// samples is not so egregious because the center weight is still high res.
	// Also, each of the low res samples is comprised of four pre-weighted high-
	// res samples, so they are effectively masked at full resolution.
	float2 deltaUV = motionVec / speed * RcpBufferDim * STEP_SIZE;
	float2 uv1 = uv;
	float2 uv2 = uv;

	// First accumulate the whole samples
	for (float i = halfSampleCount - 1.0; i > 0.0; i -= 1.0)
	{
		accum += PrepBuffer.SampleLevel(LinearSampler, uv1 += deltaUV, 0);
		accum += PrepBuffer.SampleLevel(LinearSampler, uv2 -= deltaUV, 0);
	}

	// This is almost the same as 'frac(halfSampleCount)' replaces 0 with 1.
	float remainder = 1 + halfSampleCount - ceil(halfSampleCount);

	// Then accumulate the fractional samples
	deltaUV *= remainder;
	accum += PrepBuffer.SampleLevel(LinearSampler, uv1 + deltaUV, 0) * remainder;
	accum += PrepBuffer.SampleLevel(LinearSampler, uv2 - deltaUV, 0) * remainder;

	return accum / (accum.a + 1.0);
}
