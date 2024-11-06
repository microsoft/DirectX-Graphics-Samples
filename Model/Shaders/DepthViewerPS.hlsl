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
//

#include "Common.hlsli"

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TexCoord0;
};

Texture2D<float4>	texDiffuse		: register(t0);

[RootSignature(Renderer_RootSig)]
void main(VSOutput vsOutput)
{
    if (texDiffuse.Sample(defaultSampler, vsOutput.uv).a < 0.5)
        discard;
}
