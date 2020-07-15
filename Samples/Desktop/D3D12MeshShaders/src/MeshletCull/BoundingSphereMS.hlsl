//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "VisualizerCommon.hlsli"
#include "MeshletUtils.hlsli"

#define GROUP_SIZE 128

// Base count of 127 works --- 128 breaks NVIDIA GPU, not sure why.
// Worked before 450.82 driver update.
#define BASE_COUNT GROUP_SIZE - 1 

// Resource bindings
cbuffer Globals : register(b0)
{
    float4x4 World;
    float4x4 ViewProj;
    float4   Color;
    float3   ViewUp;
    float3   ViewForward;
    float    Scale;
};

cbuffer Options : register(b1)
{
    uint MeshletOffset;
    uint MeshletCount;
}

StructuredBuffer<CullData>  MeshletCullData : register(t0);


//---------------------------------------------
// Main

[RootSignature("CBV(b0), RootConstants(b1, num32BitConstants=2), SRV(t0)")]
[NumThreads(GROUP_SIZE, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out vertices DebugVertex verts[GROUP_SIZE],
    out indices uint3 prims[GROUP_SIZE]
)
{
    // Ensure not too many threadgroups were dispatched.
    if (gid >= MeshletCount)
        return;

    CullData c = MeshletCullData[MeshletOffset + gid];

    uint vertCount = BASE_COUNT;
    uint primCount = BASE_COUNT - 1;

    SetMeshOutputCounts(vertCount, primCount);

    //--------------------------------------------------------------------
    // Export Primitive & Vertex Data

    // Vertex positions are procedurally generated
    if (gtid < vertCount)
    {
        float4 position = mul(float4(c.BoundingSphere.xyz, 1), World);

        if (gtid != 0)
        {
            uint index = gtid - 1;

            // Rotate scaled vector about camera forward vector by some angle
            float angle = (float(index) / primCount) * 2.0 * 3.1415926;
            float3 offset = RotateVector(ViewUp, -ViewForward, angle) * c.BoundingSphere.w * Scale;

            // Offset centerpoint by the rotated vector.
            position += float4(offset, 0);
        }

        DebugVertex v = (DebugVertex)0;
        v.Position = mul(position, ViewProj);
        v.Color = Color;

        verts[gtid] = v;
    }

    // Primitive indices are determined by geometric definition
    if (gtid < primCount)
    {
        uint i0 = 0;
        uint i1 = gtid % primCount + 1;
        uint i2 = (gtid + 1) % primCount + 1;

        prims[gtid] = uint3(i0, i1, i2);
    }
}
