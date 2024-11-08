cbuffer CameraData : register(b0)
{
    matrix viewProjMatrix;
    float3 cameraPos;
};

// Position (xyz) and irradiance (w).
StructuredBuffer<float4> ProbeBuffer : register(t0);

struct VS_OUTPUT {
    // Output world position to geometry shader.
    float4 worldPos : WORLD_POSITION;
    float intensity : COLOR;
};

VS_OUTPUT main(uint id : SV_VertexID) {
    float4 probeData = ProbeBuffer[id];

    VS_OUTPUT output;
    output.worldPos = float4(probeData.xyz, 1.0);
    output.intensity = probeData.w;
    
    return output;
}
