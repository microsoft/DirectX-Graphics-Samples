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
    float scPad;
};

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float2 ndc = float2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0);
    float4 clipPos = float4(ndc, depth, 1.0);
    float4 worldPos = mul(clipPos, invViewProj);
    return worldPos.xyz / worldPos.w;
}

static const float kNormalBias = 0.01;

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

    // Match LightPass: normalize(-lightDirection) is the surface-to-light direction.
    float3 rayDir = normalize(-lightDirection);

    // Offset ray origin along normal to avoid self-intersection
    float3 rayOrigin = worldPos + normal * kNormalBias;

    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.TMin = 0.001;
    ray.Direction = rayDir;
    ray.TMax = 10000.0;

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(g_tlas, 0, 0xFF, ray);

    while (q.Proceed()) {}

    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        g_shadowMask[dtid.xy] = 0.0;
    }
    else
    {
        g_shadowMask[dtid.xy] = 1.0;
    }
}
