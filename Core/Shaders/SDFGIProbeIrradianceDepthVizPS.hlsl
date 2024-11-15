Texture2DArray<float4> IrradianceAtlas : register(t0);
Texture2DArray<float> DepthAtlas : register(t1);
SamplerState LinearSampler : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    return IrradianceAtlas.SampleLevel(LinearSampler, float3(input.texCoord, /*depth_index=*/5), 0);
}
