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
// Author(s):  James Stanard
//             Alex Nankervis
//

#include "ModelViewerRS.hlsli"

cbuffer VSConstants : register(b0)
{
	float4x4 modelToProjection;
	float4x4 modelToShadow;
	float3 ViewerPos;
};

struct VSInput
{
	float3 position : POSITION;
	float2 texcoord0 : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord0 : texcoord0;
	float3 viewDir : texcoord1;
	float3 shadowCoord : texcoord2;
	float3 normal : normal;
	float3 tangent : tangent;
	float3 bitangent : bitangent;
};

[RootSignature(ModelViewer_RootSig)]
VSOutput main(VSInput vsInput)
{
	VSOutput vsOutput;

	vsOutput.position = mul(modelToProjection, float4(vsInput.position, 1.0));
	vsOutput.texcoord0 = vsInput.texcoord0;
	vsOutput.viewDir = vsInput.position - ViewerPos;
	vsOutput.shadowCoord = mul(modelToShadow, float4(vsInput.position, 1.0)).xyz;

	vsOutput.normal = vsInput.normal;
	vsOutput.tangent = vsInput.tangent;
	vsOutput.bitangent = vsInput.bitangent;

	return vsOutput;
}
