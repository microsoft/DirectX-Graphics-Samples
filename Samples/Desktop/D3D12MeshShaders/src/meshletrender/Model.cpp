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
#include "Model.h"

#include "DXSampleHelper.h"

#include <fstream>
#include <unordered_set>

using namespace Microsoft::WRL;

namespace
{
    using Attribute = Mesh::Attribute;
    using Subset = Mesh::Subset;
    using Meshlet = Mesh::Meshlet;
    using CullData = Mesh::CullData;
    using PackedTriangle = Mesh::PackedTriangle;

    D3D12_INPUT_ELEMENT_DESC s_elementDescs[Attribute::Count] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
    };


    enum EType : uint32_t
    {
        Position,
        Normal,
        TexCoord,
        Tangent,
        Bitangent,
    };

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
}

HRESULT Model::LoadFromFile(const wchar_t* filename)
{
     std::ifstream stream(filename, std::ios::binary);
     if (!stream.is_open())
     {
         return E_INVALIDARG;
     }

     std::vector<MeshHeader> meshes;
     std::vector<BufferView> bufferViews;
     std::vector<Accessor> accessors;
     
     FileHeader header;
     stream.read(reinterpret_cast<char*>(&header), sizeof(header));

     if (header.Prolog != s_prolog)
     {
         return E_FAIL; // Incorrect file format.
     }

     if (header.Version != CURRENT_FILE_VERSION)
     {
         return E_FAIL; // Version mismatch between export and import serialization code.
     }

     // Read mesh metdata
     meshes.resize(header.MeshCount);
     stream.read(reinterpret_cast<char*>(meshes.data()), meshes.size() * sizeof(meshes[0]));
     
     accessors.resize(header.AccessorCount);
     stream.read(reinterpret_cast<char*>(accessors.data()), accessors.size() * sizeof(accessors[0]));

     bufferViews.resize(header.BufferViewCount);
     stream.read(reinterpret_cast<char*>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

     m_buffer.resize(header.BufferSize);
     stream.read(reinterpret_cast<char*>(m_buffer.data()), header.BufferSize);

     char eofbyte;
     stream.read(&eofbyte, 1); // Read last byte to hit the eof bit

     assert(stream.eof()); // There's a problem if we didn't completely consume the file contents.

     stream.close();

     // Populate mesh data from binary data and metadata.
     m_meshes.resize(meshes.size());
     for (uint32_t i = 0; i < static_cast<uint32_t>(meshes.size()); ++i)
     {
         auto& meshView = meshes[i];
         auto& mesh = m_meshes[i];

         // Index data
         {
             Accessor& accessor = accessors[meshView.Indices];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.IndexSize = accessor.Size;
             mesh.IndexCount = accessor.Count;

             mesh.Indices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
         }

         // Index Subset data
         {
             Accessor& accessor = accessors[meshView.IndexSubsets];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.IndexSubsets = MakeSpan(reinterpret_cast<Subset*>(m_buffer.data() + bufferView.Offset), accessor.Count);
         }

         // Vertex data & layout metadata

         // Determine the number of unique Buffer Views associated with the vertex attributes & copy vertex buffers.
         std::vector<uint32_t> vbMap;

         mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
         mesh.LayoutDesc.NumElements = 0;

         for (uint32_t j = 0; j < Attribute::Count; ++j)
         {
             if (meshView.Attributes[j] == -1)
                 continue;

             Accessor& accessor = accessors[meshView.Attributes[j]];
             
             auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
             if (it != vbMap.end())
             {
                 continue; // Already added - continue.
             }

             // New buffer view encountered; add to list and copy vertex data
             vbMap.push_back(accessor.BufferView);
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.VertexStrides.push_back(accessor.Stride);
             mesh.Vertices.push_back(MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size));
         }

         // Populate the vertex buffer metadata from accessors.
         for (uint32_t j = 0; j < Attribute::Count; ++j)
         {
             if (meshView.Attributes[j] == -1)
                 continue;

             Accessor& accessor = accessors[meshView.Attributes[j]];

             // Determine which vertex buffer index holds this attribute's data
             auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);

             D3D12_INPUT_ELEMENT_DESC desc = s_elementDescs[j];
             desc.InputSlot = static_cast<uint32_t>(std::distance(vbMap.begin(), it));

             mesh.LayoutElems[mesh.LayoutDesc.NumElements++] = desc;
         }

         // Meshlet data
         {
             Accessor& accessor = accessors[meshView.Meshlets];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.Meshlets = MakeSpan(reinterpret_cast<Meshlet*>(m_buffer.data() + bufferView.Offset), accessor.Count);
         }

         // Meshlet Subset data
         {
             Accessor& accessor = accessors[meshView.MeshletSubsets];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.MeshletSubsets = MakeSpan(reinterpret_cast<Subset*>(m_buffer.data() + bufferView.Offset), accessor.Count);
         }

         // Unique Vertex Index data
         {
             Accessor& accessor = accessors[meshView.UniqueVertexIndices];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.UniqueVertexIndices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
         }

         // Primitive Index data
         {
             Accessor& accessor = accessors[meshView.PrimitiveIndices];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.PrimitiveIndices = MakeSpan(reinterpret_cast<PackedTriangle*>(m_buffer.data() + bufferView.Offset), accessor.Count);
         }

         // Cull data
         {
             Accessor& accessor = accessors[meshView.CullData];
             BufferView& bufferView = bufferViews[accessor.BufferView];

             mesh.CullingData = MakeSpan(reinterpret_cast<CullData*>(m_buffer.data() + bufferView.Offset), accessor.Count);
         }
     }

     return S_OK;
}

HRESULT Model::UploadGpuResources(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{
    // Create single resource for entire mesh
    auto dataDesc = CD3DX12_RESOURCE_DESC::Buffer(m_buffer.size());

    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &dataDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_resource)));

    // Calculate offset virtual addresses.
    const uint8_t* baseAddr = m_buffer.data();
    D3D12_GPU_VIRTUAL_ADDRESS baseVA = m_resource->GetGPUVirtualAddress();

    for (uint32_t i = 0; i < m_meshes.size(); ++i)
    {
        auto& m = m_meshes[i];

        int64_t offset = m.Indices.data() - baseAddr;
        m.IndicesVA = baseVA + offset;

        m.IBView.BufferLocation = m.IndicesVA;
        m.IBView.Format = m.IndexSize == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        m.IBView.SizeInBytes = m.IndexCount * m.IndexSize;

        m.VerticesVA.resize(m.Vertices.size());
        m.VBViews.resize(m.Vertices.size());

        for (uint32_t j = 0; j < m.Vertices.size(); ++j)
        {
            auto& vb = m.Vertices[j];

            offset = vb.data() - baseAddr;
            m.VerticesVA[j] = baseVA + offset;

            m.VBViews[j].BufferLocation = m.VerticesVA[j];
            m.VBViews[j].SizeInBytes = static_cast<uint32_t>(m.Vertices[j].size());
            m.VBViews[j].StrideInBytes = m.VertexStrides[j];
        }

        offset = reinterpret_cast<uint8_t*>(m.MeshletSubsets.data()) - baseAddr;
        m.MeshletSubsetsVA = baseVA + offset;

        offset = reinterpret_cast<uint8_t*>(m.Meshlets.data()) - baseAddr;
        m.MeshletsVA = baseVA + offset;

        offset = reinterpret_cast<uint8_t*>(m.UniqueVertexIndices.data()) - baseAddr;
        m.UniqueVertexIndicesVA = baseVA + offset;

        offset = reinterpret_cast<uint8_t*>(m.PrimitiveIndices.data()) - baseAddr;
        m.PrimitiveIndicesVA = baseVA + offset;

        offset = reinterpret_cast<uint8_t*>(m.CullingData.data()) - baseAddr;
        m.CullDataVA = baseVA + offset;
    }

    // Upload resource
    {
        // Create upload resource
        auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        ComPtr<ID3D12Resource> upload;
        ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &dataDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

        // Map & copy memory to upload heap
        void* memory = nullptr;
        upload->Map(0, nullptr, &memory);
        std::memcpy(memory, m_buffer.data(), m_buffer.size());
        upload->Unmap(0, nullptr);

        // Create our sync fence
        ComPtr<ID3D12Fence> fence;
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        // Populate our command list
        cmdList->Reset(cmdAlloc, nullptr);
        cmdList->CopyResource(m_resource.Get(), upload.Get());

        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        cmdList->ResourceBarrier(1, &barrier);
        cmdList->Close();

        ID3D12CommandList* ppCommandLists[] = { cmdList };
        cmdQueue->ExecuteCommandLists(1, ppCommandLists);
        cmdQueue->Signal(fence.Get(), 1);

        // Wait for GPU
        if (fence->GetCompletedValue() != 1)
        {
            HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            fence->SetEventOnCompletion(1, event);

            WaitForSingleObjectEx(event, INFINITE, false);
            CloseHandle(event);
        }
    }

    return S_OK;
}
