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

cbuffer SceneRootConstants : register(b1)
{
	uint simulatedGpuLoad;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
