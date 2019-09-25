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

class GpuResource
{
public:
    enum RWFlags {
        None = 0x0,
        AllowRead = 0x1,
        AllowWrite = 0x2,
    };

    UINT rwFlags = RWFlags::AllowRead | RWFlags::AllowWrite;
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorReadAccess = { UINT64_MAX };
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorWriteAccess = { UINT64_MAX };
    UINT srvDescriptorHeapIndex = UINT_MAX;
    UINT uavDescriptorHeapIndex = UINT_MAX;
    D3D12_RESOURCE_STATES m_UsageState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES m_TransitioningState = (D3D12_RESOURCE_STATES)-1;

    ID3D12Resource* GetResource() { return resource.Get(); }
}; 
