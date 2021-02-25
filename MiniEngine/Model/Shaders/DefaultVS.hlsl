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

#ifdef ENABLE_SKINNING
//#undef ENABLE_SKINNING
#endif

cbuffer MeshConstants : register(b0)
{
    float4x4 WorldMatrix;   // Object to world
    float3x3 WorldIT;       // Object normal to world normal
};

cbuffer GlobalConstants : register(b1)
{
    float4x4 ViewProjMatrix;
    float4x4 SunShadowMatrix;
    float3 ViewerPos;
    float3 SunDirection;
    float3 SunIntensity;
}

#ifdef ENABLE_SKINNING
struct Joint
{
    float4x4 PosMatrix;
    float4x3 NrmMatrix; // Inverse-transpose of PosMatrix
};

StructuredBuffer<Joint> Joints : register(t20);
#endif

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 tangent : TANGENT;
#endif
    float2 uv0 : TEXCOORD0;
#ifndef NO_SECOND_UV
    float2 uv1 : TEXCOORD1;
#endif
#ifdef ENABLE_SKINNING
    uint4 jointIndices : BLENDINDICES;
    float4 jointWeights : BLENDWEIGHT;
#endif
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 tangent : TANGENT;
#endif
    float2 uv0 : TEXCOORD0;
#ifndef NO_SECOND_UV
    float2 uv1 : TEXCOORD1;
#endif
    float3 worldPos : TEXCOORD2;
    float3 sunShadowCoord : TEXCOORD3;
};

[RootSignature(Renderer_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

    float4 position = float4(vsInput.position, 1.0);
    float3 normal = vsInput.normal * 2 - 1;
#ifndef NO_TANGENT_FRAME
    float4 tangent = vsInput.tangent * 2 - 1;
#endif

#ifdef ENABLE_SKINNING
    // I don't like this hack.  The weights should be normalized already, but something is fishy.
    float4 weights = vsInput.jointWeights / dot(vsInput.jointWeights, 1);

    float4x4 skinPosMat =
        Joints[vsInput.jointIndices.x].PosMatrix * weights.x +
        Joints[vsInput.jointIndices.y].PosMatrix * weights.y +
        Joints[vsInput.jointIndices.z].PosMatrix * weights.z +
        Joints[vsInput.jointIndices.w].PosMatrix * weights.w;

    position = mul(skinPosMat, position);

    float4x3 skinNrmMat =
        Joints[vsInput.jointIndices.x].NrmMatrix * weights.x +
        Joints[vsInput.jointIndices.y].NrmMatrix * weights.y +
        Joints[vsInput.jointIndices.z].NrmMatrix * weights.z +
        Joints[vsInput.jointIndices.w].NrmMatrix * weights.w;

    normal = mul(skinNrmMat, normal).xyz;
#ifndef NO_TANGENT_FRAME
    tangent.xyz = mul(skinNrmMat, tangent.xyz).xyz;
#endif

#endif

    vsOutput.worldPos = mul(WorldMatrix, position).xyz;
    vsOutput.position = mul(ViewProjMatrix, float4(vsOutput.worldPos, 1.0));
    vsOutput.sunShadowCoord = mul(SunShadowMatrix, float4(vsOutput.worldPos, 1.0)).xyz;
    vsOutput.normal = mul(WorldIT, normal);
#ifndef NO_TANGENT_FRAME
    vsOutput.tangent = float4(mul(WorldIT, tangent.xyz), tangent.w);
#endif
    vsOutput.uv0 = vsInput.uv0;
#ifndef NO_SECOND_UV
    vsOutput.uv1 = vsInput.uv1;
#endif

    return vsOutput;
}
