struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

// Fullscreen quad.
VS_OUTPUT main(uint id : SV_VertexID) {
    VS_OUTPUT output;

    float2 quadVertices[4] = {
        float2(-1.0, -1.0), // Bottom-left
        float2( 1.0, -1.0), // Bottom-right
        float2(-1.0,  1.0), // Top-left
        float2( 1.0,  1.0)  // Top-right
    };

    float2 texCoords[4] = {
        float2(0.0, 1.0), // Bottom-left
        float2(1.0, 1.0), // Bottom-right
        float2(0.0, 0.0), // Top-left
        float2(1.0, 0.0)  // Top-right
    };

    output.pos = float4(quadVertices[id], 0.0, 1.0);
    output.texCoord = texCoords[id];

    return output;
}
