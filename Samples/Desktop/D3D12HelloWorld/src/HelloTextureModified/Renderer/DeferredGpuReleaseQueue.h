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
#include "../DXSampleHelper.h"
#include "SimpleDescriptorHeapAllocator.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace Engine
{
struct DeferredGpuRelease
{
    UINT64 retireFenceValue = 0;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resources;
    std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocators;
    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> descriptorHeaps;
    std::vector<DescriptorHeapHandle> descriptorHandles;
};

class DeferredGpuReleaseQueue
{
public:
    void Retire(DeferredGpuRelease release)
    {
        m_pending.push_back(std::move(release));
    }

    void Collect(UINT64 completedFenceValue, SimpleDescriptorHeapAllocator& descriptorAllocator)
    {
        auto iter = m_pending.begin();
        while (iter != m_pending.end())
        {
            if (completedFenceValue < iter->retireFenceValue)
            {
                ++iter;
                continue;
            }

            FreeDescriptorHandles(iter->descriptorHandles, descriptorAllocator);

            iter = m_pending.erase(iter);
        }
    }

    bool Empty() const
    {
        return m_pending.empty();
    }

    std::size_t Count() const
    {
        return m_pending.size();
    }

private:
    static void FreeDescriptorHandles(const std::vector<DescriptorHeapHandle>& handles,
                                      SimpleDescriptorHeapAllocator& descriptorAllocator)
    {
        for (auto handleIter = handles.rbegin(); handleIter != handles.rend(); ++handleIter)
        {
            descriptorAllocator.Free(*handleIter);
        }
    }

    std::vector<DeferredGpuRelease> m_pending;
};
}
