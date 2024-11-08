Texture3D<float4> IrradianceTexture : register(t0);
Texture3D<float> DepthTexture : register(t1);
Texture2D<float4> IrradianceAtlas : register(t2);
Texture2D<float> DepthAtlas : register(t3);
SamplerState LinearSampler : register(s0);

cbuffer VisualizationData : register(b0)
{
    int SliceIndex;         
    int DepthDimension;     
    float MaxDepthDistance; 
    int AtlasColumns;
    int AtlasRows;
    float CellSize;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    // int2 atlasCoord : TEXCOORD1; // Grid coordinates in the atlas for each probe
};

float4 main(VS_OUTPUT input) : SV_Target
{
    // float sliceCoordZ = SliceIndex / float(DepthDimension);
    // float3 sampleCoord = float3(input.texCoord, sliceCoordZ);
    // // float3 sampleCoord = float3(input.texCoord, 0.5);
    // float4 irradiance = IrradianceTexture.SampleLevel(LinearSampler, sampleCoord, 0);
    // float depth = DepthTexture.SampleLevel(LinearSampler, sampleCoord, 0) / MaxDepthDistance;
    // float4 color = irradiance;
    // float4 depthColor = float4(depth, depth, depth, 1.0);
    // // return lerp(depthColor, color, 0.5);
    // return float4(irradiance.rgb, 1.0);
    // // return float4(1, 1, 0, 1);
    // // return color;
    // // float4 debugCoord = float4(sampleCoord, 1.0);
    // // return debugCoord;
    // return depthColor;




    // float cellWidth = 1.0 / AtlasColumns;
    // float cellHeight = 1.0 / AtlasRows;

    // float2 uv = float2(
    //     input.texCoord.x * cellWidth + input.atlasCoord.x * cellWidth,
    //     input.texCoord.y * cellHeight + input.atlasCoord.y * cellHeight
    // );

    // float4 irradiance = IrradianceAtlas.Sample(LinearSampler, uv);

    // // float depth = DepthAtlas.Sample(LinearSampler, uv) / MaxDepthDistance;
    // // float4 depthColor = float4(depth, depth, depth, 1.0);
    // // return lerp(depthColor, irradiance, 0.5);

    // return irradiance;
    // // return float4(1, 0, 0, 1.0);

    return IrradianceAtlas.Sample(LinearSampler, input.texCoord);
}
