Texture2D<float4> CubemapFaces[6] : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer GridConfig : register(b0) {
    int GridColumns;
    int GridRows; 
    float CellSize;      
};

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target {
    int faceIndex = int(input.texCoord.y * GridRows) * GridColumns + int(input.texCoord.x * GridColumns);
    
    if (faceIndex >= 6) return float4(0, 0, 0, 1);

    return CubemapFaces[faceIndex].Sample(LinearSampler, input.texCoord);
    // return float4(1, 0, 0, 1);
}
