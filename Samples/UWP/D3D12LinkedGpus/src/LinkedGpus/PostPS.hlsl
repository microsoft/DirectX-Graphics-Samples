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

#include "Post.hlsli"

float4 PSMain(PSInput input) : SV_TARGET
{
	// This adds up to more than 1.0f, but creates a decent effect
	// that doesn't make the triangles look too transparent.
	float blurWeights[6] = { 0.80f, 0.25f, 0.15f, 0.075f, 0.025f, 0.0125f };

	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
	uint index = currentRenderTarget;

	for (uint n = 0; n < renderTargetCount; n++)
	{
		result += blurWeights[n] * g_renderTargets[index].Sample(g_sampler, input.uv);
		index = (index == 0) ? renderTargetCount - 1 : index - 1;
	}

	return saturate(result);
}
