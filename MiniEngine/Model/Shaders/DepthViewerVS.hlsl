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
//

#include "Common.hlsli"

cbuffer VSConstants : register(b0)
{
    float4x4 WVP;
};

struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TexCoord0;
};

[RootSignature(Renderer_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;
    vsOutput.pos = mul(WVP, float4(vsInput.pos, 1.0));
    vsOutput.uv = vsInput.uv;
    return vsOutput;
}
