#include "FullscreenTriangle.hlsli"

Texture2D<float4> g_albedo : register(t0, space3);
Texture2D<float4> g_normal : register(t1, space3);
Texture2D<uint> g_material : register(t2, space3);
Texture2D<float2> g_motionVector : register(t3, space3);
Texture2D<float4> g_pbrParams : register(t4, space3);
Texture2D<float> g_depth : register(t5, space3);
SamplerState g_sampler : register(s0);

cbuffer GBufferDebugConstants : register(b1)
{
    uint g_debugTarget;
};

float3 HsvToRgb(float3 hsv)
{
    float3 p = abs(frac(hsv.xxx + float3(0.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
    return hsv.z * lerp(float3(1.0, 1.0, 1.0), saturate(p - 1.0), hsv.y);
}

FullscreenVSOutput VSMain(uint vertexId : SV_VertexID)
{
    return FullscreenTriangleVS(vertexId);
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    if (g_debugTarget == 0)
    {
        return g_albedo.Sample(g_sampler, input.uv);
    }
    if (g_debugTarget == 1)
    {
        float3 normal = normalize(g_normal.Sample(g_sampler, input.uv).rgb);
        return float4(normal * 0.5 + 0.5, 1.0);
    }
    if (g_debugTarget == 2)
    {
        uint materialId = g_material.Load(int3(input.position.xy, 0));
        float value = (float) materialId / 255.0;
        
        return float4(value, value, value, 1.0);        
    }
    if (g_debugTarget == 3)
    {
        float depth = g_depth.Load(int3(input.position.xy, 0));
        if (depth >= 1.0)
        {
            return float4(0.0, 0.0, 0.0, 1.0);
        }

        float2 value = g_motionVector.Load(int3(input.position.xy, 0)).rg;
        float magnitude = saturate(length(value) * 200.0);
        if (magnitude <= 0.0)
        {
            return float4(0.0, 0.0, 0.0, 1.0);
        }

        float hue = atan2(value.y, value.x) / 6.2831853 + 0.5;
        return float4(HsvToRgb(float3(hue, 1.0, magnitude)), 1.0);
    }
    if (g_debugTarget == 4)
    {
        float3 value = g_pbrParams.Sample(g_sampler, input.uv).rgb;
        return float4(value, 1.0);
    }
    if (g_debugTarget == 5)
    {
        float value = g_depth.Sample(g_sampler, input.uv);
        return float4(value, value, value, 1.0);
    }

    return float4(1.0, 0.0, 0.0, 1.0); //erroro : アップル
}
