struct Material
{
    uint textureIndex;
    float padding[3];
};

struct InstanceData
{
    float4x4 world;
    uint materialId;
    float padding[3];
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProj;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    uint instanceId : SV_InstanceID;
};

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 material : SV_Target2;
};

Texture2D g_texture[] : register(t0, space0);
SamplerState g_sampler : register(s0);
StructuredBuffer<InstanceData> g_instanceData : register(t0, space1);
StructuredBuffer<Material> g_materialData : register(t0, space2);

PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD, uint instanceId : SV_InstanceID)
{
    PSInput result;

    InstanceData inst = g_instanceData[instanceId];
    float4x4 worldViewProj = mul(inst.world, viewProj);

    result.position = mul(float4(position.xyz, 1.0), worldViewProj);
    result.uv = uv;
    result.instanceId = instanceId;

    return result;
}

GBufferOutput PSMain(PSInput input)
{
    InstanceData inst = g_instanceData[input.instanceId];
    Material mat = g_materialData[inst.materialId];

    GBufferOutput output;
    output.albedo = g_texture[mat.textureIndex].Sample(g_sampler, input.uv);
    output.normal = float4(0.5, 0.5, 1.0, 1.0);
    output.material = float4((float)mat.textureIndex / 1020.0, 0.0, 0.0, 1.0);
    return output;
}
