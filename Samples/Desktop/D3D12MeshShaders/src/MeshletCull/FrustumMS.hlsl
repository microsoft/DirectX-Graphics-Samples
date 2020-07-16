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

// Constant Definitions
#define PLANE_LEFT 0
#define PLANE_RIGHT 1
#define PLANE_BOTTOM 2
#define PLANE_TOP 3
#define PLANE_NEAR 4
#define PLANE_FAR 5

// Plane index triplets whose intersection defines a vertex of an arbitrary frustum.
static const uint3 IntersectionIndices[8] =
{
    uint3(PLANE_LEFT, PLANE_FAR, PLANE_BOTTOM),
    uint3(PLANE_RIGHT, PLANE_FAR, PLANE_BOTTOM),
    uint3(PLANE_RIGHT, PLANE_NEAR, PLANE_BOTTOM),
    uint3(PLANE_LEFT, PLANE_NEAR, PLANE_BOTTOM),
    uint3(PLANE_LEFT, PLANE_FAR, PLANE_TOP),
    uint3(PLANE_RIGHT, PLANE_FAR, PLANE_TOP),
    uint3(PLANE_RIGHT, PLANE_NEAR, PLANE_TOP),
    uint3(PLANE_LEFT, PLANE_NEAR, PLANE_TOP),
};

// Vertex indices for the line primitives composing a frustum.
static const uint2 PrimitiveIndices[12] =
{
    uint2(0, 1), uint2(1, 2), uint2(2, 3), uint2(3, 0),
    uint2(4, 5), uint2(5, 6), uint2(6, 7), uint2(7, 4),
    uint2(0, 4), uint2(1, 5), uint2(2, 6), uint2(3, 7),
};

// Calculates the intersection of three planes which is known to be a single point.
float3 IntersectPlanes(float4 p0, float4 p1, float4 p2)
{
    float3 n12 = cross(p1.xyz, p2.xyz);
    float3 n20 = cross(p2.xyz, p0.xyz);
    float3 n01 = cross(p0.xyz, p1.xyz);

    float3 r = -p0.w * n12 - p1.w * n20 - p2.w * n01;
    return r * (1 / dot(p0.xyz, n12));
}

// Resource bindings
cbuffer Globals : register(b0)
{
    float4x4 ViewProj;
    float4   Planes[6];
    float4   LineColor;
};


//---------------------------------------------
// Main

[RootSignature("CBV(b0)")]
[NumThreads(32, 1, 1)]
[OutputTopology("line")]
void main(
    uint gtid : SV_GroupThreadID,
    out vertices DebugVertex verts[8],
    out indices uint2 prims[12]
)
{
    SetMeshOutputCounts(8, 12);

    if (gtid < 8)
    {
        uint3 p = IntersectionIndices[gtid];
        float3 position = IntersectPlanes(Planes[p.x], Planes[p.y], Planes[p.z]);

        DebugVertex v = (DebugVertex)0;
        v.Position = mul(float4(position, 1), ViewProj);
        v.Color = LineColor;

        verts[gtid] = v;
    }

    if (gtid < 12)
    {
        prims[gtid] = PrimitiveIndices[gtid];
    }
}
