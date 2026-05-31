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
    uint albedoTexIndex;
    uint metallicRoughnessTexIndex;
    uint emissiveTexIndex;
    uint occlusionTexIndex;
    uint normalTexIndex;
    float roughnessFactor;
    float metallicFactor;
    float occlusionStrength;
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
    float3 normal : NORMAL;
    uint instanceId : SV_InstanceID;
};


Texture2D g_texture[] : register(t0, space0);
SamplerState g_sampler : register(s0);
StructuredBuffer<InstanceData> g_instanceData : register(t0, space1);
StructuredBuffer<Material> g_materialData : register(t0, space2);

cbuffer LightingConstants : register(b2)
{
    float3 lightDirection;
    float ambientIntensity;
    float3 lightColor;
    float diffuseIntensity;
    float4 backgroundColor;
};

PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD, float3 normal : NORMAL, float4 tangent : TANGENT, uint instanceId : SV_InstanceID)
{
    PSInput result;

    InstanceData inst = g_instanceData[instanceId];    

    float4x4 worldViewProj = mul(inst.world, viewProj);
    result.position = mul(float4(position.xyz, 1.0), worldViewProj);    
    result.uv = uv;
    result.normal = normalize(mul(normal, (float3x3)inst.world));
    result.instanceId = instanceId;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    InstanceData inst = g_instanceData[input.instanceId];
    Material mat = g_materialData[inst.materialId];
    float4 albedo = g_texture[mat.albedoTexIndex].Sample(g_sampler, input.uv);
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(-lightDirection);
    float ndotl = saturate(dot(normal, lightDir));
    float3 ambient = albedo.rgb * ambientIntensity;
    float3 diffuse = albedo.rgb * lightColor * ndotl * diffuseIntensity;
    return float4(ambient + diffuse, albedo.a);
}
