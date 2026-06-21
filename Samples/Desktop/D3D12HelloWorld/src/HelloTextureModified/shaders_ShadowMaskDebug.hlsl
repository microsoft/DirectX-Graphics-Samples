#include "FullscreenTriangle.hlsli"

Texture2D<float> g_shadowMask : register(t0, space4);
SamplerState g_sampler : register(s0);

FullscreenVSOutput VSMain(uint vertexId : SV_VertexID)
{
    return FullscreenTriangleVS(vertexId);
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    float shadow = g_shadowMask.Sample(g_sampler, input.uv);
    return float4(shadow, shadow, shadow, 1.0);
}
