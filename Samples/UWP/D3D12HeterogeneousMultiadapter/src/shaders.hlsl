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

cbuffer SceneConstantBuffer : register(b0)
{
	float4 velocity;
	float4 offset;
	float4 color;
	float4x4 projection;
};

cbuffer WorkloadConstantBuffer : register(b1)
{
	uint loopCount;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VShader(float4 position : POSITION)
{
	PSInput result;

	result.position = mul(position + offset, projection);

	float intensity = saturate((4.0f - result.position.z) / 2.0f);
	result.color = float4(color.xyz * intensity, 1.0f);

	return result;
}

float4 PShader(PSInput input) : SV_TARGET
{
	float4 result = input.color;

	// Artificially increase the workload to simulate a more complex shader.
	for (uint i = 0; i < loopCount; i++)
	{
		result += input.color;
	}

	if (loopCount > 0)
	{
		result /= loopCount + 1;
	}
	
	return result;
}
