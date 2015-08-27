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
// The VS for doing a full-screen effect without a vertex buffer.

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

QuadVS_Output main( uint vertID : SV_VertexID )
{
	float2 uv = float2( (vertID >> 1) & 1, vertID & 1 ) * 2.0f;

	QuadVS_Output Output;
    Output.Pos = float4( lerp( float2(-1.0f, 1.0f), float2(1.0f, -1.0f), uv ), 0.0f, 1.0f );
    Output.Tex = uv;
    return Output;
}
