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

Texture2D<float> LNDepthBuffer : register(t0);        // Linear/normalized depth buffer
RWTexture2D<float3> TileClass : register(u0);

groupshared float gs_ClosestDepthSearch[64];
groupshared float gs_FarthestDepthSearch[64];
groupshared float gs_MaximumCoC[64];

float MaxCoC( float4 Depths )
{
    float MaxDepthRelativeToFocus = Max4(abs(Depths - FocusCenter.xxxx));
    return max(1.0 / sqrt(MATH_CONST_PI), MAX_COC_RADIUS * saturate( MaxDepthRelativeToFocus * FocalSpread ));
}

[RootSignature(DoF_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    float2 uv = (DTid.xy * 2 + 1) * RcpBufferDim;
    float4 Depths = LNDepthBuffer.Gather(ClampSampler, uv);

    float TileMinDepth = Min4(Depths);
    float TileMaxDepth = Max4(Depths);
    float TileMaxCoC = MaxCoC(Depths);

    // Write and sync
    gs_ClosestDepthSearch[GI] = TileMinDepth;
    gs_FarthestDepthSearch[GI] = TileMaxDepth;
    gs_MaximumCoC[GI] = TileMaxCoC;
    GroupMemoryBarrierWithGroupSync();

    for (uint i = 32; i > 0; i >>= 1)
    {
        // Read and sync
        if (GI < i)
        {
            gs_ClosestDepthSearch[i] = min(gs_ClosestDepthSearch[i], gs_ClosestDepthSearch[GI + i]);
            gs_FarthestDepthSearch[i] = max(gs_FarthestDepthSearch[i], gs_FarthestDepthSearch[GI + i]);
            gs_MaximumCoC[i] = max(gs_MaximumCoC[i], gs_MaximumCoC[GI + i]);
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (GI == 0)
        TileClass[Gid.xy] = float3(gs_MaximumCoC[0], gs_FarthestDepthSearch[0], gs_FarthestDepthSearch[0]);
}
