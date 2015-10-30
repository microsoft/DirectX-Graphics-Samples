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

struct VSInput
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

// A pass-through function for the texture coordinate data.
PSInput VSSimpleBlur(VSInput input)
{
	PSInput output;
	output.position = input.position;
	output.uv = input.uv;
	return output;
}

static const float KernelOffsets[3] = { 0.0f, 1.3846153846f, 3.2307692308f };
static const float BlurWeights[3] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };

// The input texture to blur.
Texture2D tex : register(t0);
SamplerState pointSampler : register(s0);
SamplerState linearSampler : register(s1);

cbuffer GaussianBlurConstantBuffer : register(b0)
{
	float2 textureDimensions;	// The render target width/height.
	float blurXOffset;			// Controls how much of the render target is blurred along X axis [0.0. 1.0]. E.g. 1 = all of the RT is blurred, 0.5 = half of the RT is blurred, 0.0 = none of the RT is blurred.
};

cbuffer WorkloadConstantBuffer : register(b1)
{
	uint loopCount;
};

// Simple gaussian blur in the vertical direction.
float4 PSSimpleBlurV(PSInput input) : SV_TARGET
{
	float3 textureColor = float3(1.0f, 0.0f, 0.0f);
	float2 uv = input.uv;
	if (uv.x > (blurXOffset + 0.005f))
	{
		textureColor = tex.Sample(linearSampler, uv).xyz * BlurWeights[0];
		for (int i = 1; i < 3; i++)
		{
			float2 normalizedOffset = float2(0.0f, KernelOffsets[i]) / textureDimensions.y;
			textureColor += tex.Sample(linearSampler, uv + normalizedOffset).xyz * BlurWeights[i];
			textureColor += tex.Sample(linearSampler, uv - normalizedOffset).xyz * BlurWeights[i];
		}
	}
	else if (uv.x <= (blurXOffset - 0.005f))
	{
		textureColor = tex.Sample(pointSampler, uv).xyz;
	}

	// Artificially increase the workload to simulate a more complex shader.
	const float3 textureColorOrig = textureColor;
	for (uint i = 0; i < loopCount; i++)
	{
		textureColor += textureColorOrig;
	}

	if (loopCount > 0)
	{
		textureColor /= loopCount + 1;
	}

	return float4(textureColor, 1.0);
}

// Simple gaussian blur in the horizontal direction.
float4 PSSimpleBlurU(PSInput input) : SV_TARGET
{
	float3 textureColor = float3(1.0f, 0.0f, 0.0f);
	float2 uv = input.uv;
	if (uv.x > (blurXOffset + 0.005f))
	{
		textureColor = tex.Sample(linearSampler, uv).xyz * BlurWeights[0];
		for (int i = 1; i < 3; i++)
		{
			float2 normalizedOffset = float2(KernelOffsets[i], 0.0f) / textureDimensions.x;
			textureColor += tex.Sample(linearSampler, uv + normalizedOffset).xyz * BlurWeights[i];
			textureColor += tex.Sample(linearSampler, uv - normalizedOffset).xyz * BlurWeights[i];
		}
	}
	else if (uv.x <= (blurXOffset - 0.005f))
	{
		textureColor = tex.Sample(pointSampler, uv).xyz;
	}

	// Artificially increase the workload to simulate a more complex shader.
	const float3 textureColorOrig = textureColor;
	for (uint i = 0; i < loopCount; i++)
	{
		textureColor += textureColorOrig;
	}

	if (loopCount > 0)
	{
		textureColor /= loopCount + 1;
	}

	return float4(textureColor, 1.0);
}
