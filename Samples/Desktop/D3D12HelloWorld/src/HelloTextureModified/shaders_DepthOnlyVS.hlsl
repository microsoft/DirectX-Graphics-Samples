
cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProj;
    float4x4 prevViewProj;
};

struct InstanceData
{
    float4x4 world;
    float4x4 prevWorld;
    uint materialId;
    float padding[3];
};

StructuredBuffer<InstanceData> g_instanceData : register(t0, space1);

struct VSInput
{
    float3 position : POSITION;
    uint instanceId : SV_InstanceID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput o;

    InstanceData inst = g_instanceData[input.instanceId];
    float4x4 worldViewProj = mul(inst.world, viewProj);
    o.position = mul(float4(input.position.xyz, 1.0), worldViewProj);
    
    return o;
}
