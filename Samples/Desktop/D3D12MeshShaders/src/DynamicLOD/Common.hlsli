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

#define ROOT_SIG \
    "CBV(b0), \
     RootConstants(b1, num32BitConstants = 2), \
     DescriptorTable( CBV(b2, numDescriptors = 8), \
                      SRV(t0, numDescriptors = 32)), \
     SRV(t32)"


struct VertexOut
{
    float4 PositionHS   : SV_Position;
    float3 PositionVS   : POSITION0;
    uint   MeshletIndex : COLOR0;
    float4 Color        : COLOR2;
    float3 Normal       : NORMAL0;
};


// This is the data which will be exported from the Amplification Shader
// and supplied as an extra 'in' argument to its dispatched Mesh Shader
// children.
struct Payload
{
    uint InstanceCounts[MAX_LOD_LEVELS];   // The instance count for each LOD level.
    uint GroupOffsets[MAX_LOD_LEVELS + 1]; // The offset in threadgroups for each LOD level.

    // The list of instance indices after culling. Ordered as:
    // (list of LOD 0 instance indices), (list of LOD 1 instance indices), ... (list of LOD MAX_LOD_LEVELS-1 instance indices)                                            
    uint InstanceList[AS_GROUP_SIZE];
    uint InstanceOffsets[MAX_LOD_LEVELS + 1]; // The offset into the Instance List at which each LOD level begins.
};

struct MeshInfo
{
    uint IndexBytes;
    uint MeshletCount;
    uint LastMeshletVertCount;
    uint LastMeshletPrimCount;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

ConstantBuffer<Constants>  Constants : register(b0);
ConstantBuffer<DrawParams> DrawParams : register(b1);

ConstantBuffer<MeshInfo>   MeshInfo[MAX_LOD_LEVELS] : register(b2);
StructuredBuffer<Vertex>   Vertices[MAX_LOD_LEVELS] : register(t0);
StructuredBuffer<Meshlet>  Meshlets[MAX_LOD_LEVELS] : register(t8);
ByteAddressBuffer          UniqueVertexIndices[MAX_LOD_LEVELS] : register(t16);
StructuredBuffer<uint>     PrimitiveIndices[MAX_LOD_LEVELS] : register(t24);
StructuredBuffer<Instance> Instances : register(t32);


// Computes visiblity of an instance
// Performs a simple world-space bounding sphere vs. frustum plane check.
bool IsVisible(float4 boundingSphere)
{
    float4 center = float4(boundingSphere.xyz, 1.0);
    float radius = boundingSphere.w;

    for (int i = 0; i < 6; ++i)
    {
        if (dot(center, Constants.Planes[i]) < -radius)
        {
            return false;
        }
    }

    return true;
}

// Computes the LOD for a given instance.
// Calculates the spread of the instance's world-space bounding sphere in screen space.
uint ComputeLOD(float4 boundingSphere)
{
    float3 v = boundingSphere.xyz - Constants.ViewPosition;
    float r = boundingSphere.w;

    // Sphere radius in screen space
    float size = Constants.RecipTanHalfFovy * r / sqrt(dot(v, v) - r * r);
    size = min(size, 1.0);

    return (1.0 - size) * (Constants.LODCount - 1);
}

uint DivRoundUp(uint num, uint denom)
{
    return (num + denom - 1) / denom;
}
