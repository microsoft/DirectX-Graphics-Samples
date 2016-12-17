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

#include "ToneMappingUtility.hlsli"
#include "PostEffectsRS.hlsli"
#include "PixelPacking.hlsli"

StructuredBuffer<float> Exposure : register( t0 );
Texture2D<float3> Bloom : register( t1 );
#if SUPPORT_TYPED_UAV_LOADS
RWTexture2D<float3> ColorRW : register( u0 );
#else
RWTexture2D<uint> DstColor : register( u0 );
Texture2D<float3> SrcColor : register( t2 );
#endif
RWTexture2D<float> OutLuma : register( u1 );
SamplerState LinearSampler : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
	float2 g_RcpBufferDim;
	float g_BloomStrength;
	float g_ToeStrength;
};

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 TexCoord = (DTid.xy + 0.5) * g_RcpBufferDim;

	// Load HDR and bloom
#if SUPPORT_TYPED_UAV_LOADS
	float3 hdrColor = ColorRW[DTid.xy];
#else
	float3 hdrColor = SrcColor[DTid.xy];
#endif

	hdrColor += g_BloomStrength * Bloom.SampleLevel(LinearSampler, TexCoord, 0);
	hdrColor *= Exposure[0];

	// Tone map to LDR.
#if ENABLE_HDR_OUTPUT
	{
	#if SUPPORT_TYPED_UAV_LOADS
		ColorRW[DTid.xy] = hdrColor;
	#else
		DstColor[DTid.xy] = Pack_R11G11B10_FLOAT(hdrColor);
	#endif
		OutLuma[DTid.xy] = LinearToLogLuminance(ToneMapLuma(RGBToLuminance(hdrColor)));
	}
#else
	{
		float3 ldrColor = ApplyToe(ToneMap(hdrColor), g_ToeStrength);
	#if SUPPORT_TYPED_UAV_LOADS
		ColorRW[DTid.xy] = ldrColor;
	#else
		DstColor[DTid.xy] = Pack_R11G11B10_FLOAT(ldrColor);
	#endif
		OutLuma[DTid.xy] = RGBToLogLuminance(ldrColor);
	}
#endif
}
