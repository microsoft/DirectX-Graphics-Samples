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

#pragma once

#include "pch.h"
#include "GpuResource.h"
#include "EsramAllocator.h"

enum eBufferType
{
	kByteAddress,
	kStructures,
	kCountedStructures,
	kIndirectArgs
};

class CommandContext;

class GpuBuffer : public GpuResource
{
public:
	GpuBuffer()
	{
		m_UAV.ptr = ~0ull;
		m_SRV.ptr = ~0ull;
	}

	~GpuBuffer() { Destroy(); }

	void Destroy();

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create( const std::wstring& name, eBufferType type, size_t numElements, size_t elementSize,
		const void* initialData = nullptr );

	// Create a buffer in ESRAM.  On Windows, ESRAM is not used.
	void Create( const std::wstring& name, eBufferType type, size_t numElements, size_t elementSize, EsramAllocator& esramAllocator,
		const void* initialData = nullptr);

	size_t GetSize( void ) const						{ return m_BufferSize; }
	size_t GetElementCount( void ) const				{ return m_ElementCount; }
	size_t GetElementSize( void ) const					{ return m_ElementSize; }

	const D3D12_CPU_DESCRIPTOR_HANDLE&	GetUAV() const			{ return m_UAV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE&	GetSRV() const			{ return m_SRV; }

	const D3D12_CPU_DESCRIPTOR_HANDLE&	GetCounterSRV(CommandContext& Context);
	void SetCounterValue(CommandContext& Context, uint32_t Value);

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, size_t Size, size_t Stride) const;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t BaseVertexIndex = 0) const
	{
		size_t Offset = BaseVertexIndex * m_ElementSize;
		return VertexBufferView(Offset, m_BufferSize - Offset, m_ElementSize);
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t Offset, size_t Size, bool b32Bit = false) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t StartIndex = 0) const
	{
		size_t Offset = StartIndex * m_ElementSize;
		return IndexBufferView(Offset, m_BufferSize - Offset, m_ElementSize == 4);
	}

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView() const;

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView( uint32_t Offset, uint32_t Size ) const;

	GpuBuffer* GetCounterBuffer() const { return m_CounterBuffer.get(); }

private:

	D3D12_RESOURCE_DESC DescribeBuffer( eBufferType type, size_t numElements, size_t elementSize );
	void CreateDerivedViews( eBufferType type, UINT numElements, UINT elementSize );

	D3D12_CPU_DESCRIPTOR_HANDLE					m_UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE					m_SRV;

	std::unique_ptr<GpuBuffer>					m_CounterBuffer;

	size_t										m_BufferSize;
	size_t										m_ElementCount;
	size_t										m_ElementSize;
};

inline D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView( size_t Offset, size_t Size, size_t Stride ) const
{
	D3D12_VERTEX_BUFFER_VIEW VBView;
	VBView.BufferLocation = m_GpuVirtualAddress + Offset;
	VBView.SizeInBytes = (UINT)Size;
	VBView.StrideInBytes = (UINT)Stride;
	return VBView;
}

inline D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView( size_t Offset, size_t Size, bool b32Bit) const
{
	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = m_GpuVirtualAddress + Offset;
	IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	IBView.SizeInBytes = (UINT)Size;
	return IBView;
}

inline D3D12_GPU_VIRTUAL_ADDRESS GpuBuffer::RootConstantBufferView() const
{
	return m_GpuVirtualAddress;
}

inline void GpuBuffer::Create(
	const std::wstring& name, eBufferType type, size_t numElements, size_t elementSize, EsramAllocator& esramAllocator,
	const void* initialData )
{
	(void)esramAllocator;
	Create(name, type, numElements, elementSize, initialData);
}
