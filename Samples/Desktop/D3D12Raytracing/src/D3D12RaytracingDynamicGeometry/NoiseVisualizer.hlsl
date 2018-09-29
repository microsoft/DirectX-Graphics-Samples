#include "RandomNumberGenerator.hlsli"

cbuffer constants
{
    uint RNGSeed;
    uint DispatchWidth;  // for 2D dispatches
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint seed = DispatchWidth * DTid.y + DTid.x + RNGSeed;

    uint RNGState = RNG::SeedThread(seed);
    float randomValue = RNG::Random01(RNGState);
}