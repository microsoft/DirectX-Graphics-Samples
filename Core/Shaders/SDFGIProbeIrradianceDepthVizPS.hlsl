Texture3D<float4> IrradianceTexture : register(t0);
Texture3D<float> DepthTexture : register(t1);
Texture3D<float4> IrradianceAtlas : register(t2);
Texture3D<float> DepthAtlas : register(t3);
SamplerState LinearSampler : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    return IrradianceAtlas.Sample(LinearSampler, float3(input.texCoord, 1.0f));
}
