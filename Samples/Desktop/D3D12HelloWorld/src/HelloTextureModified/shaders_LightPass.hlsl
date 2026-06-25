#include "FullscreenTriangle.hlsli"
#include "Material.hlsli"

Texture2D<float4> g_albedo : register(t0, space3);
Texture2D<float4> g_normal : register(t1, space3);
Texture2D<uint> g_material : register(t2, space3);
Texture2D<float4> g_pbrParams : register(t4, space3);
Texture2D<float> g_depth : register(t5, space3);
TextureCube<float4> g_environmentMap : register(t0, space5);
TextureCube<float4> g_diffuseIrradianceMap : register(t1, space5);
TextureCube<float4> g_specularPrefilterMap : register(t2, space5);
Texture2D<float2> g_brdfLut : register(t3, space5);
SamplerState g_sampler : register(s0);
Texture2D<float> g_shadowMask : register(t0, space4);
StructuredBuffer<Material> g_materialData : register(t0, space2);

static const float PI = 3.14159265;
static const float SPECULAR_PREFILTER_MAX_MIP = 5.0;

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
    float iblIntensity;
    float3 lightColor;
    float diffuseIntensity;
    float4 backgroundColor;
    float skyboxEnabled;
    float skyboxPreview;
    float skyboxPreviewExposure;
    float lightPassDebugViewMode;
    float directLightEnabled;
    float diffuseIblEnabled;
    float specularIblEnabled;
    float emissiveEnabled;
    float iblDebugMip;
    float iblDebugExposure;
    float rayTracingSupported;
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

float3 SampleSkybox(float2 uv)
{
    float3 worldPos = ReconstructWorldPosition(uv, 1.0);
    float3 viewDir = normalize(worldPos - cameraPosition);
    return g_environmentMap.Sample(g_sampler, viewDir).rgb * skyboxPreviewExposure;
}

float3 DirectionFromEquirectUv(float2 uv)
{
    float phi = (uv.x * 2.0 - 1.0) * PI;
    float theta = (0.5 - uv.y) * PI;
    float cosTheta = cos(theta);
    return normalize(float3(sin(phi) * cosTheta, sin(theta), cos(phi) * cosTheta));
}

float4 RenderIblDebugView(float2 uv)
{
    float3 direction = DirectionFromEquirectUv(uv);

    if (lightPassDebugViewMode > 4.5 && lightPassDebugViewMode < 5.5)
    {
        float3 color = g_environmentMap.Sample(g_sampler, direction).rgb * iblDebugExposure;
        return float4(color / (1.0 + color), 1.0);
    }

    if (lightPassDebugViewMode >= 5.5 && lightPassDebugViewMode < 6.5)
    {
        float3 color = g_diffuseIrradianceMap.Sample(g_sampler, direction).rgb * iblDebugExposure;
        return float4(color / (1.0 + color), 1.0);
    }

    if (lightPassDebugViewMode >= 6.5 && lightPassDebugViewMode < 7.5)
    {
        float3 color = g_specularPrefilterMap.SampleLevel(g_sampler, direction, iblDebugMip).rgb * iblDebugExposure;
        return float4(color / (1.0 + color), 1.0);
    }

    float2 brdf = g_brdfLut.Sample(g_sampler, saturate(uv)).rg;
    return float4(brdf.r, brdf.g, 0.0, 1.0);
}

float DistributionGGX(float ndoth, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = ndoth * ndoth * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, 0.000001);
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

float3 FresnelSchlickRoughness(float cosTheta, float3 f0, float roughness)
{
    return f0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), f0) - f0) *
                    pow(saturate(1.0 - cosTheta), 5.0);
}

float ComputeSpecularOcclusion(float ndotv, float ambientOcclusion, float roughness)
{
    return saturate(pow(ndotv + ambientOcclusion, exp2(-16.0 * roughness - 1.0)) - 1.0 + ambientOcclusion);
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    float depth = g_depth.Load(int3(input.position.xy, 0));

    if (lightPassDebugViewMode > 4.5)
    {
        return RenderIblDebugView(input.uv);
    }
    
    if (skyboxPreview > 0.5)
    {
        return float4(SampleSkybox(input.uv), backgroundColor.a);
    }

    // If depth is 1.0, it means the pixel is background, so we can skip lighting.
    if (depth >= 1.0)
    {
        if (lightPassDebugViewMode > 0.5)
        {
            return float4(0.02, 0.04, 0.07, backgroundColor.a);
        }

        if (skyboxEnabled > 0.5)
        {
            return float4(SampleSkybox(input.uv), backgroundColor.a);
        }

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
    float emissive = pbrParams.a;
    float3 worldPos = ReconstructWorldPosition(input.uv, depth);
    float3 lightDir = normalize(-lightDirection);
    float3 viewDir = normalize(cameraPosition - worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    float ndotl = saturate(dot(normal, lightDir));
    float ndotv = saturate(dot(normal, viewDir));
    float ndoth = saturate(dot(normal, halfDir));
    float vdoth = saturate(dot(viewDir, halfDir));

    float receiveLighting = (material.flags & MaterialFlagUnlit) ? 0.0 : 1.0;
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 fresnel = FresnelSchlick(vdoth, f0);
    float distribution = DistributionGGX(ndoth, roughness);
    float geometry = GeometrySmith(ndotv, ndotl, roughness);
    float3 specularBrdf = distribution * geometry * fresnel / max(4.0 * ndotv * ndotl, 0.0001);
    float3 diffuseBrdf = (1.0 - fresnel) * (1.0 - metallic) * albedo / PI;
    float3 radiance = lightColor * diffuseIntensity;
    float shadowMask = 1.0;
    if (rayTracingSupported)
    {
        shadowMask = g_shadowMask.Sample(g_sampler, input.uv);
    }
    float3 directLighting = (diffuseBrdf + specularBrdf) * radiance * ndotl * receiveLighting * shadowMask * directLightEnabled;
    float3 irradiance = g_diffuseIrradianceMap.Sample(g_sampler, normal).rgb;
    float3 iblDiffuse = irradiance * albedo * (1.0 - metallic) * iblIntensity * occlusion * diffuseIblEnabled / PI;
    float3 reflectionDir = reflect(-viewDir, normal);
    if (lightPassDebugViewMode > 0.5 && lightPassDebugViewMode < 1.5)
    {
        return float4(reflectionDir * 0.5 + 0.5, 1.0);
    }
    if (lightPassDebugViewMode >= 1.5 && lightPassDebugViewMode < 2.5)
    {
        return float4(viewDir * 0.5 + 0.5, 1.0);
    }
    if (lightPassDebugViewMode >= 2.5)
    {
        if (lightPassDebugViewMode < 3.5)
        {
            return float4(saturate(worldPos * 0.05 + 0.5), 1.0);
        }

        return float4(ndotv.xxx, 1.0);
    }

    float specularMip = roughness * SPECULAR_PREFILTER_MAX_MIP;
    float3 environmentSpecular = g_specularPrefilterMap.SampleLevel(g_sampler, reflectionDir, specularMip).rgb;
    float3 specularFresnel = FresnelSchlickRoughness(ndotv, f0, roughness);
    float2 brdf = g_brdfLut.Sample(g_sampler, float2(ndotv, roughness)).rg;
    float specularOcclusion = ComputeSpecularOcclusion(ndotv, occlusion, roughness);
    float3 iblSpecular =
        environmentSpecular * (specularFresnel * brdf.x + brdf.y) * iblIntensity * specularOcclusion * specularIblEnabled;
    return float4(iblDiffuse + iblSpecular + directLighting + emissive.xxx * emissiveEnabled, 1.0);
}
