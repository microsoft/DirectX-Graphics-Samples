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

//
// DeviceResources.h - A wrapper for the Direct3D 12 device and swapchain
//

#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <deque>
#include <exception>

namespace DX
{
// Provides an interface for an application that owns DeviceResources to be notified of the device
// being lost or created.
interface IDeviceNotify
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;
    };

    // Controls all the DirectX device resources.
    class DeviceResources
    {
        DeviceResources() {}
    public:
        static const unsigned int c_AllowTearing = 0x1;
        static const unsigned int c_RequireTearingSupport = 0x2;

        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            UINT backBufferCount = 2,
            D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
            UINT flags = 0,
            UINT adapterIDoverride = UINT_MAX);
        ~DeviceResources();

        void InitializeDXGIAdapter();
        void SetAdapterOverride(UINT adapterID) { m_adapterIDoverride = adapterID; }
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(HWND window, int width, int height);
        bool WindowSizeChanged(int width, int height, bool minimized);
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify)
        {
            m_deviceNotify = deviceNotify;

            // On RS4 and higher, applications that handle device removal
            // should declare themselves as being able to do so
            __if_exists(DXGIDeclareAdapterRemovalSupport)
            {
                if (deviceNotify)
                {
                    if (FAILED(DXGIDeclareAdapterRemovalSupport()))
                    {
                        OutputDebugString(
                            L"Warning: application failed to declare adapter removal support.\n");
                    }
                }
            }
        }

        void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT);
        void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void SetVSyncEnabled(bool enabled) { m_vsyncEnabled = enabled; }
        bool IsVSyncEnabled() const { return m_vsyncEnabled; }
        void SetStablePowerStateRequested(bool enabled) { m_stablePowerStateRequested = enabled; }
        bool IsStablePowerStateEnabled() const { return m_stablePowerStateEnabled; }
        HRESULT ReleaseStablePowerState() noexcept;
        void ExecuteCommandList();
        void WaitForGpu() noexcept;

        // -----------------------------------------------------------------
        // Async display mode (for slow software rasterizers — primarily WARP).
        //
        // The worker owns rendering, presentation, queued keyboard actions, and
        // resize callbacks. WM_PAINT only validates the window while this mode
        // is active, keeping the UI responsive during slow software frames.
        // Run checks worker failures on the UI thread and joins it before teardown.
        // -----------------------------------------------------------------
        using AsyncRenderCallback = std::function<void()>;
        using AsyncResizeCallback = std::function<void(UINT width, UINT height, bool minimized)>;
        void EnableAsyncDisplay(AsyncRenderCallback render,
                                AsyncResizeCallback resize);
        void DisableAsyncDisplay();
        void RethrowAsyncFailure() const;
        bool IsAsyncDisplayActive() const { return m_asyncDisplay; }
        // Called from UI thread WM_SIZE in async mode.  Stashes the new
        // client size; the worker thread picks it up before its next
        // render iteration and runs the resize callback on its own thread.
        // Returns immediately -- UI thread never blocks.
        void RequestAsyncResize(UINT width, UINT height, bool minimized);

        // Device Accessors.
        RECT GetOutputSize() const { return m_outputSize; }
        bool IsWindowVisible() const { return m_isWindowVisible; }
        bool IsTearingSupported() const { return m_options & c_AllowTearing; }

        // Direct3D Accessors.
        IDXGIAdapter1*              GetAdapter() const { return m_adapter.Get(); }
        ID3D12Device*               GetD3DDevice() const { return m_d3dDevice.Get(); }
        IDXGIFactory4*              GetDXGIFactory() const { return m_dxgiFactory.Get(); }
        IDXGISwapChain3*            GetSwapChain() const { return m_swapChain.Get(); }
        D3D_FEATURE_LEVEL           GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
        ID3D12Resource* GetRenderTarget() const
        {
            return m_renderTargets[m_backBufferIndex].Get();
        }
        ID3D12Resource*             GetDepthStencil() const { return m_depthStencil.Get(); }
        ID3D12CommandQueue*         GetCommandQueue() const { return m_commandQueue.Get(); }
        ID3D12CommandAllocator* GetCommandAllocator() const
        {
            return m_commandAllocators[m_backBufferIndex].Get();
        }
        ID3D12GraphicsCommandList*  GetCommandList() const { return m_commandList.Get(); }
        DXGI_FORMAT                 GetBackBufferFormat() const { return m_backBufferFormat; }
        DXGI_FORMAT                 GetDepthBufferFormat() const { return m_depthBufferFormat; }
        D3D12_VIEWPORT              GetScreenViewport() const { return m_screenViewport; }
        D3D12_RECT                  GetScissorRect() const { return m_scissorRect; }
        UINT                        GetCurrentFrameIndex() const { return m_backBufferIndex; }
        UINT GetPreviousFrameIndex() const
        {
            return m_backBufferIndex == 0 ? m_backBufferCount - 1 : m_backBufferIndex - 1;
        }
        UINT                        GetBackBufferCount() const { return m_backBufferCount; }
        unsigned int                GetDeviceOptions() const { return m_options; }
        LPCWSTR GetAdapterDescription() const
        {
            return m_adapterDescription.c_str();
        }
        UINT                        GetAdapterID() const { return m_adapterID; }
        // Adapter / driver identity captured during InitializeAdapter.  Used
        // by the benchmark JSON dump so swept results can be tagged with the
        // GPU + driver they ran on (essential for cross-machine comparisons).
        // Driver version comes from IDXGIAdapter::CheckInterfaceSupport's
        // UMDVersion (Windows User-Mode Driver), formatted as A.B.C.D.  Blank
        // string if the query failed (e.g. WARP).
        LPCWSTR GetDriverVersionString() const
        {
            return m_driverVersion.c_str();
        }
        UINT                        GetAdapterVendorId()   const { return m_adapterVendorId; }
        UINT                        GetAdapterDeviceId()   const { return m_adapterDeviceId; }
        UINT64 GetDedicatedVideoMemoryBytes() const
        {
            return m_adapterDedicatedVideoMemory;
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex,
                m_rtvDescriptorSize);
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

    private:
        void MoveToNextFrame();
        void InitializeAdapter(IDXGIAdapter1** ppAdapter);
        void AsyncWorkerThreadProc();

        const static size_t MAX_BACK_BUFFER_COUNT = 3;

        UINT                                                m_adapterIDoverride;
        UINT                                                m_backBufferIndex;
        ComPtr<IDXGIAdapter1>                               m_adapter;
        UINT                                                m_adapterID;
        std::wstring                                        m_adapterDescription;
        // Adapter / driver identity -- captured during InitializeAdapter
        // (one-shot) and surfaced via Get* accessors for the benchmark
        // JSON dump.  See header comment on GetDriverVersionString().
        std::wstring                                        m_driverVersion;
        UINT                                                m_adapterVendorId            = 0;
        UINT                                                m_adapterDeviceId            = 0;
        UINT64                                              m_adapterDedicatedVideoMemory = 0;

        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D12Device>                m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>          m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   m_commandList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[MAX_BACK_BUFFER_COUNT];

        // Swap chain objects.
        Microsoft::WRL::ComPtr<IDXGIFactory4>               m_dxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain3>             m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource>              m_renderTargets[MAX_BACK_BUFFER_COUNT];
        Microsoft::WRL::ComPtr<ID3D12Resource>              m_depthStencil;

        // Presentation fence objects.
        Microsoft::WRL::ComPtr<ID3D12Fence>                 m_fence;
        UINT64                                              m_fenceValues[MAX_BACK_BUFFER_COUNT];
        Microsoft::WRL::Wrappers::Event                     m_fenceEvent;

        // Direct3D rendering objects.
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;
        UINT                                                m_rtvDescriptorSize;
        D3D12_VIEWPORT                                      m_screenViewport;
        D3D12_RECT                                          m_scissorRect;

        // Direct3D properties.
        DXGI_FORMAT                                         m_backBufferFormat;
        DXGI_FORMAT                                         m_depthBufferFormat;
        UINT                                                m_backBufferCount;
        D3D_FEATURE_LEVEL                                   m_d3dMinFeatureLevel;

        // Cached device properties.
        HWND                                                m_window;
        D3D_FEATURE_LEVEL                                   m_d3dFeatureLevel;
        RECT                                                m_outputSize;
        bool                                                m_isWindowVisible;

        // DeviceResources options (see flags above)
        unsigned int                                        m_options;
        bool                                                m_vsyncEnabled = false;
        bool                                                m_stablePowerStateRequested = false;
        bool                                                m_stablePowerStateEnabled = false;

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        IDeviceNotify*                                      m_deviceNotify;

        // === Async display state (see EnableAsyncDisplay above) ===========
        bool                                                m_asyncDisplay = false;
        AsyncRenderCallback                                 m_asyncCallback;
        AsyncResizeCallback                                 m_asyncResizeCallback;
        std::thread                                         m_asyncWorkerThread;
        std::atomic<bool>                                   m_asyncWorkerStop{false};
        std::exception_ptr                                  m_asyncFailure;
        std::atomic<bool>                                   m_asyncWorkerFailed{false};
        // Pending-resize state, written by UI thread on WM_SIZE, read by
        // worker thread between iterations.  Mutex-protected because the
        // members can't be atomically updated as a group.
        std::mutex                                          m_asyncResizeMutex;
        bool                                                m_asyncResizePending = false;
        UINT                                                m_asyncResizeWidth = 0;
        UINT                                                m_asyncResizeHeight = 0;
        bool                                                m_asyncResizeMinimized = false;

        // UI input queues actions for the worker to drain between frames.
        // Preserve input order rather than coalescing toggles. This keeps GPU
        // resource mutation on one thread without blocking the UI on a frame lock.
        std::mutex                                          m_asyncActionsMutex;
        std::deque<std::function<void()>>                   m_pendingAsyncActions;
    public:
        // True if EnableAsyncDisplay() was called -- used by Win32Application
        // to decide whether to dispatch WM_KEYDOWN via EnqueueAsyncAction
        // (worker thread) or call OnKeyDown directly (UI thread).
        bool IsAsyncDisplay() const { return m_asyncDisplay; }

        // Queue a closure for the async worker to run at the top of its
        // next iteration, before rendering.  Caller must ensure `fn` only
        // touches state that's safe to mutate on the worker thread (which
        // is essentially everything in this sample, since the sample's
        // own per-frame work also runs there in async mode).  No-op if
        // not in async mode (caller should check IsAsyncDisplay first;
        // we don't silently fall back to "run on calling thread" because
        // that would be a footgun).
        void EnqueueAsyncAction(std::function<void()> fn)
        {
            if (!m_asyncDisplay) return;
            std::lock_guard<std::mutex> lk(m_asyncActionsMutex);
            m_pendingAsyncActions.push_back(std::move(fn));
        }
    };
}
