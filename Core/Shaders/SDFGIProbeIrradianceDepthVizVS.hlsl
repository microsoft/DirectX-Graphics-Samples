struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    // int2 atlasCoord : TEXCOORD1;
};

cbuffer VisualizationData : register(b0)
{
    int SliceIndex;         
    int DepthDimension;     
    float MaxDepthDistance; 
    int AtlasColumns;
    int AtlasRows;
    float CellSize;
};

VS_OUTPUT main(uint id : SV_VertexID)
{
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


    // int probeIndex = id / 4;
    // // int vertexIndex = id % 4;

    // int atlasX = probeIndex % AtlasColumns;
    // int atlasY = probeIndex / AtlasColumns;

    // // float2 cellCenter = float2(
    // //     -1.0 + (2.0 * atlasX + 1) * CellSize,
    // //     -1.0 + (2.0 * atlasY + 1) * CellSize
    // // );

    // // output.pos = float4(cellCenter + quadVertices[vertexIndex] * CellSize, 0.0, 1.0);
    // // output.texCoord = texCoords[vertexIndex];
    // output.atlasCoord = int2(atlasX, atlasY);

    return output;
}
