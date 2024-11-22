Texture2D<float4> srcTexture : register(t0);  
RWTexture2D<float4> dstTexture : register(u0);

SamplerState samplerBilinear : register(s0); 

cbuffer DownsampleCB : register(b0) {
    float3 srcSize;  
    float pad0;
    float3 dstSize;
    float pad1;  
    float3 scale;
    float pad2;    
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint2 dstCoord = DTid.xy;

    if (dstCoord.x >= dstSize.x || dstCoord.y >= dstSize.y) return;

    // Cropped square in the middle of the source texture.
    float2 srcCoord = (0.5*srcSize - 0.5*dstSize).xy + dstCoord;
    float2 uv = srcCoord / srcSize.xy;

    float4 color = srcTexture.SampleLevel(samplerBilinear, uv, 0);

    dstTexture[dstCoord] = color;
}
