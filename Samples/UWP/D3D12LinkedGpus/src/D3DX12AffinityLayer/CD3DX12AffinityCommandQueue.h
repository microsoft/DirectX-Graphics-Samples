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

#include "Utils.h"
#include "CD3DX12AffinityPageable.h"

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityCommandQueue : public CD3DX12AffinityPageable
{
public:
    void STDMETHODCALLTYPE UpdateTileMappings(
        _In_  CD3DX12AffinityResource* pResource,
        UINT NumResourceRegions,
        _In_reads_opt_(NumResourceRegions)  const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates,
        _In_reads_opt_(NumResourceRegions)  const D3D12_TILE_REGION_SIZE* pResourceRegionSizes,
        _In_opt_  CD3DX12AffinityHeap* pHeap,
        UINT NumRanges,
        _In_reads_opt_(NumRanges)  const D3D12_TILE_RANGE_FLAGS* pRangeFlags,
        _In_reads_opt_(NumRanges)  const UINT* pHeapRangeStartOffsets,
        _In_reads_opt_(NumRanges)  const UINT* pRangeTileCounts,
        D3D12_TILE_MAPPING_FLAGS Flags,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CopyTileMappings(
        _In_  CD3DX12AffinityResource* pDstResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE* pDstRegionStartCoordinate,
        _In_  CD3DX12AffinityResource* pSrcResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoordinate,
        _In_  const D3D12_TILE_REGION_SIZE* pRegionSize,
        D3D12_TILE_MAPPING_FLAGS Flags,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE ExecuteCommandLists(
        _In_  UINT NumCommandLists,
        _In_reads_(NumCommandLists)  CD3DX12AffinityCommandList* const* ppCommandLists,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE SetMarker(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE BeginEvent(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE EndEvent(_In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE Signal(
        CD3DX12AffinityFence* pFence,
        UINT64 Value,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE Wait(
        CD3DX12AffinityFence* pFence,
        UINT64 Value,
        bool UseActiveQueueOnly = false,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetTimestampFrequency(
        _Out_  UINT64* pFrequency,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetClockCalibration(
        _Out_  UINT64* pGpuTimestamp,
        _Out_  UINT64* pCpuTimestamp,
        UINT AffinityIndex = 0);

    D3D12_COMMAND_QUEUE_DESC STDMETHODCALLTYPE GetDesc(UINT AffinityIndex = 0);

    ID3D12CommandQueue* GetQueueForSwapChainCreation(UINT AffinityIndex);
    ID3D12CommandQueue* GetChildObject(UINT AffinityIndex);
    void WaitForCompletion(UINT AffinityMask = EAffinityMask::AllNodes);

    CD3DX12AffinityCommandQueue(CD3DX12AffinityDevice* device, ID3D12CommandQueue** commandQueues, UINT Count);

private:
    std::vector<ID3D12CommandList*> mCachedCommandLists;
    ID3D12CommandQueue* mCommandQueues[D3DX12_MAX_ACTIVE_NODES];
};
