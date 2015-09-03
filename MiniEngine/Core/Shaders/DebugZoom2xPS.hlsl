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
// Author(s):	James Stanard	

#include "ShaderUtility.hlsli"

Texture2D<float3> ColorTex : register(t0);

float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
	uint2 SourceDim;
	ColorTex.GetDimensions(SourceDim.x, SourceDim.y);
	uint2 StartOffset = (SourceDim - uint2(960, 540)) / 2;
	uint2 FinalOffset = uint2(position.xy * 0.5) + StartOffset;
	return LinearToFrameBufferFormat( FrameBufferFormatToLinear( ColorTex[FinalOffset], 0), 1);
}
