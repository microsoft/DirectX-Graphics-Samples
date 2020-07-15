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
#include "MeshletCommon.hlsli"

// Packs/unpacks a 10-bit index triangle primitive into/from a uint.
uint3 UnpackPrimitive(uint primitive) { return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF); }

//--------------------------------
// Data Loaders

uint GetVertexIndex(Meshlet m, uint localIndex)
{
    localIndex = m.VertOffset + localIndex;

    if (MeshInfo.IndexSize == 4)
    {
        return UniqueVertexIndices.Load(localIndex * 4);
    }
    else // Global vertex index width is 16-bit
    {
        // Byte address must be 4-byte aligned.
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
        uint indexPair = UniqueVertexIndices.Load(byteOffset);
        uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

        return index;
    }
}

uint3 GetPrimitive(Meshlet m, uint index)
{
    return UnpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    Vertex v = Vertices[vertexIndex];

    float4 positionWS = mul(float4(v.Position, 1), Instance.World);

    VertexOut vout;
    vout.PositionVS   = mul(positionWS, Constants.View).xyz;
    vout.PositionHS   = mul(positionWS, Constants.ViewProj);
    vout.Normal       = mul(float4(v.Normal, 0), Instance.WorldInvTrans).xyz;
    vout.MeshletIndex = meshletIndex;

    return vout;
}


[RootSignature(ROOT_SIG)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint dtid : SV_DispatchThreadID,
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    in payload Payload payload,
    out vertices VertexOut verts[64],
    out indices uint3 tris[126]
)
{
    // Load the meshlet from the AS payload data
    uint meshletIndex = payload.MeshletIndices[gid];

    // Catch any out-of-range indices (in case too many MS threadgroups were dispatched from AS)
    if (meshletIndex >= MeshInfo.MeshletCount)
        return;

    // Load the meshlet
    Meshlet m = Meshlets[meshletIndex];

    // Our vertex and primitive counts come directly from the meshlet
    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    //--------------------------------------------------------------------
    // Export Primitive & Vertex Data

    if (gtid < m.VertCount)
    {
        uint vertexIndex = GetVertexIndex(m, gtid);
        verts[gtid] = GetVertexAttributes(meshletIndex, vertexIndex);
    }

    if (gtid < m.PrimCount)
    {
        tris[gtid] = GetPrimitive(m, gtid);
    }
}
