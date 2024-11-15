static const float PI = 3.14159265f;

cbuffer ProbeData : register(b0) {
    float4x4 RandomRotation;         // 64 bytes

    float3 GridSize;                 // 12 bytes
    uint ProbeCount;                 // 4 bytes

    float3 ProbeSpacing;             // 12 bytes
    float ProbeMaxDistance;          // 4 bytes

    float3 SceneMinBounds;           // 12 bytes
    uint ProbeAtlasBlockResolution;  // 4 bytes

    uint GutterSize;                 // 4 bytes
    uint Padding[3];                 // 12 bytes of padding
};

StructuredBuffer<float4> ProbePositions : register(t0);
Texture2DArray<float4> ProbeCubemapArray : register(t1);

RWTexture2DArray<float4> IrradianceAtlas : register(u0);
RWTexture2DArray<float> DepthAtlas : register(u1);

SamplerState LinearSampler : register(s0);

float2 signNotZero(float2 v) {
    return float2((v.x >= 0.0 ? 1.0 : -1.0), (v.y >= 0.0 ? 1.0 : -1.0));
}

float2 octEncode(float3 v) {
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    float2 result = v.xy * (1.0 / l1norm);
    
    if (v.z < 0.0) {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    
    return result;
}

int GetFaceIndex(float3 dir)
{
    float3 absDir = abs(dir);

    if (absDir.x > absDir.y && absDir.x > absDir.z)
    {
        // X component is largest
        return dir.x > 0 ? 0 : 1; // +X or -X
    }
    else if (absDir.y > absDir.x && absDir.y > absDir.z)
    {
        // Y component is largest
        return dir.y > 0 ? 2 : 3; // +Y or -Y
    }
    else
    {
        // Z component is largest
        return dir.z > 0 ? 4 : 5; // +Z or -Z
    }
}

float3 spherical_fibonacci(uint index, uint sample_count) {
    const float PHI = sqrt(5.0) * 0.5 + 0.5;
    float phi = 2.0 * PI * frac(index * (PHI - 1));
    float cos_theta = 1.0 - (2.0 * index + 1.0) / sample_count;
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint probeIndex = dispatchThreadID.x 
                + dispatchThreadID.y * GridSize.x 
                + dispatchThreadID.z * GridSize.x * GridSize.y;

    if (probeIndex >= ProbeCount) return;

    float3 probePosition = ProbePositions[probeIndex].xyz;

    // Shadows the ones in the cbuffer, which are unreadable for some reason.
    uint ProbeAtlasBlockResolution = 8;
    uint GutterSize = 1;

    uint3 atlasCoord = uint3(
        dispatchThreadID.x * (ProbeAtlasBlockResolution + GutterSize),
        dispatchThreadID.y * (ProbeAtlasBlockResolution + GutterSize),
        dispatchThreadID.z
    );

    // At each of the m active probes, we uniformly sample n spherical directions according to a stochastically-rotated Fibonacci spiral pattern.
    // . For a in-depth discussion of irradiance and computing irradiance using light probes, we refer readers to [Akenine-Moller et al. 2018 ¨ ] (pg.268,490).
    float3 sampleDirections[16] = {
        normalize(float3(1,  1,  0)), normalize(float3(-1,  1,  0)), normalize(float3(1, -1,  0)), normalize(float3(-1, -1,  0)),
        normalize(float3(1,  0,  1)), normalize(float3(-1,  0,  1)), normalize(float3(1,  0, -1)), normalize(float3(-1,  0, -1)),
        normalize(float3(0,  1,  1)), normalize(float3(0, -1,  1)), normalize(float3(0,  1, -1)), normalize(float3(0, -1, -1)),
        float3(1, 0, 0), float3(-1, 0, 0), float3(0, 1, 0), float3(0, 0, 1)
    };

    const uint sample_count = 64;

    for (uint i = 0; i < sample_count; ++i) {
        float3 dir = normalize(mul(RandomRotation, float4(spherical_fibonacci(i, sample_count), 1.0)).xyz);
        float2 encodedCoord = octEncode(dir);
        uint3 probeTexCoord = atlasCoord + uint3(
            (encodedCoord.x * 0.5 + 0.5) * (ProbeAtlasBlockResolution - GutterSize),
            (encodedCoord.y * 0.5 + 0.5) * (ProbeAtlasBlockResolution - GutterSize),
            0.0f
        );

        int faceIndex = GetFaceIndex(dir);

        uint textureIndex = probeIndex * 6 + faceIndex;

        // TODO: sample SDF for color and depth in direction 'dir'.

        // float4 irradianceSample = ProbeFaceTextures[faceIndex].SampleLevel(LinearSampler, encodedCoord, 0);
        float4 irradianceSample = ProbeCubemapArray.SampleLevel(LinearSampler, float3(encodedCoord.xy, textureIndex), 0);
        
        IrradianceAtlas[probeTexCoord] = irradianceSample;
        DepthAtlas[probeTexCoord] = length(probePosition - (probePosition + sampleDirections[i] * ProbeMaxDistance)) / ProbeMaxDistance;
    }
}
