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

_Use_decl_annotations_
HRESULT DynamicBuffer::Allocate(UINT32 ElementSize, UINT32 ElementCount, void** pData, UINT32* pOffset)
{
	UINT32 AllocationSize = ElementSize * ElementCount;
	assert(AllocationSize <= DYNAMIC_BUFFER_SIZE);

	ULONG_PTR pCurrentAddress = (ULONG_PTR)m_pCurrentAddress;
	ULONG_PTR pEndAddress = (ULONG_PTR)m_pBuffer->pBaseAddress + DYNAMIC_BUFFER_SIZE;

	if (pCurrentAddress + AllocationSize > pEndAddress)
	{
		//
		// Allocation would overflow buffer, return out of memory to signal to
		// the caller to flush whatever they're doing, rename the buffer,
		// and try again afterwards.
		//
		return E_OUTOFMEMORY;
	}

	*pData = (void*)pCurrentAddress;
	*pOffset = (UINT32)(m_pCurrentAddress - m_pBuffer->pBaseAddress);

	m_pCurrentAddress = pCurrentAddress + AllocationSize;
	return S_OK;
}

HRESULT DX12Framework::AllocateVersionedBuffer(Buffer** ppBuffer)
{
	//
	// Grab a buffer from our lookaside list if possible.
	//
	if (!IsListEmpty(&m_DynamicBufferListHead))
	{
		LIST_ENTRY* pEntry = RemoveHeadList(&m_DynamicBufferListHead);
		*ppBuffer = static_cast<Buffer*>(pEntry);
		return S_OK;
	}

	//
	// No available buffers, let's try to allocate a new one.
	//
	HRESULT hr;
	ID3D12Resource* pD3DBuffer = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	void* pBaseAddress = nullptr;

	hr = m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(DYNAMIC_BUFFER_SIZE),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pD3DBuffer));
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create D3D12 buffer resource, hr=0x%.8x", hr);
		goto cleanup;
	}

	//
	// Obtain a permanent CPU visible address for this buffer. In D3D12, it is okay to
	// keep a permanent virtual address mapped to an allocation while the resource is
	// accessed by the GPU. In this case, the semantics of the mapping operation are
	// always equivalent to the D3D11 MAP_WRITE_NO_OVERWRITE value.
	//
	hr = pD3DBuffer->Map(0, &readRange, &pBaseAddress);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to map base address for buffer, hr=0x%.8x", hr);
		goto cleanup;
	}

	Buffer* pBuffer = nullptr;
	try
	{
		pBuffer = new Buffer();
	}
	catch (std::bad_alloc&)
	{
		LOG_WARNING("Failed to allocate dynamic buffer");
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	pBuffer->pBaseAddress = (ULONG_PTR)pBaseAddress;
	pBuffer->pBuffer = pD3DBuffer;

	*ppBuffer = pBuffer;

	return S_OK;

cleanup:
	if (pBaseAddress)
	{
		pD3DBuffer->Unmap(0, nullptr);
	}
	SafeRelease(pD3DBuffer);
	return hr;
}

void DX12Framework::RetireVersionedBuffer(Buffer* pBuffer)
{
	//
	// Buffer is done being accessed by the GPU; add it to the lookaside list to be reused later.
	// A lookaside list will greatly reduce the overhead as buffers get renamed.
	//
	InsertHeadList(&m_DynamicBufferListHead, pBuffer);
}

void DX12Framework::RenameDynamicBuffer(DynamicBuffer* pDynamicBuffer)
{
	HRESULT hr;

retry_allocation:
	Buffer* pBuffer = nullptr;
	hr = AllocateVersionedBuffer(&pBuffer);

	if (pBuffer != nullptr)
	{
		//
		// New allocation successful, so we do not need to wait. Release this one
		// to the frame so it can be retired and reused later.
		//
		m_RenderContext.CloseBuffer(pDynamicBuffer->m_pBuffer);
	}
	else
	{
		//
		// Check if there are pending frames in flight. If so, we should wait a frame
		// and try again. In that time we may retire a pending buffer, or may end up
		// trimming or freeing enough memory to allocate a new one.
		//

		if (m_RenderContext.GetActiveFrameCount() > 0)
		{
			m_RenderContext.WaitForSingleFrame();

			if (m_RenderContext.GetActiveFrameCount() > 0)
			{
				//
				// Only retry the allocation if this was not our last frame. Otherwise
				// our current buffer is already retired, so let's just reuse it at no
				// cost.
				//
				goto retry_allocation;
			}
		}

		//
		// We've waited for all pending frames and weren't able to allocate a new buffer,
		// so we can just reuse the current one.
		//
		pBuffer = pDynamicBuffer->m_pBuffer;
	}

	//
	// Assign the new versioned buffer, and set the current pointer to the start.
	//
	pDynamicBuffer->m_pBuffer = pBuffer;
	pDynamicBuffer->m_pCurrentAddress = pBuffer->pBaseAddress;
}

//
// Aligns the buffer's current location to the provided value. Alignment does not
// need to be a power of two. This function is used to allow a single vertex buffer
// to be shared across multiple vertex types (of different sizes).
//
void DynamicBuffer::Align(UINT32 Alignment)
{
	ULONG_PTR Offset = (m_pCurrentAddress - m_pBuffer->pBaseAddress) + Alignment - 1;
	ULONG_PTR AlignedAligned = Offset - (Offset % Alignment);
	m_pCurrentAddress = m_pBuffer->pBaseAddress + AlignedAligned;
}

//
// Dynamic heaps are conceptually the same concept as dynamic buffers, but for descriptors
// instead of buffer data.
//
HRESULT DX12Framework::AllocateVersionedDescriptorHeap(DescriptorHeap** ppHeap)
{
	//
	// Grab a heap from our lookaside list if possible.
	//
	if (!IsListEmpty(&m_DynamicDescriptorHeapListHead))
	{
		LIST_ENTRY* pEntry = RemoveHeadList(&m_DynamicDescriptorHeapListHead);
		*ppHeap = static_cast<DescriptorHeap*>(pEntry);
		return S_OK;
	}

	//
	// No available buffers, let's try to allocate a new one.
	//
	HRESULT hr;
	ID3D12DescriptorHeap* pD3DDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
	Desc.NumDescriptors = DYNAMIC_HEAP_SIZE;
	Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = m_pDevice->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pD3DDescriptorHeap));
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create D3D12 descriptor heap, hr=0x%.8x", hr);
		goto cleanup;
	}

	DescriptorHeap* pHeap = nullptr;
	try
	{
		pHeap = new DescriptorHeap();
	}
	catch (std::bad_alloc&)
	{
		LOG_WARNING("Failed to allocate dynamic descriptor heap");
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	pHeap->pHeap = pD3DDescriptorHeap;

	*ppHeap = pHeap;

	return S_OK;

cleanup:
	SafeRelease(pD3DDescriptorHeap);
	return hr;
}

void DX12Framework::RetireVersionedDescriptorHeap(DescriptorHeap* pHeap)
{
	//
	// Heap is done being accessed by the GPU; add it to the lookaside list to be reused later.
	// A lookaside list will greatly reduce the overhead as heaps get renamed.
	//
	InsertHeadList(&m_DynamicDescriptorHeapListHead, pHeap);
}

void DX12Framework::RenameDynamicDescriptorHeap(DynamicDescriptorHeap* pDynamicHeap)
{
	HRESULT hr;

retry_allocation:
	DescriptorHeap* pHeap = nullptr;
	hr = AllocateVersionedDescriptorHeap(&pHeap);

	if (pHeap != nullptr)
	{
		//
		// New allocation successful, so we do not need to wait. Release this one
		// to the frame so it can be retired and reused later.
		//
		m_RenderContext.CloseHeap(pDynamicHeap->m_pHeap);
	}
	else
	{
		//
		// Check if there are pending frames in flight. If so, we should wait a frame
		// and try again. In that time we may retire a pending buffer, or may end up
		// trimming or freeing enough memory to allocate a new one.
		//
		if (m_RenderContext.GetActiveFrameCount() > 0)
		{
			m_RenderContext.WaitForSingleFrame();

			if (m_RenderContext.GetActiveFrameCount() > 0)
			{
				//
				// Only retry the allocation if this was not our last frame. Otherwise
				// our current heap is already retired, so let's just reuse it at no
				// cost.
				//
				goto retry_allocation;
			}
		}

		//
		// We've waited for all pending frames and weren't able to allocate a new heap,
		// so we can just reuse the current one.
		//
		pHeap = pDynamicHeap->m_pHeap;
	}

	//
	// Assign the new versioned heap, and set the current descriptor to the start.
	//
	pDynamicHeap->m_pHeap = pHeap;
	pDynamicHeap->m_CurrentHandleIndex = 0;
}
