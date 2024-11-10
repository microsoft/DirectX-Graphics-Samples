struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    uint faceIndex : SV_RenderTargetArrayIndex;
};

float4 main(VS_OUTPUT input) : SV_Target {
    float3 colors[6] = {
        float3(0.0, 0.2, 0.0),
        float3(0.0, 0.0, 0.2),
        float3(0.2, 0.2, 0.0),
        float3(0.2, 0.0, 0.2),
        float3(0.0, 0.2, 0.2),
        float3(0.2, 0.0, 0.0) 
    };

    
    float3 faceColor = colors[input.faceIndex % 6];
    return float4(faceColor, 1.0);
}
