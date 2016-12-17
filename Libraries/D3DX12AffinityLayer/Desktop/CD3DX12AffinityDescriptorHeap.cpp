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

#include "d3dx12affinity.h"
#include "Utils.h"

D3D12_DESCRIPTOR_HEAP_DESC STDMETHODCALLTYPE CD3DX12AffinityDescriptorHeap::GetDesc(UINT AffinityIndex)
{
    return mDescriptorHeaps[AffinityIndex]->GetDesc();
}

D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE CD3DX12AffinityDescriptorHeap::GetCPUDescriptorHandleForHeapStart(void)
{
    if (GetNodeCount() == 1)
    {
        return mDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
    }
    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = (SIZE_T)mCPUHeapStart;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE CD3DX12AffinityDescriptorHeap::GetGPUDescriptorHandleForHeapStart(void)
{
    if (GetNodeCount() == 1)
    {
        return mDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
    }
    D3D12_GPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = (SIZE_T)mGPUHeapStart;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE CD3DX12AffinityDescriptorHeap::GetActiveCPUDescriptorHandleForHeapStart(UINT AffinityIndex)
{
    return mDescriptorHeaps[AffinityIndex]->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE CD3DX12AffinityDescriptorHeap::GetActiveGPUDescriptorHandleForHeapStart(UINT AffinityIndex)
{
    return mDescriptorHeaps[AffinityIndex]->GetGPUDescriptorHandleForHeapStart();
}

void CD3DX12AffinityDescriptorHeap::InitDescriptorHandles(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    UINT const NodeCount = GetNodeCount();

    UINT maxindex = 0;
    for (UINT i = 0; i < NodeCount; ++i)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE const CPUBase = mDescriptorHeaps[i]->GetCPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE const GPUBase = mDescriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart();
        UINT HandleIncrement = 0;
        if (GetParentDevice()->GetAffinityMode() == EAffinityMode::LDA)
        {
            HandleIncrement = GetParentDevice()->GetChildObject(0)->GetDescriptorHandleIncrementSize(type);
        }
        for (UINT j = 0; j < mNumDescriptors; ++j)
        {
            mCPUHeapStart[j * NodeCount + i] = CPUBase.ptr + HandleIncrement * j;
            mGPUHeapStart[j * NodeCount + i] = GPUBase.ptr + HandleIncrement * j;
            maxindex = max(maxindex, j * NodeCount + i);
        }
    }


    DebugLog(L"Used up to index %u in heap array\n", maxindex);

    DebugLog(L"Created a descriptor heap with CPU start at 0x%IX and GPU start a 0x%IX\n", mCPUHeapStart, mGPUHeapStart);
    for (UINT i = 0; i < NodeCount; ++i)
    {
        DebugLog(L"  Device %u CPU starts at 0x%IX and GPU starts at 0x%IX\n",
            i, mDescriptorHeaps[i]->GetCPUDescriptorHandleForHeapStart().ptr, mDescriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart().ptr);
    }

#ifdef D3DX_AFFINITY_ENABLE_HEAP_POINTER_VALIDATION
    // Validation
    {
        std::lock_guard<std::mutex> lock(GetParentDevice()->MutexPointerRanges);

        GetParentDevice()->CPUHeapPointerRanges.push_back(std::make_pair((SIZE_T)mCPUHeapStart, (SIZE_T)(mCPUHeapStart + mNumDescriptors * NodeCount)));
        GetParentDevice()->GPUHeapPointerRanges.push_back(std::make_pair((SIZE_T)mGPUHeapStart, (SIZE_T)(mGPUHeapStart + mNumDescriptors * NodeCount)));
    }
#endif
}

CD3DX12AffinityDescriptorHeap::CD3DX12AffinityDescriptorHeap(CD3DX12AffinityDevice* device, ID3D12DescriptorHeap** descriptorHeaps, UINT Count)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(descriptorHeaps), Count)
    , mCPUHeapStart(nullptr)
    , mGPUHeapStart(nullptr)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mDescriptorHeaps[i] = descriptorHeaps[i];
        }
        else
        {
            mDescriptorHeaps[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"DescriptorHeap";
#endif
}

CD3DX12AffinityDescriptorHeap::~CD3DX12AffinityDescriptorHeap()
{
    if (mCPUHeapStart != nullptr)
    {
        delete[] mCPUHeapStart;
    }
    if (mGPUHeapStart != nullptr)
    {
        delete[] mGPUHeapStart;
    }
}

ID3D12DescriptorHeap* CD3DX12AffinityDescriptorHeap::GetChildObject(UINT AffinityIndex)
{
    return mDescriptorHeaps[AffinityIndex];
}
