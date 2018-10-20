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
#include "RandomNumberGenerator.hlsli"
#include "RaytracingHlslCompat.h"

ConstantBuffer<ReduceSumCS> rngCB: register(b0);
RWStructuredBuffer<uint> g_sum : register(u1);
Texture2D<float4> g_texGBufferPositionHit : register(t0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

	bool hit = g_texGBufferPositionHit[DTid.xy].x > 0.5;
    
	if (DTid.x == 0 && DTid.y == 0)
		g_sum[0] = 128;// rngCB.numSamples;
}