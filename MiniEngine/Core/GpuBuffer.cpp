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
#include "GpuBuffer.h"
#include "GraphicsCore.h"
#include "EsramAllocator.h"
#include "CommandContext.h"
#include "BufferManager.h"

using namespace Graphics;

void GpuBuffer::Create( const std::wstring& name, eBufferType type, size_t numElements, size_t elementSize, const void* initialData )
{
	D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer(type, numElements, elementSize);

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	ASSERT_SUCCEEDED( 
		g_Device->CreateCommittedResource( &HeapProps, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, m_UsageState, nullptr, MY_IID_PPV_ARGS(&m_pResource)) );

	m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

	if (initialData)
		CommandContext::InitializeBuffer(*this, initialData, numElements * elementSize);

#ifdef RELEASE
	(name);
#else
	m_pResource->SetName(name.c_str());
#endif

	CreateDerivedViews(type, (UINT)numElements, (UINT)elementSize);
}

D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer( eBufferType type, size_t numElements, size_t elementSize )
{
	m_BufferSize = numElements * elementSize;
	m_ElementCount = numElements;
	m_ElementSize = elementSize;

	D3D12_RESOURCE_DESC Desc = {};
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Height = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.MipLevels = 1;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Width = (UINT64)(numElements * elementSize);

	return Desc;
}

void GpuBuffer::CreateDerivedViews( eBufferType type, UINT numElements, UINT elementSize )
{
	auto Device = g_Device;

	if (type == kCountedStructures)
	{
		m_CounterBuffer.reset(new GpuBuffer());
		m_CounterBuffer->Create(L"UAV Counter Buffer", kByteAddress, 1, 4 );
	}

    // Create the SRV description
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.FirstElement = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.CounterOffsetInBytes = 0;
	UAVDesc.Buffer.FirstElement = 0;

	if (type == kByteAddress || type == kIndirectArgs)
	{
		SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		SRVDesc.Buffer.NumElements = (numElements * elementSize) / 4;
		SRVDesc.Buffer.StructureByteStride = 0;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		UAVDesc.Buffer.NumElements = (numElements * elementSize) / 4;
		UAVDesc.Buffer.StructureByteStride = 0;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	}
	else
	{
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.Buffer.NumElements = numElements;
		SRVDesc.Buffer.StructureByteStride = elementSize;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.Buffer.NumElements = numElements;
		UAVDesc.Buffer.StructureByteStride = elementSize;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	}

	if (m_SRV.ptr == ~0ull)
		m_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateShaderResourceView( m_pResource.Get(), &SRVDesc, m_SRV );

	ID3D12Resource* CounterResource = m_CounterBuffer ? m_CounterBuffer->GetResource() : nullptr;
	if (m_UAV.ptr == ~0ull)
		m_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateUnorderedAccessView( m_pResource.Get(), CounterResource, &UAVDesc, m_UAV );
}

D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView( uint32_t Offset, uint32_t Size ) const
{
	ASSERT(Offset + Size <= m_BufferSize);

	Size = Math::AlignUp(Size, 16);

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
	CBVDesc.BufferLocation = m_GpuVirtualAddress + (size_t)Offset;
	CBVDesc.SizeInBytes = Size;

	D3D12_CPU_DESCRIPTOR_HANDLE hCBV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateConstantBufferView(&CBVDesc, hCBV);
	return hCBV;
}

void GpuBuffer::Destroy( void )
{
	m_CounterBuffer = nullptr;
	GpuResource::Destroy();
}

void GpuBuffer::SetCounterValue(CommandContext& Context, uint32_t Value)
{
	if (m_CounterBuffer != nullptr)
	{
		Context.FillBuffer(*m_CounterBuffer, 0, Value, sizeof(uint32_t));
		Context.TransitionResource(*m_CounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE& GpuBuffer::GetCounterSRV(CommandContext& Context)
{
	Context.TransitionResource(*m_CounterBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	return m_CounterBuffer->GetSRV(); 
}
