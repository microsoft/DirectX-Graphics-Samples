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
// A Frame object stores information about each iteration of the pipeline.
// For rendering, this would correspond 1:1 with a real rendering frame (i.e. from
// one Present() call to the next).
// For paging, a frame corresponds to each unique paging operation.
// Each frame belongs to a Context object (defined below), and operations are queued
// on that Context's D3D12 command queue. At the end of each frame, the frame's
// completion fence is signaled on the command queue, allowing us to monitor
// completion of GPU work for that frame.
//
struct Frame : LIST_ENTRY
{
	ID3D12CommandAllocator* pCommandAllocator;
	ID3D12GraphicsCommandList* pCommandList;
	UINT64 CompletionFence;
};

//
// A context is used to abstract GPU scheduling state. Each context contains a command
// queue, a fence object used to signal and track GPU work, and a list of frames that
// can be running in parallel.
// The Context object is subclassed into a RenderContext and PagingContext, which create
// unique queue types and perform unique operations.
//
class Context
{
protected:
	DX12Framework* m_pFramework;

	// List entry head storing a list of all actively running frames. All actively running
	// frames are assumed to be running on the GPU until they are retired.
	LIST_ENTRY m_ActiveFrameListHead;

	// List entry head storing a list of free frame objects which can be reused.
	LIST_ENTRY m_FreeFrameListHead;

	// The current fence for this command queue. This fence value is incremented once per frame.
	UINT64 m_CurrentFence = 0;

	// Stores the fence of the most recently completed and retired frame. It can safely be
	// assumed that any fence value lower than this is referring to work that has completed
	// on the GPU, and is safe to reuse.
	UINT64 m_LastCompletedFence = 0;

	// The frame currently being recorded by the context. This value is null outside of
	// calls to Begin() and End().
	Frame* m_pCurrentFrame = nullptr;

	// The D3D12 command queue object to which all work is submitted.
	ID3D12CommandQueue* m_pCommandQueue = nullptr;

	// The D3D12 fence object used track progress on the command queue.
	ID3D12Fence* m_pFenceObject = nullptr;

	// A handle (allocated per thread with TLS) allowing a thread to safely wait on this context.
	HANDLE m_hFlushEvent = nullptr;

	// The number of active frames in flight that are accessing the GPU.
	UINT32 m_ActiveFrames = 0;

protected:
	HRESULT InitializeFrame(Frame* pFrame, D3D12_COMMAND_LIST_TYPE Type);
	virtual void RetireFrame(Frame* /*pFrame*/) { }

private:
	void RetireFrameInternal(Frame* pFrame);

public:
	Context(DX12Framework* pFramework);
	~Context();

	HRESULT CreateDeviceDependentState(D3D12_COMMAND_LIST_TYPE Type);
	void DestroyDeviceDependentState();

	void Begin();
	HRESULT Execute();
	void End();

	void WaitForFence(UINT64 Fence);
	void WaitForSingleFrame();
	void WaitForAllFrames();

	void Flush();

	inline ID3D12CommandQueue* GetCommandQueue()
	{
		return m_pCommandQueue;
	}

	inline UINT32 GetActiveFrameCount() const
	{
		return m_ActiveFrames;
	}

	inline UINT64 GetLastCompletedFence() const
	{
		return m_LastCompletedFence;
	}
};
