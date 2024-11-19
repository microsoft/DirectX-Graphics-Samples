struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

VS_OUTPUT main(uint id : SV_VertexID)
{
    VS_OUTPUT output;

    float2 quadVertices[4] = {
        float2(-1.0, -1.0),
        float2( 1.0, -1.0),
        float2(-1.0,  1.0),
        float2( 1.0,  1.0) 
    };

    float2 texCoords[4] = {
        float2(0.0, 1.0), 
        float2(1.0, 1.0), 
        float2(0.0, 0.0), 
        float2(1.0, 0.0)  
    };

    output.pos = float4(quadVertices[id], 0.0, 1.0);
    output.texCoord = texCoords[id];

    return output;
}
