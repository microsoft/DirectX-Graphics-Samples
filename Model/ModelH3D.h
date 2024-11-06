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
// Author(s):   Alex Nankervis
//              James Stanard
//

#pragma once

#include "VectorMath.h"
#include "TextureManager.h"
#include "GpuBuffer.h"
#include "DescriptorHeap.h"
#include "../Core/Math/BoundingBox.h"

namespace Renderer
{
    struct ModelData;
}

class ModelH3D
{
public:

    ModelH3D();
    ~ModelH3D();

    void Clear();

    enum
    {
        attrib_mask_0 = (1 << 0),
        attrib_mask_1 = (1 << 1),
        attrib_mask_2 = (1 << 2),
        attrib_mask_3 = (1 << 3),
        attrib_mask_4 = (1 << 4),
        attrib_mask_5 = (1 << 5),
        attrib_mask_6 = (1 << 6),
        attrib_mask_7 = (1 << 7),
        attrib_mask_8 = (1 << 8),
        attrib_mask_9 = (1 << 9),
        attrib_mask_10 = (1 << 10),
        attrib_mask_11 = (1 << 11),
        attrib_mask_12 = (1 << 12),
        attrib_mask_13 = (1 << 13),
        attrib_mask_14 = (1 << 14),
        attrib_mask_15 = (1 << 15),

        // friendly name aliases
        attrib_mask_position = attrib_mask_0,
        attrib_mask_texcoord0 = attrib_mask_1,
        attrib_mask_normal = attrib_mask_2,
        attrib_mask_tangent = attrib_mask_3,
        attrib_mask_bitangent = attrib_mask_4,
    };

    enum
    {
        attrib_0 = 0,
        attrib_1 = 1,
        attrib_2 = 2,
        attrib_3 = 3,
        attrib_4 = 4,
        attrib_5 = 5,
        attrib_6 = 6,
        attrib_7 = 7,
        attrib_8 = 8,
        attrib_9 = 9,
        attrib_10 = 10,
        attrib_11 = 11,
        attrib_12 = 12,
        attrib_13 = 13,
        attrib_14 = 14,
        attrib_15 = 15,

        // friendly name aliases
        attrib_position = attrib_0,
        attrib_texcoord0 = attrib_1,
        attrib_normal = attrib_2,
        attrib_tangent = attrib_3,
        attrib_bitangent = attrib_4,

        maxAttribs = 16
    };

    enum
    {
        attrib_format_none = 0,
        attrib_format_ubyte,
        attrib_format_byte,
        attrib_format_ushort,
        attrib_format_short,
        attrib_format_float,

        attrib_formats
    };

    struct Header
    {
        uint32_t meshCount;
        uint32_t materialCount;
        uint32_t vertexDataByteSize;
        uint32_t indexDataByteSize;
        uint32_t vertexDataByteSizeDepth;
        Math::AxisAlignedBox boundingBox;
    };

	struct Attrib
	{
		uint16_t offset; // byte offset from the start of the vertex
		uint16_t normalized; // if true, integer formats are interpreted as [-1, 1] or [0, 1]
		uint16_t components; // 1-4
		uint16_t format;
	};
	struct Mesh
    {
        Math::AxisAlignedBox boundingBox;

        uint32_t materialIndex;

        uint32_t attribsEnabled;
        uint32_t attribsEnabledDepth;
        uint32_t vertexStride;
        uint32_t vertexStrideDepth;
        Attrib attrib[maxAttribs];
        Attrib attribDepth[maxAttribs];

        uint32_t vertexDataByteOffset;
        uint32_t vertexCount;
        uint32_t indexDataByteOffset;
        uint32_t indexCount;

        uint32_t vertexDataByteOffsetDepth;
        uint32_t vertexCountDepth;
    };

    struct Material
    {
        Color diffuse;
        Color specular;
        Color ambient;
        Color emissive;
        Color transparent; // light passing through a transparent surface is multiplied by this filter color
        float opacity;
        float shininess; // specular exponent
        float specularStrength; // multiplier on top of specular color

        enum {maxTexPath = 128};
        enum {texCount = 6};
        char texDiffusePath[maxTexPath];
        char texSpecularPath[maxTexPath];
        char texEmissivePath[maxTexPath];
        char texNormalPath[maxTexPath];
        char texLightmapPath[maxTexPath];
        char texReflectionPath[maxTexPath];

        enum {maxMaterialName = 128};
        char name[maxMaterialName];
    };

    virtual bool Load(const std::wstring& filename)
	{
		return LoadH3D(filename);
    }

    bool BuildModel(Renderer::ModelData& model, const std::wstring& basePath=L"") const;

    uint32_t GetMeshCount() const { return m_Header.meshCount; }
    const Mesh& GetMesh(uint32_t meshIdx) const
    {
        ASSERT(meshIdx < m_Header.meshCount);
        return m_pMesh[meshIdx];
    }

    uint32_t GetMaterialCount() const { return m_Header.materialCount; }
    const Material& GetMaterial(uint32_t materialIdx) const
    {
        ASSERT(materialIdx < m_Header.materialCount);
        return m_pMaterial[materialIdx];
    }

	const Math::AxisAlignedBox& GetBoundingBox() const { return m_Header.boundingBox; }

    uint32_t GetVertexStride() const { return m_VertexStride; }
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBuffer() const { return m_VertexBuffer; }
    const D3D12_INDEX_BUFFER_VIEW& GetIndexBuffer() const { return m_IndexBuffer; }

    DescriptorHandle GetSRVs(uint32_t materialIdx, uint32_t subIdx=0) const
    {
        return m_SRVs + (materialIdx * 6 + subIdx) * m_SRVDescriptorSize;
    }

    const TextureRef* GetMaterialTextures(uint32_t materialIdx) const
    {
        return m_TextureReferences.data() + materialIdx * 3;
    }

    void CreateVertexBufferSRV(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) const;
    void CreateIndexBufferSRV(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) const;

//protected:

	bool LoadH3D(const std::wstring& filename);
	bool SaveH3D(const std::wstring& filename) const;

	void ComputeMeshBoundingBox(uint32_t meshIndex, Math::AxisAlignedBox& bbox) const;
	void ComputeGlobalBoundingBox(Math::AxisAlignedBox& bbox) const;
	void ComputeAllBoundingBoxes();

    void LoadTextures(const std::wstring& basePath);

    Header m_Header;
    Mesh *m_pMesh;
    Material *m_pMaterial;
    std::vector<TextureRef> m_TextureReferences;

    DescriptorHandle m_SRVs;
    uint32_t m_SRVDescriptorSize;

    // These are used at runtime during rendering
    ByteAddressBuffer m_GeometryBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_VertexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBuffer;
    uint32_t m_VertexStride;

    // optimized for depth-only rendering
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferDepth;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferDepth;
    uint32_t m_VertexStrideDepth;

    // These are allocated when constructing a model to be exported
    unsigned char* m_pVertexData;
    unsigned char* m_pIndexData;
    unsigned char* m_pVertexDataDepth;
    unsigned char* m_pIndexDataDepth;
};
