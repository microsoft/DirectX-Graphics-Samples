RWTexture2D<float> g_shadowMask : register(u0);
RaytracingAccelerationStructure g_tlas : register(t0);
Texture2D<float> g_depth : register(t1);
Texture2D<float4> g_normal : register(t2);

cbuffer CameraCB : register(b0)
{
    float4x4 viewProj;
    float4x4 prevViewProj;
    float4x4 invViewProj;
    float3 cameraPosition;
    float cbPad;
};

cbuffer ShadowConstants : register(b1)
{
    float3 lightDirection;
    float normalBias;
    float rayTMin;
    float rayTMax;
    uint enabled;
    uint softShadowEnabled;
    uint sampleCount;
    float lightAngularRadius;
    float jitterStrength;
};

// Deterministic per-pixel hash functions for blue-noise-style rotation.
uint HashPixel(uint2 p)
{
    uint state = p.x * 1664525u + p.y * 1013904223u;
    state ^= state >> 16u;
    state *= 0x85ebca6bu;
    state ^= state >> 13u;
    state *= 0xc2b2ae35u;
    state ^= state >> 16u;
    return state;
}

float HashPixel01(uint2 p)
{
    return float(HashPixel(p)) / 4294967296.0; // 2^32
}

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float2 ndc = float2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0);
    float4 clipPos = float4(ndc, depth, 1.0);
    float4 worldPos = mul(clipPos, invViewProj);
    return worldPos.xyz / worldPos.w;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID)
{
    uint width;
    uint height;
    g_shadowMask.GetDimensions(width, height);

    if (dtid.x >= width || dtid.y >= height)
    {
        return;
    }

    float depth = g_depth.Load(int3(dtid.xy, 0));

    // Background pixels (no geometry) are fully lit
    if (depth >= 1.0)
    {
        g_shadowMask[dtid.xy] = 1.0;
        return;
    }

    float2 uv = (dtid.xy + 0.5) / float2(width, height);

    float3 worldPos = ReconstructWorldPosition(uv, depth);
    float3 normal = normalize(g_normal.Load(int3(dtid.xy, 0)).rgb);

    // Match LightPass: lightDirection is treated as the surface-to-light direction.
    float3 rayDir = normalize(lightDirection);

    // If shadow is disabled, mark fully lit and skip tracing
    if (!enabled)
    {
        g_shadowMask[dtid.xy] = 1.0;
        return;
    }

    // Offset ray origin along normal to avoid self-intersection
    float3 rayOrigin = worldPos + normal * normalBias;

    if (softShadowEnabled && sampleCount > 1 && lightAngularRadius > 0)
    {
        // Build orthonormal basis around the same ray direction used by the hard-shadow path.
        float3 up = abs(rayDir.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
        float3 right = normalize(cross(up, rayDir));
        float3 forward = cross(rayDir, right);

        float spread = lightAngularRadius * jitterStrength;
        uint numSamples = min(sampleCount, 16u);

        float rotation = HashPixel01(dtid.xy) * 6.28318531;

        float totalVisibility = 0.0;
        [loop] for (uint i = 0; i < numSamples; i++)
        {
            float angle = (i + 0.5) * 6.28318531 / numSamples + rotation;
            float r = sqrt((i + 0.5) / numSamples) * spread;
            float cosA, sinA;
            sincos(angle, sinA, cosA);

            float3 sampleDir = normalize(rayDir + right * cosA * r + forward * sinA * r);

            RayDesc ray;
            ray.Origin = rayOrigin;
            ray.TMin = rayTMin;
            ray.Direction = sampleDir;
            ray.TMax = rayTMax;

            RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
            q.TraceRayInline(g_tlas, 0, 0xFF, ray);
            while (q.Proceed()) {}

            totalVisibility += (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) ? 0.0 : 1.0;
        }

        g_shadowMask[dtid.xy] = totalVisibility / numSamples;
    }
    else
    {
        // Hard-shadow: single ray trace
        RayDesc ray;
        ray.Origin = rayOrigin;
        ray.TMin = rayTMin;
        ray.Direction = rayDir;
        ray.TMax = rayTMax;

        RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
        q.TraceRayInline(g_tlas, 0, 0xFF, ray);
        while (q.Proceed()) {}

        g_shadowMask[dtid.xy] = (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) ? 0.0 : 1.0;
    }
}
