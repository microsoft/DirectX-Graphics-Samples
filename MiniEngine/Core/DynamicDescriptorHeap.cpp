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
#include "DynamicDescriptorHeap.h"
#include "CommandContext.h"
#include "GraphicsCore.h"
#include "CommandListManager.h"
#include "RootSignature.h"

using namespace Graphics;

//
// DynamicDescriptorHeap Implementation
//

std::mutex DynamicDescriptorHeap::sm_Mutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DynamicDescriptorHeap::sm_DescriptorHeapPool;
std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> DynamicDescriptorHeap::sm_RetiredDescriptorHeaps;
std::queue<ID3D12DescriptorHeap*> DynamicDescriptorHeap::sm_AvailableDescriptorHeaps;
uint32_t DynamicDescriptorHeap::sm_DescriptorSize = 0;

ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap(void)
{
	std::lock_guard<std::mutex> LockGuard(sm_Mutex);

	while (!sm_RetiredDescriptorHeaps.empty() && g_CommandManager.IsFenceComplete(sm_RetiredDescriptorHeaps.front().first))
	{
		sm_AvailableDescriptorHeaps.push(sm_RetiredDescriptorHeaps.front().second);
		sm_RetiredDescriptorHeaps.pop();
	}

	if (!sm_AvailableDescriptorHeaps.empty())
	{
		ID3D12DescriptorHeap* HeapPtr = sm_AvailableDescriptorHeaps.front();
		sm_AvailableDescriptorHeaps.pop();
		return HeapPtr;
	}
	else
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HeapDesc.NumDescriptors = kNumDescriptorsPerHeap;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HeapDesc.NodeMask = 1;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> HeapPtr;
		ASSERT_SUCCEEDED(g_Device->CreateDescriptorHeap(&HeapDesc, MY_IID_PPV_ARGS(&HeapPtr)));
		sm_DescriptorHeapPool.emplace_back(HeapPtr);
		return HeapPtr.Get();
	}
}

void DynamicDescriptorHeap::DiscardDescriptorHeaps( uint64_t FenceValue, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps )
{
	std::lock_guard<std::mutex> LockGuard(sm_Mutex);
	for (auto iter = UsedHeaps.begin(); iter != UsedHeaps.end(); ++iter)
		sm_RetiredDescriptorHeaps.push(std::make_pair(FenceValue, *iter));
}

void DynamicDescriptorHeap::RetireCurrentHeap( void )
{
	// Don't retire unused heaps.
	if (m_CurrentOffset == 0)
	{
		ASSERT(m_CurrentHeapPtr == nullptr);
		return;
	}

	ASSERT(m_CurrentHeapPtr != nullptr);
	m_RetiredHeaps.push_back(m_CurrentHeapPtr);
	m_CurrentHeapPtr = nullptr;
	m_CurrentOffset = 0;
}

void DynamicDescriptorHeap::RetireUsedHeaps( uint64_t fenceValue )
{
	DiscardDescriptorHeaps(fenceValue, m_RetiredHeaps);
	m_RetiredHeaps.clear();
}

DynamicDescriptorHeap::DynamicDescriptorHeap(CommandContext& OwningContext) : m_OwningContext(OwningContext)
{
	m_CurrentHeapPtr = nullptr;
	m_CurrentOffset = 0;
}

DynamicDescriptorHeap::~DynamicDescriptorHeap()
{
}

void DynamicDescriptorHeap::CleanupUsedHeaps( uint64_t fenceValue )
{
	RetireCurrentHeap();
	RetireUsedHeaps(fenceValue);
	m_GraphicsHandleCache.ClearCache();
	m_ComputeHandleCache.ClearCache();
}

inline ID3D12DescriptorHeap* DynamicDescriptorHeap::GetHeapPointer()
{
	if (m_CurrentHeapPtr == nullptr)
	{
		ASSERT(m_CurrentOffset == 0);
		m_CurrentHeapPtr = RequestDescriptorHeap();
		m_FirstDescriptor = DescriptorHandle(
			m_CurrentHeapPtr->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentHeapPtr->GetGPUDescriptorHandleForHeapStart());
	}

	return m_CurrentHeapPtr;
}

uint32_t DynamicDescriptorHeap::DescriptorHandleCache::ComputeStagedSize()
{
	// Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
	uint32_t NeededSpace = 0;
	uint32_t RootIndex;
	uint32_t StaleParams = m_StaleRootParamsBitMap;
	while (BitScanForward((unsigned long*)&RootIndex, StaleParams))
	{
		StaleParams ^= (1 << RootIndex);

		uint32_t MaxSetHandle;
		ASSERT(TRUE == BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
			"Root entry marked as stale but has no stale descriptors");

		NeededSpace += MaxSetHandle + 1;
	}
	return NeededSpace;
}

void DynamicDescriptorHeap::DescriptorHandleCache::CopyAndBindStaleTables(
	DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
	void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t StaleParamCount = 0;
	uint32_t TableSize[DescriptorHandleCache::kMaxNumDescriptorTables];
	uint32_t RootIndices[DescriptorHandleCache::kMaxNumDescriptorTables];
	uint32_t NeededSpace = 0;
	uint32_t RootIndex;

	// Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
	uint32_t StaleParams = m_StaleRootParamsBitMap;
	while (BitScanForward((unsigned long*)&RootIndex, StaleParams))
	{
		RootIndices[StaleParamCount] = RootIndex;
		StaleParams ^= (1 << RootIndex);

		uint32_t MaxSetHandle;
		ASSERT(TRUE == BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
			"Root entry marked as stale but has no stale descriptors");

		NeededSpace += MaxSetHandle + 1;
		TableSize[StaleParamCount] = MaxSetHandle + 1;

		++StaleParamCount;
	}

	ASSERT(StaleParamCount <= DescriptorHandleCache::kMaxNumDescriptorTables,
		"We're only equipped to handle so many descriptor tables");

	m_StaleRootParamsBitMap = 0;

	static const uint32_t kMaxDescriptorsPerCopy = 16;
	UINT NumDestDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[kMaxDescriptorsPerCopy];
	UINT pDestDescriptorRangeSizes[kMaxDescriptorsPerCopy];

	UINT NumSrcDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE pSrcDescriptorRangeStarts[kMaxDescriptorsPerCopy];
	UINT pSrcDescriptorRangeSizes[kMaxDescriptorsPerCopy];

	const uint32_t kDescriptorSize = DynamicDescriptorHeap::GetDescriptorSize();

	for (uint32_t i = 0; i < StaleParamCount; ++i)
	{
		RootIndex = RootIndices[i];
		(CmdList->*SetFunc)(RootIndex, DestHandleStart.GetGpuHandle());

		DescriptorTableCache& RootDescTable = m_RootDescriptorTable[RootIndex];

		D3D12_CPU_DESCRIPTOR_HANDLE* SrcHandles = RootDescTable.TableStart;
		uint64_t SetHandles = (uint64_t)RootDescTable.AssignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE CurDest = DestHandleStart.GetCpuHandle();
		DestHandleStart += TableSize[i] * kDescriptorSize;

		unsigned long SkipCount;
		while (BitScanForward64(&SkipCount, SetHandles))
		{
			// Skip over unset descriptor handles
			SetHandles >>= SkipCount;
			SrcHandles += SkipCount;
			CurDest.ptr += SkipCount * kDescriptorSize;

			unsigned long DescriptorCount;
			BitScanForward64(&DescriptorCount, ~SetHandles);
			SetHandles >>= DescriptorCount;

			// If we run out of temp room, copy what we've got so far
			if (NumSrcDescriptorRanges + DescriptorCount > kMaxDescriptorsPerCopy)
			{
				g_Device->CopyDescriptors(
					NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
					NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				NumSrcDescriptorRanges = 0;
				NumDestDescriptorRanges = 0;
			}

			// Setup destination range
			pDestDescriptorRangeStarts[NumDestDescriptorRanges] = CurDest;
			pDestDescriptorRangeSizes[NumDestDescriptorRanges] = DescriptorCount;
			++NumDestDescriptorRanges;

			// Setup source ranges (one descriptor each because we don't assume they are contiguous)
			for (uint32_t j = 0; j < DescriptorCount; ++j)
			{
				pSrcDescriptorRangeStarts[NumSrcDescriptorRanges] = SrcHandles[j];
				pSrcDescriptorRangeSizes[NumSrcDescriptorRanges] = 1;
				++NumSrcDescriptorRanges;
			}

			// Move the destination pointer forward by the number of descriptors we will copy
			SrcHandles += DescriptorCount;
			CurDest.ptr += DescriptorCount * kDescriptorSize;
		}
	}

	g_Device->CopyDescriptors(
		NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
		NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
	
void DynamicDescriptorHeap::CopyAndBindStagedTables( DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
	void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t NeededSize = HandleCache.ComputeStagedSize();
	if (!HasSpace(NeededSize))
	{
		RetireCurrentHeap();
		UnbindAllValid();
		NeededSize = HandleCache.ComputeStagedSize();
	}

	// This can trigger the creation of a new heap
	m_OwningContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GetHeapPointer());

	HandleCache.CopyAndBindStaleTables(Allocate(NeededSize), CmdList, SetFunc);
}

void DynamicDescriptorHeap::UnbindAllValid( void )
{
	m_GraphicsHandleCache.UnbindAllValid();
	m_ComputeHandleCache.UnbindAllValid();
}

D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::UploadDirect( D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
	if (!HasSpace(1))
	{
		RetireCurrentHeap();
		UnbindAllValid();
	}

	m_OwningContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GetHeapPointer());

	DescriptorHandle DestHandle = m_FirstDescriptor + m_CurrentOffset * GetDescriptorSize();
	m_CurrentOffset += 1;

	g_Device->CopyDescriptorsSimple(1, DestHandle.GetCpuHandle(), Handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return DestHandle.GetGpuHandle();
}

void DynamicDescriptorHeap::DescriptorHandleCache::UnbindAllValid()
{
	m_StaleRootParamsBitMap = 0;

	unsigned long TableParams = m_RootDescriptorTablesBitMap;
	unsigned long RootIndex;
	while (BitScanForward(&RootIndex, TableParams))
	{
		TableParams ^= (1 << RootIndex);
		if (m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap != 0)
			m_StaleRootParamsBitMap |= (1 << RootIndex);
	}
}

void DynamicDescriptorHeap::DescriptorHandleCache::StageDescriptorHandles( UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
{
	ASSERT(((1 << RootIndex) & m_RootDescriptorTablesBitMap) != 0, "Root parameter is not a CBV_SRV_UAV descriptor table");
	ASSERT(Offset + NumHandles <= m_RootDescriptorTable[RootIndex].TableSize);

	DescriptorTableCache& TableCache = m_RootDescriptorTable[RootIndex];
	D3D12_CPU_DESCRIPTOR_HANDLE* CopyDest = TableCache.TableStart + Offset;
	for (UINT i = 0; i < NumHandles; ++i)
		CopyDest[i] = Handles[i];
	TableCache.AssignedHandlesBitMap |= ((1 << NumHandles) - 1) << Offset;
	m_StaleRootParamsBitMap |= (1 << RootIndex);
}

void DynamicDescriptorHeap::DescriptorHandleCache::ParseRootSignature( const RootSignature& RootSig )
{
	UINT CurrentOffset = 0;

	ASSERT(RootSig.m_NumParameters <= 16, "Maybe we need to support something greater");

	m_StaleRootParamsBitMap = 0;
	m_RootDescriptorTablesBitMap = RootSig.m_DescriptorTableBitMap;

	unsigned long TableParams = m_RootDescriptorTablesBitMap;
	unsigned long RootIndex;
	while (BitScanForward(&RootIndex, TableParams))
	{
		TableParams ^= (1 << RootIndex);

		UINT TableSize = RootSig.m_DescriptorTableSize[RootIndex];
		ASSERT(TableSize > 0);

		DescriptorTableCache& RootDescriptorTable = m_RootDescriptorTable[RootIndex];
		RootDescriptorTable.AssignedHandlesBitMap = 0;
		RootDescriptorTable.TableStart = m_HandleCache + CurrentOffset;
		RootDescriptorTable.TableSize = TableSize;

		CurrentOffset += TableSize;
	}

	m_MaxCachedDescriptors = CurrentOffset;

	ASSERT(m_MaxCachedDescriptors <= kMaxNumDescriptors, "Exceeded user-supplied maximum cache size");
}
