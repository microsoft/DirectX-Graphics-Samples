#pragma once

#include "../DXSampleHelper.h"

#include <algorithm>
#include <cassert>
#include <vector>

// A slot-only handle returned by StagedDescriptorAllocator::Allocate().
// The Index is stable across Grow() calls; CPU/GPU descriptor handles
// for a given slot are obtained via StagedDescriptorAllocator::CpuHandle() /
// GpuHandle().
struct StagedDescriptorHandle
{
    UINT Index = UINT_MAX;

    bool IsValid() const
    {
        return Index != UINT_MAX;
    }
};

// A descriptor range returned by StagedDescriptorAllocator::AllocContiguous().
// Includes both the start slot and the count for safe freeing.
struct StagedDescriptorRange
{
    UINT Start = UINT_MAX;
    UINT Count = 0;

    bool IsValid() const
    {
        return Start != UINT_MAX;
    }
};

// A growable, staged descriptor allocator that writes into a reserved
// region of an externally-owned shader-visible descriptor heap.
//
// Design:
//   CPU heap (D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
//     - Authoritative storage. All writes happen here.
//     - Free list tracks available slots.
//     - Grows on demand (bounded by reservedCount).
//
//   External GPU heap (D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
//     - Owned and bound by the engine.
//     - A contiguous range [stageOffset, stageOffset + reservedCount) is
//       reserved for this allocator.
//     - Stage() copies from the CPU heap into this range.
//
//   Allocate() returns a slot-only StagedDescriptorHandle.
//   CPU/GPU descriptor handles are obtained via CpuHandle()/GpuHandle().
//
//   Stage() copies all currently allocated descriptors from CPU to the
//   external GPU heap's reserved range. Call once per frame before issuing
//   draw/dispatch commands that reference staged descriptors.
//
//   Frame-buffered staging: the external heap reserves
//   [stageOffset, stageOffset + reservedCount * kFrameCount).
//   SetFrameIndex(i) selects the i-th chunk
//   [stageOffset + i * reservedCount, stageOffset + (i+1) * reservedCount).
//   Call SetFrameIndex() before Stage() each frame so that the copy
//   destination and GpuHandle() use the same per-frame offset, avoiding
//   STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE.
//
//   Free() returns the logical index to the free list.
class StagedDescriptorAllocator
{
public:
    ~StagedDescriptorAllocator()
    {
        Destroy();
    }

    // Initialise with a starting capacity and the external GPU heap info.
    //   mainCpuStart  - CPU handle of the external shader-visible heap start
    //   mainGpuStart  - GPU handle of the external shader-visible heap start
    //   stageOffset   - offset (in descriptors) from main heap start for this
    //                   allocator's reserved range
    //   reservedCount - maximum number of descriptors this allocator may grow to
    void Init(ID3D12Device* device,
              D3D12_DESCRIPTOR_HEAP_TYPE type,
              UINT initialCapacity,
              D3D12_CPU_DESCRIPTOR_HANDLE mainCpuStart,
              D3D12_GPU_DESCRIPTOR_HANDLE mainGpuStart,
              UINT stageOffset,
              UINT reservedCount)
    {
        assert(m_device == nullptr);
        m_device = device;
        m_heapType = type;
        m_increment = device->GetDescriptorHandleIncrementSize(type);
        m_mainCpuStart = mainCpuStart;
        m_mainGpuStart = mainGpuStart;
        m_stageOffset = stageOffset;
        m_reservedCount = reservedCount;
        m_capacity = 0;
        m_maxUsedIndex = 0;

        Grow(initialCapacity);
    }

    void Destroy()
    {
        m_device = nullptr;
        m_cpuHeap.Reset();
        m_capacity = 0;
        m_increment = 0;
        m_maxUsedIndex = 0;
        m_freeIndices.clear();
        m_slotState.clear();
    }

    // Allocate one descriptor slot.
    StagedDescriptorHandle Allocate()
    {
        assert(m_device != nullptr);

        if (m_freeIndices.empty())
        {
            UINT growSize = (std::max)(m_capacity, 64u);
            Grow(growSize);
        }

        UINT idx = m_freeIndices.back();
        m_freeIndices.pop_back();

        assert(idx < m_slotState.size());
        assert(m_slotState[idx] == SlotState::Free);
        m_slotState[idx] = SlotState::Allocated;

        StagedDescriptorHandle handle;
        handle.Index = idx;

        if (idx >= m_maxUsedIndex)
        {
            m_maxUsedIndex = idx + 1;
        }

        return handle;
    }

    // Stage: copy all live descriptors from the CPU heap to the reserved
    // range of the external GPU heap.
    // Call once per frame before any draw/dispatch that references staged
    // descriptors.
    void Stage()
    {
        if (m_maxUsedIndex == 0)
        {
            return;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE srcStart = m_cpuStart;

        D3D12_CPU_DESCRIPTOR_HANDLE dstStart = {};
        dstStart.ptr = m_mainCpuStart.ptr + (CurrentStageOffset() * m_increment);

        m_device->CopyDescriptorsSimple(static_cast<UINT>(m_maxUsedIndex),
                                        dstStart,
                                        srcStart,
                                        m_heapType);
    }

    // Allocate a contiguous block of descriptor slots (for descriptor tables).
    StagedDescriptorRange AllocContiguous(UINT count)
    {
        assert(m_device != nullptr);
        assert(count > 0);

        UINT start = FindContiguousRun(count);
        if (start == UINT_MAX)
        {
            UINT growSize = (std::max)(count, (std::max)(m_capacity, 64u));
            Grow(growSize);
            start = FindContiguousRun(count);
            assert(start != UINT_MAX);
        }

        for (UINT i = 0; i < count; ++i)
        {
            UINT slot = start + i;
            assert(slot < m_slotState.size());
            assert(m_slotState[slot] == SlotState::Free);

            auto it = std::find(m_freeIndices.begin(), m_freeIndices.end(), slot);
            assert(it != m_freeIndices.end());
            *it = m_freeIndices.back();
            m_freeIndices.pop_back();

            m_slotState[slot] = SlotState::Allocated;
        }

        if (start + count > m_maxUsedIndex)
        {
            m_maxUsedIndex = start + count;
        }

        StagedDescriptorRange range;
        range.Start = start;
        range.Count = count;
        return range;
    }

    // Compute the CPU descriptor handle for a logical slot (in the CPU heap).
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle(UINT slot) const
    {
        assert(slot < m_capacity);
        D3D12_CPU_DESCRIPTOR_HANDLE h = {};
        h.ptr = m_cpuStart.ptr + (slot * m_increment);
        return h;
    }

    // Compute the GPU descriptor handle for a logical slot (in the external
    // GPU heap's reserved range).
    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle(UINT slot) const
    {
        assert(slot < m_capacity);
        D3D12_GPU_DESCRIPTOR_HANDLE h = {};
        h.ptr = m_mainGpuStart.ptr + ((CurrentStageOffset() + slot) * m_increment);
        return h;
    }

    void Free(StagedDescriptorHandle handle)
    {
        if (!handle.IsValid())
        {
            return;
        }

        UINT idx = handle.Index;
        assert(idx < m_slotState.size());
        assert(m_slotState[idx] == SlotState::Allocated);

        m_slotState[idx] = SlotState::Free;
        m_freeIndices.push_back(idx);

        if (idx + 1 == m_maxUsedIndex)
        {
            while (m_maxUsedIndex > 0)
            {
                --m_maxUsedIndex;
                auto it = std::find(m_freeIndices.begin(), m_freeIndices.end(), m_maxUsedIndex);
                if (it == m_freeIndices.end())
                {
                    ++m_maxUsedIndex;
                    break;
                }
            }
        }
    }

    void FreeContiguous(StagedDescriptorHandle first, UINT count)
    {
        if (!first.IsValid() || count == 0)
        {
            return;
        }

        for (UINT i = 0; i < count; ++i)
        {
            UINT idx = first.Index + i;
            assert(idx < m_slotState.size());
            assert(m_slotState[idx] == SlotState::Allocated);

            m_slotState[idx] = SlotState::Free;
            m_freeIndices.push_back(idx);
        }

        UINT end = first.Index + count;
        if (end == m_maxUsedIndex)
        {
            while (m_maxUsedIndex > 0)
            {
                --m_maxUsedIndex;
                auto it = std::find(m_freeIndices.begin(), m_freeIndices.end(), m_maxUsedIndex);
                if (it == m_freeIndices.end())
                {
                    ++m_maxUsedIndex;
                    break;
                }
            }
        }
    }

    void FreeContiguous(StagedDescriptorRange range)
    {
        if (!range.IsValid() || range.Count == 0)
        {
            return;
        }

        for (UINT i = 0; i < range.Count; ++i)
        {
            UINT idx = range.Start + i;
            assert(idx < m_slotState.size());
            assert(m_slotState[idx] == SlotState::Allocated);

            m_slotState[idx] = SlotState::Free;
            m_freeIndices.push_back(idx);
        }

        UINT end = range.Start + range.Count;
        if (end == m_maxUsedIndex)
        {
            while (m_maxUsedIndex > 0)
            {
                --m_maxUsedIndex;
                auto it = std::find(m_freeIndices.begin(), m_freeIndices.end(), m_maxUsedIndex);
                if (it == m_freeIndices.end())
                {
                    ++m_maxUsedIndex;
                    break;
                }
            }
        }
    }

    // Set the current frame index. This selects the per-frame chunk within the
    // external heap's reserved range. Must be called before Stage() and before
    // any code that calls GpuHandle() for the current frame.
    void SetFrameIndex(UINT frameIndex)
    {
        m_frameIndex = frameIndex;
    }

    UINT Capacity() const { return m_capacity; }
    UINT Used() const { return m_capacity - static_cast<UINT>(m_freeIndices.size()); }
    UINT DescriptorIncrement() const { return m_increment; }
    UINT CurrentFrameIndex() const { return m_frameIndex; }

private:
    void Grow(UINT additionalSlots)
    {
        UINT newCapacity = m_capacity + additionalSlots;
        if (newCapacity > m_reservedCount)
        {
            ThrowIfFailed(E_OUTOFMEMORY);
        }

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = m_heapType;
        desc.NumDescriptors = newCapacity;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        ComPtr<ID3D12DescriptorHeap> newCpuHeap;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newCpuHeap)));
        newCpuHeap->SetName(L"StagedDescriptorAllocator (CPU)");

        if (m_cpuHeap != nullptr && m_maxUsedIndex > 0)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srcStart = m_cpuHeap->GetCPUDescriptorHandleForHeapStart();
            D3D12_CPU_DESCRIPTOR_HANDLE dstStart = newCpuHeap->GetCPUDescriptorHandleForHeapStart();
            m_device->CopyDescriptorsSimple(m_maxUsedIndex, dstStart, srcStart, m_heapType);
        }

        for (UINT i = m_capacity; i < newCapacity; ++i)
        {
            m_freeIndices.push_back(i);
        }

        m_slotState.resize(newCapacity, SlotState::Free);

        m_cpuHeap.Swap(newCpuHeap);
        m_cpuStart = m_cpuHeap->GetCPUDescriptorHandleForHeapStart();
        m_capacity = newCapacity;
    }

    UINT FindContiguousRun(UINT count) const
    {
        if (m_freeIndices.size() < count)
        {
            return UINT_MAX;
        }

        std::vector<UINT> sorted = m_freeIndices;
        std::sort(sorted.begin(), sorted.end());

        if (count == 1)
        {
            return sorted[0];
        }

        UINT consecutive = 1;

        for (size_t i = 1; i < sorted.size(); ++i)
        {
            if (sorted[i] == sorted[i - 1] + 1)
            {
                ++consecutive;
                if (consecutive >= count)
                {
                    return sorted[i] - count + 1;
                }
            }
            else
            {
                consecutive = 1;
            }
        }

        return UINT_MAX;
    }

    // Current per-frame offset in the external GPU heap.
    UINT CurrentStageOffset() const
    {
        return m_stageOffset + m_frameIndex * m_reservedCount;
    }

    ID3D12Device* m_device = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE m_heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;

    ComPtr<ID3D12DescriptorHeap> m_cpuHeap;

    // Start of the CPU heap (authoritative storage).
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart{};

    // External shader-visible heap info for the staged copy destination.
    D3D12_CPU_DESCRIPTOR_HANDLE m_mainCpuStart{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_mainGpuStart{};
    UINT m_stageOffset = 0;
    UINT m_reservedCount = 0;
    UINT m_frameIndex = 0;

    UINT m_increment = 0;
    UINT m_capacity = 0;

    UINT m_maxUsedIndex = 0;

    enum class SlotState
    {
        Free,
        Allocated,
    };
    std::vector<SlotState> m_slotState;
    std::vector<UINT> m_freeIndices;
};
