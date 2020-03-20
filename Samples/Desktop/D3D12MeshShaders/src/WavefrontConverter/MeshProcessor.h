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

#include "WaveFrontReader.h"

#include <D3D12MeshletGenerator.h>

struct Attribute
{
    enum EType : uint32_t
    {
        Position,
        Normal,
        TexCoord,
        Tangent,
        Bitangent,
        Count
    };

    EType       Type;
    uint32_t    Offset;
};

inline bool HasAttribute(uint32_t base, Attribute::EType check) { return (base & (1 << check)) != 0; }
inline uint32_t AddAttribute(uint32_t base, Attribute::EType add) { return base | (1 << add); }

using AttrStream = std::vector<Attribute::EType>;
using AttrLayout = std::vector<AttrStream>;


struct ExportMesh
{
    using DataStream = std::vector<uint8_t>;
    struct StreamDesc
    {
        std::vector<Attribute> Attributes;
        uint32_t Stride;
    };

    std::vector<StreamDesc>         Layout;
    std::vector<DataStream>         Vertices;
    uint32_t                        VertexCount;

    std::vector<Subset>             IndexSubsets;
    DataStream                      Indices;
    uint32_t                        IndexSize;
    uint32_t                        IndexCount;

    std::vector<Subset>             MeshletSubsets;
    std::vector<Meshlet>            Meshlets;
    std::vector<uint8_t>            UniqueVertexIndices;
    std::vector<PackedTriangle>     PrimitiveIndices;
    std::vector<CullData>           CullData;
};


struct ProcessOptions
{
    enum ELogVerbosity
    {
        Error,
        Basic,
        Verbose
    };

    uint32_t        MeshletMaxVerts;
    uint32_t        MeshletMaxPrims;
    AttrLayout      ExportAttributes;
    float           UnitScale;
    bool            Force32BitIndices;
    bool            Flip;
    ELogVerbosity   LogLevel;

    ProcessOptions(void)
        : MeshletMaxVerts(64)
        , MeshletMaxPrims(126)
        , UnitScale(1.0f)
        , ExportAttributes{}
        , Force32BitIndices(false)
        , Flip(false)
        , LogLevel(Basic)
    { }
};

class MeshProcessor
{
public:
    MeshProcessor();

    bool Process(const ProcessOptions& options, const WaveFrontReader<uint32_t>& reader, ExportMesh& output);

private:
    void Reset();
    bool Extract(const ProcessOptions& options, const WaveFrontReader<uint32_t>& reader);
    void Export(const ProcessOptions& options, ExportMesh& output);

    template <typename T> 
    void Finalize(const ProcessOptions& options);

private:
    std::vector<uint32_t>                   m_attributes;
    std::vector<Subset>                     m_indexSubsets;
    uint32_t                                m_type;

    std::vector<uint8_t>                    m_indices;
    uint32_t                                m_indexSize;
    uint32_t                                m_indexCount;

    std::vector<DirectX::XMFLOAT3>          m_positions;
    std::vector<DirectX::XMFLOAT3>          m_normals;
    std::vector<DirectX::XMFLOAT2>          m_uvs;
    std::vector<DirectX::XMFLOAT3>          m_tangents;
    std::vector<DirectX::XMFLOAT3>          m_bitangents;

    std::vector<Subset>                     m_meshletSubsets;
    std::vector<Meshlet>                    m_meshlets;
    std::vector<uint8_t>                    m_uniqueVertexIndices;
    std::vector<PackedTriangle>             m_primitiveIndices;
    std::vector<CullData>                   m_cullData;

    std::unordered_map<size_t, uint32_t>    m_uniqueVertices;

    std::vector<uint32_t>                   m_faceRemap;
    std::vector<uint32_t>                   m_vertexRemap;
    std::vector<uint8_t>                    m_indexReorder;
    std::vector<uint32_t>                   m_dupVerts;

    std::vector<DirectX::XMFLOAT3>          m_positionReorder;
    std::vector<DirectX::XMFLOAT3>          m_normalReorder;
    std::vector<DirectX::XMFLOAT2>          m_uvReorder;
};
