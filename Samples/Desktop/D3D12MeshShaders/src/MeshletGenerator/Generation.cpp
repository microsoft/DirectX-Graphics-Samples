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
#include "Generation.h"
#include "Utilities.h"

#include <DirectXMath.h>

#include <algorithm>
#include <unordered_set>

using namespace DirectX;

///
// External Interface

namespace internal
{
    template <typename T>
    void Meshletize(
        uint32_t maxVerts, uint32_t maxPrims,
        const T* indices, uint32_t indexCount,
        const XMFLOAT3* positions, uint32_t vertexCount,
        std::vector<InlineMeshlet<T>>& output);
}

void Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<uint16_t>>& output
)
{
    return internal::Meshletize(maxVerts, maxPrims, indices, indexCount, positions, vertexCount, output);
}

void Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<uint32_t>>& output
)
{
    return internal::Meshletize(maxVerts, maxPrims, indices, indexCount, positions, vertexCount, output);
}


///
// Helpers

// Sort in reverse order to use vector as a queue with pop_back.
bool CompareScores(const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b)
{
    return a.second > b.second;
}

XMVECTOR ComputeNormal(XMFLOAT3* tri)
{
    XMVECTOR p0 = XMLoadFloat3(&tri[0]);
    XMVECTOR p1 = XMLoadFloat3(&tri[1]);
    XMVECTOR p2 = XMLoadFloat3(&tri[2]);

    XMVECTOR v01 = p0 - p1;
    XMVECTOR v02 = p0 - p2;

    return XMVector3Normalize(XMVector3Cross(v01, v02));
}

// Compute number of triangle vertices already exist in the meshlet
template <typename T>
uint32_t ComputeReuse(const InlineMeshlet<T>& meshlet, T (&triIndices)[3])
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < static_cast<uint32_t>(meshlet.UniqueVertexIndices.size()); ++i)
    {
        for (uint32_t j = 0; j < 3u; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == triIndices[j])
            {
                ++count;
            }
        }
    }

    return count;
}

// Computes a candidacy score based on spatial locality, orientational coherence, and vertex re-use within a meshlet.
template <typename T>
float ComputeScore(const InlineMeshlet<T>& meshlet, XMVECTOR sphere, XMVECTOR normal, T (&triIndices)[3], XMFLOAT3* triVerts)
{
    const float reuseWeight = 0.334f;
    const float locWeight = 0.333f;
    const float oriWeight = 0.333f;
    
    // Vertex reuse
    uint32_t reuse = ComputeReuse(meshlet, triIndices);
    XMVECTOR reuseScore = g_XMOne - (XMVectorReplicate(float(reuse)) / 3.0f);

    // Distance from center point
    XMVECTOR maxSq = g_XMZero;
    for (uint32_t i = 0; i < 3u; ++i)
    {
        XMVECTOR v = sphere - XMLoadFloat3(&triVerts[i]);
        maxSq = XMVectorMax(maxSq, XMVector3Dot(v, v));
    }
    XMVECTOR r = XMVectorSplatW(sphere);
    XMVECTOR r2 = r * r;
    XMVECTOR locScore = XMVectorLog(maxSq / r2 + g_XMOne);

    // Angle between normal and meshlet cone axis
    XMVECTOR n = ComputeNormal(triVerts);
    XMVECTOR d = XMVector3Dot(n, normal);
    XMVECTOR oriScore = (-d + g_XMOne) / 2.0f;

    XMVECTOR b = reuseWeight * reuseScore + locWeight * locScore + oriWeight * oriScore;

    return XMVectorGetX(b);
}

// Determines whether a candidate triangle can be added to a specific meshlet; if it can, does so.
template <typename T>
bool AddToMeshlet(uint32_t maxVerts, uint32_t maxPrims, InlineMeshlet<T>& meshlet, T (&tri)[3])
{
    // Are we already full of vertices?
    if (meshlet.UniqueVertexIndices.size() == maxVerts)
        return false;

    // Are we full, or can we store an additional primitive?
    if (meshlet.PrimitiveIndices.size() == maxPrims)
        return false;

    static const uint32_t Undef = uint32_t(-1);
    uint32_t indices[3] = { Undef, Undef, Undef };
    uint32_t newCount = 3;

    for (uint32_t i = 0; i < meshlet.UniqueVertexIndices.size(); ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == tri[j])
            {
                indices[j] = i;
                --newCount;
            }
        }
    }

    // Will this triangle fit?
    if (meshlet.UniqueVertexIndices.size() + newCount > maxVerts)
        return false;

    // Add unique vertex indices to unique vertex index list
    for (uint32_t j = 0; j < 3; ++j)
    {
        if (indices[j] == Undef)
        {
            indices[j] = static_cast<uint32_t>(meshlet.UniqueVertexIndices.size());
            meshlet.UniqueVertexIndices.push_back(tri[j]);
        }
    }

    // Add the new primitive 
    typename InlineMeshlet<T>::PackedTriangle prim = {};
    prim.i0 = indices[0];
    prim.i1 = indices[1];
    prim.i2 = indices[2];

    meshlet.PrimitiveIndices.push_back(prim);

    return true;
}

template <typename T>
bool IsMeshletFull(uint32_t maxVerts, uint32_t maxPrims, const InlineMeshlet<T>& meshlet)
{
    assert(meshlet.UniqueVertexIndices.size() <= maxVerts);
    assert(meshlet.PrimitiveIndices.size() <= maxPrims);

    return meshlet.UniqueVertexIndices.size() == maxVerts
        || meshlet.PrimitiveIndices.size() == maxPrims;
}


///
// Implementation 

template <typename T>
void internal::Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const T* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<T>>& output
)
{
    const uint32_t triCount = indexCount / 3;

    // Build a primitive adjacency list
    std::vector<uint32_t> adjacency;
    adjacency.resize(indexCount);

    BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency.data());

    // Rest our outputs
    output.clear();
    output.emplace_back();
    auto* curr = &output.back();

    // Bitmask of all triangles in mesh to determine whether a specific one has been added.
    std::vector<bool> checklist;
    checklist.resize(triCount);

    std::vector<XMFLOAT3> m_positions;
    std::vector<XMFLOAT3> normals;
    std::vector<std::pair<uint32_t, float>> candidates;
    std::unordered_set<uint32_t> candidateCheck;

    XMVECTOR psphere, normal;

    // Arbitrarily start at triangle zero.
    uint32_t triIndex = 0;
    candidates.push_back(std::make_pair(triIndex, 0.0f));
    candidateCheck.insert(triIndex);
    
    // Continue adding triangles until 
    while (!candidates.empty())
    {
        uint32_t index = candidates.back().first;
        candidates.pop_back();

        T tri[3] =
        {
            indices[index * 3],
            indices[index * 3 + 1],
            indices[index * 3 + 2],
        };

        assert(tri[0] < vertexCount);
        assert(tri[1] < vertexCount);
        assert(tri[2] < vertexCount);

        // Try to add triangle to meshlet
        if (AddToMeshlet(maxVerts, maxPrims, *curr, tri))
        {
            // Success! Mark as added.
            checklist[index] = true;

            // Add m_positions & normal to list
            XMFLOAT3 points[3] =
            {
                positions[tri[0]],
                positions[tri[1]],
                positions[tri[2]],
            };

            m_positions.push_back(points[0]);
            m_positions.push_back(points[1]);
            m_positions.push_back(points[2]);

            XMFLOAT3 Normal;
            XMStoreFloat3(&Normal, ComputeNormal(points));
            normals.push_back(Normal);

            // Compute new bounding sphere & normal axis
            psphere = MinimumBoundingSphere(m_positions.data(), static_cast<uint32_t>(m_positions.size()));
            
            XMVECTOR nsphere = MinimumBoundingSphere(normals.data(), static_cast<uint32_t>(normals.size()));
            normal = XMVector3Normalize(nsphere);

            // Find and add all applicable adjacent triangles to candidate list
            const uint32_t adjIndex = index * 3;

            uint32_t adj[3] =
            {
                adjacency[adjIndex],
                adjacency[adjIndex + 1],
                adjacency[adjIndex + 2],
            };

            for (uint32_t i = 0; i < 3u; ++i)
            {
                // Invalid triangle in adjacency slot
                if (adj[i] == -1)
                    continue;
                
                // Already processed triangle
                if (checklist[adj[i]])
                    continue;

                // Triangle already in the candidate list
                if (candidateCheck.count(adj[i]))
                    continue;

                candidates.push_back(std::make_pair(adj[i], FLT_MAX));
                candidateCheck.insert(adj[i]);
            }

            // Re-score remaining candidate triangles
            for (uint32_t i = 0; i < static_cast<uint32_t>(candidates.size()); ++i)
            {
                uint32_t candidate = candidates[i].first;

                T triIndices[3] =
                {
                    indices[candidate * 3],
                    indices[candidate * 3 + 1],
                    indices[candidate * 3 + 2],
                };

                assert(triIndices[0] < vertexCount);
                assert(triIndices[1] < vertexCount);
                assert(triIndices[2] < vertexCount);

                XMFLOAT3 triVerts[3] =
                {
                    positions[triIndices[0]],
                    positions[triIndices[1]],
                    positions[triIndices[2]],
                };

                candidates[i].second = ComputeScore(*curr, psphere, normal, triIndices, triVerts);
            }

            // Determine whether we need to move to the next meshlet.
            if (IsMeshletFull(maxVerts, maxPrims, *curr))
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                // Use one of our existing candidates as the next meshlet seed.
                if (!candidates.empty())
                {
                    candidates[0] = candidates.back();
                    candidates.resize(1);
                    candidateCheck.insert(candidates[0].first);
                }

                output.emplace_back();
                curr = &output.back();
            }
            else
            {
                std::sort(candidates.begin(), candidates.end(), &CompareScores);
            }
        }
        else
        {
            if (candidates.empty())
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                output.emplace_back();
                curr = &output.back();
            }
        }

        // Ran out of candidates; add a new seed candidate to start the next meshlet.
        if (candidates.empty())
        {
            while (triIndex < triCount && checklist[triIndex])
                ++triIndex;

            if (triIndex == triCount)
                break;

            candidates.push_back(std::make_pair(triIndex, 0.0f));
            candidateCheck.insert(triIndex);
        }
    }

    // The last meshlet may have never had any primitives added to it - in which case we want to remove it.
    if (output.back().PrimitiveIndices.empty())
    {
        output.pop_back();
    }
}
