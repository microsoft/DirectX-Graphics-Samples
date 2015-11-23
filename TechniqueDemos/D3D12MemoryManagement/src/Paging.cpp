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
// PagingWorkerThread
//
// The paging worker thread is the powerhouse of the sample. This is where all the streaming,
// prefetching, trimming, etc occurs. The worker thread is responsible for creating and
// mapping the heaps for the resource mipmaps as they are needed.
//

PagingWorkerThread::PagingWorkerThread(DX12Framework* pFramework) :
	m_pFramework(pFramework),
	m_hThread(nullptr),
	m_CurrentStatus(EWTS_Suspended),
	m_RequestedStatus(EWTS_Suspended),
	m_BudgetNotificationCookie(0)
{
	InitializeListHead(&m_PrioritizationListHead);
	for (int i = 0; i < _ERP_COUNT; ++i)
	{
		InitializeListHead(&m_PriorityQueues[i]);
	}

	InitializeCriticalSection(&m_PrioritizationListLock);

	ZeroMemory(m_hWakeEvents, sizeof(m_hWakeEvents));
}

PagingWorkerThread::~PagingWorkerThread()
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		//
		// Signal the paging thread to exit, and wait for completion. The paging thread
		// will drain (discard) any additional work and exit the thread's main function.
		//
		SetStatus(EWTS_Shutdown);

		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
	}

	for (UINT i = 0; i < _countof(m_hWakeEvents); ++i)
	{
		if (m_hWakeEvents[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hWakeEvents[i]);
		}
	}

	if (m_hStatusChangeEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hStatusChangeEvent);
	}

	if (m_BudgetNotificationCookie != 0)
	{
		m_pFramework->GetAdapter()->UnregisterVideoMemoryBudgetChangeNotification(m_BudgetNotificationCookie);
	}
}

HRESULT PagingWorkerThread::Init()
{
	HRESULT hr;

	for (UINT i = 0; i < _countof(m_hWakeEvents); ++i)
	{
		HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (hEvent == nullptr)
		{
			LOG_ERROR("Failed to create paging wake event handle, Error=0x%.8x", GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}

		m_hWakeEvents[i] = hEvent;
	}

	m_hStatusChangeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_hStatusChangeEvent == nullptr)
	{
		LOG_ERROR("Failed to create status change event, Error=0x%.8x", GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	m_hThread = CreateThread(nullptr, 0, PagingWorkerThread::ThreadEntry, this, 0, nullptr);
	if (m_hThread == nullptr)
	{
		LOG_ERROR("Failed to create paging thread, Error=0x%.8x", GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	//
	// Register for a budget change notification from the graphics kernel. This event will be
	// signaled by kernel any time the process budget changes. The application is then responsible
	// for responding to that notification, either by trimming resources in the event that the
	// budget is lowered, or to prefetch more data if the budget increases.
	//
	IDXGIAdapter3* pDXGIAdapter = m_pFramework->GetAdapter();
	hr = pDXGIAdapter->RegisterVideoMemoryBudgetChangeNotificationEvent(m_hWakeEvents[EWR_BudgetNotification], &m_BudgetNotificationCookie);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to register budget change notification event, hr=0x%.8x", hr);
		return hr;
	}

	assert(m_BudgetNotificationCookie != 0);

	return S_OK;
}

void PagingWorkerThread::Flush()
{
	bool bMoreWork = true;
	while (bMoreWork)
	{
		ProcessSubmission(&bMoreWork);
	}
}

DWORD CALLBACK PagingWorkerThread::ThreadEntry(void* pArg)
{
	PagingWorkerThread* pWorkerThread = (PagingWorkerThread*)pArg;

	//
	// Create a waitable event in TLS for the paging thread. This lets us wait on rendering
	// fences to complete before trimming visible mipmaps.
	//
	HANDLE hFlushEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!hFlushEvent)
	{
		LOG_CRITICAL_ERROR("Failed to create thread wait event, Error=0x%.8x", GetLastError());
	}

	TlsSetValue(pWorkerThread->m_pFramework->GetThreadContextWaitHandleIndex(), (void*)hFlushEvent);

	//
	// Run worker thread until we are issued a shutdown request.
	//
	DWORD Result = pWorkerThread->Run();

	CloseHandle(hFlushEvent);

	return Result;
}

DWORD PagingWorkerThread::Run()
{
	bool bMoreWork = false;

	for (;;)
	{
		//
		// By using a timeout of 0 when there is more work, it allows us to detect when
		// an event was signaled and needs processing, but otherwise returns immediately
		// if there is not. Otherwise we will starve notification requests (such as a
		// worker thread state change to 'shutdown'), which will block for long periods
		// of time.
		//
		DWORD Timeout = bMoreWork ? 0 : INFINITE;
		DWORD WaitResult = WaitForMultipleObjects(_countof(m_hWakeEvents), m_hWakeEvents, FALSE, Timeout);

		if (WaitResult == WAIT_FAILED)
		{
			LOG_CRITICAL_ERROR("Worker thread wait failed, Error=0x%.8x", GetLastError());
		}
		else if (WaitResult == WAIT_TIMEOUT)
		{
			assert(bMoreWork);
		}
		else
		{
			//
			// Get the reason for signaling the worker thread.
			//
			WakeReason Reason = static_cast<WakeReason>(WaitResult - WAIT_OBJECT_0);

			if (Reason == EWR_ChangeStatus)
			{
				LOG_MESSAGE("WT: Status change request from %d to %d", m_CurrentStatus, m_RequestedStatus);

				ProcessStatusChangeRequest();
				if (m_CurrentStatus == EWTS_Shutdown)
				{
					break;
				}
			}
			else if (Reason == EWR_Submission)
			{
				LOG_MESSAGE("WT: New submission");
				bMoreWork = true;
			}
			else if (Reason == EWR_BudgetNotification)
			{
				LOG_MESSAGE("WT: Budget change notification");
				ProcessBudgetChangeNotification();
				bMoreWork = true;
			}
			else
			{
				assert(false);
			}
		}

		if (bMoreWork)
		{
			//
			// Reprioritize any resources which may have visibility changes.
			//
			ReprioritizeResources();

			//
			// Process the next highest priority paging operation.
			//
			ProcessSubmission(&bMoreWork);
		}
	}

	return 0;
}

void PagingWorkerThread::DiscardPendingWork()
{
	for (int i = 0; i < _ERP_COUNT; ++i)
	{
		InitializeListHead(&m_PriorityQueues[i]);
	}
}

void PagingWorkerThread::ProcessStatusChangeRequest()
{
	assert(m_CurrentStatus != EWTS_Shutdown);

	if (m_CurrentStatus != m_RequestedStatus)
	{
		if (m_RequestedStatus == EWTS_Shutdown)
		{
			DiscardPendingWork();
		}
	}

	LOG_MESSAGE("WT: Status successfully changed from %d to %d", m_CurrentStatus, m_RequestedStatus);

	m_CurrentStatus = m_RequestedStatus;
	SetEvent(m_hStatusChangeEvent);
}

void PagingWorkerThread::ProcessSubmission(bool* pMoreWork)
{
	*pMoreWork = true;

	//
	// Select the highest priority paging operation from the priority queues. SelectResource
	// may return null if there are no entries, or if none of the operations can be selected
	// (e.g. paging in the resources may go over the budget)
	//
	Resource* pResource = SelectResource();
	if (pResource == nullptr)
	{
		*pMoreWork = false;
		return;
	}

	//
	// Process the request.
	//
	HRESULT hr = m_pFramework->PageInNextLevelOfDetail(pResource);
	if (FAILED(hr))
	{
		*pMoreWork = false;
	}

	//
	// After the paging operation completes, we need to reprioritize this specific resource.
	//
	PrioritizeResource(pResource);

	//
	// Update the video memory info to see if we need to trim anything. This may be the case
	// if the kernel recalculated the budget while processing the operation, or if we paged in
	// a critical resource (such as a packed mipmap), which can let us go over budget.
	//
	m_pFramework->UpdateVideoMemoryInfo();
	if (m_pFramework->IsOverBudget())
	{
		m_pFramework->TrimToBudget(pResource->TrimLimit);
	}
}

void PagingWorkerThread::ProcessBudgetChangeNotification()
{
	HRESULT hr;

	//
	// Update our budgeting information, and trim our usage until it is under the budget.
	//
	hr = m_pFramework->UpdateVideoMemoryInfo();
	if (SUCCEEDED(hr))
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO VideoMemoryInfo = m_pFramework->GetLocalVideoMemoryInfo();
		if (VideoMemoryInfo.CurrentUsage > VideoMemoryInfo.Budget)
		{
			m_pFramework->TrimToBudget(ERTP_Visible);
		}
	}
}

void PagingWorkerThread::SetStatus(WorkerThreadStatus Status)
{
	m_RequestedStatus = Status;
	SetEvent(m_hWakeEvents[EWR_ChangeStatus]);

	WaitForSingleObject(m_hStatusChangeEvent, INFINITE);
}

void PagingWorkerThread::EnqueueResource(Resource* pResource)
{
	//
	// Take the prioritization lock as we insert resources into the prioritization list.
	// This synchronizes the rendering thread queuing work items with the paging thread
	// prioritizing it.
	//
	EnterCriticalSection(&m_PrioritizationListLock);

	if (pResource->PrioritizationEntry.Flink != nullptr)
	{
		RemoveEntryList(&pResource->PrioritizationEntry);
	}
	InsertTailList(&m_PrioritizationListHead, &pResource->PrioritizationEntry);

	LeaveCriticalSection(&m_PrioritizationListLock);

	SetEvent(m_hWakeEvents[EWR_Submission]);
}

void PagingWorkerThread::ReprioritizeResources()
{
	EnterCriticalSection(&m_PrioritizationListLock);
	while (!IsListEmpty(&m_PrioritizationListHead))
	{
		LIST_ENTRY* pEntry = RemoveHeadList(&m_PrioritizationListHead);
		Resource* pResource = CONTAINING_RECORD(pEntry, Resource, PrioritizationEntry);
		pResource->PrioritizationEntry.Flink = nullptr;

		PrioritizeResource(pResource);
	}
	LeaveCriticalSection(&m_PrioritizationListLock);
}

void PagingWorkerThread::PrioritizeResource(Resource* pResource)
{
	UINT8 MostDetailedMipResident = pResource->MostDetailedMipResident;
	UINT8 VisibleMip = pResource->VisibleMip;
	UINT8 PrefetchMip = pResource->PrefetchMip;

	if (pResource->PagingEntry.Flink != nullptr)
	{
		RemoveEntryList(&pResource->PagingEntry);
		pResource->PagingEntry.Flink = nullptr;
	}

	bool AnyPackedMipsMissing = MostDetailedMipResident > GetLeastDetailedMipHeapIndex(pResource);
	bool IsInPrefetchZone = (PrefetchMip != UNDEFINED_MIPMAP_INDEX);

	if (AnyPackedMipsMissing && IsInPrefetchZone)
	{
		//
		// If the resource has not been loaded at all, and it's in the prefetch zone,
		// consider it very high priority. We will not only add it to the high priority
		// queue, but also push it in at the front to ensure this occurs before anything
		// else. We want to make sure the user has *something* to see, even if it's just
		// the 1x1 mipmap of a rough color.
		//
		InsertTailList(&m_PriorityQueues[ERP_VeryHigh], &pResource->PagingEntry);
		pResource->TrimLimit = ERTP_Visible;
		pResource->bIgnoreBudget = true;
	}
	else if (IsMoreDetailedMip(MostDetailedMipResident, VisibleMip))
	{
		//
		// The user has requested a visible mipmap that is of a greater detail than the
		// one currently resident. This is high priority, because we want what's on screen
		// to be visually correct.
		//
		InsertTailList(&m_PriorityQueues[ERP_High], &pResource->PagingEntry);
		pResource->TrimLimit = ERTP_NonVisible;
	}
	else if (AnyPackedMipsMissing)
	{
		//
		// The resource has not been loaded, but is a somewhat safe distance away from the
		// camera to be considered a lower priority. We will make sure that the stuff the user
		// sees on screen gets loaded before this.
		//
		InsertHeadList(&m_PriorityQueues[ERP_Medium], &pResource->PagingEntry);
		pResource->TrimLimit = ERTP_Visible;
		pResource->bIgnoreBudget = true;
	}
	else if (IsMoreDetailedMip(MostDetailedMipResident, PrefetchMip))
	{
		//
		// This is a proximity prefetched mipmap. The user cannot see this mipmap yet, but it
		// is nearby. We want to reduce any texture popping that may occur as the user scrolls
		//
		InsertTailList(&m_PriorityQueues[ERP_Medium], &pResource->PagingEntry);
		pResource->TrimLimit = ERTP_NonPrefetchable;

		assert(PrefetchMip != UNDEFINED_MIPMAP_INDEX);
	}
	else if (MostDetailedMipResident != 0)
	{
		//
		// This texture is not near the user, but we are under our budget and we haven't loaded
		// all the mipmaps for this texture yet. This is a low priority work item that will
		// occur after everything else, but will help guarantee that the user gets a smooth
		// experience at all times by prefetching the texture data prior to being needed.
		//
		InsertTailList(&m_PriorityQueues[ERP_Low], &pResource->PagingEntry);
		pResource->TrimLimit = ERTP_None;
	}
}

//
// SelectResource will look at each of the priority queues and select the best operation
// to process. Unless marked otherwise, paging operations will not be selected if the
// resulting paging operation is within a specific threshold of going over the budget.
//
Resource* PagingWorkerThread::SelectResource()
{
	for (int i = 0; i < _ERP_COUNT; ++i)
	{
		//
		// A small bias is applied to the current local budget to help prevent resources from
		// going over. The size calculated by the driver may differ slightly from the size
		// calculated by the kernel (due to various alignment and segment restrictions), and so
		// a buffer is used to prevent the operation from accidentally going over.
		//
		// The bias is determined by the priority of the operation. There is a 1MB minimum
		// bias as a "safety zone," and an 8MB buffer for each priority after that.
		//
		UINT64 BudgetBias = _1MB + _8MB * i;

		if (!IsListEmpty(&m_PriorityQueues[i]))
		{
			LIST_ENTRY* pEntry = m_PriorityQueues[i].Flink;
			Resource* pResource = CONTAINING_RECORD(pEntry, Resource, PagingEntry);

			//
			// The paging thread will only page in one mipmap at a time to be fair to all
			// resources. This allows resources to be selected in a round-robin sequence,
			// preventing prefetching of low priority allocations to delay visibility changes
			// from reprioritizing operations.
			//
			// Even though the paging operations are asycnhronous from rendering (i.e. they
			// should not impact performance), it is still possible for the paging operations,
			// if long enough, to block loading mipmaps that are otherwise important. For example,
			// loading a large 8Kx8K mipmap far off screen could cause a significant enough delay
			// to prevent a mipmap on screen from being loaded by the time it is actually visible.
			//
			UINT NextMip = IncreaseMipQuality(pResource->MostDetailedMipResident, 1);
			UINT64 MipSize = 0;
			if (NextMip < pResource->PackedMipHeapIndex)
			{
				MipSize = GetNonPackedMipSize(pResource, NextMip);
			}

			//
			// When prioritizing operations, packed mipmaps are considered critical operations,
			// and should never be restricted by the budget. This is because packed mipmaps represent
			// the application's minimum working set. Although the application should try as hard as
			// possible to remain under its budget, every application will have a minimum requirement
			// to run. For this sample, packed mipmaps are considered the lowest quality that will
			// be tolerated. It is not expected for packed mipmaps to account for a significant amount
			// of space, since most will fit within a single 64KB tile. This means the rough estimate
			// cost of all packed mipmaps is 64KB*NumResources.
			//
			if (!pResource->bIgnoreBudget)
			{
				DXGI_QUERY_VIDEO_MEMORY_INFO MemoryInfo = m_pFramework->GetLocalVideoMemoryInfo();
				UINT64 TargetUsage = MemoryInfo.Budget - (MipSize + BudgetBias);

				if (!m_pFramework->IsWithinBudgetThreshold(MipSize + BudgetBias))
				{
					if (!m_pFramework->TrimToTarget(pResource->TrimLimit, TargetUsage))
					{
						continue;
					}
				}
			}

			RemoveEntryList(pEntry);
			pEntry->Flink = nullptr;
			pResource->bIgnoreBudget = false;

			return pResource;
		}
	}

	return nullptr;
}

//
// PagingContext
//
#define PAGING_CONTEXT_COMMAND_LIST_TYPE D3D12_COMMAND_LIST_TYPE_COPY

PagingContext::PagingContext(DX12Framework* pFramework) :
	Context(pFramework)
{
}

PagingContext::~PagingContext()
{
}

HRESULT PagingContext::CreateDeviceDependentState()
{
	HRESULT hr;

	hr = Context::CreateDeviceDependentState(PAGING_CONTEXT_COMMAND_LIST_TYPE);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize paging context");
		return hr;
	}

	try
	{
		//
		// The paging context always has only a single frame, since we will only process
		// one paging operation at a time.
		//
		PagingFrame* pFrame = new PagingFrame();

		hr = InitializeFrame(pFrame, PAGING_CONTEXT_COMMAND_LIST_TYPE);
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to initialize frame object, hr=0x%.8x", hr);
			return hr;
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

void PagingContext::DestroyDeviceDependentState()
{
	//
	// Flush all pending operations before we tear down the device dependent state of the context.
	//
	Flush();

	assert(IsListEmpty(&m_ActiveFrameListHead));

	LIST_ENTRY* pEntry = m_FreeFrameListHead.Flink;

	while (pEntry != &m_FreeFrameListHead)
	{
		PagingFrame* pFrame = static_cast<PagingFrame*>(pEntry);
		pEntry = pEntry->Flink;

		RemoveEntryList(pFrame);

		SafeRelease(pFrame->pCommandAllocator);
		SafeRelease(pFrame->pCommandList);

		SafeDelete(pFrame);
	}

	Context::DestroyDeviceDependentState();
}
