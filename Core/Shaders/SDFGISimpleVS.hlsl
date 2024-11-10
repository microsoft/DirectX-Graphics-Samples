cbuffer FaceIndexCB : register(b0)
{
    uint faceIndex; 
}

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    uint faceIndex : SV_RenderTargetArrayIndex;
};

VS_OUTPUT main(uint id : SV_VertexID) {
    VS_OUTPUT output;
    float2 quadVertices[4] = {
        float2(-1.0, -1.0), // Bottom-left
        float2(1.0, -1.0),  // Bottom-right
        float2(-1.0, 1.0),  // Top-left
        float2(1.0, 1.0)    // Top-right
    };
    output.pos = float4(quadVertices[id], 0.0, 1.0);
    output.texCoord = quadVertices[id] * 0.5 + 0.5; // Transform to [0,1] range
    output.faceIndex = faceIndex; // Set face index
    return output;
}
