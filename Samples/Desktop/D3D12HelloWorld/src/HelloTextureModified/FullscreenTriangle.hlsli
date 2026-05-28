struct FullscreenVSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

FullscreenVSOutput FullscreenTriangleVS(uint vertexId)
{
    FullscreenVSOutput output;

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
