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

struct MeshInfo
{
    uint IndexSize;
    uint MeshletCount;

    uint LastMeshletVertCount;
    uint LastMeshletPrimCount;
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

struct CullData
{
    float4 BoundingSphere;
    uint   NormalCone;
    float  ApexOffset;
};

bool IsConeDegenerate(CullData c)
{
    return (c.NormalCone >> 24) == 0xff;
}

float4 UnpackCone(uint packed)
{
    float4 v;
    v.x = float((packed >> 0) & 0xFF);
    v.y = float((packed >> 8) & 0xFF);
    v.z = float((packed >> 16) & 0xFF);
    v.w = float((packed >> 24) & 0xFF);

    v = v / 255.0;
    v.xyz = v.xyz * 2.0 - 1.0;

    return v;
}