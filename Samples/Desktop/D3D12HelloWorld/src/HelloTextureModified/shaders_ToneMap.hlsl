#include "FullscreenTriangle.hlsli"

Texture2D<float4> g_hdrSceneColor : register(t0, space4);
SamplerState g_sampler : register(s0);

cbuffer ToneMapConstants : register(b3)
{
    uint toneMapOperator; // 0: None, 1: Reinhard, 2: ACES
    uint transferFunction; // 0: Linear, 1: ST.2084 PQ, 2: HLG
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

float3 ExpandToneMappedRange(float3 toneMapped, float3 toneMappedWhite, float peakDisplayLinear)
{
    // Reinhard/ACES curves return 0..1 display-linear values. Remap them so
    // exposed 1.0 becomes paper white (1.0), while highlights can reach the
    // display peak expressed as maxDisplayNits / paperWhiteNits.
    // Reinhard/ACES は 0..1 の display-linear 値を返すため、exposed 1.0 が
    // paper white (1.0) になり、ハイライトが maxDisplayNits / paperWhiteNits
    // まで伸びるように再マップする。
    float3 white = max(toneMappedWhite, 0.000001);
    float3 belowWhite = toneMapped / white;
    float3 aboveWhite = 1.0 + (toneMapped - white) * (peakDisplayLinear - 1.0) / max(1.0 - white, 0.000001);
    return min(max(lerp(aboveWhite, belowWhite, toneMapped <= white), 0.0), peakDisplayLinear);
}

float3 ApplyToneMap(float3 hdrColor)
{
    float3 exposed = max(hdrColor, 0.0) * max(exposure, 0.0);
    float referenceWhiteNits = max(paperWhiteNits, 1.0);
    float outputMaxNits = max(maxDisplayNits, referenceWhiteNits);
    float peakDisplayLinear = outputMaxNits / referenceWhiteNits;

    if (toneMapOperator == 1)
    {
        return ExpandToneMappedRange(ReinhardToneMap(exposed), ReinhardToneMap(1.0), peakDisplayLinear);
    }
    if (toneMapOperator == 2)
    {
        return ExpandToneMappedRange(AcesToneMap(exposed), AcesToneMap(1.0), peakDisplayLinear);
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

float3 LinearToHlg(float3 normalizedLinear)
{
    const float a = 0.17883277;
    const float b = 0.28466892;
    const float c = 0.55991073;

    normalizedLinear = saturate(normalizedLinear);
    float3 low = sqrt(3.0 * normalizedLinear);
    float3 high = a * log(max(12.0 * normalizedLinear - b, 0.000001)) + c;
    return lerp(high, low, normalizedLinear <= 1.0 / 12.0);
}

float3 ToneMappedLinearToNits(float3 mapped)
{
    float outputMaxNits = max(maxDisplayNits, 1.0);
    float referenceWhiteNits = max(paperWhiteNits, 1.0);

    // Tone mapping returns HDR display-linear values where 1.0 is paper white.
    // Convert that paper-white-relative value to absolute display luminance.
    // Tone mapping は 1.0 を paper white とする HDR display-linear 値を返す。
    // その paper white 基準の値を、絶対輝度 nits に変換する。
    float3 nits = mapped * referenceWhiteNits;
    return min(max(nits, 0.0), outputMaxNits);
}

float3 ApplyTransferFunction(float3 nits)
{
    float outputMaxNits = max(maxDisplayNits, 1.0);
    float referenceWhiteNits = max(paperWhiteNits, 1.0);

    if (transferFunction == 1)
    {
        return LinearToSt2084Pq(nits);
    }
    if (transferFunction == 2)
    {
        return LinearToHlg(nits / outputMaxNits);
    }

    // Linear output keeps HDR headroom in the float render target:
    // 1.0 is paper white, and values above 1.0 are brighter highlights.
    // Linear 出力では float render target 上に HDR headroom を残す:
    // 1.0 が paper white で、1.0 を超える値はより明るいハイライト。
    return max(nits, 0.0) / referenceWhiteNits;
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    float4 hdr = g_hdrSceneColor.Sample(g_sampler, input.uv);
    float3 mapped = ApplyToneMap(hdr.rgb);

    float3 nits = ToneMappedLinearToNits(mapped);
    float3 outputColor = ApplyTransferFunction(nits);

    return float4(outputColor, hdr.a);
}
