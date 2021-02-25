//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):	Alex Nankervis
//

#include "ModelAssimp.h"
#include "IndexOptimizePostTransform.h"

#include <string.h>

void AssimpModel::OptimizeRemoveDuplicateVertices(bool depth)
{
    unsigned char *deduplicatedVertexData = new unsigned char [depth ? m_Header.vertexDataByteSizeDepth : m_Header.vertexDataByteSize];
    uint32_t deduplicatedVertexDataSize = 0;

    for (unsigned int meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
    {
        Mesh *mesh = m_pMesh + meshIndex;
        unsigned int vertexStride = depth ? mesh->vertexStrideDepth : mesh->vertexStride;
        unsigned char *meshVertexData = depth ? (m_pVertexDataDepth + mesh->vertexDataByteOffsetDepth) : (m_pVertexData + mesh->vertexDataByteOffset);

        unsigned char *meshDeduplicatedVertexData = deduplicatedVertexData + deduplicatedVertexDataSize;
        unsigned int deduplicatedCount = 0;

        unsigned int vertexCount = depth ? mesh->vertexCountDepth : mesh->vertexCount;
        uint32_t *vertexRemap = new uint32_t [vertexCount];
        memset(vertexRemap, (uint32_t)-1, sizeof(uint32_t) * vertexCount);
        assert(vertexCount <= (uint32_t)-1);

        for (unsigned int v1 = 0; v1 < vertexCount; v1++)
        {
            if (vertexRemap[v1] != (uint32_t)-1)
                continue; // this was already found to be a duplicate

            const unsigned char *v1Data = meshVertexData + v1 * vertexStride;

            // this is a new unique vertex
            uint32_t remappedSlot = deduplicatedCount++;
            vertexRemap[v1] = remappedSlot;
            memcpy(meshDeduplicatedVertexData + remappedSlot * vertexStride, v1Data, vertexStride);

            // scan for duplicates
            for (unsigned int v2 = v1 + 1; v2 < vertexCount; v2++)
            {
                if (vertexRemap[v2] != (uint32_t)-1)
                    continue; // this was already found to be a duplicate of another vertex

                const unsigned char *v2Data = meshVertexData + v2 * vertexStride;

                if (0 == memcmp(v1Data, v2Data, vertexStride))
                {
                    vertexRemap[v2] = remappedSlot;
                }
            }
        }

        unsigned int indexCount = mesh->indexCount;
        uint16_t *indexArray = (uint16_t*)((depth ? m_pIndexDataDepth : m_pIndexData) + mesh->indexDataByteOffset);
        for (unsigned int n = 0; n < indexCount; n++)
        {
            indexArray[n] = vertexRemap[indexArray[n]];
        }

        delete [] vertexRemap;

        if (depth)
        {
            mesh->vertexCountDepth = deduplicatedCount;
            mesh->vertexDataByteOffsetDepth = deduplicatedVertexDataSize;
        }
        else
        {
            mesh->vertexCount = deduplicatedCount;
            mesh->vertexDataByteOffset = deduplicatedVertexDataSize;
        }
        deduplicatedVertexDataSize += deduplicatedCount * vertexStride;
    }

    if (depth)
    {
        delete [] m_pVertexDataDepth;
        m_pVertexDataDepth = deduplicatedVertexData;
        m_Header.vertexDataByteSizeDepth = deduplicatedVertexDataSize;
    }
    else
    {
        delete [] m_pVertexData;
        m_pVertexData = deduplicatedVertexData;
        m_Header.vertexDataByteSize = deduplicatedVertexDataSize;
    }
}

void AssimpModel::OptimizePostTransform(bool depth)
{
    enum {lruCacheSize = 64};

    for (unsigned int meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
    {
        Mesh *mesh = m_pMesh + meshIndex;

        uint16_t *srcIndices = new uint16_t [mesh->indexCount];
        uint16_t *dstIndices = (uint16_t*)((depth ? m_pIndexDataDepth : m_pIndexData) + mesh->indexDataByteOffset);
        memcpy(srcIndices, dstIndices, sizeof(uint16_t) * mesh->indexCount);

        OptimizeFaces<uint16_t>(srcIndices, mesh->indexCount, dstIndices, lruCacheSize);

        delete [] srcIndices;
    }
}

void AssimpModel::OptimizePreTransform(bool depth)
{
    unsigned char *reorderedVertexData = new unsigned char [depth ? m_Header.vertexDataByteSizeDepth : m_Header.vertexDataByteSize];

    for (unsigned int meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
    {
        Mesh *mesh = m_pMesh + meshIndex;
        unsigned int indexCount = mesh->indexCount;
        unsigned int vertexStride = depth ? mesh->vertexStrideDepth : mesh->vertexStride;
        unsigned char *meshVertexData = depth ? (m_pVertexDataDepth + mesh->vertexDataByteOffsetDepth) : (m_pVertexData + mesh->vertexDataByteOffset);

        unsigned char *meshReorderedVertexData = reorderedVertexData + (depth ? mesh->vertexDataByteOffsetDepth : mesh->vertexDataByteOffset);
        unsigned int reorderedCount = 0;

        unsigned int vertexCount = depth ? mesh->vertexCountDepth : mesh->vertexCount;
        uint32_t *vertexRemap = new uint32_t [vertexCount];
        memset(vertexRemap, (uint32_t)-1, sizeof(uint32_t) * vertexCount);
        assert(vertexCount <= (uint32_t)-1);

        uint16_t *indexArray = (uint16_t*)((depth ? m_pIndexDataDepth : m_pIndexData) + mesh->indexDataByteOffset);
        for (unsigned int n = 0; n < indexCount; n++)
        {
            uint16_t index = indexArray[n];
            if (vertexRemap[index] == (uint32_t)-1)
            {
                // not relocated yet
                const unsigned char *vSrc = meshVertexData + index * vertexStride;
                unsigned char *vDst = meshReorderedVertexData + reorderedCount * vertexStride;
                memcpy(vDst, vSrc, vertexStride);

                vertexRemap[index] = reorderedCount;
                reorderedCount++;
            }
            indexArray[n] = vertexRemap[index];
        }

        delete [] vertexRemap;
    }

    if (depth)
    {
        delete [] m_pVertexDataDepth;
        m_pVertexDataDepth = reorderedVertexData;
    }
    else
    {
        delete [] m_pVertexData;
        m_pVertexData = reorderedVertexData;
    }
}

void AssimpModel::Optimize()
{
    // TODO: quantize/compress vertex data

    OptimizeRemoveDuplicateVertices(false);
    OptimizeRemoveDuplicateVertices(true);

    // re-order indices for post transform cache
    OptimizePostTransform(false);
    OptimizePostTransform(true);

    // re-order vertices for linear memory access
    OptimizePreTransform(false);
    OptimizePreTransform(true);
}
