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

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>

class CommandListManager
{
	friend class CommandContext;

public:
	CommandListManager();
	~CommandListManager();

	void Create(ID3D12Device* pDevice);
	void Shutdown();

	inline bool IsReady()
	{
		return m_CommandQueue != nullptr;
	}

	uint64_t IncrementFence(void);
	bool IsFenceComplete(uint64_t FenceValue);
	void WaitForFence(uint64_t FenceValue);
	void IdleGPU(void) { WaitForFence(IncrementFence()); }

	ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }

private:

	void CreateNewCommandList( ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator );
	uint64_t ExecuteCommandList( ID3D12CommandList* List );
	ID3D12CommandAllocator* RequestAllocator(void);
	void DiscardAllocator( uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator );

	ID3D12Device* m_Device;
	ID3D12CommandQueue* m_CommandQueue;

	// Since there is only a "main pool" so far, everything below corresponds to that pool. It should be renamed and/or 
	// restructured if we add other pools.
	std::vector<ID3D12CommandAllocator*> m_AllocatorPool;
	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
	std::mutex m_AllocatorMutex;
	std::mutex m_FenceMutex;
	std::mutex m_EventMutex;

	// Lifetime of these objects is managed by the descriptor cache
	ID3D12Fence* m_pFence;
	uint64_t m_NextFenceValue;
	uint64_t m_LastCompletedFenceValue;
	HANDLE m_FenceEventHandle;
};

