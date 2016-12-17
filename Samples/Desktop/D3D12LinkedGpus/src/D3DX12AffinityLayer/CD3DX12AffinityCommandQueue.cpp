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

void STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::UpdateTileMappings(
    CD3DX12AffinityResource* pResource,
    UINT NumResourceRegions,
    const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates,
    const D3D12_TILE_REGION_SIZE* pResourceRegionSizes,
    CD3DX12AffinityHeap* pHeap,
    UINT NumRanges,
    const D3D12_TILE_RANGE_FLAGS* pRangeFlags,
    const UINT* pHeapRangeStartOffsets,
    const UINT* pRangeTileCounts,
    D3D12_TILE_MAPPING_FLAGS Flags,
    UINT AffinityMask)
{
    UINT ActiveNodeIndex = GetActiveNodeIndex();
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            {
                ID3D12CommandQueue* Queue = mCommandQueues[i];

                Queue->UpdateTileMappings(
                    pResource->mResources[i],
                    NumResourceRegions,
                    pResourceRegionStartCoordinates,
                    pResourceRegionSizes,
                    pHeap->GetChildObject(i),
                    NumRanges,
                    pRangeFlags,
                    pHeapRangeStartOffsets,
                    pRangeTileCounts,
                    Flags);
            }
        }
    }
}

void STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::CopyTileMappings(
    CD3DX12AffinityResource* pDstResource,
    const D3D12_TILED_RESOURCE_COORDINATE* pDstRegionStartCoordinate,
    CD3DX12AffinityResource* pSrcResource,
    const D3D12_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoordinate,
    const D3D12_TILE_REGION_SIZE* pRegionSize,
    D3D12_TILE_MAPPING_FLAGS Flags,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            ID3D12CommandQueue* Queue = mCommandQueues[i];

            Queue->CopyTileMappings(
                pDstResource->mResources[i],
                pDstRegionStartCoordinate,
                pSrcResource->mResources[i],
                pSrcRegionStartCoordinate,
                pRegionSize,
                Flags);
        }
    }
}

void STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::ExecuteCommandLists(
    UINT NumCommandLists,
    CD3DX12AffinityCommandList* const* ppCommandLists,
    UINT AffinityMask)
{
    std::vector<ID3D12CommandList*> mCachedCommandLists;
    mCachedCommandLists.resize(NumCommandLists);
    UINT ActiveNodeIndex = GetActiveNodeIndex();
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            {
                ID3D12CommandQueue* Queue = mCommandQueues[i];

                UINT index = 0;
                for (UINT c = 0; c < NumCommandLists; ++c)
                {
                    CD3DX12AffinityGraphicsCommandList* AffinityCommandList = static_cast<CD3DX12AffinityGraphicsCommandList*>(ppCommandLists[c]);
                    if (AffinityCommandList->GetActiveAffinityMask() & (1 << i))
                    {
                        mCachedCommandLists[index++] = AffinityCommandList->GetChildObject(i);
                    }
                }

                Queue->ExecuteCommandLists(index, mCachedCommandLists.data());

#ifdef SERIALIZE_COMMNANDLIST_EXECUTION
                ID3D12Fence* pFence;
                GetParentDevice()->mDevices[0]->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
                Queue->Signal(pFence, 1);
                HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                pFence->SetEventOnCompletion(1, hEvent);
                WaitForSingleObject(hEvent, INFINITE);
                CloseHandle(hEvent);
                pFence->Release();
                if (FAILED(GetParentDevice()->mDevices[0]->GetDeviceRemovedReason()))
                {
                    __debugbreak();
                }
#endif
            }
        }
    }
    ReleaseLog(L"D3DX12AffinityLayer: [event] ExecuteCommandLists\n");
}

void STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::SetMarker(
    UINT Metadata,
    const void* pData,
    UINT Size,
    UINT AffinityMask)
{
    UINT i = GetActiveNodeIndex();
    {
        ID3D12CommandQueue* Queue = mCommandQueues[i];

        Queue->SetMarker(Metadata, pData, Size);
    }
}

void STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::BeginEvent(
    UINT Metadata,
    const void* pData,
    UINT Size,
    UINT AffinityMask)
{
    UINT i = GetActiveNodeIndex();
    {
        ID3D12CommandQueue* Queue = mCommandQueues[i];

        Queue->BeginEvent(Metadata, pData, Size);
    }
}

void STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::EndEvent(UINT AffinityMask)
{
    UINT i = GetActiveNodeIndex();
    {
        ID3D12CommandQueue* Queue = mCommandQueues[i];

        Queue->EndEvent();
    }
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::Signal(
    CD3DX12AffinityFence* pFence,
    UINT64 Value,
    UINT AffinityMask)
{
    UINT ActiveNodeIndex = GetActiveNodeIndex();
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            {
                ID3D12CommandQueue* Queue = mCommandQueues[i];

                HRESULT const hr = Queue->Signal(pFence->mFences[i], Value);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::Wait(
    CD3DX12AffinityFence* pFence,
    UINT64 Value,
    bool UseActiveQueueOnly,
    UINT AffinityMask)
{
    UINT ActiveNodeIndex = GetActiveNodeIndex();
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            ID3D12CommandQueue* Queue = UseActiveQueueOnly ? mCommandQueues[ActiveNodeIndex] : mCommandQueues[i];

            HRESULT const hr = Queue->Wait(pFence->mFences[i], Value);

            if (hr != S_OK)
            {
                return hr;
            }
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::GetTimestampFrequency(
    UINT64* pFrequency,
    UINT AffinityIndex)
{
    return mCommandQueues[AffinityIndex]->GetTimestampFrequency(pFrequency);
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::GetClockCalibration(
    UINT64* pGpuTimestamp,
    UINT64* pCpuTimestamp,
    UINT AffinityIndex)
{
    return mCommandQueues[AffinityIndex]->GetClockCalibration(pGpuTimestamp, pCpuTimestamp);
}

D3D12_COMMAND_QUEUE_DESC STDMETHODCALLTYPE CD3DX12AffinityCommandQueue::GetDesc(UINT AffinityIndex)
{
    return mCommandQueues[AffinityIndex]->GetDesc();
}

ID3D12CommandQueue* CD3DX12AffinityCommandQueue::GetQueueForSwapChainCreation(UINT AffinityIndex)
{
    return mCommandQueues[AffinityIndex];
}

CD3DX12AffinityCommandQueue::CD3DX12AffinityCommandQueue(CD3DX12AffinityDevice* device, ID3D12CommandQueue** commandQueues, UINT Count)
    : CD3DX12AffinityPageable(device, reinterpret_cast<ID3D12Pageable**>(commandQueues), Count)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mCommandQueues[i] = commandQueues[i];
        }
        else
        {
            mCommandQueues[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"CommandQueue";
#endif
}

ID3D12CommandQueue* CD3DX12AffinityCommandQueue::GetChildObject(UINT AffinityIndex)
{
    return mCommandQueues[AffinityIndex];
}

void CD3DX12AffinityCommandQueue::WaitForCompletion(UINT AffinityMask)
{
    // Very inefficient. Should be used very sparingly.
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & EffectiveAffinityMask) != 0)
        {
            ID3D12Fence* pFence;
            ID3D12Device* pDevice = GetParentDevice()->GetAffinityMode() == EAffinityMode::LDA ?
                GetParentDevice()->GetChildObject(0) : GetParentDevice()->GetChildObject(i);
            pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
            mCommandQueues[i]->Signal(pFence, 1);
            HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            pFence->SetEventOnCompletion(1, hEvent);
            WaitForSingleObject(hEvent, INFINITE);
            CloseHandle(hEvent);
            pFence->Release();
            if (FAILED(pDevice->GetDeviceRemovedReason()))
            {
                __debugbreak();
            }
        }
    }
}
