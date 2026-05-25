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
    float padding[3];
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
    float4 pbrParams : SV_Target4;
};

Texture2D g_texture[] : register(t0, space0);
SamplerState g_sampler : register(s0);
StructuredBuffer<InstanceData> g_instanceData : register(t0, space1);
StructuredBuffer<Material> g_materialData : register(t0, space2);

float3x3 BuildTangentFrame(float3 normal)
{
    float3 up = abs(normal.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, bitangent, normal);
}

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
    output.albedo = g_texture[mat.albedoTexIndex].Sample(g_sampler, input.uv);

    float3 baseNormal = normalize(input.normal); // We should use the interpolated normal from vertex shader as the base normal for normal mapping, otherwise the normal map will not work correctly on flat surfaces.
    float3 normalTex = g_texture[mat.normalTexIndex].Sample(g_sampler, input.uv).xyz * 2.0 - 1.0;
    float3 mappedNormal = normalize(mul(normalTex, BuildTangentFrame(baseNormal)));
    output.normal = float4(mappedNormal, 1.0);
    output.material = inst.materialId;
    
    float2 curNdc = input.currClipPos.xy / input.currClipPos.w;
    float2 prevNdc = input.prevClipPos.xy / input.prevClipPos.w;    
    output.motionVector = curNdc - prevNdc;

    float4 metallicRoughness = g_texture[mat.metallicRoughnessTexIndex].Sample(g_sampler, input.uv);
    float occlusion = g_texture[mat.occlusionTexIndex].Sample(g_sampler, input.uv).r;
    float3 emissive = g_texture[mat.emissiveTexIndex].Sample(g_sampler, input.uv).rgb;

    float metallic = saturate(metallicRoughness.b * mat.metallicFactor);
    float roughness = saturate(metallicRoughness.g * mat.roughnessFactor);
    float ambientOcclusion = lerp(1.0, occlusion, mat.occlusionStrength);
    float emissiveLuminance = dot(emissive, float3(0.2126, 0.7152, 0.0722));
    output.pbrParams = float4(metallic, roughness, ambientOcclusion, emissiveLuminance);
    
    return output;
}
