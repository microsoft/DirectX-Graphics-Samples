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
// Description:  This is a dynamic graphics memory allocator for DX12.  It's designed to work in concert
// with the CommandContext class and to do so in a thread-safe manner.  There may be many command contexts,
// each with its own linear allocators.  They act as windows into a global memory pool by reserving a
// context-local memory page.  Requesting a new page is done in a thread-safe manner by guarding accesses
// with a mutex lock.
//
// When a command context is finished, it will receive a fence ID that indicates when it's safe to reclaim
// used resources.  The CleanupUsedPages() method must be invoked at this time so that the used pages can be
// scheduled for reuse after the fence has cleared.

#pragma once

#include "GpuResource.h"
#include <vector>
#include <queue>
#include <mutex>

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256

// Various types of allocations may contain NULL pointers.  Check before dereferencing if you are unsure.
struct DynAlloc
{
    DynAlloc(GpuResource& BaseResource, size_t ThisOffset, size_t ThisSize)
        : Buffer(BaseResource), Offset(ThisOffset), Size(ThisSize) {}

    GpuResource& Buffer;	// The D3D buffer associated with this memory.
    size_t Offset;			// Offset from start of buffer resource
    size_t Size;			// Reserved size of this allocation
    void* DataPtr;			// The CPU-writeable address
    D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;	// The GPU-visible address
};

class LinearAllocationPage : public GpuResource
{
public:
    LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Usage) : GpuResource()
    {
        m_pResource.Attach(pResource);
        m_UsageState = Usage;
        m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
        m_pResource->Map(0, nullptr, &m_CpuVirtualAddress);
    }

    ~LinearAllocationPage()
    {
        Unmap();
    }

    void Map(void)
    {
        if (m_CpuVirtualAddress == nullptr)
        {
            m_pResource->Map(0, nullptr, &m_CpuVirtualAddress);
        }
    }

    void Unmap(void)
    {
        if (m_CpuVirtualAddress != nullptr)
        {
            m_pResource->Unmap(0, nullptr);
            m_CpuVirtualAddress = nullptr;
        }
    }

    void* m_CpuVirtualAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};

enum LinearAllocatorType
{
    kInvalidAllocator = -1,

    kGpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
    kCpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

    kNumAllocatorTypes
};

enum
{
    kGpuAllocatorPageSize = 0x10000,	// 64K
    kCpuAllocatorPageSize = 0x200000	// 2MB
};

class LinearAllocatorPageManager
{
public:

    LinearAllocatorPageManager();
    LinearAllocationPage* RequestPage( void );
    LinearAllocationPage* CreateNewPage( size_t PageSize = 0 );

    // Discarded pages will get recycled.  This is for fixed size pages.
    void DiscardPages( uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages );

    // Freed pages will be destroyed once their fence has passed.  This is for single-use,
    // "large" pages.
    void FreeLargePages( uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages );

    void Destroy( void ) { m_PagePool.clear(); }

private:

    static LinearAllocatorType sm_AutoType;

    LinearAllocatorType m_AllocationType;
    std::vector<std::unique_ptr<LinearAllocationPage> > m_PagePool;
    std::queue<std::pair<uint64_t, LinearAllocationPage*> > m_RetiredPages;
    std::queue<std::pair<uint64_t, LinearAllocationPage*> > m_DeletionQueue;
    std::queue<LinearAllocationPage*> m_AvailablePages;
    std::mutex m_Mutex;
};

class LinearAllocator
{
public:

    LinearAllocator(LinearAllocatorType Type) : m_AllocationType(Type), m_PageSize(0), m_CurOffset(~(size_t)0), m_CurPage(nullptr)
    {
        ASSERT(Type > kInvalidAllocator && Type < kNumAllocatorTypes);
        m_PageSize = (Type == kGpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
    }

    DynAlloc Allocate( size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN );

    void CleanupUsedPages( uint64_t FenceID );

    static void DestroyAll( void )
    {
        sm_PageManager[0].Destroy();
        sm_PageManager[1].Destroy();
    }

private:

    DynAlloc AllocateLargePage( size_t SizeInBytes );

    static LinearAllocatorPageManager sm_PageManager[2];

    LinearAllocatorType m_AllocationType;
    size_t m_PageSize;
    size_t m_CurOffset;
    LinearAllocationPage* m_CurPage;
    std::vector<LinearAllocationPage*> m_RetiredPages;
    std::vector<LinearAllocationPage*> m_LargePageList;
};
