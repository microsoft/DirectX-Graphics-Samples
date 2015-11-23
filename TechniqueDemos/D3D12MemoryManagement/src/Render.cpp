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

//
// Render contexts should use direct command lists to execute on the GPU's 3D engine.
//
#define RENDER_CONTEXT_COMMAND_LIST_TYPE D3D12_COMMAND_LIST_TYPE_DIRECT

RenderContext::RenderContext(DX12Framework* pFramework) :
	Context(pFramework)
{
}

RenderContext::~RenderContext()
{
}

HRESULT RenderContext::CreateDeviceDependentState(UINT FrameCount)
{
	HRESULT hr;

	hr = Context::CreateDeviceDependentState(RENDER_CONTEXT_COMMAND_LIST_TYPE);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize render context");
		return hr;
	}

	try
	{
		//
		// Create the frame tracking objects for the context.
		//
		for (UINT i = 0; i < FrameCount; ++i)
		{
			RenderFrame* pFrame = new RenderFrame();

			hr = InitializeFrame(pFrame, RENDER_CONTEXT_COMMAND_LIST_TYPE);
			if (FAILED(hr))
			{
				LOG_WARNING("Failed to initialize frame object, hr=0x%.8x", hr);
				return hr;
			}

			hr = m_pFramework->InitDynamicBuffer(&pFrame->VertexBuffer);
			if (FAILED(hr))
			{
				LOG_WARNING("Failed to initialize dynamic vertex buffer");
				return hr;
			}

			hr = m_pFramework->InitDynamicBuffer(&pFrame->ConstantBuffer);
			if (FAILED(hr))
			{
				LOG_WARNING("Failed to initialize dynamic constant buffer");
				return hr;
			}

			hr = m_pFramework->InitDynamicDescriptorHeap(&pFrame->SrvCbvHeap);
			if (FAILED(hr))
			{
				LOG_WARNING("Failed to initialize dynamic descriptor heap");
				return hr;
			}

			InitializeListHead(&pFrame->PendingBufferListHead);
			InitializeListHead(&pFrame->PendingHeapListHead);
		}
	}
	catch (std::bad_alloc&)
	{
		//
		// Frames are inserted into the free frame list on creation, so the destructor
		// will be able to clean up any created frames.
		//
		LOG_ERROR("Failed to allocate frame tracking structure");
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

void RenderContext::DestroyDeviceDependentState()
{
	Flush();

	assert(IsListEmpty(&m_ActiveFrameListHead));

	LIST_ENTRY* pEntry = m_FreeFrameListHead.Flink;

	while (pEntry != &m_FreeFrameListHead)
	{
		RenderFrame* pFrame = static_cast<RenderFrame*>(pEntry);
		pEntry = pEntry->Flink;

		RemoveEntryList(pFrame);

		SafeRelease(pFrame->pCommandAllocator);
		SafeRelease(pFrame->pCommandList);
		m_pFramework->DestroyDynamicBuffer(&pFrame->VertexBuffer);
		m_pFramework->DestroyDynamicBuffer(&pFrame->ConstantBuffer);
		m_pFramework->DestroyDynamicDescriptorHeap(&pFrame->SrvCbvHeap);

		SafeDelete(pFrame);
	}

	Context::DestroyDeviceDependentState();
}

void RenderContext::CloseBuffer(Buffer* pBuffer)
{
	RenderFrame* pFrame = (RenderFrame*)m_pCurrentFrame;
	if (pFrame == nullptr)
	{
		LOG_CRITICAL_ERROR("Trying to close a buffer for renaming outside the context of a frame");
		return;
	}

	InsertTailList(&pFrame->PendingBufferListHead, pBuffer);
}

void RenderContext::CloseHeap(DescriptorHeap* pHeap)
{
	RenderFrame* pFrame = (RenderFrame*)m_pCurrentFrame;
	if (pFrame == nullptr)
	{
		LOG_CRITICAL_ERROR("Trying to close a buffer for renaming outside the context of a frame");
		return;
	}

	InsertTailList(&pFrame->PendingHeapListHead, pHeap);
}

void RenderContext::RetireFrame(Frame* pFrame)
{
	RenderFrame* pRenderFrame = (RenderFrame*)pFrame;

	LIST_ENTRY* pEntry;

	pEntry = RemoveHeadList(&pRenderFrame->PendingBufferListHead);
	while (pEntry != &pRenderFrame->PendingBufferListHead)
	{
		Buffer* pBuffer = (Buffer*)pEntry;
		m_pFramework->RetireVersionedBuffer(pBuffer);

		pEntry = RemoveHeadList(&pRenderFrame->PendingBufferListHead);
	}

	pEntry = RemoveHeadList(&pRenderFrame->PendingHeapListHead);
	while (pEntry != &pRenderFrame->PendingHeapListHead)
	{
		DescriptorHeap* pDescriptorHeap = (DescriptorHeap*)pEntry;
		m_pFramework->RetireVersionedDescriptorHeap(pDescriptorHeap);

		pEntry = RemoveHeadList(&pRenderFrame->PendingHeapListHead);
	}
}
