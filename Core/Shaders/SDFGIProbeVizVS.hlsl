cbuffer CameraData : register(b0)
{
    matrix viewProjMatrix;
    float3 cameraPos;
};

// Probe positions.
StructuredBuffer<float3> ProbeBuffer : register(t0);

struct VS_OUTPUT {
    // Output world position to geometry shader.
    float4 worldPos : WORLD_POSITION;
};

VS_OUTPUT main(uint id : SV_VertexID) {
    float3 probeData = ProbeBuffer[id];

    VS_OUTPUT output;
    output.worldPos = float4(probeData, 1.0);
    
    return output;
}
