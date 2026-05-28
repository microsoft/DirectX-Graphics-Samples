#include "FullscreenTriangle.hlsli"

Texture2D<float4> g_hdrSceneColor : register(t0, space4);
SamplerState g_sampler : register(s0);

cbuffer ToneMapConstants : register(b3)
{
    uint toneMapOperator; // 0: None, 1: Reinhard, 2: ACES
    float exposure;
    float paperWhiteNits;
    float maxDisplayNits;
};

FullscreenVSOutput VSMain(uint vertexId : SV_VertexID)
{
    return FullscreenTriangleVS(vertexId);
}

float3 ReinhardToneMap(float3 color)
{
    return color / (1.0 + color);
}

float3 AcesToneMap(float3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

float3 ApplyToneMap(float3 hdrColor)
{
    float3 exposed = max(hdrColor, 0.0) * exp2(exposure);

    if (toneMapOperator == 1)
    {
        return ReinhardToneMap(exposed);
    }
    if (toneMapOperator == 2)
    {
        return AcesToneMap(exposed);
    }

    return exposed;
}

float3 LinearToSt2084Pq(float3 nits)
{
    const float m1 = 2610.0 / 16384.0;
    const float m2 = 2523.0 / 32.0;
    const float c1 = 3424.0 / 4096.0;
    const float c2 = 2413.0 / 128.0;
    const float c3 = 2392.0 / 128.0;

    float3 normalizedNits = saturate(nits / 10000.0);
    float3 n = pow(normalizedNits, m1);
    return pow((c1 + c2 * n) / (1.0 + c3 * n), m2);
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    float4 hdr = g_hdrSceneColor.Sample(g_sampler, input.uv);
    float3 mapped = ApplyToneMap(hdr.rgb);

    float outputMaxNits = max(maxDisplayNits, 1.0);
    float referenceWhiteNits = max(paperWhiteNits, 1.0);
    float3 nits = toneMapOperator == 0 ? mapped * referenceWhiteNits : saturate(mapped) * outputMaxNits;
    float3 pq = LinearToSt2084Pq(nits);

    return float4(pq, hdr.a);
}
