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

#include "ShaderUtility.hlsli"
#include "TemporalRS.hlsli"

Texture2D<float4> TemporalColor : register(t0);
RWTexture2D<float3> OutColor : register(u0);

[RootSignature(Temporal_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 Color = TemporalColor[DTid.xy];
    OutColor[DTid.xy] = Color.rgb / max(Color.w, 1e-6);
}
