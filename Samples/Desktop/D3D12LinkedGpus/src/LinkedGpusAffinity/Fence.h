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

#include "DXSampleHelper.h"

using Microsoft::WRL::ComPtr;

// Fence helper class.
// Wraps a D3D12 fence object and provides functionality for common operations.
class Fence
{
public:
	// Create a fence and associate it with the specified command queue for
	// convenience in working with fences.
	Fence(CD3DX12AffinityCommandQueue* pQueue) :
		m_currentFenceValue(0)
	{
		ThrowIfFailed(pQueue->QueryInterface(IID_PPV_ARGS(&m_queue)));

		ComPtr<CD3DX12AffinityDevice> device;
		ThrowIfFailed(m_queue->GetDevice(IID_PPV_ARGS(&device)));

		ThrowIfFailed(device->CreateFence(m_currentFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

	~Fence()
	{
		if (m_fenceEvent)
		{
			CloseHandle(m_fenceEvent);
			m_fenceEvent = nullptr;
		}
	}

	// Drain the GPU command queue.
	// (Blocks the calling CPU thread)
	inline void FlushGpuQueue()
	{
		Signal();
		m_queue->WaitForCompletion();
	}

	// Issue a Signal command on the command queue.
	// Use the current value of the fence or override it with your own value.
	inline void Signal(UINT64 value = UnspecifiedFenceValue)
	{
		if (value == UnspecifiedFenceValue)
		{
			m_currentFenceValue++;
		}
		else
		{
			_ASSERT(value > m_currentFenceValue);
			m_currentFenceValue = value;
		}

		ThrowIfFailed(m_queue->Signal(m_fence.Get(), m_currentFenceValue));
	}

	// Instruct the GPU queue associated with this fence to wait for a value to be signaled.
	// Use the current value of the fence or override it with your own value.
	// (Does not block the calling CPU thread)
	inline void GpuWait(UINT64 value = UnspecifiedFenceValue)
	{
		if (value == UnspecifiedFenceValue)
		{
			value = m_currentFenceValue;
		}

		ThrowIfFailed(m_queue->Wait(m_fence.Get(), value));
	}

	// Instruct a GPU queue on the active node to wait for the fences on the other nodes.
	// Use the current values of the fences or override them with your own value.
	// (Does not block the calling CPU thread)
	inline static void GpuWait(CD3DX12AffinityCommandQueue* pQueue, Fence* pFence, UINT64 value = UnspecifiedFenceValue)
	{
		if (value == UnspecifiedFenceValue)
		{
			value = pFence->m_currentFenceValue;
		}

		pQueue->Wait(pFence->m_fence.Get(), value, true, Settings::SharedNodeMask);
	}

protected:
	// Block the calling CPU thread until the GPU has signaled the specified fence.
	inline void CpuWait(UINT64 fenceValue = UnspecifiedFenceValue)
	{
		if (fenceValue == UnspecifiedFenceValue)
		{
			fenceValue = m_currentFenceValue;
		}

		UINT nodeMask = 1 << m_fence->GetActiveNodeIndex();
		if (m_fence->GetCompletedValue(nodeMask) < fenceValue)
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}
	}

	static const UINT64 UnspecifiedFenceValue = UINT64_MAX;

	ComPtr<CD3DX12AffinityFence> m_fence;			// The D3D12 fence object.
	ComPtr<CD3DX12AffinityCommandQueue> m_queue;	// The command queue associated with this fence.
	UINT64 m_currentFenceValue;						// The last value signaled by this fence.
	HANDLE m_fenceEvent;							// CPU-waitable event handle.
};

class LinearFence : public Fence
{
public:
	// This fence implementation manages a ring buffer of issued Signal events.
	// The "Next" API instructs the Fence to move to the next slot in the ring buffer.
	// If the next slot represents a fence value that has not yet been signaled by
	// the GPU, then the CPU will wait until that fence is signaled before continuing.
	// 
	// In this sample, LinearFences are used to guard against premature resetting
	// of command allocators and modifying of upload heaps that might currently be
	// in use by the GPU.
	LinearFence(CD3DX12AffinityCommandQueue* pQueue, UINT count) :
		Fence(pQueue),
		m_signalIndex(0)
	{
		m_signalHistory.resize(count * pQueue->GetNodeCount());
	}

	// If the oldest Signal event in the ring buffer has not yet been processed by
	// the GPU, block the calling CPU thread until it is.
	inline void Next()
	{
		m_signalHistory[m_signalIndex] = m_currentFenceValue;
		m_signalIndex = (m_signalIndex + 1) % m_signalHistory.size();

		Fence::CpuWait(m_signalHistory[m_signalIndex]);
	}

private:
	std::vector<UINT64> m_signalHistory;	// The history of signal events.
	UINT m_signalIndex;						// The index to the next slot in the signal history ring buffer.
};
