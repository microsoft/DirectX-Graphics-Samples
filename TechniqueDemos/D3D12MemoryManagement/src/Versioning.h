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

//
// A Buffer that encapsulates a D3D12 resource, and the base address of the allocation.
// A buffer object is part of a dynamic buffer, and can be versioned (or renamed) on
// demand.
//
struct Buffer : LIST_ENTRY
{
	ID3D12Resource* pBuffer;
	ULONG_PTR pBaseAddress;
};

//
// A DynamicBuffer allows the application to allocate buffer storage from a backing
// D3D12 resource, while maintaining information used for versioning the buffers as
// they run out of space.
//
class DynamicBuffer
{
	friend class DX12Framework;

private:
	Buffer* m_pBuffer;
	ULONG_PTR m_pCurrentAddress;

public:
	//
	// Align does not need to be a power of two, simply a boundary to align to. This is
	// used to share the vertex buffer across shaders by aligning the vertices
	// to multiples of the vertex stride.
	//
	void Align(UINT32 Alignment);
	HRESULT Allocate(UINT32 ElementSize, UINT32 ElementCount, _Outptr_ void** pData, UINT32* pOffset);

	inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{
		return m_pBuffer->pBuffer->GetGPUVirtualAddress();
	}

	inline void Reset()
	{
		m_pCurrentAddress = m_pBuffer->pBaseAddress;
	}
};

//
// A DescriptorHeap keeps track of a single instance of a renamable D3D12 descriptor
// heap. It is part of a DynamicDescriptorHeap object, which can be versioned on demand.
//
struct DescriptorHeap : LIST_ENTRY
{
	ID3D12DescriptorHeap* pHeap;
};

//
// A DynamicDescriptorHeap allows the application to allocate descriptors from a backing
// D3D12 descriptor heap, while maintaining information used for versioning the heaps as
// they run out of space.
//
class DynamicDescriptorHeap
{
	friend class DX12Framework;

private:
	DescriptorHeap* m_pHeap;
	UINT32 m_CurrentHandleIndex;
	UINT32 m_DescriptorSize;

public:
	DynamicDescriptorHeap() :
		m_CurrentHandleIndex(0)
	{
	}

	HRESULT Allocate(UINT32 ElementCount, D3D12_GPU_DESCRIPTOR_HANDLE* pGpuHandleStart, D3D12_CPU_DESCRIPTOR_HANDLE* pCpuHandleStart)
	{
		if (m_CurrentHandleIndex + ElementCount > DYNAMIC_HEAP_SIZE)
		{
			return E_OUTOFMEMORY;
		}

		UINT32 Offset = m_CurrentHandleIndex * m_DescriptorSize;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle(m_pHeap->pHeap->GetGPUDescriptorHandleForHeapStart(), Offset);
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle(m_pHeap->pHeap->GetCPUDescriptorHandleForHeapStart(), Offset);

		*pGpuHandleStart = GpuHandle;
		*pCpuHandleStart = CpuHandle;

		m_CurrentHandleIndex += ElementCount;

		return S_OK;
	}

	void Reset()
	{
		m_CurrentHandleIndex = 0;
	}
};
