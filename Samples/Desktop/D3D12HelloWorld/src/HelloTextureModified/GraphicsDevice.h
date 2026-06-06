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

#include "DXSampleHelper.h"

using Microsoft::WRL::ComPtr;

struct GraphicsDeviceDesc
{
    HWND hwnd = nullptr;
    UINT width = 0;
    UINT height = 0;
    UINT bufferCount = 0;
    DXGI_FORMAT swapChainFormat = DXGI_FORMAT_UNKNOWN;
    bool useWarpDevice = false;
};

struct GraphicsDevice
{
    void Initialize(const GraphicsDeviceDesc& desc);
    HWND Hwnd() const;
    UINT Width() const;
    UINT Height() const;
    ID3D12Device* Device() const;
    IDXGIFactory4* DxgiFactory() const;
    IDXGISwapChain3* SwapChain() const;
    ID3D12CommandQueue* CommandQueue() const;
    void RefreshDxgiFactoryIfNeeded();
    bool HasSwapChain() const;
    UINT CurrentBackBufferIndex() const;
    void GetBackBuffer(UINT index, REFIID riid, void** resource) const;
    void ExecuteCommandLists(UINT commandListCount, ID3D12CommandList* const* commandLists);
    void CreateFence(UINT64 initialValue);
    void SignalFence(UINT64 value);
    UINT64 CompletedFenceValue() const;
    void WaitForFenceValue(UINT64 value);
    void CloseFenceEvent();
    void Present(UINT syncInterval, UINT flags);
    void ResizeSwapChain(UINT bufferCount, UINT newWidth, UINT newHeight, DXGI_FORMAT format, UINT flags);

private:
    void SetWindowHandle(HWND newHwnd);
    void SetSize(UINT newWidth, UINT newHeight);

    HWND m_hwnd = nullptr;
    UINT m_width = 0;
    UINT m_height = 0;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGIFactory4> m_dxgiFactory;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
};
