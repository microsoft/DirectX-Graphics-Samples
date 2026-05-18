struct Material
{
    uint textureIndex;
    float padding[3];
};

struct InstanceData
{
    float4x4 world;
    float4x4 prevWorld;
    uint materialId;
    float padding[3];
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProj;
    float4x4 prevViewProj;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 currClipPos : TEXCOORD1;
    float4 prevClipPos : TEXCOORD2;
    uint instanceId : SV_InstanceID;
};

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    uint material : SV_Target2;
    float2 motionVector : SV_Target3;
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
    float3 worldNormal = normalize(mul(float4(normal, 0.0), inst.world).xyz);
    float4 worldPos = mul(float4(position.xyz, 1.0), inst.world);
    float4 prevWorldPos = mul(float4(position.xyz, 1.0), inst.prevWorld);
   
    result.position = mul(float4(position.xyz, 1.0), worldViewProj);
    result.uv = uv;
    result.normal = worldNormal;
    result.currClipPos = mul(worldPos, viewProj);
    result.prevClipPos = mul(prevWorldPos, prevViewProj);    
    result.instanceId = instanceId;

    return result;
}

GBufferOutput PSMain(PSInput input)
{
    InstanceData inst = g_instanceData[input.instanceId];
    Material mat = g_materialData[inst.materialId];

    GBufferOutput output;
    output.albedo = g_texture[mat.textureIndex].Sample(g_sampler, input.uv);
    output.normal = float4(normalize(input.normal), 1.0);
    output.material = mat.textureIndex;
    
    float2 curNdc = input.currClipPos.xy / input.currClipPos.w;
    float2 prevNdc = input.prevClipPos.xy / input.prevClipPos.w;    
    output.motionVector = curNdc - prevNdc;
    
    return output;
}
