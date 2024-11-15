struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID) {
    //VSOutput output;

    //// Using bitwise operations for a triangle strip
    //// vertexID:     0    1    2    3
    //// x position:  -1   -1    1    1
    //// y position:   1   -1    1   -1
    //output.position = float4(
    //    (float)(vertexID & 2) * 1.0f - 1.0f,   // -1, -1, 1, 1
    //    (float)((vertexID & 1) * -2.0f + 1),   // 1, -1, 1, -1
    //    0.0f,
    //    1.0f
    //);

    //output.texcoord = float2(
    //    (float)(vertexID & 2) * 0.5f,          // 0, 0, 1, 1
    //    (float)(vertexID & 1) * 1.0f           // 0, 1, 0, 1
    //);

    //return output;

    VSOutput output;
    output.uv = float2(uint2(vertexID, vertexID << 1) & 2);
    output.pos = float4(lerp(float2(-1, 1), float2(1, -1), output.uv), 0, 1);
    return output;
}