//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// Copyright (c) 2026 Microsoft Corporation. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "DXSampleHelper.h"
#include "DeviceResources.h"
#include <stdexcept>

using Microsoft::WRL::ComPtr;

DeviceResources::~DeviceResources()
{
    WaitForGpuNoThrow();
    ReleaseDeviceResources();
}

void DeviceResources::ReleaseDeviceResources()
{
    m_backBuffers = {};
    m_rtvHeap.Reset();
    m_swapChain.Reset();
    m_dxr2CommandList.Reset();
    m_commandList.Reset();
    m_allocator.Reset();
    m_queue.Reset();
    m_fence.Reset();
    m_infoQueue.Reset();
    m_dxr2Device.Reset();
    m_device.Reset();
    m_adapter.Reset();
    m_factory.Reset();
    if (m_fenceEvent)
        CloseHandle(m_fenceEvent);
    m_fenceEvent = nullptr;
    m_fenceValue = 0;
    m_allowTearing = false;
}

bool DeviceResources::IsDeviceLost(HRESULT result) const
{
    return result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET ||
           (m_device && FAILED(m_device->GetDeviceRemovedReason()));
}

void DeviceResources::HandleDeviceLost()
{
    // Notify before releasing the device: the sample must release every old
    // resource, descriptor, state object, and interface alias first.
    if (m_deviceNotify)
        m_deviceNotify->OnDeviceLost();
    ReleaseDeviceResources();
    CreateDevice(m_debug, m_requestedAdapter);
    CreateSwapChain(m_window, m_width, m_height);
    if (m_deviceNotify)
        m_deviceNotify->OnDeviceRestored();
}

void DeviceResources::CreateDevice(bool debug, UINT requestedAdapter)
{
    m_debug = debug;
    m_requestedAdapter = requestedAdapter;
    const UUID previewFeatures[] = {D3D12ExperimentalShaderModels, D3D12RaytracingPreview};
    ThrowIfFailed(D3D12EnableExperimentalFeatures(_countof(previewFeatures), previewFeatures, nullptr, nullptr),
                  "D3D12EnableExperimentalFeatures");
    if (debug)
    {
        ComPtr<ID3D12Debug> controller;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&controller)), "D3D12GetDebugInterface");
        controller->EnableDebugLayer();
    }
    ThrowIfFailed(CreateDXGIFactory2(debug ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&m_factory)),
                  "CreateDXGIFactory2");

    // Query each adapter before selecting it; the first hardware adapter may
    // have no PTLAS support on a machine with multiple GPUs.
    for (UINT index = 0;; ++index)
    {
        ComPtr<IDXGIAdapter1> candidate;
        const HRESULT enumerated = m_factory->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                                         IID_PPV_ARGS(&candidate));
        if (enumerated == DXGI_ERROR_NOT_FOUND)
            break;
        ThrowIfFailed(enumerated, "EnumAdapterByGpuPreference");
        if (requestedAdapter != UINT_MAX && index != requestedAdapter)
            continue;
        DXGI_ADAPTER_DESC1 description{};
        ThrowIfFailed(candidate->GetDesc1(&description), "GetDesc1");
        if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        ComPtr<ID3D12Device5> device;
        if (FAILED(D3D12CreateDevice(candidate.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
            continue;
        ComPtr<ID3D12Device16> capabilities;
        if (FAILED(device.As(&capabilities)))
            continue;
        D3D12_FEATURE_DATA_D3D12_OPTIONS_RAYTRACING2 support{};
        if (FAILED(capabilities->CheckFeatureSupport1(D3D12_FEATURE_D3D12_OPTIONS_RAYTRACING2, &support,
                                                      sizeof(support), D3D12_PREVIEW_SDK_VERSION)) ||
            !support.ClustersAndPTLASSupported)
            continue;
        m_adapter = candidate;
        m_device = device;
        m_selectedAdapterIndex = index;
        m_adapterDescription = description.Description;
        break;
    }
    if (!m_device)
        throw std::runtime_error("No compatible DXR 2 PTLAS adapter was found");
    ThrowIfFailed(m_device.As(&m_dxr2Device), "QueryInterface(ID3D12DeviceRaytracing2)");
    if (debug)
    {
        ThrowIfFailed(m_device.As(&m_infoQueue), "QueryInterface(ID3D12InfoQueue)");
        if (IsDebuggerPresent())
        {
            m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        }
    }

    BOOL supported = FALSE;
    if (SUCCEEDED(m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
        m_allowTearing = supported != FALSE;

    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue)), "CreateCommandQueue");
    ThrowIfFailed(m_queue->SetName(L"DXR 2 direct queue"), "Name command queue");
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator)),
                  "CreateCommandAllocator");
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.Get(), nullptr,
                                              IID_PPV_ARGS(&m_commandList)),
                  "CreateCommandList");
    ThrowIfFailed(m_commandList->SetName(L"DXR 2 frame commands"), "Name command list");
    ThrowIfFailed(m_commandList.As(&m_dxr2CommandList), "QueryInterface(ID3D12CommandListRaytracing2)");
    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "CreateFence");
    ThrowIfFailed(m_fence->SetName(L"DXR 2 frame fence"), "Name frame fence");
    m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent)
        throw std::runtime_error("CreateEventW failed");
}

void DeviceResources::CreateSwapChain(HWND window, UINT width, UINT height)
{
    m_window = window;
    m_width = width;
    m_height = height;
    DXGI_SWAP_CHAIN_DESC1 description{};
    description.Width = width;
    description.Height = height;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.BufferCount = kFrameCount;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    description.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_factory->CreateSwapChainForHwnd(m_queue.Get(), window, &description, nullptr, nullptr, &swapChain),
                  "CreateSwapChainForHwnd");
    ThrowIfFailed(swapChain.As(&m_swapChain), "QueryInterface(IDXGISwapChain3)");
    // Alt+Enter is handled as borderless fullscreen by Win32Application.
    // DXGI's default exclusive fullscreen is incompatible with tearing presents.
    ThrowIfFailed(m_factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation");
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescription{};
    rtvDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDescription.NumDescriptors = kFrameCount;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvDescription, IID_PPV_ARGS(&m_rtvHeap)),
                  "CreateDescriptorHeap(RTV)");
    CreateBackBufferViews();
}

void DeviceResources::ResizeSwapChain(UINT width, UINT height)
{
    m_width = width;
    m_height = height;
    WaitForGpu();
    for (auto& buffer : m_backBuffers)
        buffer.Reset();
    ThrowIfFailed(m_swapChain->ResizeBuffers(kFrameCount, width, height, DXGI_FORMAT_UNKNOWN,
                                             m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0),
                  "ResizeBuffers");
    CreateBackBufferViews();
}

void DeviceResources::CreateBackBufferViews()
{
    const UINT stride = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (UINT index = 0; index < kFrameCount; ++index)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(index, IID_PPV_ARGS(&m_backBuffers[index])), "GetBuffer");
        wchar_t name[48];
        swprintf_s(name, L"Back buffer %u", index);
        ThrowIfFailed(m_backBuffers[index]->SetName(name), "Name back buffer");
        m_rtvHandles[index] = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        m_rtvHandles[index].ptr += static_cast<SIZE_T>(index) * stride;
        m_device->CreateRenderTargetView(m_backBuffers[index].Get(), nullptr, m_rtvHandles[index]);
    }
}

void DeviceResources::Present(bool vsync)
{
    const HRESULT result =
        m_swapChain->Present(vsync ? 1 : 0, !vsync && m_allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
    if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
        ThrowIfFailed(m_device->GetDeviceRemovedReason(), "Device removed during Present");
    ThrowIfFailed(result, "Present");
}

UINT64 DeviceResources::Signal()
{
    const UINT64 value = ++m_fenceValue;
    ThrowIfFailed(m_queue->Signal(m_fence.Get(), value), "Signal(fence)");
    return value;
}

void DeviceResources::WaitForFence(UINT64 value)
{
    // A removed device reports UINT64_MAX as the completed value. Do not treat
    // that as successful completion and continue mapping its readback buffers.
    ThrowIfFailed(m_device->GetDeviceRemovedReason(), "Device removed while waiting for GPU");
    if (m_fence->GetCompletedValue() < value)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(value, m_fenceEvent), "SetEventOnCompletion");
        if (WaitForSingleObject(m_fenceEvent, INFINITE) != WAIT_OBJECT_0)
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "WaitForSingleObject(fence)");
    }
    ThrowIfFailed(m_device->GetDeviceRemovedReason(), "Device removed while waiting for GPU");
}

void DeviceResources::WaitForGpu()
{
    WaitForFence(Signal());
}

void DeviceResources::WaitForGpuNoThrow() noexcept
{
    if (!m_queue || !m_fence || !m_fenceEvent)
        return;
    try
    {
        WaitForGpu();
    }
    catch (const std::exception&)
    {
        // Preserve the original exception during cleanup of a removed device.
    }
}

void DeviceResources::ExecuteAndWait()
{
    ThrowIfFailed(m_commandList->Close(), "Close(initialization commands)");
    ID3D12CommandList* lists[] = {m_commandList.Get()};
    m_queue->ExecuteCommandLists(1, lists);
    WaitForGpu();
    BeginInitialization();
}

void DeviceResources::BeginInitialization()
{
    ThrowIfFailed(m_allocator->Reset(), "Reset(initialization allocator)");
    ThrowIfFailed(m_commandList->Reset(m_allocator.Get(), nullptr), "Reset(initialization commands)");
}
