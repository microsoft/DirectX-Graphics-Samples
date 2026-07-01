RWByteAddressBuffer g_result : register(u0);
RaytracingAccelerationStructure g_tlas : register(t0);

cbuffer SpecularDebugRayQueryConstants : register(b0)
{
    float3 rayOrigin;
    float rayTMin;
    float3 rayDirection;
    float rayTMax;
};

static const uint kHitFlagOffset = 0;
static const uint kHitDistanceOffset = 4;
static const uint kHitPositionOffset = 8;

[numthreads(1, 1, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.TMin = rayTMin;
    ray.Direction = normalize(rayDirection);
    ray.TMax = rayTMax;

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(g_tlas, 0, 0xFF, ray);
    while (q.Proceed()) {}

    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        const float t = q.CommittedRayT();
        const float3 hitPosition = ray.Origin + ray.Direction * t;
        g_result.Store(kHitFlagOffset, 1);
        g_result.Store(kHitDistanceOffset, asuint(t));
        g_result.Store3(kHitPositionOffset, asuint(hitPosition));
    }
    else
    {
        g_result.Store(kHitFlagOffset, 0);
        g_result.Store(kHitDistanceOffset, 0);
        g_result.Store3(kHitPositionOffset, uint3(0, 0, 0));
    }
}
