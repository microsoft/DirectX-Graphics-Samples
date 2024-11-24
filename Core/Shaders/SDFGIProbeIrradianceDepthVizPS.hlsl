Texture2DArray<float4> IrradianceAtlas : register(t0);
Texture2DArray<float2> DepthAtlas : register(t1);
SamplerState LinearSampler : register(s0);

cbuffer ProbeData : register(b0) {
    float MaxWorldDepth;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    return IrradianceAtlas.SampleLevel(LinearSampler, float3(input.texCoord, /*depth_index=*/0), 0);

    // float worldDepth = DepthAtlas.SampleLevel(LinearSampler, float3(input.texCoord, /*depth_index=*/0), 0).r;
    // float normalizedDepth = clamp((worldDepth - 0.1f) / (MaxWorldDepth - 0.1f), 0.0f, 1.0f);
    // return normalizedDepth;
}
