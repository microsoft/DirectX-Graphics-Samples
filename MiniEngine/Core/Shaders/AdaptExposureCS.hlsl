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
// The group size is 16x16, but one group iterates over an entire 16-wide column of pixels (384 pixels tall)
// Assuming the total workspace is 640x384, there will be 40 thread groups computing the histogram in parallel.
// The histogram measures logarithmic luminance ranging from 2^-12 up to 2^4.  This should provide a nice window
// where the exposure would range from 2^-4 up to 2^4.

#include "PostEffectsRS.hlsli"

ByteAddressBuffer Histogram : register(t0);
RWStructuredBuffer<float> Exposure : register(u0);

cbuffer cb0 : register(b1)
{
	float TargetLuminance;
	float AdaptationRate;
	float MinExposure;
	float MaxExposure;
	float PeakIntensity;
	float PixelCount; 
}

groupshared float gs_Accum[256];

[RootSignature(PostEffects_RootSig)]
[numthreads( 256, 1, 1 )]
void main( uint GI : SV_GroupIndex )
{
	float WeightedSum = (float)GI * (float)Histogram.Load(GI * 4);

	[unroll]
	for (uint i = 128; i > 0; i /= 2)
	{
		gs_Accum[GI] = WeightedSum;					// Write
		GroupMemoryBarrierWithGroupSync();			// Sync
		WeightedSum += gs_Accum[(GI + i) % 256];	// Read
		GroupMemoryBarrierWithGroupSync();			// Sync
	}

	// Average histogram value is the weighted sum of all pixels divided by the total number of pixels
	// minus those pixels which provided no weight (i.e. black pixels.)
	float weightedHistAvg = WeightedSum / (max(1, PixelCount - Histogram.Load(0)));
	float logAvgLuminance = exp2( weightedHistAvg / 16.0 - 12.0 );
	float targetExposure = TargetLuminance / logAvgLuminance;
	float exposure = Exposure[0];
	exposure = lerp(exposure, targetExposure, AdaptationRate);
	exposure = clamp(exposure, MinExposure, MaxExposure);

	if (GI == 0)
	{
		Exposure[0] = exposure;
		Exposure[1] = 1.0 / exposure;
		Exposure[2] = exposure / PeakIntensity;
		Exposure[3] = weightedHistAvg;
	}
}
