// Stub placeholder: real shadow mask generation will be implemented here.
// Currently outputs 1.0 (fully lit) everywhere.
// The camera-ray TLAS debug visualization has been moved to shaders_RayQueryTlasDebug.hlsl.
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

    // Placeholder: write fully lit
    g_shadowMask[dtid.xy] = 1.0;
}
