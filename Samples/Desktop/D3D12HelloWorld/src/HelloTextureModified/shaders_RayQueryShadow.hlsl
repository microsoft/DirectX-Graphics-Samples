RWTexture2D<float> g_shadowMask : register(u0);

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

    g_shadowMask[dtid.xy] = 1.0f;
}
