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

Texture2D		g_txDiffuse : register( t0 );
SamplerState	g_sampler : register( s0 );

float4 PSSceneMain( PSSceneIn input ) : SV_Target
{
	return g_txDiffuse.Sample( g_sampler, input.tex );
}
