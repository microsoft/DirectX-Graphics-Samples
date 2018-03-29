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
class D3D12Context
{
public:
    enum class CreationFlags
    {
        None = 0x0,
        ForceHardware = 0x1,
    };

    D3D12Context(CreationFlags flags = CreationFlags::None);
    ~D3D12Context();
    ID3D12Device &GetDevice() { return *m_pDevice; }
    ID3D12CommandQueue &GetQueue() { return *m_pCommandQueue; }
    void GetGraphicsCommandList(ID3D12GraphicsCommandList **pCommandList);
    void ExecuteCommandList(ID3D12GraphicsCommandList *pGraphicsCommandList);
    void WaitForGpuWork();

    void CreateResourceWithInitialData(const void *pInputData, UINT64 dataSize, ID3D12Resource **ppResource);
    void ReadbackResource(ID3D12Resource *pResource, void *pOutputData, UINT dataSize);
    void ReadbackTexture(ID3D12Resource *pResource, std::vector<byte> &data, UINT64 &pitch);

    UINT GetTotalLaneCount();
private:
    CComPtr<ID3D12Device> m_pDevice;
    CComPtr<ID3D12CommandQueue> m_pCommandQueue;
    CComPtr<ID3D12CommandAllocator> m_pAllocator;
    
    CComPtr<ID3D12Fence> m_pFence;
    UINT m_lastSignaledValue;

    std::vector<CComPtr<ID3D12GraphicsCommandList>> m_commandListsToDelete;
};
