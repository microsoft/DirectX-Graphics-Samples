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

#include "ShaderUtility.hlsli"
#include "MotionBlurRS.hlsli"
#include "PixelPacking.hlsli"

#define USE_LINEAR_Z

Texture2D<float2> ReprojectionBuffer : register(t0);
Texture2D<float4> TemporalIn : register(t1);

#if SUPPORT_TYPED_UAV_LOADS
RWTexture2D<float3> DstColor : register(u0);		// final output color (blurred and temporally blended)
#else
RWTexture2D<uint> DstColor : register(u0);
#endif
RWTexture2D<float4> TemporalOut : register(u1);		// color to save for next frame including its validity in alpha

SamplerState LinearSampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	float2 RcpBufferDim;	// 1 / width, 1 / height
	float TemporalBlendFactor;
}

struct MRT
{
	float3 BlendedColor : SV_Target0;
	float4 TemporalOut : SV_Target1;
};

[RootSignature(MotionBlur_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	uint2 st = DTid.xy;
	float2 position = st + 0.5;
	float2 curUV = position * RcpBufferDim;
	float2 velocity = ReprojectionBuffer[st];
	float2 preUV = (position + velocity) * RcpBufferDim;
#if 1
	float4 lastColor = TemporalIn.SampleLevel( LinearSampler, preUV, 0 );
#else
	float4 lastColor = 0.25 * (
		TemporalIn.SampleLevel(LinearSampler, preUV + float2(+0.5, +0.5) * RcpBufferDim, 0) +
		TemporalIn.SampleLevel(LinearSampler, preUV + float2(+0.5, -0.5) * RcpBufferDim, 0) +
		TemporalIn.SampleLevel(LinearSampler, preUV + float2(-0.5, +0.5) * RcpBufferDim, 0) +
		TemporalIn.SampleLevel(LinearSampler, preUV + float2(-0.5, -0.5) * RcpBufferDim, 0));
#endif

#if SUPPORT_TYPED_UAV_LOADS
	float3 thisColor = DstColor[st];
#else
	float3 thisColor = Unpack_R11G11B10_FLOAT(DstColor[st]);
#endif
	float thisValidity = 1.0;// - saturate(length(velocity) / 4.0);

#if 1
	// 2x super sampling with no feedback
	float3 displayColor = lerp( thisColor, lastColor.rgb, 0.5 * min(thisValidity, lastColor.a) );
	float4 savedColor = float4( thisColor, thisValidity );
#else
	// 4x super sampling via controlled feedback
	float3 displayColor = lerp( thisColor, lastColor.rgb, TemporalBlendFactor * min(thisValidity, lastColor.a) );
	float4 savedColor = float4( displayColor, thisValidity );
#endif

#if SUPPORT_TYPED_UAV_LOADS
	DstColor[st] = displayColor;
#else
	DstColor[st] = Pack_R11G11B10_FLOAT(displayColor);
#endif
	TemporalOut[st] = savedColor;
}