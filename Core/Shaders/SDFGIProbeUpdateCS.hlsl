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
RWTexture2D<float4> IrradianceAtlas : register(u2);
RWTexture2D<float> DepthAtlas : register(u3);

float2 signNotZero(float2 v) {
    return float2((v.x >= 0.0 ? 1.0 : -1.0), (v.y >= 0.0 ? 1.0 : -1.0));
}

// See https://github.com/RomkoSI/G3D/blob/master/data-files/shader/octahedral.glsl.
float2 octEncode(float3 v) {
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    float2 result = v.xy * (1.0 / l1norm);
    
    if (v.z < 0.0) {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    
    return result;
}

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

    float2 octEncodedDirections[6] = {
        octEncode(float3(1, 0, 0)), octEncode(float3(-1, 0, 0)),
        octEncode(float3(0, 1, 0)), octEncode(float3(0, -1, 0)),
        octEncode(float3(0, 0, 1)), octEncode(float3(0, 0, -1))
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

    uint probeBlockSize = 4;
    uint gutterSize = 1;

    uint2 atlasCoord = uint2(
        dispatchThreadID.x * (probeBlockSize + gutterSize) + gutterSize,
        dispatchThreadID.y * (probeBlockSize + gutterSize) + gutterSize
    );

    float4 irradianceSample = float4(dispatchThreadID, 1.0);
    float depthSample = 0.5;

    for (uint i = 0; i < probeBlockSize; i++) {
        for (uint j = 0; j < probeBlockSize; j++) {
            uint2 pixelCoord = atlasCoord + uint2(i, j);
            IrradianceAtlas[pixelCoord] = irradianceSample;
            DepthAtlas[pixelCoord] = depthSample;
        }
    }
}
