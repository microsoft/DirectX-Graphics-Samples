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
// Author:  Alex Nankervis
//

#include "Model.h"
#include <string.h>
#include <float.h>

Model::Model()
    : m_pMesh(nullptr)
    , m_pMaterial(nullptr)
    , m_pVertexData(nullptr)
    , m_pIndexData(nullptr)
    , m_pVertexDataDepth(nullptr)
    , m_pIndexDataDepth(nullptr)
    , m_SRVs(nullptr)
{
    Clear();
}

Model::~Model()
{
    Clear();
}

void Model::Clear()
{
    m_VertexBuffer.Destroy();
    m_IndexBuffer.Destroy();
    m_VertexBufferDepth.Destroy();
    m_IndexBufferDepth.Destroy();

    delete [] m_pMesh;
    m_pMesh = nullptr;
    m_Header.meshCount = 0;

    delete [] m_pMaterial;
    m_pMaterial = nullptr;
    m_Header.materialCount = 0;

    delete [] m_pVertexData;
    delete [] m_pIndexData;
    delete [] m_pVertexDataDepth;
    delete [] m_pIndexDataDepth;

    m_pVertexData = nullptr;
    m_Header.vertexDataByteSize = 0;
    m_pIndexData = nullptr;
    m_Header.indexDataByteSize = 0;
    m_pVertexDataDepth = nullptr;
    m_Header.vertexDataByteSizeDepth = 0;
    m_pIndexDataDepth = nullptr;

    ReleaseTextures();

    m_Header.boundingBox.min = Vector3(0.0f);
    m_Header.boundingBox.max = Vector3(0.0f);
}

// assuming at least 3 floats for position
void Model::ComputeMeshBoundingBox(unsigned int meshIndex, BoundingBox &bbox) const
{
    const Mesh *mesh = m_pMesh + meshIndex;

    if (mesh->vertexCount > 0)
    {
        unsigned int vertexStride = mesh->vertexStride;

        const float *p = (float*)(m_pVertexData + mesh->vertexDataByteOffset + mesh->attrib[attrib_position].offset);
        const float *pEnd = (float*)(m_pVertexData + mesh->vertexDataByteOffset + mesh->vertexCount * mesh->vertexStride + mesh->attrib[attrib_position].offset);
        bbox.min = Scalar(FLT_MAX);
        bbox.max = Scalar(-FLT_MAX);

        while (p < pEnd)
        {
            Vector3 pos(*(p + 0), *(p + 1), *(p + 2));

            bbox.min = Min(bbox.min, pos);
            bbox.max = Max(bbox.max, pos);

            (*(uint8_t**)&p) += vertexStride;
        }
    }
    else
    {
        bbox.min = Scalar(0.0f);
        bbox.max = Scalar(0.0f);
    }
}

void Model::ComputeGlobalBoundingBox(BoundingBox &bbox) const
{
    if (m_Header.meshCount > 0)
    {
        bbox.min = Scalar(FLT_MAX);
        bbox.max = Scalar(-FLT_MAX);
        for (unsigned int meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
        {
            const Mesh *mesh = m_pMesh + meshIndex;

            bbox.min = Min(bbox.min, mesh->boundingBox.min);
            bbox.max = Max(bbox.max, mesh->boundingBox.max);
        }
    }
    else
    {
        bbox.min = Scalar(0.0f);
        bbox.max = Scalar(0.0f);
    }
}

void Model::ComputeAllBoundingBoxes()
{
    for (unsigned int meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
    {
        Mesh *mesh = m_pMesh + meshIndex;
        ComputeMeshBoundingBox(meshIndex, mesh->boundingBox);
    }
    ComputeGlobalBoundingBox(m_Header.boundingBox);
}
