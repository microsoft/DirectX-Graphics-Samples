cbuffer CameraData : register(b0)
{
    matrix ViewProjectionMatrix;
}

// Position (xyz) and irradiance (w).
StructuredBuffer<float4> ProbeBuffer : register(t0);

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float intensity : COLOR;
};

VS_OUTPUT main(uint id : SV_VertexID) {
    float4 probeData = ProbeBuffer[id];    
    VS_OUTPUT output;
    output.pos = mul(ViewProjectionMatrix, float4(probeData.xyz, 1.0));
    output.intensity = probeData.w;
    return output;
}
