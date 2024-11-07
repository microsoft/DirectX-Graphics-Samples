struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float intensity : COLOR;
};

float4 main(VS_OUTPUT input) : SV_Target {
    float3 color = float3(input.intensity, input.intensity, input.intensity);
    // return float4(color, 1.0); // RGBA output
    return float4(0.0, 1.0, 0.0, 1.0);
}