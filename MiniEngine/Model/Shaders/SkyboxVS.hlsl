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
    float4x4 ProjInverse;
    float3x3 ViewInverse;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 viewDir : TEXCOORD3;
};

[RootSignature(Renderer_RootSig)]
VSOutput main(uint VertID : SV_VertexID)
{
    float2 ScreenUV = float2(uint2(VertID, VertID << 1) & 2);
    float4 ProjectedPos = float4(lerp(float2(-1, 1), float2(1, -1), ScreenUV), 0, 1);
    float4 PosViewSpace = mul(ProjInverse, ProjectedPos);

    VSOutput vsOutput;
    vsOutput.position = ProjectedPos;
    vsOutput.viewDir = mul(ViewInverse, PosViewSpace.xyz / PosViewSpace.w);

    return vsOutput;
}
