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

#include "Span.h"

#include <DirectXCollision.h>

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

    EType    Type;
    uint32_t Offset;
};

struct Subset
{
    uint32_t Offset;
    uint32_t Count;
};

struct MeshInfo
{
    uint32_t IndexSize;
    uint32_t MeshletCount;

    uint32_t LastMeshletVertCount;
    uint32_t LastMeshletPrimCount;
};

struct Meshlet
{
    uint32_t VertCount;
    uint32_t VertOffset;
    uint32_t PrimCount;
    uint32_t PrimOffset;
};

struct PackedTriangle
{
    uint32_t i0 : 10;
    uint32_t i1 : 10;
    uint32_t i2 : 10;
};

struct CullData
{
    DirectX::XMFLOAT4 BoundingSphere; // xyz = center, w = radius
    uint8_t           NormalCone[4];  // xyz = axis, w = -cos(a + 90)
    float             ApexOffset;     // apex = center - axis * offset
};

struct Mesh
{
    D3D12_INPUT_ELEMENT_DESC   LayoutElems[Attribute::Count];
    D3D12_INPUT_LAYOUT_DESC    LayoutDesc;

    std::vector<Span<uint8_t>> Vertices;
    std::vector<uint32_t>      VertexStrides;
    uint32_t                   VertexCount;
    DirectX::BoundingSphere    BoundingSphere;

    Span<Subset>               IndexSubsets;
    Span<uint8_t>              Indices;
    uint32_t                   IndexSize;
    uint32_t                   IndexCount;

    Span<Subset>               MeshletSubsets;
    Span<Meshlet>              Meshlets;
    Span<uint8_t>              UniqueVertexIndices;
    Span<PackedTriangle>       PrimitiveIndices;
    Span<CullData>             CullingData;

    // D3D resource references
    std::vector<D3D12_VERTEX_BUFFER_VIEW>  VBViews;
    D3D12_INDEX_BUFFER_VIEW                IBView;

    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> VertexResources;
    Microsoft::WRL::ComPtr<ID3D12Resource>              IndexResource;
    Microsoft::WRL::ComPtr<ID3D12Resource>              MeshletResource;
    Microsoft::WRL::ComPtr<ID3D12Resource>              UniqueVertexIndexResource;
    Microsoft::WRL::ComPtr<ID3D12Resource>              PrimitiveIndexResource;
    Microsoft::WRL::ComPtr<ID3D12Resource>              CullDataResource;
    Microsoft::WRL::ComPtr<ID3D12Resource>              MeshInfoResource;

    // Calculates the number of instances of the last meshlet which can be packed into a single threadgroup.
    uint32_t GetLastMeshletPackCount(uint32_t subsetIndex, uint32_t maxGroupVerts, uint32_t maxGroupPrims) 
    { 
        if (Meshlets.size() == 0)
            return 0;

        auto& subset = MeshletSubsets[subsetIndex];
        auto& meshlet = Meshlets[subset.Offset + subset.Count - 1];

        return min(maxGroupVerts / meshlet.VertCount, maxGroupPrims / meshlet.PrimCount);
    }

    void GetPrimitive(uint32_t index, uint32_t& i0, uint32_t& i1, uint32_t& i2) const
    {
        auto prim = PrimitiveIndices[index];
        i0 = prim.i0;
        i1 = prim.i1;
        i2 = prim.i2;
    }

    uint32_t GetVertexIndex(uint32_t index) const
    {
        const uint8_t* addr = UniqueVertexIndices.data() + index * IndexSize;
        if (IndexSize == 4)
        {
            return *reinterpret_cast<const uint32_t*>(addr);
        }
        else 
        {
            return *reinterpret_cast<const uint16_t*>(addr);
        }
    }
};

class Model
{
public:
    HRESULT LoadFromFile(const wchar_t* filename);
    HRESULT UploadGpuResources(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);

    uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_meshes.size()); }
    const Mesh& GetMesh(uint32_t i) const { return m_meshes[i]; }

    const DirectX::BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }

    // Iterator interface
    auto begin() { return m_meshes.begin(); }
    auto end() { return m_meshes.end(); }

private:
    std::vector<Mesh>                      m_meshes;
    DirectX::BoundingSphere                m_boundingSphere;

    std::vector<uint8_t>                   m_buffer;
};
