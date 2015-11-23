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
// Specifies the reason that the paging thread was woken up.
//
enum WakeReason
{
	// Indicates that another thread has requested that the paging thread change
	// states, such as shutting down to exit the process.
	EWR_ChangeStatus,

	// Indicates that the state of one or more resources has changed, and the
	// worker thread should reprioritize the resources and perform more paging work.
	EWR_Submission,

	// Indicates that the graphics kernel has changed this process's memory budget,
	// and the worker thread may need to react accordingly. If the budget has
	// increased, the worker thread may retry failed paging operations (such as
	// prefetching) that failed because the operaiton would bring the process
	// over budget. If the budget has decreased, the paging thread must trim
	// resources to remain under the budget.
	EWR_BudgetNotification,

	_EWR_COUNT
};

//
// The current status of the worker thread.
//
enum WorkerThreadStatus
{
	EWTS_Suspended,
	EWTS_Running,
	EWTS_Shutdown,

	_EWTS_COUNT
};

//
// The paging worker thread is the powerhouse behind all paging and texture streaming
// for the sample.
//
// The worker thread's primary goal is to stream texture data for the rendering pipeline
// asynchronously, and prioritize video memory based on the new budgetting information
// present in DXGI.
//
class PagingWorkerThread
{
private:
	friend class DX12Framework;

	DX12Framework* m_pFramework;

	WorkerThreadStatus m_CurrentStatus;
	WorkerThreadStatus m_RequestedStatus;

	//
	// Thread management
	//
	HANDLE m_hThread;
	HANDLE m_hWakeEvents[_EWR_COUNT];
	HANDLE m_hStatusChangeEvent;
	DWORD m_BudgetNotificationCookie;

	// Lock for accessing the prioritization list head, which is accessed by both
	// the rendering and paging threads.
	CRITICAL_SECTION m_PrioritizationListLock;

	// A list of unprioritized resources. A resource must be prioritized before
	// any paging operations can occur, so that the paging thread knows how to process
	// the resource.
	LIST_ENTRY m_PrioritizationListHead;

	// An array of priority-ordered linked list heads. The worker thread will process
	// resources in these arrays in strict order.
	LIST_ENTRY m_PriorityQueues[_ERP_COUNT];

private:
	PagingWorkerThread(DX12Framework* pFramework);
	~PagingWorkerThread();

	HRESULT Init();

	void Flush();
	void DiscardPendingWork();
	DWORD Run();

	void EnqueueResource(Resource* pResource);
	void ReprioritizeResources();
	void PrioritizeResource(Resource* pResource);
	Resource* SelectResource();

	void ProcessStatusChangeRequest();
	void ProcessSubmission(bool* pMoreWork);
	void ProcessBudgetChangeNotification();

	void SetStatus(WorkerThreadStatus Status);

	static DWORD CALLBACK ThreadEntry(void* pArg);
};

//
// A paging frame does not have any special state.
//
struct PagingFrame : Frame
{
};

//
// The paging context is a specialization of the Context object that creates a
// unique D3D12 command queue and set of frames for paging operations. The command
// queue is created as a copy command queue, which can map to paging engines on
// the graphics hardware. In addition to the scheduling granularity offered by
// the graphics kernel scheduler, being mapped to a hardware paging engine allows
// the paging operations to run completely in parallel with 3D graphics work
// submitted on the 3D engines.
//
class PagingContext : public Context
{
public:
	PagingContext(DX12Framework* pFramework);
	~PagingContext();

	HRESULT CreateDeviceDependentState();
	void DestroyDeviceDependentState();

	inline PagingFrame* GetCurrentFrame() const
	{
		return static_cast<PagingFrame*>(m_pCurrentFrame);
	}
};
