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
    float padding[3];    
};

struct InstanceData
{
    float4 offset;
    Material material;
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


PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD, uint instanceId : SV_InstanceID)
{
    PSInput result;

    InstanceData inst = g_instanceData[instanceId];
    
    result.position = position + inst.offset;
    result.uv = uv;
    result.instanceId = instanceId;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    InstanceData inst = g_instanceData[input.instanceId];
    return g_texture[inst.material.textureIndex].Sample(g_sampler, input.uv);
}
