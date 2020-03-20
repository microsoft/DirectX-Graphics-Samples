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
#include "Utilities.h"

#include <unordered_map>
#include <memory>

using namespace DirectX;

namespace
{
    struct EdgeEntry
    {
        uint32_t   i0;
        uint32_t   i1;
        uint32_t   i2;
                   
        uint32_t   Face;
        EdgeEntry* Next;
    };

    size_t CRCHash(const uint32_t* dwords, uint32_t dwordCount)
    {
        size_t h = 0;

        for (uint32_t i = 0; i < dwordCount; ++i)
        {
            uint32_t highOrd = h & 0xf8000000;
            h = h << 5;
            h = h ^ (highOrd >> 27);
            h = h ^ size_t(dwords[i]);
        }

        return h;
    }

    template <typename T>
    inline size_t Hash(const T& val)
    {
        return std::hash<T>()(val);
    }
}

namespace std
{
    template <> struct hash<XMFLOAT3> { size_t operator()(const XMFLOAT3& v) const { return CRCHash(reinterpret_cast<const uint32_t*>(&v), sizeof(v) / 4); } };
}


///
// External Interface

namespace internal
{
    template <typename T>
    void BuildAdjacencyList(
        const T* indices, uint32_t indexCount,
        const XMFLOAT3* positions, uint32_t vertexCount,
        uint32_t* adjacency);
}

void BuildAdjacencyList(
    const uint16_t* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    uint32_t* adjacency
)
{
    internal::BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency);
}

void BuildAdjacencyList(
    const uint32_t* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    uint32_t* adjacency
)
{
    internal::BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency);
}


///
// Implementation

template <typename T>
void internal::BuildAdjacencyList(
    const T* indices, uint32_t indexCount,
    const XMFLOAT3* positions, uint32_t vertexCount,
    uint32_t* adjacency
)
{
    const uint32_t triCount = indexCount / 3;
    // Find point reps (unique positions) in the position stream
    // Create a mapping of non-unique vertex indices to point reps
    std::vector<T> pointRep;
    pointRep.resize(vertexCount);

    std::unordered_map<size_t, T> uniquePositionMap;
    uniquePositionMap.reserve(vertexCount);

    for (uint32_t i = 0; i < vertexCount; ++i)
    {
        XMFLOAT3 position = *(positions + i);
        size_t hash = Hash(position);

        auto it = uniquePositionMap.find(hash);
        if (it != uniquePositionMap.end())
        {
            // Position already encountered - reference previous index
            pointRep[i] = it->second;
        }
        else
        {
            // New position found - add to hash table and LUT
            uniquePositionMap.insert(std::make_pair(hash, static_cast<T>(i)));
            pointRep[i] = static_cast<T>(i);
        }
    }

    // Create a linked list of edges for each vertex to determine adjacency
    const uint32_t hashSize = vertexCount / 3;

    std::unique_ptr<EdgeEntry*[]> hashTable(new EdgeEntry*[hashSize]);
    std::unique_ptr<EdgeEntry[]> entries(new EdgeEntry[triCount * 3]);

    std::memset(hashTable.get(), 0, sizeof(EdgeEntry*) * hashSize);
    uint32_t entryIndex = 0;

    for (uint32_t iFace = 0; iFace < triCount; ++iFace)
    {
        uint32_t index = iFace * 3;

        // Create a hash entry in the hash table for each each.
        for (uint32_t iEdge = 0; iEdge < 3; ++iEdge)
        {
            T i0 = pointRep[indices[index + (iEdge % 3)]];
            T i1 = pointRep[indices[index + ((iEdge + 1) % 3)]];
            T i2 = pointRep[indices[index + ((iEdge + 2) % 3)]];

            auto& entry = entries[entryIndex++];
            entry.i0 = i0;
            entry.i1 = i1;
            entry.i2 = i2;

            uint32_t key = entry.i0 % hashSize;

            entry.Next = hashTable[key];
            entry.Face = iFace;

            hashTable[key] = &entry;
        }
    }


    // Initialize the adjacency list
    std::memset(adjacency, uint32_t(-1), indexCount * sizeof(uint32_t));

    for (uint32_t iFace = 0; iFace < triCount; ++iFace)
    {
        uint32_t index = iFace * 3;

        for (uint32_t point = 0; point < 3; ++point)
        {
            if (adjacency[iFace * 3 + point] != uint32_t(-1))
                continue;

            // Look for edges directed in the opposite direction.
            T i0 = pointRep[indices[index + ((point + 1) % 3)]];
            T i1 = pointRep[indices[index + (point % 3)]];
            T i2 = pointRep[indices[index + ((point + 2) % 3)]];

            // Find a face sharing this edge
            uint32_t key = i0 % hashSize;

            EdgeEntry* found = nullptr;
            EdgeEntry* foundPrev = nullptr;

            for (EdgeEntry* current = hashTable[key], *prev = nullptr; current != nullptr; prev = current, current = current->Next)
            {
                if (current->i1 == i1 && current->i0 == i0)
                {
                    found = current;
                    foundPrev = prev;
                    break;
                }
            }

            // Cache this face's normal
            XMVECTOR n0;
            {
                XMVECTOR p0 = XMLoadFloat3(&positions[i1]);
                XMVECTOR p1 = XMLoadFloat3(&positions[i0]);
                XMVECTOR p2 = XMLoadFloat3(&positions[i2]);

                XMVECTOR e0 = p0 - p1;
                XMVECTOR e1 = p1 - p2;

                n0 = XMVector3Normalize(XMVector3Cross(e0, e1));
            }

            // Use face normal dot product to determine best edge-sharing candidate.
            float bestDot = -2.0f;
            for (EdgeEntry* current = found, *prev = foundPrev; current != nullptr; prev = current, current = current->Next)
            {
                if (bestDot == -2.0f || (current->i1 == i1 && current->i0 == i0))
                {
                    XMVECTOR p0 = XMLoadFloat3(&positions[current->i0]);
                    XMVECTOR p1 = XMLoadFloat3(&positions[current->i1]);
                    XMVECTOR p2 = XMLoadFloat3(&positions[current->i2]);

                    XMVECTOR e0 = p0 - p1;
                    XMVECTOR e1 = p1 - p2;

                    XMVECTOR n1 = XMVector3Normalize(XMVector3Cross(e0, e1));

                    float dot = XMVectorGetX(XMVector3Dot(n0, n1));

                    if (dot > bestDot)
                    {
                        found = current;
                        foundPrev = prev;
                        bestDot = dot;
                    }
                }
            }

            // Update hash table and adjacency list
            if (found && found->Face != uint32_t(-1))
            {
                // Erase the found from the hash table linked list.
                if (foundPrev != nullptr)
                {
                    foundPrev->Next = found->Next;
                }
                else
                {
                    hashTable[key] = found->Next;
                }

                // Update adjacency information
                adjacency[iFace * 3 + point] = found->Face;

                // Search & remove this face from the table linked list
                uint32_t key2 = i1 % hashSize;

                for (EdgeEntry* current = hashTable[key2], *prev = nullptr; current != nullptr; prev = current, current = current->Next)
                {
                    if (current->Face == iFace && current->i1 == i0 && current->i0 == i1)
                    {
                        if (prev != nullptr)
                        {
                            prev->Next = current->Next;
                        }
                        else
                        {
                            hashTable[key2] = current->Next;
                        }

                        break;
                    }
                }

                bool linked = false;
                for (uint32_t point2 = 0; point2 < point; ++point2)
                {
                    if (found->Face == adjacency[iFace * 3 + point2])
                    {
                        linked = true;
                        adjacency[iFace * 3 + point] = uint32_t(-1);
                        break;
                    }
                }

                if (!linked)
                {
                    uint32_t edge2 = 0;
                    for (; edge2 < 3; ++edge2)
                    {
                        T k = indices[found->Face * 3 + edge2];
                        if (k == uint32_t(-1))
                            continue;

                        if (pointRep[k] == i0)
                            break;
                    }

                    if (edge2 < 3)
                    {
                        adjacency[found->Face * 3 + edge2] = iFace;
                    }
                }
            }
        }
    }
}

XMVECTOR MinimumBoundingSphere(XMFLOAT3* points, uint32_t count)
{
    assert(points != nullptr && count != 0);

    // Find the min & max points indices along each axis.
    uint32_t minAxis[3] = { 0, 0, 0 };
    uint32_t maxAxis[3] = { 0, 0, 0 };

    for (uint32_t i = 1; i < count; ++i)
    {
        float* point = (float*)(points + i);

        for (uint32_t j = 0; j < 3; ++j)
        {
            float* min = (float*)(&points[minAxis[j]]);
            float* max = (float*)(&points[maxAxis[j]]);

            minAxis[j] = point[j] < min[j] ? i : minAxis[j];
            maxAxis[j] = point[j] > max[j] ? i : maxAxis[j];
        }
    }

    // Find axis with maximum span.
    XMVECTOR distSqMax = g_XMZero;
    uint32_t axis = 0;

    for (uint32_t i = 0; i < 3u; ++i)
    {
        XMVECTOR min = XMLoadFloat3(&points[minAxis[i]]);
        XMVECTOR max = XMLoadFloat3(&points[maxAxis[i]]);

        XMVECTOR distSq = XMVector3LengthSq(max - min);
        if (XMVector3Greater(distSq, distSqMax))
        {
            distSqMax = distSq;
            axis = i;
        }
    }

    // Calculate an initial starting center point & radius.
    XMVECTOR p1 = XMLoadFloat3(&points[minAxis[axis]]);
    XMVECTOR p2 = XMLoadFloat3(&points[maxAxis[axis]]);

    XMVECTOR center = (p1 + p2) * 0.5f;
    XMVECTOR radius = XMVector3Length(p2 - p1) * 0.5f;
    XMVECTOR radiusSq = radius * radius;

    // Add all our points to bounding sphere expanding radius & recalculating center point as necessary.
    for (uint32_t i = 0; i < count; ++i)
    {
        XMVECTOR point = XMLoadFloat3(points + i);
        XMVECTOR distSq = XMVector3LengthSq(point - center);

        if (XMVector3Greater(distSq, radiusSq))
        {
            XMVECTOR dist = XMVectorSqrt(distSq);
            XMVECTOR k = (radius / dist) * 0.5f + XMVectorReplicate(0.5f);

            center = center * k + point * (g_XMOne - k);
            radius = (radius + dist) * 0.5f;
        }
    }

    // Populate a single XMVECTOR with center & radius data.
    XMVECTOR select0001 = XMVectorSelectControl(0, 0, 0, 1);
    return XMVectorSelect(center, radius, select0001);
}

