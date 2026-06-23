#include "stdafx.h"

#include "StagedDescriptorAllocator.h"

#include <cstdio>

// Helper: create a temporary shader-visible heap to pass to the allocator.
static ComPtr<ID3D12DescriptorHeap> CreateTestExternalHeap(ID3D12Device* device, UINT count)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = count;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ComPtr<ID3D12DescriptorHeap> heap;
    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap)));
    return heap;
}

// Quick smoke test for StagedDescriptorAllocator.
// Verifies allocation, staging, freeing, and growth.
// Returns true on success.
static bool RunStagedAllocatorTest(ID3D12Device* device)
{
    auto extHeap = CreateTestExternalHeap(device, 64);
    D3D12_CPU_DESCRIPTOR_HANDLE mainCpu = extHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE mainGpu = extHeap->GetGPUDescriptorHandleForHeapStart();

    StagedDescriptorAllocator alloc;
    alloc.Init(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4,
               mainCpu, mainGpu, 0, 64);

    // Allocate 3 slots
    auto a = alloc.Allocate();
    auto b = alloc.Allocate();
    auto c = alloc.Allocate();

    if (!a.IsValid() || !b.IsValid() || !c.IsValid())
    {
        printf("FAIL: Allocate returned invalid handles\n");
        return false;
    }

    if (alloc.Used() != 3 || alloc.Capacity() != 4)
    {
        printf("FAIL: Used=%u (expected 3), Capacity=%u (expected 4)\n",
               alloc.Used(), alloc.Capacity());
        return false;
    }

    // Free one and re-allocate (reuses the freed slot)
    alloc.Free(b);
    if (alloc.Used() != 2)
    {
        printf("FAIL: After free, Used=%u (expected 2)\n", alloc.Used());
        return false;
    }

    auto d = alloc.Allocate();
    if (!d.IsValid())
    {
        printf("FAIL: Re-allocate after free failed\n");
        return false;
    }

    if (alloc.Used() != 3)
    {
        printf("FAIL: After re-allocate, Used=%u (expected 3)\n", alloc.Used());
        return false;
    }

    // Trigger growth
    alloc.Allocate(); // fills slot 0..3
    auto e = alloc.Allocate(); // triggers Grow(4)

    if (!e.IsValid())
    {
        printf("FAIL: Grow allocation failed\n");
        return false;
    }

    if (alloc.Capacity() != 8)
    {
        printf("FAIL: After grow, Capacity=%u (expected 8)\n", alloc.Capacity());
        return false;
    }

    if (alloc.Used() != 5)
    {
        printf("FAIL: After grow+alloc, Used=%u (expected 5)\n", alloc.Used());
        return false;
    }

    // Stage() uses the per-frame offset set by SetFrameIndex.
    alloc.SetFrameIndex(0);
    alloc.Stage();

    printf("PASS: StagedDescriptorAllocator smoke test ok (cap=%u, used=%u)\n",
           alloc.Capacity(), alloc.Used());

    return true;
}

// Test contiguous block allocation.
static bool RunContiguousAllocTest(ID3D12Device* device)
{
    auto extHeap = CreateTestExternalHeap(device, 64);
    D3D12_CPU_DESCRIPTOR_HANDLE mainCpu = extHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE mainGpu = extHeap->GetGPUDescriptorHandleForHeapStart();

    StagedDescriptorAllocator alloc;
    alloc.Init(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8,
               mainCpu, mainGpu, 0, 64);

    // AllocContiguous(1) should work (edge case from review-3).
    auto single = alloc.AllocContiguous(1);
    if (!single.IsValid())
    {
        printf("FAIL: AllocContiguous(1) returned invalid handle\n");
        return false;
    }

    // Free the single slot so the next test starts from a clean state.
    alloc.FreeContiguous(single);
    if (alloc.Used() != 0)
    {
        printf("FAIL: After freeing single slot, Used=%u (expected 0)\n", alloc.Used());
        return false;
    }

    // Allocate a block of 3 contiguous slots.
    auto block = alloc.AllocContiguous(3);
    if (!block.IsValid())
    {
        printf("FAIL: AllocContiguous returned invalid handle\n");
        return false;
    }

    if (alloc.Used() != 3 || alloc.Capacity() != 8)
    {
        printf("FAIL: Contiguous alloc Used=%u (expected 3), Cap=%u (expected 8)\n",
               alloc.Used(), alloc.Capacity());
        return false;
    }

    // Verify the three slots are actually consecutive by checking CPU handle spacing.
    UINT inc = alloc.DescriptorIncrement();
    D3D12_CPU_DESCRIPTOR_HANDLE base3 = alloc.CpuHandle(block.Start);
    for (UINT i = 1; i < block.Count; ++i)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE slot = alloc.CpuHandle(block.Start + i);
        if (slot.ptr != base3.ptr + i * inc)
        {
            printf("FAIL: Contiguous block slot %u not at expected offset "
                   "(expected %llu, got %llu)\n",
                   i, (unsigned long long)(base3.ptr + i * inc), (unsigned long long)slot.ptr);
            return false;
        }
    }

    // Verify that a new Allocate() does not return any of the contiguous slots.
    for (UINT i = 0; i < 5; ++i)
    {
        auto h = alloc.Allocate();
        if (!h.IsValid())
        {
            printf("FAIL: Could not allocate after contiguous block\n");
            return false;
        }
        for (UINT j = 0; j < block.Count; ++j)
        {
            if (h.Index == block.Start + j)
            {
                printf("FAIL: Allocate returned a slot (%u) from the contiguous block\n",
                       h.Index);
                return false;
            }
        }
    }

    // Free the contiguous block via FreeContiguous(range) and verify the slots
    // go back to the free list.
    alloc.FreeContiguous(block);
    if (alloc.Used() != 5)
    {
        printf("FAIL: After freeing contiguous block, Used=%u (expected 5)\n", alloc.Used());
        return false;
    }

    // The freed contiguous range should now be reusable by a single AllocContiguous.
    auto block3 = alloc.AllocContiguous(3);
    if (!block3.IsValid())
    {
        printf("FAIL: Re-allocation of freed contiguous block failed\n");
        return false;
    }

    alloc.SetFrameIndex(0);
    alloc.Stage();

    printf("PASS: ContiguousAlloc test ok\n");
    return true;
}

// Entry point called from D3D12HelloTexture after device init (debug builds only).
void RunStagedAllocatorTests(ID3D12Device* device)
{
    RunStagedAllocatorTest(device);
    RunContiguousAllocTest(device);
}
