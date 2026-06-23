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

float3 CameraRayDirection(float2 uv)
{
    float2 ndc = float2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0);
    float4 clipNear = float4(ndc, 0.0, 1.0);
    float4 clipFar  = float4(ndc, 1.0, 1.0);
    float4 worldNear = mul(clipNear, invViewProj);
    worldNear /= worldNear.w;
    float4 worldFar  = mul(clipFar,  invViewProj);
    worldFar  /= worldFar.w;
    return normalize(worldFar.xyz - worldNear.xyz);
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

    float2 uv = (dtid.xy + 0.5) / float2(width, height);

    float3 rayOrigin = cameraPosition;
    float3 rayDir = CameraRayDirection(uv);

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
        float t = q.CommittedRayT();
        g_shadowMask[dtid.xy] = saturate(t / 50.0);
    }
    else
    {
        g_shadowMask[dtid.xy] = 0.0;
    }
}
