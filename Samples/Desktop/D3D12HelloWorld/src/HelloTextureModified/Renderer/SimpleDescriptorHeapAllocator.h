#pragma once

#include "../DXSampleHelper.h"
#include "../MyDx12Utils.h"

#include <cassert>
#include <vector>

struct DescriptorHeapHandle
{
    UINT Index = UINT_MAX;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpu{};

    bool IsValid() const
    {
        return Index != UINT_MAX;
    }
};

struct SimpleDescriptorHeapAllocator;

class DescriptorAllocation
{
public:
    DescriptorAllocation() = default;
    DescriptorAllocation(SimpleDescriptorHeapAllocator* allocator, DescriptorHeapHandle handle);
    ~DescriptorAllocation();

    DescriptorAllocation(const DescriptorAllocation&) = delete;
    DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

    DescriptorAllocation(DescriptorAllocation&& other) noexcept;
    DescriptorAllocation& operator=(DescriptorAllocation&& other) noexcept;

    const DescriptorHeapHandle& Handle() const
    {
        return m_handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Cpu() const
    {
        return m_handle.cpu;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE Gpu() const
    {
        return m_handle.gpu;
    }

    bool IsValid() const
    {
        return m_handle.IsValid();
    }

    void Reset();

private:
    SimpleDescriptorHeapAllocator* m_allocator = nullptr;
    DescriptorHeapHandle m_handle;
};

// Simple free list based allocator
struct SimpleDescriptorHeapAllocator
{
    ID3D12DescriptorHeap* Heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
    D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
    UINT HeapHandleIncrement;
    std::vector<UINT> FreeIndices;

    // Initialize the allocator with a descriptor heap.
    // The heap must not have been used before and must not be used elsewhere after initialization.
    void Init(ID3D12Device* device, ID3D12DescriptorHeap* heap)
    {
        assert(Heap == nullptr && FreeIndices.empty());
        Heap = heap;
        D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
        HeapType = desc.Type;
        HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
        HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
        HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
        FreeIndices.reserve((int)desc.NumDescriptors);
        for (int n = desc.NumDescriptors; n > 0; n--)
        {
            FreeIndices.push_back(n - 1);
        }
    }
    void Destroy()
    {
        Heap = nullptr;
        FreeIndices.clear();
    }

    // For ImGui
    UINT Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle,
               D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle = nullptr)
    {
        assert(FreeIndices.size() > 0);
        UINT idx = FreeIndices.back();
        FreeIndices.pop_back();
        if (out_cpu_desc_handle)
            out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
        if (out_gpu_desc_handle)
            out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);

        DBG_PRINT("[SimpleDescriptorHeapAllocator] Alloc() returns idx=%d\n", idx);
        return idx;
    }

    DescriptorHeapHandle AllocWithHandle()
    {
        DescriptorHeapHandle handle = {};
        handle.Index = Alloc(&handle.cpu, &handle.gpu);
        return handle;
    }

    DescriptorAllocation Allocate()
    {
        return DescriptorAllocation(this, AllocWithHandle());
    }

    void Free(DescriptorHeapHandle handle)
    {
        if (!handle.IsValid())
        {
            return;
        }

        FreeIndices.push_back(handle.Index);
    }

    // For ImGui
    void Free(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle,
              D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle = nullptr)
    {
        if (out_cpu_desc_handle == nullptr || out_gpu_desc_handle == nullptr)
        {
            assert(0 && "Both CPU and GPU descriptor handles must be provided to free a descriptor.");
        }
        int idx = -1;
        if (out_cpu_desc_handle)
        {
            idx = (int)((out_cpu_desc_handle->ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
        }
        else if (out_gpu_desc_handle)
        {
            idx = (int)((out_gpu_desc_handle->ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
        }
        FreeIndices.push_back(idx);
    }
};

inline DescriptorAllocation::DescriptorAllocation(SimpleDescriptorHeapAllocator* allocator, DescriptorHeapHandle handle)
    : m_allocator(allocator), m_handle(handle)
{
}

inline DescriptorAllocation::~DescriptorAllocation()
{
    Reset();
}

inline DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& other) noexcept
    : m_allocator(other.m_allocator), m_handle(other.m_handle)
{
    other.m_allocator = nullptr;
    other.m_handle = {};
}

inline DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other) noexcept
{
    if (this != &other)
    {
        Reset();
        m_allocator = other.m_allocator;
        m_handle = other.m_handle;
        other.m_allocator = nullptr;
        other.m_handle = {};
    }

    return *this;
}

inline void DescriptorAllocation::Reset()
{
    if (m_allocator != nullptr && m_handle.IsValid())
    {
        m_allocator->Free(m_handle);
    }

    m_allocator = nullptr;
    m_handle = {};
}
