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

// Encapsulates a typed array view into a blob of data.
template <typename T>
class Span
{
public:
    Span()
        : m_data(nullptr)
        , m_count(0)
    { }

    Span(T* data, uint32_t count)
        : m_data(data)
        , m_count(count)
    { }

    // std library container interface
    T* data() { return m_data; }
    const T* data() const { return m_data; }

    size_t size() const { return m_count; }

    // Iterator interface
    T* begin() { return m_data; }
    T* end() { return m_data + m_count; }

private:
    T* m_data;
    uint32_t m_count;
};

template <typename T>
Span<T> MakeSpan(T* data, uint32_t size) { return Span<T>(data, size); }



struct Mesh
{
    using DataStream = Span<uint8_t>;

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

        EType   Type;
        uint32_t Offset;
    };

    struct Subset
    {
        uint32_t Offset;
        uint32_t Count;
    };

    struct Meshlet
    {
        uint32_t VertCount;
        uint32_t VertOffset;
        uint32_t PrimCount;
        uint32_t PrimOffset;
    };

    union PackedTriangle
    {
        struct
        {
            uint32_t i0 : 10;
            uint32_t i1 : 10;
            uint32_t i2 : 10;
            uint32_t _unused : 2;
        } indices;
        uint32_t packed;
    };

    struct CullData
    {
        DirectX::XMFLOAT4 BoundingSphere; // xyz = center, w = radius
        uint8_t           NormalCone[4];  // xyz = axis, w = sin(a + 90)
        float             ApexOffset;     // apex = center - axis * offset
    };

    
    D3D12_INPUT_ELEMENT_DESC                LayoutElems[Attribute::Count];
    D3D12_INPUT_LAYOUT_DESC                 LayoutDesc;

    std::vector<DataStream>                 Vertices;
    std::vector<uint32_t>                   VertexStrides;
    uint32_t                                VertexCount;

    Span<Subset>                            IndexSubsets;
    DataStream                              Indices;
    uint32_t                                IndexSize;
    uint32_t                                IndexCount;

    Span<Subset>                            MeshletSubsets;
    Span<Meshlet>                           Meshlets;
    Span<uint8_t>                           UniqueVertexIndices;
    Span<PackedTriangle>                    PrimitiveIndices;
    Span<CullData>                          CullingData;
    
    // D3D resource references
    std::vector<D3D12_VERTEX_BUFFER_VIEW>  VBViews;
    D3D12_INDEX_BUFFER_VIEW                IBView;

    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> VerticesVA;
    D3D12_GPU_VIRTUAL_ADDRESS              IndicesVA;
    D3D12_GPU_VIRTUAL_ADDRESS              MeshletSubsetsVA;
    D3D12_GPU_VIRTUAL_ADDRESS              MeshletsVA;
    D3D12_GPU_VIRTUAL_ADDRESS              UniqueVertexIndicesVA;
    D3D12_GPU_VIRTUAL_ADDRESS              PrimitiveIndicesVA;
    D3D12_GPU_VIRTUAL_ADDRESS              CullDataVA;
};

class Model
{
public:
    HRESULT LoadFromFile(const wchar_t* filename);
    HRESULT UploadGpuResources(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);

    uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_meshes.size()); }
    const Mesh& GetMesh(uint32_t i) const { return m_meshes[i]; }

    // Iterator interface
    auto begin() { return m_meshes.begin(); }
    auto end() { return m_meshes.end(); }

private:
    std::vector<Mesh> m_meshes;
    
    std::vector<uint8_t> m_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
};
