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

Context::Context(DX12Framework* pFramework) :
	m_pFramework(pFramework)
{
	InitializeListHead(&m_ActiveFrameListHead);
	InitializeListHead(&m_FreeFrameListHead);
}

Context::~Context()
{
}

HRESULT Context::CreateDeviceDependentState(D3D12_COMMAND_LIST_TYPE Type)
{
	HRESULT hr;

	ID3D12Device* pDevice = m_pFramework->GetDevice();

	D3D12_COMMAND_QUEUE_DESC Desc = {};
	Desc.Type = Type;
	Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	Desc.NodeMask = 0;
	Desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	hr = pDevice->CreateCommandQueue(&Desc, IID_PPV_ARGS(&m_pCommandQueue));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create command queue with Type=%d, hr=0x%.8x", Type, hr);
		return hr;
	}

	hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFenceObject));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create fence object, hr=0x%.8x", hr);
		return hr;
	}

	m_hFlushEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_hFlushEvent == nullptr)
	{
		LOG_ERROR("Failed to create paging flush event, Error=0x%.8x", GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

void Context::DestroyDeviceDependentState()
{
	assert(m_pCurrentFrame == nullptr);
	assert(m_ActiveFrames == 0);

	SafeRelease(m_pFenceObject);
	SafeRelease(m_pCommandQueue);

	if (m_hFlushEvent)
	{
		CloseHandle(m_hFlushEvent);
	}

	assert(IsListEmpty(&m_FreeFrameListHead));
}

void Context::WaitForFence(UINT64 Fence)
{
	UINT64 CompletedFence = m_pFenceObject->GetCompletedValue();
	if (CompletedFence >= Fence)
	{
		return;
	}

	HANDLE hWaitHandle = TlsGetValue(m_pFramework->GetThreadContextWaitHandleIndex());
	m_pFenceObject->SetEventOnCompletion(Fence, hWaitHandle);

	WaitForSingleObject(hWaitHandle, INFINITE);
}

void Context::Flush()
{
	WaitForAllFrames();
}

void Context::RetireFrameInternal(Frame* pFrame)
{
	// Retire resources.

	RemoveEntryList(pFrame);
	InsertHeadList(&m_FreeFrameListHead, pFrame);

	assert(m_ActiveFrames > 0);
	--m_ActiveFrames;

	HRESULT hr = pFrame->pCommandAllocator->Reset();
	if (FAILED(hr))
	{
		LOG_WARNING("Resetting the command allocator failed, hr=0x%.8x", hr);
	}

	RetireFrame(pFrame);

	m_LastCompletedFence = pFrame->CompletionFence;
}

void Context::Begin()
{
	assert(m_pCurrentFrame == nullptr);

	if (IsListEmpty(&m_FreeFrameListHead))
	{
		WaitForSingleFrame();
	}

	assert(!IsListEmpty(&m_FreeFrameListHead));

	LIST_ENTRY* pFrameEntry = RemoveHeadList(&m_FreeFrameListHead);
	m_pCurrentFrame = static_cast<Frame*>(pFrameEntry);
	++m_CurrentFence;
	m_pCurrentFrame->CompletionFence = m_CurrentFence;

	HRESULT hr = m_pCurrentFrame->pCommandList->Reset(m_pCurrentFrame->pCommandAllocator, nullptr);
	if (FAILED(hr))
	{
		LOG_CRITICAL_ERROR("Failed to reset command list, hr=0x%.8x", hr);
	}
}

HRESULT Context::Execute()
{
	HRESULT hr;

	hr = m_pCurrentFrame->pCommandList->Close();
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to close command list, hr=0x%.8x", hr);
		return hr;
	}

	ID3D12CommandList* ppCommandLists[] = { m_pCurrentFrame->pCommandList };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return S_OK;
}

void Context::End()
{
	assert(m_pCurrentFrame != nullptr);

	//
	// Signal can return an error during device removed, but we will hide this error from the caller
	// and instead handle any resulting errors during Present calls.
	//
	m_pCommandQueue->Signal(m_pFenceObject, m_CurrentFence);

	InsertTailList(&m_ActiveFrameListHead, m_pCurrentFrame);

	++m_ActiveFrames;

	m_pCurrentFrame = nullptr;
}

void Context::WaitForSingleFrame()
{
	if (!IsListEmpty(&m_ActiveFrameListHead))
	{
		Frame* pFrame = static_cast<Frame*>(m_ActiveFrameListHead.Flink);
		WaitForFence(pFrame->CompletionFence);

		RetireFrameInternal(pFrame);
	}
}

void Context::WaitForAllFrames()
{
	if (!IsListEmpty(&m_ActiveFrameListHead))
	{
		Frame* pLastFrame = static_cast<Frame*>(m_ActiveFrameListHead.Blink);
		WaitForFence(pLastFrame->CompletionFence);

		LIST_ENTRY* pFrameEntry = m_ActiveFrameListHead.Flink;
		while (pFrameEntry != &m_ActiveFrameListHead)
		{
			Frame* pFrame = static_cast<Frame*>(pFrameEntry);
			pFrameEntry = pFrameEntry->Flink;

			RetireFrameInternal(pFrame);
		}
	}
}

HRESULT Context::InitializeFrame(Frame* pFrame, D3D12_COMMAND_LIST_TYPE Type)
{
	HRESULT hr;

	InsertTailList(&m_FreeFrameListHead, pFrame);

	hr = m_pFramework->GetDevice()->CreateCommandAllocator(Type, IID_PPV_ARGS(&pFrame->pCommandAllocator));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create command allocator with Type=%d, hr=0x%.8x", Type, hr);
		return hr;
	}

	hr = m_pFramework->GetDevice()->CreateCommandList(0, Type, pFrame->pCommandAllocator, nullptr, IID_PPV_ARGS(&pFrame->pCommandList));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create command list with Type=%d, hr=0x%.8x", Type, hr);
		return hr;
	}

	pFrame->pCommandList->Close();

	return S_OK;
}
