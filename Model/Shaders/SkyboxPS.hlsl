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

#include "Common.hlsli"

cbuffer PSConstants : register(b0)
{
    float TextureLevel;
};

TextureCube<float3> radianceIBLTexture      : register(t10);

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 viewDir : TEXCOORD3;
};

[RootSignature(Renderer_RootSig)]
float4 main(VSOutput vsOutput) : SV_Target0
{
    return float4(radianceIBLTexture.SampleLevel(defaultSampler, vsOutput.viewDir, TextureLevel), 1);
}
