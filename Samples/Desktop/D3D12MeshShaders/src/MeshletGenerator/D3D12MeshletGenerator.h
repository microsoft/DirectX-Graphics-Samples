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
#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>

enum Flags : uint32_t
{
    CNORM_WIND_CW = 0x4
};

struct Subset
{
    uint32_t Offset;
    uint32_t Count;
};

struct Meshlet
{
    uint32_t VertCount;
    uint32_t VertOffset;
    uint32_t PrimCount;
    uint32_t PrimOffset;
};

union PackedTriangle
{
    struct
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
        uint32_t _unused : 2;
    } indices;
    uint32_t packed;
};

struct CullData
{
    DirectX::XMFLOAT4 BoundingSphere; // xyz = center, w = radius
    uint8_t           NormalCone[4];  // xyz = axis, w = sin(a + 90)
    float             ApexOffset;     // apex = center - axis * offset
};


HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t* indices, uint32_t nFaces, 
    const Subset* indexSubsets, uint32_t nSubsets,
    const DirectX::XMFLOAT3* positions, uint32_t nVerts,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices);

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t* indices, uint32_t nFaces,
    const Subset* indexSubsets, uint32_t nSubsets,
    const DirectX::XMFLOAT3* positions, uint32_t nVerts,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices);

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t* indices, uint32_t nFaces, 
    const DirectX::XMFLOAT3* positions, uint32_t nVerts,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices);

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t* indices, uint32_t nFaces, 
    const DirectX::XMFLOAT3* positions, uint32_t nVerts,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices);


HRESULT ComputeCullData(
    const DirectX::XMFLOAT3* positions, uint32_t nVerts,
    const Meshlet* meshlets, uint32_t nMeshlets,
    const uint16_t* uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
);

HRESULT ComputeCullData(
    const DirectX::XMFLOAT3* positions, uint32_t nVerts,
    const Meshlet* meshlets, uint32_t nMeshlets,
    const uint32_t* uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
);
