//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "DoFCommon.hlsli"

Texture2D<float3> InputColor : register(t0);
Texture2D<float> InputAlpha : register(t1);
StructuredBuffer<uint> WorkQueue : register(t2);
RWTexture2D<float3> OutputColor : register(u0);
RWTexture2D<float> OutputAlpha : register(u1);

// This define will run a 3x3 median filter an all four channels separately.  This is
// the slowest and probably not the most correct way to do things.
//#define PER_CHANNEL_MEDIAN

// This enables separate 3x3 median filters on the luminance of the color and the alpha
// buffer.  The color brightness is rescaled to the median luminance.  This is faster
// than doing a median four separate channels, and it's potentially more correct because
// filtering R, G, and B separately will cause color shifts.
//#define LUMA_ALPHA_MEDIAN

// The fastest and default option is to filter only the luminance.  The four components
// will be selected from the pixel that had the median luminance.  Luminance is not
// rescaled.  The other code paths have been left in until it has been fully decided
// that they are unnecessary or wrong.

groupshared uint gs_RG[100];
groupshared float gs_L[100];
#ifdef SEPARATE_ALPHA_MEDIAN
groupshared float gs_B[100];
groupshared float gs_A[100];
#else
groupshared uint gs_BA[100];
#endif

float Med9( float x0, float x1, float x2,
            float x3, float x4, float x5,
            float x6, float x7, float x8 )
{
    float A = Max3(Min3(x0, x1, x2), Min3(x3, x4, x5), Min3(x6, x7, x8));
    float B = Min3(Max3(x0, x1, x2), Max3(x3, x4, x5), Max3(x6, x7, x8));
    float C = Med3(Med3(x0, x1, x2), Med3(x3, x4, x5), Med3(x6, x7, x8));
    return Med3(A, B, C);
}

void StoreColor( uint idx, float R, float G, float B, float A )
{
    gs_RG[idx] = f32tof16(R) << 16 | f32tof16(G);
#ifdef SEPARATE_ALPHA_MEDIAN
    gs_A[idx] = A;
    gs_B[idx] = B;
#else
    gs_BA[idx] = f32tof16(B) << 16 | f32tof16(A);
#endif

    float Luma = dot(float3(R, G, B), float3(0.212671, 0.715160, 0.072169)) + 1.0;
    gs_L[idx] = asfloat((asuint(Luma) & ~0xFF) | idx);
}

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID )
{
    uint TileCoord = WorkQueue[Gid.x];
    uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);
    uint2 st = Tile * 8 + GTid.xy;


    if (GTid.x < 5 && GTid.y < 5)
    {
        float2 PrefetchUV = 2 * (st + GTid.xy) * RcpBufferDim;
        float4 R = InputColor.GatherRed(  ClampSampler, PrefetchUV);
        float4 G = InputColor.GatherGreen(ClampSampler, PrefetchUV);
        float4 B = InputColor.GatherBlue( ClampSampler, PrefetchUV);
        float4 A = InputAlpha.GatherRed(  ClampSampler, PrefetchUV);
        uint destIdx = GTid.x * 2 + GTid.y * 2 * 10;
        StoreColor(destIdx   , R.w, G.w, B.w, A.w);
        StoreColor(destIdx+ 1, R.z, G.z, B.z, A.z);
        StoreColor(destIdx+10, R.x, G.x, B.x, A.x);
        StoreColor(destIdx+11, R.y, G.y, B.y, A.y);
    }

    GroupMemoryBarrierWithGroupSync();

    uint ulIdx = GTid.x + GTid.y * 10;

    float MedL = Med9(
        gs_L[ulIdx   ], gs_L[ulIdx+ 1], gs_L[ulIdx+ 2],
        gs_L[ulIdx+10], gs_L[ulIdx+11], gs_L[ulIdx+12],
        gs_L[ulIdx+20], gs_L[ulIdx+21], gs_L[ulIdx+22]);

    uint cIdx = asuint(MedL) & 0xFF;
    uint RG = gs_RG[cIdx];
#ifdef SEPARATE_ALPHA_MEDIAN
    float Blue = gs_B[cIdx];
#else
    uint BA = gs_BA[cIdx];
    float Blue = f16tof32(BA >> 16);
#endif
    OutputColor[st] = float3( f16tof32(RG >> 16), f16tof32(RG), Blue );

#ifdef SEPARATE_ALPHA_MEDIAN
    OutputAlpha[st] = Med9(
        gs_A[ulIdx   ], gs_A[ulIdx+ 1], gs_A[ulIdx+ 2],
        gs_A[ulIdx+10], gs_A[ulIdx+11], gs_A[ulIdx+12],
        gs_A[ulIdx+20], gs_A[ulIdx+21], gs_A[ulIdx+22]);
#else
    OutputAlpha[st] = f16tof32(BA);
#endif
}
