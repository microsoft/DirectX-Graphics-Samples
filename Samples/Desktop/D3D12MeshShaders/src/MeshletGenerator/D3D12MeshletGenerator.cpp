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
#include "D3D12MeshletGenerator.h"

#include "Generation.h"
#include "Utilities.h"

using namespace DirectX;

namespace
{
    inline XMVECTOR QuantizeSNorm(XMVECTOR value)
    {
        return (XMVectorClamp(value, g_XMNegativeOne, g_XMOne) * 0.5f + XMVectorReplicate(0.5f)) * 255.0f;
    }

    inline XMVECTOR QuantizeUNorm(XMVECTOR value)
    {
        return (XMVectorClamp(value, g_XMZero, g_XMOne)) * 255.0f;
    }
} 

namespace internal
{
    template <typename T>
    HRESULT ComputeMeshlets(
        uint32_t maxVerts, uint32_t maxPrims,
        const T* indices, uint32_t indexCount,
        const Subset* indexSubsets, uint32_t subsetCount,
        const XMFLOAT3* positions, uint32_t vertexCount,
        std::vector<Subset>& meshletSubsets,
        std::vector<Meshlet>& meshlets,
        std::vector<uint8_t>& uniqueVertexIndices,
        std::vector<PackedTriangle>& primitiveIndices);

    template <typename T>
    HRESULT ComputeCullData(
        const XMFLOAT3* positions, uint32_t vertexCount,
        const Meshlet* meshlets, uint32_t meshletCount,
        const T* uniqueVertexIndices,
        const PackedTriangle* primitiveIndices,
        DWORD flags,
        CullData* cullData
    );
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t* indices, uint32_t indexCount,
    const Subset* indexSubsets, uint32_t subsetCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, indexSubsets, subsetCount, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t* indices, uint32_t indexCount,
    const Subset* indexSubsets, uint32_t subsetCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, indexSubsets, subsetCount, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
    Subset s = { 0, indexCount };
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, &s, 1u, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
    Subset s = { 0, indexCount };
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, &s, 1u, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeCullData(
    const XMFLOAT3* positions, uint32_t vertexCount,
    const Meshlet* meshlets, uint32_t meshletCount,
    const uint16_t* uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
)
{
    return internal::ComputeCullData(positions, vertexCount, meshlets, meshletCount, uniqueVertexIndices, primitiveIndices, flags, cullData);
}

HRESULT ComputeCullData(
    const XMFLOAT3* positions, uint32_t vertexCount,
    const Meshlet* meshlets, uint32_t meshletCount,
    const uint32_t* uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
)
{
    return internal::ComputeCullData(positions, vertexCount, meshlets, meshletCount, uniqueVertexIndices, primitiveIndices, flags, cullData);
}


template <typename T>
HRESULT internal::ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const T* indices, uint32_t indexCount,
    const Subset* indexSubsets, uint32_t subsetCount,
    const DirectX::XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<Subset>& meshletSubsets,
    std::vector<Meshlet>& meshlets,
    std::vector<uint8_t>& uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
    UNREFERENCED_PARAMETER(indexCount);

    for (uint32_t i = 0; i < subsetCount; ++i)
    {
        Subset s = indexSubsets[i];

        assert(s.Offset + s.Count <= indexCount);

        std::vector<InlineMeshlet<T>> builtMeshlets;
        Meshletize(maxVerts, maxPrims, indices + s.Offset, s.Count, positions, vertexCount, builtMeshlets);

        Subset meshletSubset;
        meshletSubset.Offset = static_cast<uint32_t>(meshlets.size());
        meshletSubset.Count = static_cast<uint32_t>(builtMeshlets.size());
        meshletSubsets.push_back(meshletSubset);

        // Determine final unique vertex index and primitive index counts & offsets.
        uint32_t startVertCount = static_cast<uint32_t>(uniqueVertexIndices.size()) / sizeof(T);
        uint32_t startPrimCount = static_cast<uint32_t>(primitiveIndices.size());

        uint32_t uniqueVertexIndexCount = startVertCount;
        uint32_t primitiveIndexCount = startPrimCount;

        // Resize the meshlet output array to hold the newly formed meshlets.
        uint32_t meshletCount = static_cast<uint32_t>(meshlets.size());
        meshlets.resize(meshletCount + builtMeshlets.size());

        for (uint32_t j = 0, dest = meshletCount; j < static_cast<uint32_t>(builtMeshlets.size()); ++j, ++dest)
        {
            meshlets[dest].VertOffset = uniqueVertexIndexCount;
            meshlets[dest].VertCount = static_cast<uint32_t>(builtMeshlets[j].UniqueVertexIndices.size());
            uniqueVertexIndexCount += static_cast<uint32_t>(builtMeshlets[j].UniqueVertexIndices.size());

            meshlets[dest].PrimOffset = primitiveIndexCount;
            meshlets[dest].PrimCount = static_cast<uint32_t>(builtMeshlets[j].PrimitiveIndices.size());
            primitiveIndexCount += static_cast<uint32_t>(builtMeshlets[j].PrimitiveIndices.size());
        }

        // Allocate space for the new data.
        uniqueVertexIndices.resize(uniqueVertexIndexCount * sizeof(T));
        primitiveIndices.resize(primitiveIndexCount);

        // Copy data from the freshly built meshlets into the output buffers.
        auto vertDest = reinterpret_cast<T*>(uniqueVertexIndices.data()) + startVertCount;
        auto primDest = reinterpret_cast<uint32_t*>(primitiveIndices.data()) + startPrimCount;

        for (uint32_t j = 0; j < static_cast<uint32_t>(builtMeshlets.size()); ++j)
        {
            std::memcpy(vertDest, builtMeshlets[j].UniqueVertexIndices.data(), builtMeshlets[j].UniqueVertexIndices.size() * sizeof(T));
            std::memcpy(primDest, builtMeshlets[j].PrimitiveIndices.data(), builtMeshlets[j].PrimitiveIndices.size() * sizeof(uint32_t));

            vertDest += builtMeshlets[j].UniqueVertexIndices.size();
            primDest += builtMeshlets[j].PrimitiveIndices.size();
        }
    }

    return S_OK;
}

//
// Strongly influenced by https://github.com/zeux/meshoptimizer - Thanks amigo!
//

template <typename T>
HRESULT internal::ComputeCullData(
    const XMFLOAT3* positions, uint32_t vertexCount,
    const Meshlet* meshlets, uint32_t meshletCount,
    const T* uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
)
{
    UNREFERENCED_PARAMETER(vertexCount);

    XMFLOAT3 vertices[256];
    XMFLOAT3 normals[256];

    for (uint32_t mi = 0; mi < meshletCount; ++mi)
    {
        auto& m = meshlets[mi];
        auto& c = cullData[mi];

        // Cache vertices
        for (uint32_t i = 0; i < m.VertCount; ++i)
        {
            uint32_t vIndex = uniqueVertexIndices[m.VertOffset + i];

            assert(vIndex < vertexCount);
            vertices[i] = positions[vIndex];
        }

        // Generate primitive normals & cache
        for (uint32_t i = 0; i < m.PrimCount; ++i)
        {
            auto primitive = primitiveIndices[m.PrimOffset + i];

            XMVECTOR triangle[3]
            {
                XMLoadFloat3(&vertices[primitive.indices.i0]),
                XMLoadFloat3(&vertices[primitive.indices.i1]),
                XMLoadFloat3(&vertices[primitive.indices.i2]),
            };

            XMVECTOR p10 = triangle[1] - triangle[0];
            XMVECTOR p20 = triangle[2] - triangle[0];
            XMVECTOR n = XMVector3Normalize(XMVector3Cross(p10, p20));

            XMStoreFloat3(&normals[i], (flags & CNORM_WIND_CW) != 0 ? -n : n);
        }

        // Calculate spatial bounds
        XMVECTOR positionBounds = MinimumBoundingSphere(vertices, m.VertCount);
        XMStoreFloat4(&c.BoundingSphere, positionBounds);

        // Calculate the normal cone
        // 1. Normalized center point of minimum bounding sphere of unit normals == conic axis
        XMVECTOR normalBounds = MinimumBoundingSphere(normals, m.PrimCount);

        // 2. Calculate dot product of all normals to conic axis, selecting minimum
        XMVECTOR axis = XMVectorSetW(XMVector3Normalize(normalBounds), 0);

        XMVECTOR minDot = g_XMOne;
        for (uint32_t i = 0; i < m.PrimCount; ++i)
        {
            XMVECTOR dot = XMVector3Dot(axis, XMLoadFloat3(&normals[i]));
            minDot = XMVectorMin(minDot, dot);
        }

        if (XMVector4Less(minDot, XMVectorReplicate(0.1f)))
        {
            // Degenerate cone
            c.NormalCone[0] = 127;
            c.NormalCone[1] = 127;
            c.NormalCone[2] = 127;
            c.NormalCone[3] = 255;
            continue;
        }

        // Find the point on center-t*axis ray that lies in negative half-space of all triangles
        float maxt = 0;

        for (uint32_t i = 0; i < m.PrimCount; ++i)
        {
            auto primitive = primitiveIndices[m.PrimOffset + i];

            uint32_t indices[3]
            {
                primitive.indices.i0,
                primitive.indices.i1,
                primitive.indices.i2,
            };

            XMVECTOR triangle[3]
            {
                XMLoadFloat3(&vertices[indices[0]]),
                XMLoadFloat3(&vertices[indices[1]]),
                XMLoadFloat3(&vertices[indices[2]]),
            };

            XMVECTOR c = positionBounds - triangle[0];

            XMVECTOR n = XMLoadFloat3(&normals[i]);
            float dc = XMVectorGetX(XMVector3Dot(c, n));
            float dn = XMVectorGetX(XMVector3Dot(axis, n));

            // dn should be larger than mindp cutoff above
            assert(dn > 0.0f);
            float t = dc / dn;

            maxt = (t > maxt) ? t : maxt;
        }

        // cone apex should be in the negative half-space of all cluster triangles by construction
        c.ApexOffset = maxt;

        // cos(a) for normal cone is minDot; we need to add 90 degrees on both sides and invert the cone
        // which gives us -cos(a+90) = -(-sin(a)) = sin(a) = sqrt(1 - cos^2(a))
        XMVECTOR coneCutoff = XMVectorSqrt(g_XMOne - minDot * minDot);

        // 3. Quantize to uint8
        XMVECTOR quantized = QuantizeSNorm(axis);
        c.NormalCone[0] = (uint8_t)XMVectorGetX(quantized);
        c.NormalCone[1] = (uint8_t)XMVectorGetY(quantized);
        c.NormalCone[2] = (uint8_t)XMVectorGetZ(quantized);

        XMVECTOR error = ((quantized / 255.0f) * 2.0f - g_XMOne) - axis;
        error = XMVectorSum(XMVectorAbs(error));

        quantized = QuantizeUNorm(coneCutoff + error);
        quantized = XMVectorMin(quantized + g_XMOne, XMVectorReplicate(255.0f));
        c.NormalCone[3] = (uint8_t)XMVectorGetX(quantized);
    }

    return S_OK;
}
