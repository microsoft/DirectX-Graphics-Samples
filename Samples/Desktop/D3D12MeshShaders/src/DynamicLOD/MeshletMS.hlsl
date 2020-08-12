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
#include "Shared.h"
#include "Common.hlsli"

float4 LODColor(float4 boundingSphere)
{
    uint lodLevel = ComputeLOD(boundingSphere);
    float alpha = float(lodLevel) / (Constants.LODCount - 1);

    return lerp(float4(1, 0, 0, 1), float4(0, 1, 0, 1), alpha);
}

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(uint lodIndex, Meshlet m, uint index)
{
    return UnpackPrimitive(PrimitiveIndices[lodIndex][m.PrimOffset + index]);
}

uint GetVertexIndex(uint lodIndex, Meshlet m, uint localIndex)
{
    localIndex = m.VertOffset + localIndex;

    if (MeshInfo[lodIndex].IndexBytes == 4) // 32-bit Vertex Indices
    {
        return UniqueVertexIndices[lodIndex].Load(localIndex * 4);
    }
    else // 16-bit Vertex Indices
    {
        // Byte address must be 4-byte aligned.
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
        uint indexPair = UniqueVertexIndices[lodIndex].Load(byteOffset);
        uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

        return index;
    }
}

VertexOut GetVertexAttributes(uint lodIndex, uint meshletIndex, uint vertexIndex, uint instanceIndex)
{
    Instance n = Instances[DrawParams.InstanceOffset + instanceIndex];
    Vertex v = Vertices[lodIndex][vertexIndex];

    float4 positionWS = mul(float4(v.Position, 1), n.World);

    VertexOut vout;
    vout.PositionVS   = mul(positionWS, Constants.View).xyz;
    vout.PositionHS   = mul(positionWS, Constants.ViewProj);
    vout.Normal       = mul(float4(v.Normal, 0), n.WorldInvTranspose).xyz;
    vout.Color        = LODColor(n.BoundingSphere);
    vout.MeshletIndex = meshletIndex;

    return vout;
}


[RootSignature(ROOT_SIG)]
[NumThreads(MS_GROUP_SIZE, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint gtid : SV_GroupIndex,
    uint gid : SV_GroupID,
    in payload Payload payload,
    out vertices VertexOut verts[MAX_VERTS],
    out indices uint3 tris[MAX_PRIMS])
{
    // Find the LOD to which this threadgroup is assigned.
    // Each wave does this independently to avoid groupshared memory & sync.
    uint offsetCheck = 0;
    uint laneIndex = gtid % WaveGetLaneCount();

    if (laneIndex <= MAX_LOD_LEVELS)
    {
        offsetCheck = WaveActiveCountBits(gid >= payload.GroupOffsets[laneIndex]) - 1;
    }
    uint lodIndex = WaveReadLaneFirst(offsetCheck);

    // Load our LOD meshlet offset & LOD instance count
    uint lodOffset = payload.GroupOffsets[lodIndex];
    uint lodCount = payload.InstanceCounts[lodIndex];

    // Calculate and load our meshlet.
    uint meshletIndex = (gid - lodOffset) / lodCount;
    Meshlet m = Meshlets[lodIndex][meshletIndex];

    // Determine instance count - only 1 instance per threadgroup in the general case
    uint instanceCount = 1;

    // Last meshlet in mesh may be be packed - multiple instances rendered from a single threadgroup.
    if (meshletIndex == MeshInfo[lodIndex].MeshletCount - 1)
    {
        // Determine how many packed instances there are in this group
        uint unpackedGroupCount = (MeshInfo[lodIndex].MeshletCount - 1) * lodCount;
        uint packedIndex = gid - (unpackedGroupCount + lodOffset);

        uint instancesPerGroup = min(MAX_VERTS / m.VertCount, MAX_PRIMS / m.PrimCount);
        uint startInstance = packedIndex * instancesPerGroup;

        instanceCount = min(lodCount - startInstance, instancesPerGroup);
    }

    // Compute our total vertex & primitive counts
    uint totalVertCount = m.VertCount * instanceCount;
    uint totalPrimCount = m.PrimCount * instanceCount;

    SetMeshOutputCounts(totalVertCount, totalPrimCount);

    //--------------------------------------------------------------------
    // Export Primitive & Vertex Data

    if (gtid < totalVertCount)
    {
        uint readIndex = gtid % m.VertCount;  // Wrap our reads for packed instancing.
        uint vertexIndex = GetVertexIndex(lodIndex, m, readIndex);

        // Determine our instance index
        uint instanceId = gtid / m.VertCount; // Instance index into this threadgroup's instances (only non-zero for packed threadgroups.)

        uint lodInstance = (gid - lodOffset) % lodCount + instanceId;           // Instance index into this LOD level's instances
        uint instanceOffset = payload.InstanceOffsets[lodIndex] + lodInstance;  // Instance index into the payload instance list

        uint instanceIndex = payload.InstanceList[instanceOffset]; // The final instance index of this vertex.

        verts[gtid] = GetVertexAttributes(lodIndex, meshletIndex, vertexIndex, instanceIndex);
    }

    if (gtid < totalPrimCount)
    {
        uint readIndex = gtid % m.PrimCount;  // Wrap our reads for packed instancing.
        uint instanceId = gtid / m.PrimCount; // Instance index within this threadgroup (only non-zero in last meshlet threadgroups.)

        // Must offset the vertex indices to this thread's instanced verts
        tris[gtid] = GetPrimitive(lodIndex, m, readIndex) + (m.VertCount * instanceId);
    }
}
