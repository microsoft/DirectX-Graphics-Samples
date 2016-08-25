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
#include "PresentRS.hlsli"

Texture2D<float3> ColorTex : register(t0);
Texture2D<float4> Overlay : register(t1);

struct PS_OUT
{
	float3 HdrOutput : SV_Target0;
};

cbuffer CB0 : register(b0)
{
	float PaperWhite;
	float MaxBrightness;
	float ToeStrength;
	uint DebugMode;
}

[RootSignature(Present_RootSig)]
PS_OUT main( float4 position : SV_Position )
{
	PS_OUT Out;

	float4 UI = Overlay[(int2)position.xy];
	float3 HDR = ColorTex[(int2)position.xy];
	float3 SDR = ApplyToeRGB(ToneMapRGB(HDR), ToeStrength);

	// Better to blend in linear space (unlike the hardware compositor)
	UI.rgb = SRGBToLinear(UI.rgb);

	// SDR was not explicitly clamped to [0, 1] on input, but it will be on output
	SDR = saturate(SDR) * (1 - UI.a) + UI.rgb;

	HDR = REC709toREC2020(HDR);
	UI.rgb = REC709toREC2020(UI.rgb) * PaperWhite;
	SDR = REC709toREC2020(SDR) * PaperWhite;

	HDR = ApplyToeRGB(ToneMapRGB(HDR * ComputeHDRRescale(PaperWhite, MaxBrightness)), ToeStrength) * MaxBrightness;

	// Composite HDR buffer with UI
	HDR = HDR * (1 - UI.a) + UI.rgb;

	float3 FinalOutput;
	switch (DebugMode)
	{
	case 0: FinalOutput = HDR; break;
	case 1: FinalOutput = SDR; break;
	default: FinalOutput = (position.x < 960 ? HDR : SDR); break;
	}

	// Current values are specified in nits.  Normalize to max specified brightness.
	Out.HdrOutput = LinearToREC2084(FinalOutput / 10000.0);
	
	return Out;
}