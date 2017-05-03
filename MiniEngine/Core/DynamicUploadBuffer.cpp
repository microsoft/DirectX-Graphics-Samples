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
// Author:  James Stanard 
//

#include "pch.h"
#include "GraphicsCore.h"
#include "DynamicUploadBuffer.h"

using namespace Graphics;

void DynamicUploadBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize)
{
    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;
    HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC ResourceDesc;
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Width = NumElements * ElementSize;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ASSERT_SUCCEEDED( g_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, MY_IID_PPV_ARGS(&m_pResource)) );

    m_pResource->SetName(name.c_str());

    m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
    m_CpuVirtualAddress = nullptr;
}

void DynamicUploadBuffer::Destroy( void )
{
    if (m_pResource.Get() != nullptr)
    {
        if (m_CpuVirtualAddress != nullptr)
            Unmap();

        m_pResource = nullptr;
        m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }
}

void* DynamicUploadBuffer::Map( void )
{
    ASSERT(m_CpuVirtualAddress == nullptr, "Buffer is already locked");
    ASSERT_SUCCEEDED(m_pResource->Map(0, nullptr, &m_CpuVirtualAddress));
    return m_CpuVirtualAddress;
}

void DynamicUploadBuffer::Unmap( void )
{
    ASSERT(m_CpuVirtualAddress != nullptr, "Buffer is not locked");
    m_pResource->Unmap(0, nullptr);
    m_CpuVirtualAddress = nullptr;
}

D3D12_VERTEX_BUFFER_VIEW DynamicUploadBuffer::VertexBufferView(uint32_t NumVertices, uint32_t Stride, uint32_t Offset ) const
{
    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = m_GpuVirtualAddress + Offset;
    vbv.SizeInBytes = NumVertices * Stride;
    vbv.StrideInBytes = Stride;
    return vbv;
}

D3D12_INDEX_BUFFER_VIEW DynamicUploadBuffer::IndexBufferView(uint32_t NumIndices, bool _32bit, uint32_t Offset ) const
{
    D3D12_INDEX_BUFFER_VIEW ibv;
    ibv.BufferLocation = m_GpuVirtualAddress + Offset;
    ibv.Format = _32bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    ibv.SizeInBytes = NumIndices * (_32bit ? 4 : 2);
    return ibv;
}
