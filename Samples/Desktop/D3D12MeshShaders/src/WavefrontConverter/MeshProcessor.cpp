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
#include "stdafx.h"
#include "MeshProcessor.h"

#include <Utilities.h>

#include <DirectXMesh.h>

using namespace DirectX;

namespace
{
    struct Vertex
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT2 TexCoord;
    };

    // Basic hash function + syntactic helpers
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
    inline size_t CRCHash(const T& value)
    {
        static_assert((sizeof(value) & 0x3) == 0, "Template type 'T' must be a multiple of 4 bytes.");
        return CRCHash(reinterpret_cast<const uint32_t*>(&value), sizeof(value) / 4);
    }

    template <typename T>
    inline size_t Hash(const T& val)
    {
        return std::hash<T>()(val);
    }
}

namespace std
{
    // Hash template 
    template <>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& v) const
        {
            return CRCHash(v);
        }
    };
}


MeshProcessor::MeshProcessor() 
    : m_type(0)
    , m_indexSize(4)
    , m_indexCount(0)
{ }

void MeshProcessor::Reset()
{
    m_indexSubsets.clear();

    m_positions.clear();
    m_normals.clear();
    m_uvs.clear();
    m_tangents.clear();
    m_bitangents.clear();

    m_uniqueVertices.clear();

    m_faceRemap.clear();
    m_vertexRemap.clear();
    m_indexReorder.clear();

    m_positionReorder.clear();
    m_normalReorder.clear();
    m_uvReorder.clear();

    m_type = 0;
    m_indexSize = 4;
    m_indexCount = 0;
}

bool MeshProcessor::Process(const ProcessOptions& options, const WaveFrontReader<uint32_t>& reader, ExportMesh& output)
{
    if (!Extract(options, reader))
    {
        return false;
    }

    if (m_indexSize == 4)
    {
        Finalize<uint32_t>(options);
    }
    else
    {
        Finalize<uint16_t>(options);
    }

    Export(options, output);

    if (options.LogLevel >= ProcessOptions::Verbose)
    {
        std::cout << "Stats: " << std::endl;
        std::cout << "\t" << output.VertexCount << " vertices" << std::endl;
        std::cout << "\t" << output.IndexCount << " indices" << std::endl;
        std::cout << "\t" << output.IndexSize << " byte indices" << std::endl;
        std::cout << "\t" << output.IndexSubsets.size() << " material subsets" << std::endl;
        std::cout << "\t" << output.Meshlets.size() << " meshlets" << std::endl;
    }

    Reset();

    return true;
}

bool MeshProcessor::Extract(const ProcessOptions& options, const WaveFrontReader<uint32_t>& reader)
{
    m_type = AddAttribute(m_type, Attribute::Position);
    m_positions.resize(reader.vertices.size());

    if (reader.hasNormals)
    {
        m_type = AddAttribute(m_type, Attribute::Normal);
        m_normals.resize(reader.vertices.size());
    }

    if (reader.hasTexcoords)
    {
        m_type = AddAttribute(m_type, Attribute::TexCoord);
        m_uvs.resize(reader.vertices.size());
    }

    for (uint32_t i = 0; i < reader.vertices.size(); ++i)
    {
        XMStoreFloat3(&m_positions[i], XMLoadFloat3(&reader.vertices[i].position) * options.UnitScale);

        if (reader.hasNormals)
        {
            m_normals[i] = reader.vertices[i].normal;
        }

        if (reader.hasTexcoords)
        {
            m_uvs[i] = reader.vertices[i].textureCoordinate;
        }
    }

    // Determine our index properties
    m_indexCount = static_cast<uint32_t>(reader.indices.size());
    m_indexSize = m_indexCount > 65536 ? 4 : 2;
    m_indices.resize(m_indexSize * m_indexCount);

    // Copy our indices over to their final buffer.
    if (m_indexSize == 4 || options.Force32BitIndices)
    {
        // Already stored at 32-bits - simple copy over.
        std::memcpy(m_indices.data(), reader.indices.data(), m_indexCount * m_indexSize);
    }
    else
    {
        // Type conversion - must copy one value at a time.
        const uint32_t* src = reader.indices.data();
        uint8_t* dest = m_indices.data();

        for (uint32_t i = 0; i < m_indexCount; ++i, ++src, dest += m_indexSize)
        {
            std::memcpy(dest, src, m_indexSize);
        }
    }

    m_attributes.resize(reader.attributes.size());
    std::memcpy(m_attributes.data(), reader.attributes.data(), reader.attributes.size() * sizeof(uint32_t));

    return true;
}

template <typename T>
void MeshProcessor::Finalize(const ProcessOptions& options)
{
    // Pull out some final counts for readability
    const uint32_t vertexCount = static_cast<uint32_t>(m_positions.size());
    const uint32_t triCount = m_indexCount / 3;

    // Resize all our interim data buffers to appropriate sizes for the mesh
    m_positionReorder.resize(vertexCount);
    m_indexReorder.resize(m_indexCount * m_indexSize);

    m_faceRemap.resize(triCount);
    m_vertexRemap.resize(vertexCount);

    ///
    // Use DirectXMesh to optimize our vertex buffer data

    // Clean the mesh, sort faces by material, and reorder
    ThrowIfFailed(DirectX::Clean(reinterpret_cast<T*>(m_indices.data()), triCount, vertexCount, nullptr, m_attributes.data(), m_dupVerts, true));
    ThrowIfFailed(DirectX::AttributeSort(triCount, m_attributes.data(), m_faceRemap.data()));
    ThrowIfFailed(DirectX::ReorderIB(reinterpret_cast<T*>(m_indices.data()), triCount, m_faceRemap.data(), reinterpret_cast<T*>(m_indexReorder.data())));

    std::swap(m_indices, m_indexReorder);

    // Optimize triangle faces and reorder
    ThrowIfFailed(DirectX::OptimizeFacesLRU(reinterpret_cast<T*>(m_indices.data()), triCount, m_faceRemap.data()));
    ThrowIfFailed(DirectX::ReorderIB(reinterpret_cast<T*>(m_indices.data()), triCount, m_faceRemap.data(), reinterpret_cast<T*>(m_indexReorder.data())));

    std::swap(m_indices, m_indexReorder);

    // Optimize our vertex data
    ThrowIfFailed(DirectX::OptimizeVertices(reinterpret_cast<T*>(m_indices.data()), triCount, vertexCount, m_vertexRemap.data()));

    // Finalize the index & vertex buffers (potential reordering)
    ThrowIfFailed(DirectX::FinalizeIB(reinterpret_cast<T*>(m_indices.data()), triCount, m_vertexRemap.data(), vertexCount, reinterpret_cast<T*>(m_indexReorder.data())));
    ThrowIfFailed(DirectX::FinalizeVB(m_positions.data(), sizeof(XMFLOAT3), vertexCount, m_dupVerts.data(), m_dupVerts.size(), m_vertexRemap.data(), m_positionReorder.data()));

    std::swap(m_indices, m_indexReorder);
    std::swap(m_positions, m_positionReorder);

    if (HasAttribute(m_type, Attribute::Normal))
    {
        m_normalReorder.resize(vertexCount);
        ThrowIfFailed(DirectX::FinalizeVB(m_normals.data(), sizeof(XMFLOAT3), vertexCount, m_dupVerts.data(), m_dupVerts.size(), m_vertexRemap.data(), m_normalReorder.data()));

        std::swap(m_normals, m_normalReorder);
    }

    if (HasAttribute(m_type, Attribute::TexCoord))
    {
        m_uvReorder.resize(vertexCount);
        ThrowIfFailed(DirectX::FinalizeVB(m_uvs.data(), sizeof(XMFLOAT2), vertexCount, m_dupVerts.data(), m_dupVerts.size(), m_vertexRemap.data(), m_uvReorder.data()));

        std::swap(m_uvs, m_uvReorder);
    }

    // Populate material subset data
    auto subsets = DirectX::ComputeSubsets(m_attributes.data(), m_attributes.size());

    m_indexSubsets.resize(subsets.size());
    for (uint32_t i = 0; i < subsets.size(); ++i)
    {
        m_indexSubsets[i].Offset = static_cast<uint32_t>(subsets[i].first) * 3;
        m_indexSubsets[i].Count = static_cast<uint32_t>(subsets[i].second) * 3;
    }

    if (!HasAttribute(m_type, Attribute::Normal))
    {
        m_normals.resize(vertexCount);

        ThrowIfFailed(DirectX::ComputeNormals(reinterpret_cast<T*>(m_indices.data()), triCount, m_positions.data(), vertexCount, CNORM_DEFAULT, m_normals.data()));
    }

    // Compute our tangent frame
    if (HasAttribute(m_type, Attribute::Normal) &&
        HasAttribute(m_type, Attribute::TexCoord))
    {
        m_type = AddAttribute(m_type, Attribute::Tangent);
        m_type = AddAttribute(m_type, Attribute::Bitangent);

        m_tangents.resize(vertexCount);
        m_bitangents.resize(vertexCount);

        ThrowIfFailed(DirectX::ComputeTangentFrame(reinterpret_cast<T*>(m_indices.data()), triCount, m_positions.data(), m_normals.data(), m_uvs.data(), vertexCount, m_tangents.data(), m_bitangents.data()));
    }

    // Meshletize our mesh and generate per-meshlet culling data
    ThrowIfFailed(ComputeMeshlets(
        options.MeshletMaxVerts, options.MeshletMaxPrims,
        reinterpret_cast<T*>(m_indices.data()), m_indexCount,
        m_indexSubsets.data(), static_cast<uint32_t>(m_indexSubsets.size()),
        m_positions.data(), static_cast<uint32_t>(m_positions.size()),
        m_meshletSubsets,
        m_meshlets,
        m_uniqueVertexIndices,
        m_primitiveIndices
    ));

    m_cullData.resize(m_meshlets.size());

    ThrowIfFailed(ComputeCullData(
        m_positions.data(), static_cast<uint32_t>(m_positions.size()),
        m_meshlets.data(), static_cast<uint32_t>(m_meshlets.size()),
        reinterpret_cast<T*>(m_uniqueVertexIndices.data()),
        m_primitiveIndices.data(),
        CNORM_DEFAULT,
        m_cullData.data()
    ));
}

void MeshProcessor::Export(const ProcessOptions& options, ExportMesh& output)
{
    // Determine vertex layout - mapping the extracted mesh type to the output type
    const uint32_t s_sizeMap[] = 
    {
        12, // Position
        12, // Normal
        8,  // TexCoord
        12, // Tangent
        12, // Bitangent
    };

    // Determine our final layout attributes
    for (auto& layout : options.ExportAttributes)
    {
        ExportMesh::StreamDesc desc;
        desc.Stride = 0;

        for (auto& attr : layout)
        {
            if (!HasAttribute(m_type, attr))
            {
                continue;
            }

            Attribute attribute;
            attribute.Type = attr;
            attribute.Offset = desc.Stride;

            desc.Attributes.push_back(attribute);
            desc.Stride += s_sizeMap[attr];
        }

        if (desc.Stride > 0)
        {
            output.Layout.push_back(desc);
        }
    }

    // Copy vertex data to export streams
    const uint32_t vertexCount = static_cast<uint32_t>(m_positions.size());

    output.VertexCount = vertexCount;
    output.Vertices.resize(output.Layout.size());

    for (uint32_t i = 0; i < output.Layout.size(); ++i)
    {
        output.Vertices[i].resize(output.Layout[i].Stride * vertexCount);
    }

    for (uint32_t i = 0; i < output.Layout.size(); ++i)
    {
        auto& stream = output.Layout[i];

        for (uint32_t j = 0; j < stream.Attributes.size(); ++j)
        {
            auto& attr = stream.Attributes[j];

            uint8_t* dest = output.Vertices[i].data() + attr.Offset;

            const uint8_t* src = nullptr;
            uint32_t size = 0;

            switch (attr.Type)
            {
            case Attribute::Position:
                src = reinterpret_cast<uint8_t*>(m_positions.data());
                size = sizeof(XMFLOAT3);
                break;

            case Attribute::Normal:
                src = reinterpret_cast<uint8_t*>(m_normals.data());
                size = sizeof(XMFLOAT3);
                break;

            case Attribute::TexCoord:
                src = reinterpret_cast<uint8_t*>(m_uvs.data());
                size = sizeof(XMFLOAT2);
                break;

            case Attribute::Tangent:
                src = reinterpret_cast<uint8_t*>(m_tangents.data());
                size = sizeof(XMFLOAT3);
                break;

            case Attribute::Bitangent:
                src = reinterpret_cast<uint8_t*>(m_bitangents.data());
                size = sizeof(XMFLOAT3);
                break;

            default:
                throw std::exception("Invalid attribute type.");
            }

            for (uint32_t k = 0; k < vertexCount; ++k, src += size, dest += stream.Stride)
            {
                std::memcpy(dest, src, size);
            }
        }
    }

    // Copy index data to export stream
    output.IndexCount = m_indexCount;
    output.IndexSize = m_indexSize;

    output.Indices.resize(m_indexSize * m_indexCount);
    std::memcpy(output.Indices.data(), m_indices.data(), m_indexSize * m_indexCount);

    std::swap(output.IndexSubsets, m_indexSubsets);

    // Copy meshlet data to export streams
    std::swap(output.MeshletSubsets, m_meshletSubsets);
    std::swap(output.Meshlets, m_meshlets);
    std::swap(output.UniqueVertexIndices, m_uniqueVertexIndices);
    std::swap(output.PrimitiveIndices, m_primitiveIndices);
    std::swap(output.CullData, m_cullData);
}
