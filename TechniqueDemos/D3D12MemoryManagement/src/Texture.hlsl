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

Texture2D GTexture : register(t0);
SamplerState GSampler : register(s0);

cbuffer Projection : register(b0)
{
	float4x4 GProjectionMatrix;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 UV : TEXCOORD;
};

PSInput VShader(float3 Position : POSITION, float2 UV : TEXCOORD, float4 Color : COLOR)
{
	PSInput Result;

	Result.Position = float4(Position, 1.0f);
	Result.Position = mul(GProjectionMatrix, Result.Position);
	Result.UV = UV;
	Result.Color = Color;

	return Result;
}

float4 PShader(PSInput Input) : SV_TARGET
{
	return GTexture.Sample(GSampler, Input.UV) * Input.Color;
}
