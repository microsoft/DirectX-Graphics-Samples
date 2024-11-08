cbuffer ProbeData : register(b0) {
    uint ProbeCount;
    float ProbeMaxDistance;
    float3 GridSize;
    float3 ProbeSpacing;
    float3 SceneMinBounds;
};

StructuredBuffer<float4> ProbePositions : register(t0);
RWTexture3D<float4> IrradianceTexture : register(u0);
RWTexture3D<float> DepthTexture : register(u1);

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint probeIndex = dispatchThreadID.x 
                + dispatchThreadID.y * GridSize.x 
                + dispatchThreadID.z * GridSize.x * GridSize.y;

    if (probeIndex >= ProbeCount) return;

    float3 probePosition = ProbePositions[probeIndex].xyz;

    float4 accumulatedIrradiance = float4(0, 0, 0, 0);
    float accumulatedDepth = 0;

    float3 directions[6] = {
        float3(1, 0, 0), float3(-1, 0, 0),
        float3(0, 1, 0), float3(0, -1, 0),
        float3(0, 0, 1), float3(0, 0, -1)
    };

    for (int i = 0; i < 6; ++i) {
        float3 sampleDir = directions[i];
        float3 samplePos = probePosition + sampleDir * ProbeMaxDistance;

        float4 sampleIrradiance = float4(dispatchThreadID, 1.0);
        float sampleDepth = length(samplePos - probePosition) / ProbeMaxDistance;

        accumulatedIrradiance += sampleIrradiance;
        accumulatedDepth += sampleDepth;
    }

    accumulatedIrradiance /= 6;
    accumulatedDepth /= 6;

    uint3 probeCoord = uint3(
        probeIndex % GridSize.x,
        (probeIndex / GridSize.x) % GridSize.y,
        probeIndex / (GridSize.x * GridSize.y)
    );

    IrradianceTexture[probeCoord] = accumulatedIrradiance;
    DepthTexture[probeCoord] = accumulatedDepth;
}
