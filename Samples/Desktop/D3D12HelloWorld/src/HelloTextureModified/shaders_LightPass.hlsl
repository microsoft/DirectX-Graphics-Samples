struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> g_albedo : register(t0, space3);
Texture2D<float4> g_normal : register(t1, space3);
Texture2D<uint> g_material : register(t2, space3);
Texture2D<float> g_depth : register(t4, space3);
SamplerState g_sampler : register(s0);

cbuffer LightingConstants : register(b2)
{
    float3 lightDirection;
    float ambientIntensity;
    float3 lightColor;
    float diffuseIntensity;
    float4 backgroundColor;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    VSOutput output;

    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2(-1.0, 3.0),
        float2(3.0, -1.0),
    };
    float2 uvs[3] = {
        float2(0.0, 1.0),
        float2(0.0, -1.0),
        float2(2.0, 1.0),
    };

    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.uv = uvs[vertexId];
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float depth = g_depth.Load(int3(input.position.xy, 0));
    
    // If depth is 1.0, it means the pixel is background, so we can skip lighting.
    if (depth >= 1.0)
    {
        return backgroundColor;
    }

    float3 albedo = g_albedo.Sample(g_sampler, input.uv).rgb;
    float3 normal = normalize(g_normal.Sample(g_sampler, input.uv).rgb);
    float3 lightDir = normalize(lightDirection);
    float ndotl = saturate(dot(normal, -lightDir));

    float3 ambient = albedo * ambientIntensity;
    float3 diffuse = albedo * lightColor * ndotl * diffuseIntensity;
    return float4(ambient + diffuse, 1.0);
}
