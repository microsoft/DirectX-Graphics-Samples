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
// Author: Jack Elliott.
//
// Allocates blocks from a fixed range using buddy allocation method.
// Buddy allocation allows reasonably fast allocation of arbitrary size blocks
// with minimal fragmentation and provides efficient reuse of freed ranges.
// When a block is de-allocated an attempt is made to merge it with it's 
// neighbour (buddy) if it is contiguous and free.
// Based on reference implementation by Bill Kristiansen
//  

#pragma once

#include "GpuBuffer.h"
#include <vector>
#include <queue>
#include <mutex>
#include <set>

// Unfortunately the api restricts the minimum size of a placed buffer resource to 64k
#define MIN_PLACED_BUFFER_SIZE (64 * 1024)

#if defined(PROFILE) || defined(_DEBUG)
#define INCREASE_BUDDY_COUNTER(A, B) (A = A + B);
#define DECREASE_BUDDY_COUNTER(A, B) (A = A - B);
#else
#define INCREASE_BUDDY_COUNTER(A, B)
#define DECREASE_BUDDY_COUNTER(A, B)
#endif

enum kBuddyAllocationStrategy
{
    // This strategy uses Placed Resources to sub-allocate a buffer out of an underlying ID3D12Heap.
    // The benefit of this is that each buffer can have it's own resource state and can be treated
    // as any other buffer. The downside of this strategy is the API limitiation which enforces
    // the minimum buffer size to 64k leading to large internal fragmentation in the allocator
    kPlacedResourceStrategy,
    // The alternative is to manualy sub-allocate out of a single large buffer which allows block
    // allocation granularity down to 1 byte. However, this strategy is only really valid for buffers which
    // will be treated as read-only after their creation (i.e. most Index and Vertex buffers). This 
    // is because the underlying resource can only have one state at a time.
    kManualSubAllocationStrategy
};

struct BuddyBlock
{
    ByteAddressBuffer* m_pBuffer;
    ID3D12Heap* m_pBackingHeap;

    size_t m_offset;
    size_t m_size;
    size_t m_unpaddedSize;
    uint64_t m_fenceValue;

    inline size_t GetOffset() const { return m_offset; }
    inline size_t GetSize() const { return m_size; }

    BuddyBlock() : m_pBuffer(nullptr), m_pBackingHeap(nullptr), m_offset(0), m_size(0), m_unpaddedSize(0), m_fenceValue(0) {};

    BuddyBlock(uint32_t heapOffset, uint32_t totalSize, uint32_t unpaddedSize);

    void InitPlaced(ID3D12Heap* pBackingHeap, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);

    void InitFromResource(ByteAddressBuffer* pBuffer, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);

    void Destroy();
};

class BuddyAllocator
{
public:

    BuddyAllocator(kBuddyAllocationStrategy allocationStrategy, D3D12_HEAP_TYPE heapType, size_t maxBlockSize, size_t minBlockSize = MIN_PLACED_BUFFER_SIZE, size_t baseOffset = 0);

    void Initialize();

    void Destroy();

    BuddyBlock* Allocate(uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);

    void Deallocate(BuddyBlock* pBlock);

    inline bool IsOwner(const BuddyBlock &block)
    {
        return block.GetOffset() >= m_baseOffset && block.GetSize() <= m_maxBlockSize;
    }

    inline void Reset()
    {
        // Clear the free blocks collection  
        m_freeBlocks.clear();

        // Initialize the pool with a free inner block of max inner block size  
        m_freeBlocks.resize(m_maxOrder + 1);
        m_freeBlocks[m_maxOrder].insert((size_t)0);
    }

    void CleanUpAllocations();

private:
    ID3D12Heap* m_pBackingHeap;
    ByteAddressBuffer m_BackingResource;

    const D3D12_HEAP_TYPE m_heapType;

    std::queue<BuddyBlock*> m_deferredDeletionQueue;
    std::vector<std::set<size_t>> m_freeBlocks;
    UINT m_maxOrder;
    const size_t m_baseOffset;
    const size_t m_maxBlockSize;
    const size_t m_minBlockSize;

    const kBuddyAllocationStrategy m_allocationStrategy;

    inline size_t SizeToUnitSize(size_t size) const
    {
        return (size + (m_minBlockSize - 1)) / m_minBlockSize;
    }

    inline UINT UnitSizeToOrder(size_t size) const
    {
        return UINT(ceil(log2f(float(size))));
    }

    inline size_t GetBuddyOffset(const size_t &offset, const size_t &size)
    {
        return offset ^ size;
    }

    void DeallocateInternal(BuddyBlock* pBlock);

    size_t OrderToUnitSize(UINT order) const { return ((size_t)1) << order; }
    size_t AllocateBlock(UINT order);
    void DeallocateBlock(size_t offset, UINT order);

#if defined(PROFILE) || defined(_DEBUG)
    size_t m_SpaceUsed;
    size_t m_InternalFragmentation;
#endif
};
