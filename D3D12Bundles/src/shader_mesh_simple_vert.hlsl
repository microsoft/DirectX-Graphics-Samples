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

struct VSSceneIn
{
	float3 pos		: POSITION;
	float3 norm		: NORMAL;
	float2 tex		: TEXCOORD0;
	float3 tangent	: TANGENT;
};

struct PSSceneIn
{
	float4 pos		: SV_Position;
	float2 tex		: TEXCOORD0;
};

cbuffer cb0
{
	float4x4 g_mWorldViewProj;
};

Texture2D		g_txDiffuse : register( t0 );
SamplerState	g_sampler : register( s0 );

PSSceneIn VSSceneMain(VSSceneIn input)
{
	PSSceneIn output;
	
	output.pos = mul(float4(input.pos, 1.0), g_mWorldViewProj);
	output.tex = input.tex;
	
	return output;
}
