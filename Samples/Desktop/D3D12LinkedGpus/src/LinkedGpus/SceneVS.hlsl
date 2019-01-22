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

#include "Scene.hlsli"

PSInput VSMain(float4 position : POSITION)
{
	PSInput result;

	result.position = mul(position + offset, projection);

	float intensity = saturate((4.0f - result.position.z) / 2.0f);
	float4 c = float4(color.xyz * intensity, 1.0f);

	if (simulatedGpuLoad > 0)
	{
		float4 total = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Simulate additional workload on the GPU.
		for (uint x = 0; x < simulatedGpuLoad; x++)
		{
			total += c;
		}

		result.color = total / simulatedGpuLoad;
	}
	else
	{
		result.color = c;
	}

	return result;
}
