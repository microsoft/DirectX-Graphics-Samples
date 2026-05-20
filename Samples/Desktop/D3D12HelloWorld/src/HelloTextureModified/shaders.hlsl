//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct Material
{
    uint textureIndex;
    float roughness;
    float metallic;
    uint flags;
};

struct InstanceData
{
    float4x4 world;
    float4x4 prevWorld;
    uint materialId;
    float padding[3]; //16 byte alignment
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProj;
    float4x4 prevViewProj;
    float4x4 invViewProj;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    uint instanceId : SV_InstanceID;
};


Texture2D g_texture[] : register(t0, space0);
SamplerState g_sampler : register(s0);
StructuredBuffer<InstanceData> g_instanceData : register(t0, space1);
StructuredBuffer<Material> g_materialData : register(t0, space2);

PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD, float3 normal : NORMAL, uint instanceId : SV_InstanceID)
{
    PSInput result;

    InstanceData inst = g_instanceData[instanceId];    

    float4x4 worldViewProj = mul(inst.world, viewProj);
    result.position = mul(float4(position.xyz, 1.0), worldViewProj);    
    result.uv = uv;
    result.instanceId = instanceId;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    InstanceData inst = g_instanceData[input.instanceId];
    Material mat = g_materialData[inst.materialId];
    return g_texture[mat.textureIndex].Sample(g_sampler, input.uv);
}
