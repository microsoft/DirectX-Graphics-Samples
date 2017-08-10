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
#include "DescriptorHeap.h"
#include "GraphicsCore.h"
#include "CommandListManager.h"

using namespace Graphics;

//
// DescriptorAllocator implementation
//
std::mutex DescriptorAllocator::sm_AllocationMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sm_DescriptorHeapPool;

void DescriptorAllocator::DestroyAll(void)
{
    sm_DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
    std::lock_guard<std::mutex> LockGuard(sm_AllocationMutex);

    D3D12_DESCRIPTOR_HEAP_DESC Desc;
    Desc.Type = Type;
    Desc.NumDescriptors = sm_NumDescriptorsPerHeap;
    Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Desc.NodeMask = 1;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
    ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&Desc, MY_IID_PPV_ARGS(&pHeap)));
    sm_DescriptorHeapPool.emplace_back(pHeap);
    return pHeap.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate( uint32_t Count )
{
    if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < Count)
    {
        m_CurrentHeap = RequestNewHeap(m_Type);
        m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
        m_RemainingFreeHandles = sm_NumDescriptorsPerHeap;

        if (m_DescriptorSize == 0)
            m_DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(m_Type);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
    m_CurrentHandle.ptr += Count * m_DescriptorSize;
    m_RemainingFreeHandles -= Count;
    return ret;
}

//
// UserDescriptorHeap implementation
//

void UserDescriptorHeap::Create( const std::wstring& DebugHeapName )
{
    ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&m_HeapDesc, MY_IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf())));
#ifdef RELEASE
    (void)DebugHeapName;
#else
    m_Heap->SetName(DebugHeapName.c_str());
#endif

    m_DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(m_HeapDesc.Type);
    m_NumFreeDescriptors = m_HeapDesc.NumDescriptors;
    m_FirstHandle = DescriptorHandle( m_Heap->GetCPUDescriptorHandleForHeapStart(),  m_Heap->GetGPUDescriptorHandleForHeapStart() );
    m_NextFreeHandle = m_FirstHandle;
}

DescriptorHandle UserDescriptorHeap::Alloc( uint32_t Count )
{
    ASSERT(HasAvailableSpace(Count), "Descriptor Heap out of space.  Increase heap size.");
    DescriptorHandle ret = m_NextFreeHandle;
    m_NextFreeHandle += Count * m_DescriptorSize;
    return ret;
}

bool UserDescriptorHeap::ValidateHandle( const DescriptorHandle& DHandle ) const
{
    if (DHandle.GetCpuHandle().ptr < m_FirstHandle.GetCpuHandle().ptr ||
        DHandle.GetCpuHandle().ptr >= m_FirstHandle.GetCpuHandle().ptr + m_HeapDesc.NumDescriptors * m_DescriptorSize)
        return false;

    if (DHandle.GetGpuHandle().ptr - m_FirstHandle.GetGpuHandle().ptr !=
        DHandle.GetCpuHandle().ptr - m_FirstHandle.GetCpuHandle().ptr)
        return false;

    return true;
}
