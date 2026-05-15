#pragma once

struct DescriptorHeapHandle
{
    UINT Index = UINT_MAX;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
};

// Simple free list based allocator
struct SimpleDescriptorHeapAllocator
{
    ID3D12DescriptorHeap *Heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
    D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
    UINT HeapHandleIncrement;
    std::vector<UINT> FreeIndices;

    // Initialize the allocator with a descriptor heap.
    // The heap must not have been used before and must not be used elsewhere after initialization.
    void Init(ID3D12Device *device, ID3D12DescriptorHeap *heap)
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
    UINT Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_desc_handle,
               D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_desc_handle = nullptr)
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

    // For ImGui
    void Free(D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_desc_handle,
              D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_desc_handle = nullptr)
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