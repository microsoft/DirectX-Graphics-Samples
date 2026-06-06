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

#include "stdafx.h"

#include "GraphicsDevice.h"

static void GetHardwareAdapter(IDXGIFactory1* pFactory,
                               IDXGIAdapter1** ppAdapter,
                               bool requestHighPerformanceAdapter = false)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (UINT adapterIndex = 0;
             SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                 adapterIndex,
                 requestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                               : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                 IID_PPV_ARGS(&adapter)));
             ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

HWND GraphicsDevice::Hwnd() const
{
    return hwnd;
}

UINT GraphicsDevice::Width() const
{
    return width;
}

UINT GraphicsDevice::Height() const
{
    return height;
}

ID3D12Device* GraphicsDevice::Device() const
{
    return device.Get();
}

IDXGIFactory4* GraphicsDevice::DxgiFactory() const
{
    return dxgiFactory.Get();
}

IDXGISwapChain3* GraphicsDevice::SwapChain() const
{
    return swapChain.Get();
}

ID3D12CommandQueue* GraphicsDevice::CommandQueue() const
{
    return commandQueue.Get();
}

void GraphicsDevice::RefreshDxgiFactoryIfNeeded()
{
    if (dxgiFactory != nullptr && dxgiFactory->IsCurrent())
    {
        return;
    }

    UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
}

void GraphicsDevice::SetWindowHandle(HWND newHwnd)
{
    hwnd = newHwnd;
}

void GraphicsDevice::SetSize(UINT newWidth, UINT newHeight)
{
    width = newWidth;
    height = newHeight;
}

void GraphicsDevice::Initialize(const GraphicsDeviceDesc& desc)
{
    SetWindowHandle(desc.hwnd);
    SetSize(desc.width, desc.height);

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    if (desc.useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(dxgiFactory.Get(), &hardwareAdapter);
        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = desc.bufferCount;
    swapChainDesc.Width = Width();
    swapChainDesc.Height = Height();
    swapChainDesc.Format = desc.swapChainFormat;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    ThrowIfFailed(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&swapChain));
}

bool GraphicsDevice::HasSwapChain() const
{
    return device.Get() != nullptr && swapChain.Get() != nullptr;
}

UINT GraphicsDevice::CurrentBackBufferIndex() const
{
    return swapChain->GetCurrentBackBufferIndex();
}

void GraphicsDevice::GetBackBuffer(UINT index, REFIID riid, void** resource) const
{
    ThrowIfFailed(swapChain->GetBuffer(index, riid, resource));
}

void GraphicsDevice::ExecuteCommandLists(UINT commandListCount, ID3D12CommandList* const* commandLists)
{
    commandQueue->ExecuteCommandLists(commandListCount, commandLists);
}

void GraphicsDevice::CreateFence(UINT64 initialValue)
{
    ThrowIfFailed(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void GraphicsDevice::SignalFence(UINT64 value)
{
    ThrowIfFailed(commandQueue->Signal(fence.Get(), value));
}

UINT64 GraphicsDevice::CompletedFenceValue() const
{
    return fence->GetCompletedValue();
}

void GraphicsDevice::WaitForFenceValue(UINT64 value)
{
    ThrowIfFailed(fence->SetEventOnCompletion(value, fenceEvent));
    WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
}

void GraphicsDevice::CloseFenceEvent()
{
    if (fenceEvent != nullptr)
    {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
}

void GraphicsDevice::Present(UINT syncInterval, UINT flags)
{
    ThrowIfFailed(swapChain->Present(syncInterval, flags));
}

void GraphicsDevice::ResizeSwapChain(UINT bufferCount, UINT newWidth, UINT newHeight, DXGI_FORMAT format, UINT flags)
{
    ThrowIfFailed(swapChain->ResizeBuffers(bufferCount, newWidth, newHeight, format, flags));
    SetSize(newWidth, newHeight);
}
