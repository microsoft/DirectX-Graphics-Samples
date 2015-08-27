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
// Author:  James Stanard 
//

//--------------------------------------------------------------------------------------
// Simple bicubic filter
//
// http://en.wikipedia.org/wiki/Bicubic_interpolation
// http://http.developer.nvidia.com/GPUGems/gpugems_ch24.html
//
//--------------------------------------------------------------------------------------

#include "ShaderUtility.hlsli"

Texture2D<float3>	ColorTex	: register(t0);

cbuffer Constants : register(b0)
{
	float A;
}

float W1( float x )
{
	return x * x * ((A + 2) * x - (A + 3)) + 1.0;
}

float W2( float x )
{
	return A * (x * (x * (x - 5) + 8) - 4);
}

float3 Cubic( float d1, float3 c0, float3 c1, float3 c2, float3 c3 )
{
	float d0 = 1.0 + d1;
	float d2 = 1.0 - d1;
	float d3 = 2.0 - d1;

    return c0 * W2(d0) + c1 * W1(d1) + c2 * W1(d2) + c3 * W2(d3);
}

float3 GetColor( uint s, uint t )
{
	return ColorTex[uint2(s, t)];
}

float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
	uint2 TextureSize;
	ColorTex.GetDimensions( TextureSize.x, TextureSize.y );

	float2 t = uv * TextureSize + 0.5;
	float2 f = frac(t);
	int2 st = int2(t);

	uint s0 = max(st.x - 2, 0);
	uint s1 = max(st.x - 1, 0);
	uint s2 = min(st.x + 0, TextureSize.x - 1);
	uint s3 = min(st.x + 1, TextureSize.x - 1);

	uint t0 = max(st.y - 2, 0);
	uint t1 = max(st.y - 1, 0);
	uint t2 = min(st.y + 0, TextureSize.y - 1);
	uint t3 = min(st.y + 1, TextureSize.y - 1);

	float3 c0 = Cubic(f.x, GetColor(s0, t0), GetColor(s1, t0), GetColor(s2, t0), GetColor(s3, t0));
	float3 c1 = Cubic(f.x, GetColor(s0, t1), GetColor(s1, t1), GetColor(s2, t1), GetColor(s3, t1));
	float3 c2 = Cubic(f.x, GetColor(s0, t2), GetColor(s1, t2), GetColor(s2, t2), GetColor(s3, t2));
	float3 c3 = Cubic(f.x, GetColor(s0, t3), GetColor(s1, t3), GetColor(s2, t3), GetColor(s3, t3));

	return LinearToFrameBufferFormat( Cubic(f.y, c0, c1, c2, c3), 1 );
}
