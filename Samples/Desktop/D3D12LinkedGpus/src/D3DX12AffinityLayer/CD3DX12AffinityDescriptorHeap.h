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

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityDescriptorHeap : public CD3DX12AffinityPageable
{
    friend class CD3DX12AffinityDevice;

public:
    D3D12_DESCRIPTOR_HEAP_DESC STDMETHODCALLTYPE GetDesc(UINT AffinityIndex = 0);

    D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetCPUDescriptorHandleForHeapStart(void);

    D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetGPUDescriptorHandleForHeapStart(void);

    D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetActiveCPUDescriptorHandleForHeapStart(UINT AffinityIndex);

    D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetActiveGPUDescriptorHandleForHeapStart(UINT AffinityIndex);

    void InitDescriptorHandles(D3D12_DESCRIPTOR_HEAP_TYPE type);

    CD3DX12AffinityDescriptorHeap(CD3DX12AffinityDevice* device, ID3D12DescriptorHeap** descriptorHeaps, UINT Count);
    CD3DX12AffinityDescriptorHeap::~CD3DX12AffinityDescriptorHeap();

    ID3D12DescriptorHeap* GetChildObject(UINT AffinityIndex);

private:
    UINT mNumDescriptors;
    ID3D12DescriptorHeap* mDescriptorHeaps[D3DX12_MAX_ACTIVE_NODES];
    UINT64* mCPUHeapStart;
    UINT64* mGPUHeapStart;
};
