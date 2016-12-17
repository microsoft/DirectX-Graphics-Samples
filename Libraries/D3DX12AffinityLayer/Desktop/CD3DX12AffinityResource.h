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

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityResource : public CD3DX12AffinityPageable
{
public:
    HRESULT STDMETHODCALLTYPE Map(
        UINT Subresource,
        _In_opt_  const D3D12_RANGE* pReadRange,
        _Outptr_opt_result_bytebuffer_(_Inexpressible_("Dependent on resource"))  void** ppData);

    void STDMETHODCALLTYPE Unmap(
        UINT Subresource,
        _In_opt_  const D3D12_RANGE* pWrittenRange);

    D3D12_RESOURCE_DESC STDMETHODCALLTYPE GetDesc(UINT AffinityIndex = 0);

    D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE GetGPUVirtualAddress(void);

    HRESULT STDMETHODCALLTYPE WriteToSubresource(
        UINT DstSubresource,
        _In_opt_  const D3D12_BOX* pDstBox,
        _In_  const void* pSrcData,
        UINT SrcRowPitch,
        UINT SrcDepthPitch);

    HRESULT STDMETHODCALLTYPE ReadFromSubresource(
        _Out_  void* pDstData,
        UINT DstRowPitch,
        UINT DstDepthPitch,
        UINT SrcSubresource,
        _In_opt_  const D3D12_BOX* pSrcBox,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetHeapProperties(
        _Out_opt_  D3D12_HEAP_PROPERTIES* pHeapProperties,
        _Out_opt_  D3D12_HEAP_FLAGS* pHeapFlags,
        UINT AffinityIndex = 0);

    CD3DX12AffinityResource(CD3DX12AffinityDevice* device, ID3D12Resource** resources, UINT Count, ID3D12Heap** heaps = nullptr);
    ~CD3DX12AffinityResource();

    ID3D12Resource* GetChildObject(UINT AffinityIndex);
    void SynchronizeAcrossDevices();

    static void UpdatePersistentMaps(CD3DX12AffinityDevice* pDevice);


    ID3D12Resource* mResources[D3DX12_MAX_ACTIVE_NODES];
    ID3D12Heap* mHeaps[D3DX12_MAX_ACTIVE_NODES];
    std::vector<void*> mMappedAddresses;
    int mReferenceCount;
    void* mShadowBuffer;
    UINT64 mBufferSize;
    D3D12_CPU_PAGE_PROPERTY mCPUPageProperty;
    D3D12_GPU_VIRTUAL_ADDRESS mVirtualAddress;

    ID3D12CommandList* mSyncCommandLists[D3DX12_MAX_ACTIVE_NODES];
    ID3D12CommandAllocator* mSyncCommandAllocators[D3DX12_MAX_ACTIVE_NODES];
};
