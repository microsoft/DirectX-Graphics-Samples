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
#include "Export.h"

#include <fstream>
#include <iostream>

namespace
{
    const char s_padding[4096] = {};
    
    const uint32_t s_sizeMap[] =
    {
        12, // Position
        12, // Normal
        8,  // TexCoord
        12, // Tangent
        12, // Bitangent
    };

    const uint32_t s_prolog = 'MSHL';

    enum FileVersion
    {
        FILE_VERSION_INITIAL = 0,
        CURRENT_FILE_VERSION = FILE_VERSION_INITIAL
    };

    struct FileHeader
    {
        uint32_t Prolog;
        uint32_t Version;

        uint32_t MeshCount;
        uint32_t AccessorCount;
        uint32_t BufferViewCount;
        uint32_t BufferSize;
    };

    struct MeshHeader
    {
        uint32_t Indices;
        uint32_t IndexSubsets;
        uint32_t Attributes[Attribute::Count];

        uint32_t Meshlets;
        uint32_t MeshletSubsets;
        uint32_t UniqueVertexIndices;
        uint32_t PrimitiveIndices;
        uint32_t CullData;
    };

    struct BufferView
    {
        uint32_t Offset;
        uint32_t Size;
    };

    struct Accessor
    {
        uint32_t BufferView;
        uint32_t Offset;
        uint32_t Size;
        uint32_t Stride;
        uint32_t Count;
    };

    void AddAlignUp(uint32_t& offset, uint32_t size, uint32_t align = 4096u)
    {
        offset += size;

        uint32_t pad = (align - offset % align);
        offset += pad;
    }

    template <typename T>
    inline void WriteAlignUp(std::ostream& stream, const T* data, uint32_t count, uint32_t& offset, uint32_t align = 4096u)
    {
        stream.write(reinterpret_cast<const char*>(data), sizeof(T) * count);
        offset += sizeof(T) * count;

        uint32_t pad = (align - offset % align);

        stream.write(s_padding, pad);
        offset += pad;
    }
}


bool ExportMeshes(const char* filename, std::vector<ExportMesh>& exportMeshes)
{
    // Build data descriptors and compute data length
    std::vector<MeshHeader> meshes;
    std::vector<Accessor> accessors;
    std::vector<BufferView> bufferViews;

    uint32_t dataOffset = 0;
    for (auto& m : exportMeshes)
    {
        MeshHeader meshView;

        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = m.IndexCount * m.IndexSize;

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = m.IndexCount;
            accessor.Offset = 0;
            accessor.Size = m.IndexSize;
            accessor.Stride = m.IndexSize;

            meshView.Indices = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = static_cast<uint32_t>(m.IndexSubsets.size()) * sizeof(m.IndexSubsets[0]);

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = static_cast<uint32_t>(m.IndexSubsets.size());
            accessor.Offset = 0;
            accessor.Size = sizeof(m.IndexSubsets[0]);
            accessor.Stride = sizeof(m.IndexSubsets[0]);

            meshView.IndexSubsets = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        for (auto& l : m.Layout)
        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = m.VertexCount * l.Stride;

            for (uint32_t k = 0; k < _countof(meshView.Attributes); ++k)
            {
                meshView.Attributes[k] = uint32_t(-1);
            }
            
            for (auto& a : l.Attributes)
            {
                Accessor accessor;
                accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
                accessor.Count = m.VertexCount;
                accessor.Offset = a.Offset;
                accessor.Size = s_sizeMap[a.Type];
                accessor.Stride = l.Stride;

                meshView.Attributes[a.Type] = static_cast<uint32_t>(accessors.size());
                accessors.push_back(accessor);
            }


            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        // Meshlet data
        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = static_cast<uint32_t>(m.Meshlets.size()) * sizeof(m.Meshlets[0]);

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = static_cast<uint32_t>(m.Meshlets.size());
            accessor.Offset = 0;
            accessor.Size = sizeof(m.Meshlets[0]);
            accessor.Stride = sizeof(m.Meshlets[0]);

            meshView.Meshlets = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = static_cast<uint32_t>(m.MeshletSubsets.size()) * sizeof(m.MeshletSubsets[0]);

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = static_cast<uint32_t>(m.MeshletSubsets.size());
            accessor.Offset = 0;
            accessor.Size = sizeof(m.MeshletSubsets[0]);
            accessor.Stride = sizeof(m.MeshletSubsets[0]);

            meshView.MeshletSubsets = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        // UniqueVertexIndex Data
        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = static_cast<uint32_t>(m.UniqueVertexIndices.size());

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = static_cast<uint32_t>(m.UniqueVertexIndices.size()) / m.IndexSize;
            accessor.Offset = 0;
            accessor.Size = m.IndexSize;
            accessor.Stride = m.IndexSize;

            meshView.UniqueVertexIndices = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        // PrimitiveIndex Data
        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = static_cast<uint32_t>(m.PrimitiveIndices.size()) * sizeof(m.PrimitiveIndices[0]);

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = static_cast<uint32_t>(m.PrimitiveIndices.size());
            accessor.Offset = 0;
            accessor.Size = sizeof(m.PrimitiveIndices[0]);
            accessor.Stride = sizeof(m.PrimitiveIndices[0]);

            meshView.PrimitiveIndices = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        // Cull Data
        {
            BufferView bufferView;
            bufferView.Offset = dataOffset;
            bufferView.Size = static_cast<uint32_t>(m.CullData.size()) * sizeof(m.CullData[0]);

            Accessor accessor;
            accessor.BufferView = static_cast<uint32_t>(bufferViews.size());
            accessor.Count = static_cast<uint32_t>(m.CullData.size());
            accessor.Offset = 0;
            accessor.Size = sizeof(m.CullData[0]);
            accessor.Stride = sizeof(m.CullData[0]);

            meshView.CullData = static_cast<uint32_t>(accessors.size());
            accessors.push_back(accessor);

            bufferViews.push_back(bufferView);
            AddAlignUp(dataOffset, bufferView.Size);
        }

        meshes.push_back(meshView);
    }

    // Populate the file header
    FileHeader header;
    header.Prolog = s_prolog;
    header.Version = CURRENT_FILE_VERSION;
    header.MeshCount = static_cast<uint32_t>(meshes.size());
    header.AccessorCount = static_cast<uint32_t>(accessors.size());
    header.BufferViewCount = static_cast<uint32_t>(bufferViews.size());
    header.BufferSize = dataOffset;


    // Open file for writes and serialize metadata followed by data.
    std::ofstream stream(filename, std::ios::binary);
    if (!stream.is_open())
    {
        std::cout << "Failed to open file '" << filename << "' for writing." << std::endl;
        return false;
    }

    // Serialize the metadata
    stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
    stream.write(reinterpret_cast<const char*>(meshes.data()), meshes.size() * sizeof(meshes[0]));
    stream.write(reinterpret_cast<const char*>(accessors.data()), accessors.size() * sizeof(accessors[0]));
    stream.write(reinterpret_cast<const char*>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

    uint32_t offset = 0;

    // Serialize the mesh data
    for (auto& m : exportMeshes)
    {
        // Index data
        WriteAlignUp(stream, m.Indices.data(), static_cast<uint32_t>(m.Indices.size()), offset);
        WriteAlignUp(stream, m.IndexSubsets.data(), static_cast<uint32_t>(m.IndexSubsets.size()), offset);

        // Vertex data
        for (auto& vb : m.Vertices)
        {
            WriteAlignUp(stream, vb.data(), static_cast<uint32_t>(vb.size()), offset);
        }

        // Meshlet data
        WriteAlignUp(stream, m.Meshlets.data(), static_cast<uint32_t>(m.Meshlets.size()), offset);
        WriteAlignUp(stream, m.MeshletSubsets.data(), static_cast<uint32_t>(m.MeshletSubsets.size()), offset);
        WriteAlignUp(stream, m.UniqueVertexIndices.data(), static_cast<uint32_t>(m.UniqueVertexIndices.size()), offset);
        WriteAlignUp(stream, m.PrimitiveIndices.data(), static_cast<uint32_t>(m.PrimitiveIndices.size()), offset);
        WriteAlignUp(stream, m.CullData.data(), static_cast<uint32_t>(m.CullData.size()), offset);
    }

    stream.close();
    return true;
}
