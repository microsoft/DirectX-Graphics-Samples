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
// Author(s):    Alex Nankervis
//

#include "ModelViewerRS.hlsli"
#include "LightGrid.hlsli"

struct VSOutput
{
    sample float4 position : SV_Position;
    sample float3 worldPos : worldPos;
    sample float2 texcoord0 : texcoord0;
    sample float3 viewDir : texcoord1;
    sample float3 shadowCoord : texcoord2;
    sample float3 normal : normal;
    sample float3 tangent : tangent;
    sample float3 bitangent : bitangent;
};

Texture2D<float3> texDiffuse        : register(t0);
Texture2D<float3> texSpecular        : register(t1);
//Texture2D<float4> texEmissive        : register(t2);
Texture2D<float3> texNormal            : register(t3);
//Texture2D<float4> texLightmap        : register(t4);
//Texture2D<float4> texReflection    : register(t5);
Texture2D<float> texSSAO            : register(t64);
Texture2D<float> texShadow            : register(t65);

StructuredBuffer<LightData> lightBuffer : register(t66);
Texture2DArray<float> lightShadowArrayTex : register(t67);
ByteAddressBuffer lightGrid : register(t68);

cbuffer PSConstants : register(b0)
{
    float3 SunDirection;
    float3 SunColor;
    float3 AmbientColor;
    float4 ShadowTexelSize;

    float4 InvTileDim;
    uint4 TileCount;
    uint4 FirstLightIndex;
}

SamplerState sampler0 : register(s0);
SamplerComparisonState shadowSampler : register(s1);

[RootSignature(ModelViewer_RootSig)]
float3 main(VSOutput vsOutput) : SV_Target0
{
    uint2 tilePos = GetTilePos(vsOutput.position.xy, InvTileDim.xy);
    uint tileIndex = GetTileIndex(tilePos, TileCount.x);
    uint tileOffset = GetTileOffset(tileIndex);

    // There are three counts in one UINT
    uint tileLightCount = lightGrid.Load(tileOffset + 0);
    tileLightCount = (tileLightCount & 0xFF) + ((tileLightCount >> 8) & 0xFF) + ((tileLightCount >> 16) & 0xFF);

    return lerp(float3(0, 1, 0), float3(1, 0, 0), tileLightCount / 32.0);
}
