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
// Author(s):  Jack Elliott
//

#include "pch.h"
#include "BuddyAllocator.h"
#include "GraphicsCore.h"
#include "CommandListManager.h"
#include "CommandContext.h"

using namespace Graphics;
using namespace std;

BuddyBlock::BuddyBlock(uint32_t heapOffset, uint32_t totalSize, uint32_t unpaddedSize) :
    m_pBuffer(nullptr)
    , m_pBackingHeap(nullptr)
    , m_offset(heapOffset)
    , m_fenceValue(0)
    , m_size(totalSize)
    , m_unpaddedSize(unpaddedSize)
{};

void BuddyBlock::InitPlaced(ID3D12Heap* pBackingHeap, uint32_t numElements, uint32_t elementSize, const void* initialData)
{
    m_pBuffer = new ByteAddressBuffer();
    m_pBackingHeap = pBackingHeap;

    const wstring name(L"Buddy Block");
    m_pBuffer->CreatePlaced(name, m_pBackingHeap, uint32_t(m_offset), numElements, elementSize, initialData);
}

void BuddyBlock::InitFromResource(ByteAddressBuffer* pBuffer, uint32_t numElements, uint32_t elementSize, const void* initialData)
{
    m_pBuffer = pBuffer;

    if (initialData)
    {
        CommandContext::InitializeBuffer(*pBuffer, initialData, numElements * elementSize, m_offset);
    }
}

void BuddyBlock::Destroy()
{
    m_pBuffer->Destroy();
    m_pBuffer = nullptr;
}

BuddyAllocator::BuddyAllocator(kBuddyAllocationStrategy allocationStrategy, D3D12_HEAP_TYPE heapType, size_t maxBlockSize, size_t MinBlockSize, size_t baseOffset)
    : m_allocationStrategy(allocationStrategy)
    , m_heapType(heapType)
    , m_baseOffset(baseOffset)
    , m_maxBlockSize(maxBlockSize)
    , m_minBlockSize(MinBlockSize)
    , m_pBackingHeap(nullptr)
#if defined(PROFILE) || defined(_DEBUG)
    , m_SpaceUsed(0)
    , m_InternalFragmentation(0)
#endif
{
    ASSERT(Math::IsDivisible(maxBlockSize, m_minBlockSize));
    ASSERT(Math::IsPowerOfTwo(maxBlockSize / m_minBlockSize));

    m_maxOrder = UnitSizeToOrder(SizeToUnitSize(maxBlockSize));

    Reset();
}

void BuddyAllocator::Initialize()
{
    if (m_allocationStrategy == kBuddyAllocationStrategy::kPlacedResourceStrategy)
    {
        D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(m_heapType);

        D3D12_HEAP_DESC desc = {};
        desc.SizeInBytes = m_maxBlockSize;
        desc.Properties = heapProps;
        desc.Alignment = MIN_PLACED_BUFFER_SIZE;
        desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

        ASSERT_SUCCEEDED(g_Device->CreateHeap(&desc, MY_IID_PPV_ARGS(&m_pBackingHeap)));
    }
    else
    {
        m_BackingResource.Create(L"Buddy Allocator Backing Resource", uint32_t(m_maxBlockSize), 1, nullptr);
    }
}

void BuddyAllocator::Destroy()
{
    if (m_allocationStrategy == kBuddyAllocationStrategy::kPlacedResourceStrategy)
    {
        m_pBackingHeap->Release();
    }
    else
    {
        m_BackingResource.Destroy();
    }
}

size_t BuddyAllocator::AllocateBlock(UINT order)
{
    size_t offset;

    if (order > m_maxOrder)
    {
        throw(std::bad_alloc()); // Can't allocate a block that large  
    }

    auto it = m_freeBlocks[order].begin();

    if (it == m_freeBlocks[order].end())
    {
        // No free nodes in the requested pool.  Try to find a higher-order block and split it.  
        size_t left = AllocateBlock(order + 1);

        size_t size = OrderToUnitSize(order);

        size_t right = left + size;

        m_freeBlocks[order].insert(right); // Add the right block to the free pool  

        offset = left; // Return the left block  
    }

    else
    {
        offset = *it;

        // Remove the block from the free list  
        m_freeBlocks[order].erase(it);
    }

    return offset;
}

void BuddyAllocator::DeallocateBlock(size_t offset, UINT order)
{
    // See if the buddy block is free  
    size_t size = OrderToUnitSize(order);

    size_t buddy = GetBuddyOffset(offset, size);

    auto it = m_freeBlocks[order].find(buddy);

    if (it != m_freeBlocks[order].end())
    {
        // Deallocate merged blocks  
        DeallocateBlock(min(offset, buddy), order + 1);
        // Remove the buddy from the free list  
        m_freeBlocks[order].erase(it);
    }
    else
    {
        // Add the block to the free list  
        m_freeBlocks[order].insert(offset); // throw(std::bad_alloc) 
    }
}

BuddyBlock* BuddyAllocator::Allocate(uint32_t numElements, uint32_t elementSize, const void* initialData)
{
    size_t size = numElements * elementSize;
    size_t unitSize = SizeToUnitSize(size);
    UINT order = UnitSizeToOrder(unitSize);

    try
    {
        size_t offset = AllocateBlock(order);
        uint32_t paddedSize = uint32_t(OrderToUnitSize(order) * m_minBlockSize);

        uint32_t blockOffset = uint32_t(m_baseOffset + (offset * m_minBlockSize));

        INCREASE_BUDDY_COUNTER(m_SpaceUsed, paddedSize);
        INCREASE_BUDDY_COUNTER(m_InternalFragmentation, (paddedSize - size));

        BuddyBlock* pBlock = new BuddyBlock(blockOffset, //offset
            paddedSize, //total size (padded to fit a block)
            numElements * elementSize);
            
        if (m_allocationStrategy == kBuddyAllocationStrategy::kPlacedResourceStrategy)
        {
            pBlock->InitPlaced(m_pBackingHeap, numElements, elementSize, initialData);
        }
        else
        {
            //TODO: To be truely thread-safe this operation should be atomic to guard against
            //      the case in which blocks from this allocator are used on multiple threads 
            //      (because it's really only 1 resource underneath)
            pBlock->InitFromResource(&m_BackingResource, numElements, elementSize, initialData);
        }

        return pBlock;
    }

    catch (std::bad_alloc&)
    {
        // There are no blocks available for the requested size so  
        // return the NULL block type  
        return new BuddyBlock();
    }
}

/*
void BuddyAllocator::Deallocate(BuddyBlock* pBlock)
{
    pBlock->m_fenceValue = g_CommandManager.GetNextFenceValue();
    m_deferredDeletionQueue.push(pBlock);
}
*/

void BuddyAllocator::DeallocateInternal(BuddyBlock* pBlock)
{
    ASSERT(IsOwner(*pBlock));

    size_t offset = SizeToUnitSize(pBlock->GetOffset() - m_baseOffset);

    size_t size = SizeToUnitSize(pBlock->GetSize());

    UINT order = UnitSizeToOrder(size);

    try
    {
        DeallocateBlock(offset, order); // throw(std::bad_alloc)

        DECREASE_BUDDY_COUNTER(m_SpaceUsed, size);
        DECREASE_BUDDY_COUNTER(m_InternalFragmentation, (size - pBlock->m_unpaddedSize));
        
        if (m_allocationStrategy == kBuddyAllocationStrategy::kPlacedResourceStrategy)
        {
            // Release the resource
            pBlock->Destroy();
        }
        delete(pBlock);
    }

    catch (std::bad_alloc&)
    {
        // Deallocate failed trying to add the free block to the pool  
        // resulting in a leak.  Unfortunately there is not much we can do.  
        // Fortunately this is expected to be extremely rare as the storage  
        // needed for each deallocate is very small.  
    }
};

/*
void BuddyAllocator::CleanUpAllocations()
{
    while (m_deferredDeletionQueue.empty() == false &&
        g_CommandManager.IsFenceComplete(m_deferredDeletionQueue.front()->m_fenceValue))
    {
        BuddyBlock* pBlock = m_deferredDeletionQueue.front();
        m_deferredDeletionQueue.pop();

        DeallocateInternal(pBlock);
    }
}*/
