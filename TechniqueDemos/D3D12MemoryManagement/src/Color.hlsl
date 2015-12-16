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

cbuffer Projection : register(b0)
{
	float4x4 GProjectionMatrix;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

PSInput VShader(float3 Position : POSITION, float4 Color : COLOR)
{
	PSInput Output;

	Output.Position = float4(Position, 1.0f);
	Output.Position = mul(GProjectionMatrix, Output.Position);
	Output.Color = Color;

	return Output;
}

float4 PShader(PSInput Input) : SV_TARGET
{
	return Input.Color;
}
