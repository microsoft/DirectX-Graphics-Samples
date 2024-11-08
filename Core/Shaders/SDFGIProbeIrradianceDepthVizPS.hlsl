Texture3D<float4> IrradianceTexture : register(t0);
Texture3D<float> DepthTexture : register(t1);
SamplerState LinearSampler : register(s0);

cbuffer VisualizationData : register(b0)
{
    int SliceIndex;         
    int DepthDimension;     
    float MaxDepthDistance; 
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    float sliceCoordZ = SliceIndex / float(DepthDimension);

    float3 sampleCoord = float3(input.texCoord, sliceCoordZ);
    // float3 sampleCoord = float3(input.texCoord, 0.5);
    float4 irradiance = IrradianceTexture.SampleLevel(LinearSampler, sampleCoord, 0);
    float depth = DepthTexture.SampleLevel(LinearSampler, sampleCoord, 0) / MaxDepthDistance;

    float4 color = irradiance;
    
    float4 depthColor = float4(depth, depth, depth, 1.0);
    // return lerp(depthColor, color, 0.5);
    return float4(irradiance.rgb, 1.0);
    // return float4(1, 1, 0, 1);
    // return color;

    // float4 debugCoord = float4(sampleCoord, 1.0);
    // return debugCoord;
    return depthColor;
}
