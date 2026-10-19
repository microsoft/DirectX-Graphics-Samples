//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <array>
#include <string>

inline constexpr UINT kFrameCount = 2;

// Same notification contract as the Microsoft DXR samples.
struct IDeviceNotify
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;

protected:
    ~IDeviceNotify() = default;
};

// Device, presentation, and queue lifetime. PTLAS resources stay in the sample.
class DeviceResources
{
public:
    ~DeviceResources();
    void CreateDevice(bool debug, UINT requestedAdapter);
    void CreateSwapChain(HWND window, UINT width, UINT height);
    void ResizeSwapChain(UINT width, UINT height);
    void ExecuteAndWait();
    void BeginInitialization();
    void Present(bool vsync);
    UINT64 Signal();
    void WaitForFence(UINT64 value);
    void WaitForGpu();
    void WaitForGpuNoThrow() noexcept;
    void RegisterDeviceNotify(IDeviceNotify* notify) { m_deviceNotify = notify; }
    void HandleDeviceLost();
    bool IsDeviceLost(HRESULT result) const;

    ID3D12Device5* GetDevice() const { return m_device.Get(); }
    ID3D12DeviceRaytracing2* GetRaytracingDevice() const { return m_dxr2Device.Get(); }
    ID3D12GraphicsCommandList4* GetCommandList() const { return m_commandList.Get(); }
    ID3D12CommandListRaytracing2* GetRaytracingCommandList() const { return m_dxr2CommandList.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return m_allocator.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_queue.Get(); }
    ID3D12InfoQueue* GetInfoQueue() const { return m_infoQueue.Get(); }
    ID3D12Resource* GetRenderTarget() const { return m_backBuffers[m_swapChain->GetCurrentBackBufferIndex()].Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
    {
        return m_rtvHandles[m_swapChain->GetCurrentBackBufferIndex()];
    }
    UINT GetAdapterIndex() const { return m_selectedAdapterIndex; }
    const std::wstring& GetAdapterDescription() const { return m_adapterDescription; }

private:
    void CreateBackBufferViews();
    void ReleaseDeviceResources();
    IDeviceNotify* m_deviceNotify{};
    HWND m_window{};
    UINT m_width{};
    UINT m_height{};
    bool m_debug{};
    UINT m_requestedAdapter{UINT_MAX};

    Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
    Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
    Microsoft::WRL::ComPtr<ID3D12DeviceRaytracing2> m_dxr2Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandListRaytracing2> m_dxr2CommandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_infoQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kFrameCount> m_backBuffers;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kFrameCount> m_rtvHandles{};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    HANDLE m_fenceEvent{};
    UINT64 m_fenceValue{};
    bool m_allowTearing{};
    UINT m_selectedAdapterIndex{UINT_MAX};
    std::wstring m_adapterDescription;
};
