#include "FullscreenTriangle.hlsli"

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

Texture2D<float4> g_albedo : register(t0, space3);
Texture2D<float4> g_normal : register(t1, space3);
Texture2D<uint> g_material : register(t2, space3);
Texture2D<float4> g_pbrParams : register(t4, space3);
Texture2D<float> g_depth : register(t5, space3);
SamplerState g_sampler : register(s0);
StructuredBuffer<Material> g_materialData : register(t0, space2);

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProj;
    float4x4 prevViewProj;
    float4x4 invViewProj;
    float3 cameraPosition;
    float constantBufferPadding;
};

cbuffer LightingConstants : register(b2)
{
    float3 lightDirection;
    float ambientIntensity;
    float3 lightColor;
    float diffuseIntensity;
    float4 backgroundColor;
};

FullscreenVSOutput VSMain(uint vertexId : SV_VertexID)
{
    return FullscreenTriangleVS(vertexId);
}

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float2 ndc = float2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0);
    float4 clipPos = float4(ndc, depth, 1.0);
    float4 worldPos = mul(clipPos, invViewProj);
    return worldPos.xyz / worldPos.w;
}

float DistributionGGX(float ndoth, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = ndoth * ndoth * (a2 - 1.0) + 1.0;
    return a2 / max(3.14159265 * denom * denom, 0.000001);
}

float GeometrySchlickGGX(float ndotx, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return ndotx / max(ndotx * (1.0 - k) + k, 0.000001);
}

float GeometrySmith(float ndotv, float ndotl, float roughness)
{
    return GeometrySchlickGGX(ndotv, roughness) * GeometrySchlickGGX(ndotl, roughness);
}

float3 FresnelSchlick(float cosTheta, float3 f0)
{
    return f0 + (1.0 - f0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    float depth = g_depth.Load(int3(input.position.xy, 0));
    
    // If depth is 1.0, it means the pixel is background, so we can skip lighting.
    if (depth >= 1.0)
    {
        return backgroundColor;
    }

    float3 albedo = g_albedo.Sample(g_sampler, input.uv).rgb;
    float3 normal = normalize(g_normal.Sample(g_sampler, input.uv).rgb);
    uint materialId = g_material.Load(int3(input.position.xy, 0));
    Material material = g_materialData[materialId];
    float4 pbrParams = g_pbrParams.Sample(g_sampler, input.uv);
    float metallic = pbrParams.r;
    float roughness = max(pbrParams.g, 0.04);
    float occlusion = pbrParams.b;
    float3 worldPos = ReconstructWorldPosition(input.uv, depth);
    float3 lightDir = normalize(-lightDirection);
    float3 viewDir = normalize(cameraPosition - worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    float ndotl = saturate(dot(normal, lightDir));
    float ndotv = saturate(dot(normal, viewDir));
    float ndoth = saturate(dot(normal, halfDir));
    float vdoth = saturate(dot(viewDir, halfDir));

    float3 ambient = albedo * ambientIntensity * occlusion;
    float receiveLighting = (material.flags & 1) ? 0.0 : 1.0;
    float3 diffuse = albedo * lightColor * ndotl * diffuseIntensity * (1.0 - metallic) * receiveLighting;
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 fresnel = FresnelSchlick(vdoth, f0);
    float distribution = DistributionGGX(ndoth, roughness);
    float geometry = GeometrySmith(ndotv, ndotl, roughness);
    float3 specularBrdf = distribution * geometry * fresnel / max(4.0 * ndotv * ndotl, 0.0001);
    float3 specular = lightColor * diffuseIntensity * specularBrdf * ndotl * receiveLighting;
    return float4(ambient + diffuse + specular, 1.0);
}
