#define SAMPLE_SDF 1

static const float PI = 3.14159265f;
static int MAX_MARCHING_STEPS = 512;

cbuffer ProbeData : register(b0) {
    float4x4 RandomRotation;         

    float3 GridSize;  
    float pad0;

    float3 ProbeSpacing;
    float pad1;

    float3 SceneMinBounds;
    float pad2;

    uint ProbeCount;
    uint ProbeAtlasBlockResolution;
    uint GutterSize;
    float MaxWorldDepth;
};

cbuffer SDFData : register(b1) {
    // world space texture bounds
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
    // texture resolution
    float sdfResolution;
};

StructuredBuffer<float4> ProbePositions : register(t0);
Texture2DArray<float4> ProbeCubemapArray : register(t1);

RWTexture2DArray<float4> IrradianceAtlas : register(u0);
RWTexture2DArray<float2> DepthAtlas : register(u1);

RWTexture3D<float4> AlbedoTex : register(u2);
RWTexture3D<float> SDFTex : register(u3);

SamplerState LinearSampler : register(s0);

// --- SDF Helper Functions ---

// assuming that the 3D texture covers a box (in world space) that:
//      * is centered at the origin
//      * Xbounds = [-2000, 2000]
//      * Ybounds = [-2000, 2000]
//      * Zbounds = [-2000, 2000]
float3 WorldSpaceToTextureSpace(float3 worldPos) {
    float3 texCoord = float3(0, 0, 0);

    // world coord to [0, 1] coords
    texCoord.x = (worldPos.x - xmin) / (xmax - xmin);
    texCoord.y = (worldPos.y - ymin) / (ymax - ymin);
    texCoord.z = (worldPos.z - zmin) / (zmax - zmin);

    // assuming a 128 * 128 * 128 texture, but we could make this dynamic
    // u' = u * (tmax - tmin) + tmin
    // where tmax == 127 and tmin == 0
    return texCoord * (sdfResolution - 1);
}

float3 TextureSpaceToWorldSpace(float3 texCoord) {
    // Texture space bounds.
    float tmin = 0.0;
    float tmax = sdfResolution - 1;

    // Normalize texture coordinates to [0, 1].
    float3 normCoord = texCoord / tmax;

    // Map normalized coordinates back to world space.
    float3 worldPos;
    worldPos.x = normCoord.x * (xmax - xmin) + xmin;
    worldPos.y = normCoord.y * (ymax - ymin) + ymin;
    worldPos.z = normCoord.z * (zmax - zmin) + zmin;

    return worldPos;
}

float4 SampleSDFAlbedo(float3 worldPos, float3 marchingDirection, out float3 worldHitPos) {
    float3 eye = WorldSpaceToTextureSpace(worldPos); 

    // Ray March Code
    float start = 0;
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        int3 hit = (eye + depth * marchingDirection);
        if (any(hit > int3(sdfResolution - 1, sdfResolution - 1, sdfResolution - 1)) || any(hit < int3(0, 0, 0))) {
            return float4(0., 0., 0., 1.);
        }
        hit.y = sdfResolution - 1 - hit.y;
        hit.z = sdfResolution - 1 - hit.z;
        float dist = SDFTex[hit];
        if (dist == 0.f) {
            worldHitPos = TextureSpaceToWorldSpace(eye + depth * marchingDirection);
            return AlbedoTex[hit];
        }
        depth += dist;
    }
    return float4(0., 0., 0., 1.);
}

// --- Atlas Helper Functions ---

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

float2 normalized_oct_coord(int2 fragCoord, int probe_side_length) {

    int probe_with_border_side = probe_side_length + 2;
    float2 octahedral_texel_coordinates = int2((fragCoord.x - 1) % probe_with_border_side, (fragCoord.y - 1) % probe_with_border_side);

    octahedral_texel_coordinates += float2(0.5f, 0.5f);
    octahedral_texel_coordinates *= (2.0f / float(probe_side_length));
    octahedral_texel_coordinates -= float2(1.0f, 1.0f);

    return octahedral_texel_coordinates;
}

float sign_not_zero(in float k) {
    return (k >= 0.0) ? 1.0 : -1.0;
}

float2 sign_not_zero2(in float2 v) {
    return float2(sign_not_zero(v.x), sign_not_zero(v.y));
}


float3 oct_decode(float2 o) {
    float3 v = float3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
    if (v.z < 0.0) {
        v.xy = (1.0 - abs(v.yx)) * sign_not_zero2(v.xy);
    }
    return normalize(v);
}

// --- Shader Start ---

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint probeIndex = dispatchThreadID.x
        + dispatchThreadID.y * GridSize.x
        + dispatchThreadID.z * GridSize.x * GridSize.y;

    if (probeIndex >= ProbeCount) return;

    float3 probePosition = ProbePositions[probeIndex].xyz;

    uint3 atlasCoord = uint3(GutterSize, GutterSize, 0) + uint3(
        dispatchThreadID.x * (ProbeAtlasBlockResolution + GutterSize),
        dispatchThreadID.y * (ProbeAtlasBlockResolution + GutterSize),
        dispatchThreadID.z
    );

    const uint sample_count = ProbeAtlasBlockResolution*ProbeAtlasBlockResolution;

    for (uint x = 0; x < ProbeAtlasBlockResolution; ++x) {
        for (uint y = 0; y < ProbeAtlasBlockResolution; ++y) {

            uint i = x + y * ProbeAtlasBlockResolution;

            float3 dir = normalize(mul(RandomRotation, float4(spherical_fibonacci(i, sample_count), 1.0)).xyz);

            int2 coord = int2(x, y);
            float3 texelDirection = oct_decode(normalized_oct_coord(coord, ProbeAtlasBlockResolution));
            float weight = max(0.0, dot(texelDirection, dir));
            weight = 1.0f;

            uint3 probeTexCoord = atlasCoord + uint3(coord, 0.0f);

    #if SAMPLE_SDF
            float3 worldHitPos;
            float4 irradianceSample = SampleSDFAlbedo(probePosition, normalize(texelDirection+dir), worldHitPos);
            IrradianceAtlas[probeTexCoord] = weight * irradianceSample;
            float worldDepth = min(length(worldHitPos - probePosition), MaxWorldDepth);
            DepthAtlas[probeTexCoord] = float2(worldDepth, worldDepth*worldDepth);
    #else
            int faceIndex = GetFaceIndex(dir);
            uint textureIndex = probeIndex * 6 + faceIndex;
            float4 irradianceSample = ProbeCubemapArray.SampleLevel(LinearSampler, float3(coord.xy * 0.5 + 0.5, textureIndex), 0);
            IrradianceAtlas[probeTexCoord] = weight * irradianceSample;
            DepthAtlas[probeTexCoord] = 1;
    #endif
        }
    }
}
