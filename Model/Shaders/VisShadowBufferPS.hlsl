Texture2D<float4> depthTexture : register(t0);     // Depth texture (SRV in slot t0)
SamplerState samplerState : register(s0);  // Static sampler (Sampler in slot s0)

struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET{
    //// Sample the depth from the depth texture
float depth = depthTexture.Sample(samplerState, input.uv).r;
//// Return grayscale color based on depth
return float4(depth, depth, depth, 1.0);
    //return float4(0.0f, input.uv.x, input.uv.y, 1.0f); // Red color
}