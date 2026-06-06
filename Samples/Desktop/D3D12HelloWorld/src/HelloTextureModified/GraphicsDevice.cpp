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
    return m_hwnd;
}

UINT GraphicsDevice::Width() const
{
    return m_width;
}

UINT GraphicsDevice::Height() const
{
    return m_height;
}

ID3D12Device* GraphicsDevice::Device() const
{
    return m_device.Get();
}

IDXGIFactory4* GraphicsDevice::DxgiFactory() const
{
    return m_dxgiFactory.Get();
}

IDXGISwapChain3* GraphicsDevice::SwapChain() const
{
    return m_swapChain.Get();
}

ID3D12CommandQueue* GraphicsDevice::CommandQueue() const
{
    return m_commandQueue.Get();
}

void GraphicsDevice::RefreshDxgiFactoryIfNeeded()
{
    if (m_dxgiFactory != nullptr && m_dxgiFactory->IsCurrent())
    {
        return;
    }

    UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));
}

void GraphicsDevice::SetWindowHandle(HWND newHwnd)
{
    m_hwnd = newHwnd;
}

void GraphicsDevice::SetSize(UINT newWidth, UINT newHeight)
{
    m_width = newWidth;
    m_height = newHeight;
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

    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

    if (desc.useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);
        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = desc.bufferCount;
    swapChainDesc.Width = Width();
    swapChainDesc.Height = Height();
    swapChainDesc.Format = desc.swapChainFormat;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        m_hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&m_swapChain));
}

bool GraphicsDevice::HasSwapChain() const
{
    return m_device.Get() != nullptr && m_swapChain.Get() != nullptr;
}

UINT GraphicsDevice::CurrentBackBufferIndex() const
{
    return m_swapChain->GetCurrentBackBufferIndex();
}

void GraphicsDevice::GetBackBuffer(UINT index, REFIID riid, void** resource) const
{
    ThrowIfFailed(m_swapChain->GetBuffer(index, riid, resource));
}

void GraphicsDevice::ExecuteCommandLists(UINT commandListCount, ID3D12CommandList* const* commandLists)
{
    m_commandQueue->ExecuteCommandLists(commandListCount, commandLists);
}

void GraphicsDevice::CreateFence(UINT64 initialValue)
{
    ThrowIfFailed(m_device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void GraphicsDevice::SignalFence(UINT64 value)
{
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), value));
}

UINT64 GraphicsDevice::CompletedFenceValue() const
{
    return m_fence->GetCompletedValue();
}

void GraphicsDevice::WaitForFenceValue(UINT64 value)
{
    ThrowIfFailed(m_fence->SetEventOnCompletion(value, m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
}

void GraphicsDevice::CloseFenceEvent()
{
    if (m_fenceEvent != nullptr)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
}

void GraphicsDevice::Present(UINT syncInterval, UINT flags)
{
    ThrowIfFailed(m_swapChain->Present(syncInterval, flags));
}

void GraphicsDevice::ResizeSwapChain(UINT bufferCount, UINT newWidth, UINT newHeight, DXGI_FORMAT format, UINT flags)
{
    ThrowIfFailed(m_swapChain->ResizeBuffers(bufferCount, newWidth, newHeight, format, flags));
    SetSize(newWidth, newHeight);
}
