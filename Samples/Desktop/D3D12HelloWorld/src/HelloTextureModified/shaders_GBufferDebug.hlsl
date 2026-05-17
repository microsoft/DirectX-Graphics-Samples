struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D g_gbuffer : register(t0, space3);
SamplerState g_sampler : register(s0);

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
    return g_gbuffer.Sample(g_sampler, input.uv);
}
