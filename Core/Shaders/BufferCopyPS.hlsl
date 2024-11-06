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

#include "PresentRS.hlsli"

Texture2D ColorTex : register(t0);
SamplerState BilinearSampler : register(s0);

cbuffer Constants : register(b0)
{
    float2 RcpDestDim;
}

[RootSignature(Present_RootSig)]
float4 main( float4 position : SV_Position ) : SV_Target0
{
    //float2 UV = saturate(RcpDestDim * position.xy);
    //return ColorTex.SampleLevel(BilinearSampler, UV, 0);
    return ColorTex[(int2)position.xy];
}
