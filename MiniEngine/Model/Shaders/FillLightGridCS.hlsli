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
// Author(s):	Alex Nankervis
//

#include "LightGrid.hlsli"

// outdated warning about for-loop variable scope
#pragma warning (disable: 3078)

#define FLT_MIN         1.175494351e-38F        // min positive value
#define FLT_MAX         3.402823466e+38F        // max value
#define PI				3.1415926535f
#define TWOPI			6.283185307f

cbuffer CSConstants : register(b0)
{
    uint ViewportWidth, ViewportHeight;
    float InvTileDim;
    float RcpZMagic;
    uint TileCountX;
    float4x4 ViewProjMatrix;
};

StructuredBuffer<LightData> lightBuffer : register(t0);
Texture2D<float> depthTex : register(t1);
RWByteAddressBuffer lightGrid : register(u0);
RWByteAddressBuffer lightGridBitMask : register(u1);

groupshared uint minDepthUInt;
groupshared uint maxDepthUInt;

groupshared uint tileLightCountSphere;
groupshared uint tileLightCountCone;
groupshared uint tileLightCountConeShadowed;

groupshared uint tileLightIndicesSphere[MAX_LIGHTS];
groupshared uint tileLightIndicesCone[MAX_LIGHTS];
groupshared uint tileLightIndicesConeShadowed[MAX_LIGHTS];

groupshared uint4 tileLightBitMask;

#define _RootSig \
    "RootFlags(0), " \
    "CBV(b0), " \
    "DescriptorTable(SRV(t0, numDescriptors = 2))," \
    "DescriptorTable(UAV(u0, numDescriptors = 2))"

[RootSignature(_RootSig)]
[numthreads(8, 8, 1)]
void main(
    uint2 Gid : SV_GroupID,
    uint2 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex)
{
    // initialize shared data
    if (GI == 0)
    {
        tileLightCountSphere = 0;
        tileLightCountCone = 0;
        tileLightCountConeShadowed = 0;
        tileLightBitMask = 0;
        minDepthUInt = 0xffffffff;
        maxDepthUInt = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    // Read all depth values for this tile and compute the tile min and max values
    for (uint dx = GTid.x; dx < WORK_GROUP_SIZE_X; dx += 8)
    {
        for (uint dy = GTid.y; dy < WORK_GROUP_SIZE_Y; dy += 8)
        {
            uint2 DTid = Gid * uint2(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y) + uint2(dx, dy);

            // If pixel coordinates are in bounds...
            if (DTid.x < ViewportWidth && DTid.y < ViewportHeight)
            {
                // Load and compare depth
                uint depthUInt = asuint(depthTex[DTid.xy]);
                InterlockedMin(minDepthUInt, depthUInt);
                InterlockedMax(maxDepthUInt, depthUInt);
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();
    //float tileMinDepth = asfloat(minDepthUInt);
    //float tileMaxDepth = asfloat(maxDepthUInt);
    float tileMinDepth = (rcp(asfloat(maxDepthUInt)) - 1.0) * RcpZMagic;
    float tileMaxDepth = (rcp(asfloat(minDepthUInt)) - 1.0) * RcpZMagic;
    float tileDepthRange = tileMaxDepth - tileMinDepth;
    tileDepthRange = max(tileDepthRange, FLT_MIN); // don't allow a depth range of 0
    float invTileDepthRange = rcp(tileDepthRange);
    // TODO: near/far clipping planes seem to be falling apart at or near the max depth with infinite projections

    // construct transform from world space to tile space (projection space constrained to tile area)
    float2 invTileSize2X = float2(ViewportWidth, ViewportHeight) * InvTileDim;
    // D3D-specific [0, 1] depth range ortho projection
    // (but without negation of Z, since we already have that from the projection matrix)
    float3 tileBias = float3(
        -2.0 * float(Gid.x) + invTileSize2X.x - 1.0,
        -2.0 * float(Gid.y) + invTileSize2X.y - 1.0,
        -tileMinDepth * invTileDepthRange);
    float4x4 projToTile = float4x4(
        invTileSize2X.x, 0, 0, tileBias.x,
        0, -invTileSize2X.y, 0, tileBias.y,
        0, 0, invTileDepthRange, tileBias.z,
        0, 0, 0, 1
        );
    float4x4 tileMVP = mul(projToTile, ViewProjMatrix);
    
    // extract frustum planes (these will be in world space)
    float4 frustumPlanes[6];
    frustumPlanes[0] = tileMVP[3] + tileMVP[0];
    frustumPlanes[1] = tileMVP[3] - tileMVP[0];
    frustumPlanes[2] = tileMVP[3] + tileMVP[1];
    frustumPlanes[3] = tileMVP[3] - tileMVP[1];
    frustumPlanes[4] = tileMVP[3] + tileMVP[2];
    frustumPlanes[5] = tileMVP[3] - tileMVP[2];
    for (int n = 0; n < 6; n++)
    {
        frustumPlanes[n] *= rsqrt(dot(frustumPlanes[n].xyz, frustumPlanes[n].xyz));
    }

    uint tileIndex = GetTileIndex(Gid.xy, TileCountX);
    uint tileOffset = GetTileOffset(tileIndex);

    uint4 perThreadLightBitMask = 0;

    // find set of lights that overlap this tile
    for (uint lightIndex = GI; lightIndex < MAX_LIGHTS; lightIndex += 64)
    {
        LightData lightData = lightBuffer[lightIndex];
        float3 lightWorldPos = lightData.pos;
        float lightCullRadius = sqrt(lightData.radiusSq);

        bool overlapping = true;
        for (int p = 0; p < 6; p++)
        {
            float d = dot(lightWorldPos, frustumPlanes[p].xyz) + frustumPlanes[p].w;
            if (d < -lightCullRadius)
            {
                overlapping = false;
            }
        }
        
        if (!overlapping)
            continue;

        uint slot;

        switch (lightData.type)
        {
        case 0: // sphere
            InterlockedAdd(tileLightCountSphere, 1, slot);
            tileLightIndicesSphere[slot] = lightIndex;
            break;

        case 1: // cone
            InterlockedAdd(tileLightCountCone, 1, slot);
            tileLightIndicesCone[slot] = lightIndex;
            break;

        case 2: // cone w/ shadow map
            InterlockedAdd(tileLightCountConeShadowed, 1, slot);
            tileLightIndicesConeShadowed[slot] = lightIndex;
            break;
        }

        // update bitmask
        InterlockedOr(tileLightBitMask[lightIndex / 32], 1 << (lightIndex % 32));
    }

    GroupMemoryBarrierWithGroupSync();

    if (GI == 0)
    {
        uint lightCount = 
            ((tileLightCountSphere & 0xff) << 0) |
            ((tileLightCountCone & 0xff) << 8) |
            ((tileLightCountConeShadowed & 0xff) << 16);
        lightGrid.Store(tileOffset + 0, lightCount);

        uint storeOffset = tileOffset + 4;
        uint n;
        for (n = 0; n < tileLightCountSphere; n++)
        {
            lightGrid.Store(storeOffset, tileLightIndicesSphere[n]);
            storeOffset += 4;
        }
        for (n = 0; n < tileLightCountCone; n++)
        {
            lightGrid.Store(storeOffset, tileLightIndicesCone[n]);
            storeOffset += 4;
        }
        for (n = 0; n < tileLightCountConeShadowed; n++)
        {
            lightGrid.Store(storeOffset, tileLightIndicesConeShadowed[n]);
            storeOffset += 4;
        }

        lightGridBitMask.Store4(tileIndex * 16, tileLightBitMask);
    }
}
