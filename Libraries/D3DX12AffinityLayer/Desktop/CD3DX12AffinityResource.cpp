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

HRESULT STDMETHODCALLTYPE CD3DX12AffinityResource::Map(
    UINT Subresource,
    const D3D12_RANGE* pReadRange,
    void** ppData)
{
    if (GetParentDevice()->GetAffinityMode() == EAffinityMode::LDA)
    {
        return mResources[0]->Map(Subresource, pReadRange, ppData);
    }

    if (++mReferenceCount == 1)
    {
        DEBUG_ASSERT(mMappedAddresses.empty());

        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & mAffinityMask) != 0)
            {
                ID3D12Resource* Resource = mResources[i];
                void* MappedAddress = nullptr;
                HRESULT const hr = Resource->Map(Subresource, pReadRange, &MappedAddress);

                if (S_OK == hr)
                {
                    mMappedAddresses.push_back(MappedAddress);
                }
                else
                {
                    return hr;
                }
            }

            {
                std::lock_guard<std::mutex> lock(GetParentDevice()->MutexStillMappedResources);
                GetParentDevice()->StillMappedResources.insert(this);
            }
        }
    }

    switch (mCPUPageProperty)
    {
    case D3D12_CPU_PAGE_PROPERTY_WRITE_BACK:
        *ppData = mMappedAddresses[0];
        break;
    case D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE:
        *ppData = mShadowBuffer;
        break;
    default:
    case D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE:
    case D3D12_CPU_PAGE_PROPERTY_UNKNOWN:
        DEBUG_ASSERT(false);
        return S_FALSE;
    }

    return S_OK;
}

void STDMETHODCALLTYPE CD3DX12AffinityResource::Unmap(
    UINT Subresource,
    const D3D12_RANGE* pWrittenRange)
{
    if (GetParentDevice()->GetAffinityMode() == EAffinityMode::LDA)
    {
        mResources[0]->Unmap(Subresource, pWrittenRange);
        return;
    }

    if (--mReferenceCount == 0)
    {
        SynchronizeAcrossDevices();

        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & mAffinityMask) != 0)
            {
                ID3D12Resource* Resource = mResources[i];
                Resource->Unmap(Subresource, pWrittenRange);
            }
        }

        {
            std::lock_guard<std::mutex> lock(GetParentDevice()->MutexStillMappedResources);
            GetParentDevice()->StillMappedResources.erase(this);
        }
        mMappedAddresses.clear();
    }

    DEBUG_ASSERT(mReferenceCount >= 0);
}

D3D12_RESOURCE_DESC STDMETHODCALLTYPE CD3DX12AffinityResource::GetDesc(UINT AffinityIndex)
{
    while (!mResources[AffinityIndex])
    {
        AffinityIndex++;
    }
    return mResources[AffinityIndex]->GetDesc();
}

D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE CD3DX12AffinityResource::GetGPUVirtualAddress(void)
{
    if (GetNodeCount() == 1
#if TILE_MAPPING_GPUVA
        || GetParentDevice()->GetAffinityMode() == EAffinityMode::LDA
#endif
        )
    {
        return mResources[0]->GetGPUVirtualAddress();
    }
    if (0 == mVirtualAddress)
    {
        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> Addresses(GetNodeCount());
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & mAffinityMask) != 0)
            {
                Addresses[i] = mResources[i]->GetGPUVirtualAddress();
            }
        }
        mVirtualAddress = Addresses[0];

        GetParentDevice()->MutexGPUVirtualAddresses.lock();
        GetParentDevice()->GPUVirtualAddresses[mVirtualAddress] = std::move(Addresses);
        GetParentDevice()->MutexGPUVirtualAddresses.unlock();
    }

    return mVirtualAddress;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityResource::WriteToSubresource(
    UINT DstSubresource,
    const D3D12_BOX* pDstBox,
    const void* pSrcData,
    UINT SrcRowPitch,
    UINT SrcDepthPitch)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12Resource* Resource = mResources[i];
            HRESULT const hr = Resource->WriteToSubresource(
                DstSubresource,
                pDstBox,
                pSrcData,
                SrcRowPitch,
                SrcDepthPitch);

            if (S_OK != hr)
            {
                return hr;
            }
        }
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityResource::ReadFromSubresource(
    void* pDstData,
    UINT DstRowPitch,
    UINT DstDepthPitch,
    UINT SrcSubresource,
    const D3D12_BOX* pSrcBox,
    UINT AffinityIndex)
{
    return mResources[AffinityIndex]->ReadFromSubresource(
        pDstData,
        DstRowPitch,
        DstDepthPitch,
        SrcSubresource,
        pSrcBox);
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityResource::GetHeapProperties(
    D3D12_HEAP_PROPERTIES* pHeapProperties,
    D3D12_HEAP_FLAGS* pHeapFlags,
    UINT AffinityIndex)
{
    return mResources[AffinityIndex]->GetHeapProperties(pHeapProperties, pHeapFlags);
}

CD3DX12AffinityResource::CD3DX12AffinityResource(CD3DX12AffinityDevice* device, ID3D12Resource** resources, UINT Count, ID3D12Heap** heaps)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(resources), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mResources[i] = resources[i];
            mHeaps[i] = heaps ? heaps[i] : nullptr;
        }
        else
        {
            mResources[i] = nullptr;
            mHeaps[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"Resource";
#endif
    mVirtualAddress = 0;
}

CD3DX12AffinityResource::~CD3DX12AffinityResource()
{
    std::lock_guard<std::mutex> lock(GetParentDevice()->MutexStillMappedResources);
    GetParentDevice()->StillMappedResources.erase(this);

    if (0 != mVirtualAddress)
    {
        GetParentDevice()->GPUVirtualAddresses.erase(mVirtualAddress);
    }

    for (UINT i = 0; i < GetNodeCount(); i++)
    {
        if (mHeaps[i])
        {
            mHeaps[i]->Release();
            mHeaps[i] = nullptr;
        }
    }
}

void CD3DX12AffinityResource::SynchronizeAcrossDevices()
{
    if (GetParentDevice()->GetAffinityMode() == EAffinityMode::LDA)
    {
        return;
    }

    switch (mCPUPageProperty)
    {
    case D3D12_CPU_PAGE_PROPERTY_WRITE_BACK:
        for (size_t i = 1; i < mMappedAddresses.size(); ++i)
        {
            memcpy(mMappedAddresses[i], mMappedAddresses[0], static_cast<size_t>(mBufferSize));
        }
        ReleaseLog(L"D3DX12AffinityLayer: [memcpy] Synchronized %lu bytes of WRITE_BACK memory.\n", mBufferSize);
        break;
    case D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE:
    {
        static UINT const WrittenAddressBufferSize = 8192;
        UINT NumUpdated = 0;
        UINT64 BytesCopied = 0;

#ifdef DO_FULL_MAPPED_MEM_COPY
        for (size_t i = 0; i < mMappedAddresses.size(); ++i)
        {
            memcpy(mMappedAddresses[i], mShadowBuffer, static_cast<size_t>(mBufferSize));
            BytesCopied += mBufferSize;
        }
#else
        static thread_local void** WrittenAddresses = nullptr;
        if (nullptr == WrittenAddresses)
        {
            WrittenAddresses = new void*[WrittenAddressBufferSize];
        }

        ULONG_PTR Count = WrittenAddressBufferSize;
        ULONG Granularity = 0;
        GetWriteWatch(WRITE_WATCH_FLAG_RESET, mShadowBuffer, static_cast<size_t>(mBufferSize), WrittenAddresses, &Count, &Granularity);

        while (Count)
        {
            for (ULONG_PTR address = 0; address < Count; ++address)
            {
                ULONG_PTR const PageOffsetFromBasePtr = ((byte*)WrittenAddresses[address] - (byte*)mShadowBuffer);
                for (size_t i = 0; i < mMappedAddresses.size(); ++i)
                {
                    memcpy((byte*)mMappedAddresses[i] + PageOffsetFromBasePtr, WrittenAddresses[address], Granularity);
                }
                NumUpdated++;
                BytesCopied += Granularity;
            }

            Count = WrittenAddressBufferSize;
            GetWriteWatch(WRITE_WATCH_FLAG_RESET, mShadowBuffer, static_cast<size_t>(mBufferSize), WrittenAddresses, &Count, &Granularity);
        }
#endif

        ReleaseLog(L"D3DX12AffinityLayer: [memcpy] Synchronized %lu bytes of shadow buffered memory.\n", BytesCopied);
        break;
    }
    case D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE:
        DEBUG_FAIL_MESSAGE(L"Property of CPU Page unnavailable while synchronizing.");
        break;
    case D3D12_CPU_PAGE_PROPERTY_UNKNOWN:
        DEBUG_FAIL_MESSAGE(L"Property of CPU Page unknown while synchronizing.");
        break;
    default:
        DebugLog(L"Unexpected CPU page value: %d\n", mCPUPageProperty);
        DEBUG_FAIL_MESSAGE(L"Property of CPU Page entirely unexpected while synchronizing. Possible corruption.");
        break;
    }
}

void CD3DX12AffinityResource::UpdatePersistentMaps(CD3DX12AffinityDevice* pDevice)
{
    std::lock_guard<std::mutex> lock(pDevice->MutexStillMappedResources);
    for (CD3DX12AffinityResource* Resource : pDevice->StillMappedResources)
    {
        Resource->SynchronizeAcrossDevices();
    }
}

ID3D12Resource* CD3DX12AffinityResource::GetChildObject(UINT AffinityIndex)
{
    return mResources[AffinityIndex];
}
