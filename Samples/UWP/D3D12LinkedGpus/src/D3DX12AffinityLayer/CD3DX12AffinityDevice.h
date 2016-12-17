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
#include "d3dx12affinity_structs.h"
#include "CD3DX12AffinityObject.h"

struct D3DX12_CPU_DESCRIPTOR_HANDLE_COMPARATOR
{
    bool operator()(D3D12_CPU_DESCRIPTOR_HANDLE const& Left, D3D12_CPU_DESCRIPTOR_HANDLE const& Right) const
    {
        return Left.ptr < Right.ptr;
    }
};

struct D3DX12_GPU_DESCRIPTOR_HANDLE_COMPARATOR
{
    bool operator()(D3D12_GPU_DESCRIPTOR_HANDLE const& Left, D3D12_GPU_DESCRIPTOR_HANDLE const& Right) const
    {
        return Left.ptr < Right.ptr;
    }
};

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityDevice : public CD3DX12AffinityObject
{
    friend class CD3DX12AffinityResource;

public:
    virtual void STDMETHODCALLTYPE SetAffinity(
        _In_  UINT AffinityMask);

    UINT STDMETHODCALLTYPE GetNodeCount(void);

    HRESULT STDMETHODCALLTYPE CreateCommandQueue(
        _In_  const D3D12_COMMAND_QUEUE_DESC* pDesc,
        _In_ REFIID riid,
        _Outptr_  void** ppCommandQueue,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateCommandAllocator(
        _In_  D3D12_COMMAND_LIST_TYPE type,
        _In_ REFIID riid,
        _Outptr_  void** ppCommandAllocator,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateGraphicsPipelineState(
        _In_  const D3DX12_AFFINITY_GRAPHICS_PIPELINE_STATE_DESC* pDesc,
        _In_ REFIID riid,
        _Outptr_  void** ppPipelineState,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateComputePipelineState(
        _In_  const D3DX12_AFFINITY_COMPUTE_PIPELINE_STATE_DESC* pDesc,
        _In_ REFIID riid,
        _Outptr_  void** ppPipelineState,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateCommandList(
        _In_  UINT nodeMask,
        _In_  D3D12_COMMAND_LIST_TYPE type,
        _In_  CD3DX12AffinityCommandAllocator* pCommandAllocator,
        _In_opt_  CD3DX12AffinityPipelineState* pInitialState,
        _In_ REFIID riid,
        _Outptr_  void** ppCommandList,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CheckFeatureSupport(
        D3D12_FEATURE Feature,
        _Inout_updates_bytes_(FeatureSupportDataSize)  void* pFeatureSupportData,
        UINT FeatureSupportDataSize,
        _In_ UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE CreateDescriptorHeap(
        _In_  const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc,
        _In_ REFIID riid,
        _Outptr_  void** ppvHeap,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    UINT STDMETHODCALLTYPE GetDescriptorHandleIncrementSize(
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    UINT STDMETHODCALLTYPE GetActiveDescriptorHandleIncrementSize(
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType,
        _In_opt_  UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE CreateRootSignature(
        _In_  UINT nodeMask,
        _In_reads_(blobLengthInBytes)  const void* pBlobWithRootSignature,
        _In_  SIZE_T blobLengthInBytes,
        _In_ REFIID riid,
        _Outptr_  void** ppvRootSignature,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CreateConstantBufferView(
        _In_opt_  const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CreateShaderResourceView(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void STDMETHODCALLTYPE CreateShaderResourceViewWithAffinity(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CreateUnorderedAccessView(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  CD3DX12AffinityResource* pCounterResource,
        _In_opt_  const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void STDMETHODCALLTYPE CreateUnorderedAccessViewWithAffinity(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  CD3DX12AffinityResource* pCounterResource,
        _In_opt_  const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CreateRenderTargetView(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void STDMETHODCALLTYPE CreateRenderTargetViewWithAffinity(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CreateDepthStencilView(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void STDMETHODCALLTYPE CreateDepthStencilViewWithAffinity(
        _In_opt_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CreateSampler(
        _In_  const D3D12_SAMPLER_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CopyDescriptors(
        _In_  UINT NumDestDescriptorRanges,
        _In_reads_(NumDestDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts,
        _In_reads_opt_(NumDestDescriptorRanges)  const UINT* pDestDescriptorRangeSizes,
        _In_  UINT NumSrcDescriptorRanges,
        _In_reads_(NumSrcDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts,
        _In_reads_opt_(NumSrcDescriptorRanges)  const UINT* pSrcDescriptorRangeSizes,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CopyDescriptorsOne(
        _In_  UINT NumDestDescriptorRanges,
        _In_reads_(NumDestDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts,
        _In_reads_opt_(NumDestDescriptorRanges)  const UINT* pDestDescriptorRangeSizes,
        _In_  UINT NumSrcDescriptorRanges,
        _In_reads_(NumSrcDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts,
        _In_reads_opt_(NumSrcDescriptorRanges)  const UINT* pSrcDescriptorRangeSizes,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE CopyDescriptorsSimple(
        _In_  UINT NumDescriptors,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE GetResourceAllocationInfo(
        _In_  UINT visibleMask,
        _In_  UINT numResourceDescs,
        _In_reads_(numResourceDescs)  const D3D12_RESOURCE_DESC* pResourceDescs,
        UINT AffinityIndex = 0);

    D3D12_HEAP_PROPERTIES STDMETHODCALLTYPE GetCustomHeapProperties(
        _In_  UINT nodeMask,
        D3D12_HEAP_TYPE heapType,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE CreateCommittedResource(
        _In_  const D3D12_HEAP_PROPERTIES* pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        _In_  const D3D12_RESOURCE_DESC* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        _In_ REFIID riid,
        _Outptr_opt_  void** ppvResource,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateHeap(
        _In_  const D3D12_HEAP_DESC* pDesc,
        REFIID riid,
        _Outptr_opt_  void** ppvHeap,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreatePlacedResource(
        _In_  CD3DX12AffinityHeap* pHeap,
        UINT64 HeapOffset,
        _In_  const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riid,
        _Outptr_opt_  void** ppvResource,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateReservedResource(
        _In_  const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riid,
        _Outptr_opt_  void** ppvResource,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE MakeResident(
        UINT NumObjects,
        _In_reads_(NumObjects)  CD3DX12AffinityPageable* const* ppObjects,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE Evict(
        UINT NumObjects,
        _In_reads_(NumObjects)  CD3DX12AffinityPageable* const* ppObjects,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateFence(
        UINT64 InitialValue,
        D3D12_FENCE_FLAGS Flags,
        _In_ REFIID riid,
        _COM_Outptr_  void** ppFence,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason(UINT AffinityIndex = 0);

    void STDMETHODCALLTYPE GetCopyableFootprints(
        _In_  const D3D12_RESOURCE_DESC* pResourceDesc,
        _In_range_(0, D3D12_REQ_SUBRESOURCES)  UINT FirstSubresource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource)  UINT NumSubresources,
        UINT64 BaseOffset,
        _Out_writes_opt_(NumSubresources)  D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
        _Out_writes_opt_(NumSubresources)  UINT* pNumRows,
        _Out_writes_opt_(NumSubresources)  UINT64* pRowSizeInBytes,
        _Out_opt_  UINT64* pTotalBytes,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateQueryHeap(
        _In_  const D3D12_QUERY_HEAP_DESC* pDesc,
        _In_ REFIID riid,
        _Outptr_opt_  void** ppvHeap,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE SetStablePowerState(
        BOOL Enable,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE CreateCommandSignature(
        _In_  const D3D12_COMMAND_SIGNATURE_DESC* pDesc,
        _In_opt_  CD3DX12AffinityRootSignature* pRootSignature,
        _In_ REFIID riid,
        _Outptr_opt_  CD3DX12AffinityCommandSignature** ppvCommandSignature,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    void STDMETHODCALLTYPE GetResourceTiling(
        _In_  CD3DX12AffinityResource* pTiledResource,
        _Out_opt_  UINT* pNumTilesForEntireResource,
        _Out_opt_  D3D12_PACKED_MIP_INFO* pPackedMipDesc,
        _Out_opt_  D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips,
        _Inout_opt_  UINT* pNumSubresourceTilings,
        _In_  UINT FirstSubresourceTilingToGet,
        _Out_writes_(*pNumSubresourceTilings)  D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips,
        UINT AffinityIndex = 0);

    LUID STDMETHODCALLTYPE GetAdapterLuid(UINT AffinityIndex = 0);
    ID3D12Device* GetChildObject(UINT AffinityIndex);

    CD3DX12AffinityDevice(ID3D12Device** devices, UINT Count, EAffinityMode::Mask affinitymode);
    virtual ~CD3DX12AffinityDevice();
    EAffinityMode::Mask GetAffinityMode();
    EAffinityRenderingMode::Mask GetAffinityRenderingMode();
    void SetAffinityRenderingMode(EAffinityRenderingMode::Mask renderingmode);
    UINT AffinityIndexToNodeMask(UINT const Index);
    UINT GetDeviceCount();

    UINT LDAAllNodeMasks();
    UINT GetActiveNodeMask();
    void SwitchToNextNode();

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHeapPointer(D3D12_CPU_DESCRIPTOR_HANDLE const& Original, UINT const NodeIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHeapPointer(D3D12_GPU_DESCRIPTOR_HANDLE const& Original, UINT const NodeIndex);
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS const& Original, UINT const NodeIndex);

protected:
    virtual bool IsD3D();

private:
    void UpdateActiveDevices();
    ID3D12Device* mDevices[D3DX12_MAX_ACTIVE_NODES];

    UINT mNumActiveDevices = 0;
    EAffinityMode::Mask mAffinityMode;
    EAffinityRenderingMode::Mask mAffinityRenderingMode;

    UINT mDeviceCount = 0;

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> DestDescriptorRangeStartsCache;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> SrcDescriptorRangeStartsCache;

    std::mutex MutexPointerRanges;
    std::vector<std::pair<SIZE_T, SIZE_T>> CPUHeapPointerRanges;
    std::vector<std::pair<SIZE_T, SIZE_T>> GPUHeapPointerRanges;

    std::map<D3D12_GPU_VIRTUAL_ADDRESS, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>> GPUVirtualAddresses;
    std::mutex MutexGPUVirtualAddresses;

    std::set<CD3DX12AffinityResource*> StillMappedResources;
    std::mutex MutexStillMappedResources;

    UINT mLDANodeCount = 0;

    ID3D12CommandQueue* mSyncCommandQueues[D3DX12_MAX_ACTIVE_NODES];
    ID3D12Fence* mSyncFences[D3DX12_MAX_ACTIVE_NODES];
    std::mutex MutexSyncResources;
    std::vector<CD3DX12AffinityResource*> mSyncResources;
    ID3D12InfoQueue* InfoQueue = nullptr;

public:
    void WriteApplicationMessage(D3D12_MESSAGE_SEVERITY const Severity, char const* const Message);

    // Static Member Variables

    // Used to do super simple "device that's doing work in an AFR scenario" tracking
    // Probably this should be application controlled, but this very simple mechanism
    // is a good starting point and also helps keep the first affinity integration step
    // as minimal as possible.
    static UINT g_ActiveNodeIndex;
    static UINT g_CachedNodeCount;
    static UINT g_CachedNodeMask;
};
