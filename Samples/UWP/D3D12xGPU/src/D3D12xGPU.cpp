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
#include "D3D12xGPU.h"
#include "UILayer.h"
#include <dxgidebug.h>
#include "ShadowsFogScatteringSquidScene.h"

using namespace std;

D3D12xGPU* D3D12xGPU::s_app = nullptr;

D3D12xGPU::D3D12xGPU(UINT width, UINT height, wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_activeAdapter(0),
    m_bCtrlKeyIsPressed(false),
    m_activeGpuPreference(DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE),
    m_fps(0.0f),
    m_manualAdapterSelection(false),
    m_adapterChangeEvent(NULL),
    m_adapterChangeRegistrationCookie(0),
    m_fenceValues{},
    m_windowVisible(true),
    m_dxgiFactoryFlags(0),
    m_windowedMode(true)
{
#ifdef PIXSUPPORT
    m_enableUI = false;
#endif
#ifndef PIXSUPPORT
    ThrowIfFailed(DXGIDeclareAdapterRemovalSupport());
#endif
    if (!m_tearingSupport)
    {
        // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since Threshold II.
        // Since the sample demonstrates APIs available in Redstone 4, we don't need to handle non-tearing cases.
        // Tearing being supported or not matters in how adapter switching is handled in fullscreen presentations.
        OutputDebugString(L"Sample must be run on an OS with tearing support.");
        exit(EXIT_FAILURE);
    }

    if (m_useWarpDevice)
    {
        OutputDebugString(L"Sample doesn't support warp parameter.");
        exit(EXIT_FAILURE);
    }

    s_app = this;

    m_gpuPreferenceToName[DXGI_GPU_PREFERENCE_UNSPECIFIED] = L"Unspecified";
    m_gpuPreferenceToName[DXGI_GPU_PREFERENCE_MINIMUM_POWER] = L"Minimum power";
    m_gpuPreferenceToName[DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE] = L"High performance";
}

D3D12xGPU::~D3D12xGPU()
{
}

void D3D12xGPU::OnInit()
{
    LoadPipeline();
    LoadAssets();
    LoadSizeDependentResources();
}

// Load the rendering pipeline dependencies.
void D3D12xGPU::LoadPipeline()
{
    m_dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif
    ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

#ifdef USE_DXGI_1_6
    ComPtr<IDXGIFactory7> spDxgiFactory7;
    if (SUCCEEDED(m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&spDxgiFactory7))))
    {
        m_adapterChangeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_adapterChangeEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
        ThrowIfFailed(spDxgiFactory7->RegisterAdaptersChangedEvent(m_adapterChangeEvent, &m_adapterChangeRegistrationCookie));
    }
#endif

    EnumerateGPUadapters();

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetGPUAdapter(m_activeAdapter, &hardwareAdapter);
    ThrowIfFailed(D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    ));
    m_activeAdapterLuid = m_gpuAdapterDescs[m_activeAdapter].desc.AdapterLuid;

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    NAME_D3D12_OBJECT(m_commandQueue);

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    // It is recommended to always use the tearing flag when it is available.
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_dxgiFactory->CreateSwapChainForCoreWindow(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
        &swapChainDesc,
        nullptr,
        &swapChain
    ));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create synchronization objects.
    {
        ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_frameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

// Load assets required for rendering.
void D3D12xGPU::LoadAssets()
{
    if (!m_scene)
    {
        m_scene = make_unique<ShadowsFogScatteringSquidScene>(FrameCount, this);
    }

    // Create a temporary command queue and command list for initializing data on the GPU.
    // Performance tip: Copy command queues are optimized for transfer over PCIe.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

    ComPtr<ID3D12CommandQueue> copyCommandQueue;
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&copyCommandQueue)));
    NAME_D3D12_OBJECT(copyCommandQueue);

    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(m_device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&commandAllocator)));
    NAME_D3D12_OBJECT(commandAllocator);

    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(m_device->CreateCommandList(0, queueDesc.Type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
    NAME_D3D12_OBJECT(commandList);

    m_scene->Initialize(m_device.Get(), m_commandQueue.Get(), commandList.Get(), m_frameIndex);

    ThrowIfFailed(commandList->Close());

    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    copyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Wait until assets have been uploaded to the GPU.
    WaitForGpu(copyCommandQueue.Get());
}

// Load resources that are dependent on the size of the main window.
void D3D12xGPU::LoadSizeDependentResources()
{
    for (UINT i = 0; i < FrameCount; i++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
    }

    m_scene->LoadSizeDependentResources(m_device.Get(), m_renderTargets, m_width, m_height);

    if (m_enableUI)
    {
        if (!m_uiLayer)
        {
            m_uiLayer = make_unique<UILayer>(FrameCount, m_device.Get(), m_commandQueue.Get());
        }
        m_uiLayer->Resize(m_renderTargets, m_width, m_height);
    }
}

// Release resources that are dependent on the size of the main window.
void D3D12xGPU::ReleaseSizeDependentResources()
{
    m_scene->ReleaseSizeDependentResources();
    if (m_enableUI)
    {
        m_uiLayer.reset();
    }
    for (UINT i = 0; i < FrameCount; i++)
    {
        m_renderTargets[i].Reset();
    }
}

void D3D12xGPU::UpdateUI()
{
    vector<wstring> labels;
    {
        wstringstream wLabel;
        wLabel.precision(1);
        wLabel << fixed << L"FPS: " << m_fps
            << L"\n";
        labels.push_back(wLabel.str());
    }
    labels.push_back(L"GPU preference sorting mode (press a CTRL + key to select):\n");
    for (auto &gpuPreferenceName : m_gpuPreferenceToName)
    {
        wstringstream wLabel;
        wLabel << L" " << to_wstring(gpuPreferenceName.first) << L": " << gpuPreferenceName.second
            << (gpuPreferenceName.first == m_activeGpuPreference ? L" [x]" : L"")
            << L"\n";
        labels.push_back(wLabel.str());
    }
    labels.push_back(L"\n");

    {
        wstringstream wLabel;
        wLabel << L"Adapter selection (press 'A' key to toggle): " << (m_manualAdapterSelection ? L"manual" : L"always use adapter 0") << L"\n\n";
        labels.push_back(wLabel.str());
    }

    {
        wstringstream wLabel;
        wLabel << L"Available GPU adapters sorted by preference mode" << (m_manualAdapterSelection ? L" (press a key to select):" : L":") << L"\n";
        for (UINT i = 0; i < m_gpuAdapterDescs.size(); i++)
        {
            bool supportsDx12FL11 = m_gpuAdapterDescs[i].supportsDx12FL11;
            const DXGI_ADAPTER_DESC1 &desc = m_gpuAdapterDescs[i].desc;
            wLabel << L" " << (supportsDx12FL11 ? to_wstring(i) : L"(non-compliant)") << L": " << desc.Description
                << (i == m_activeAdapter ? L" [x]" : L"")
                << L"\n";
        }
        labels.push_back(wLabel.str());
    }
    wstring uiText = L"";
    for (auto s : labels)
    {
        uiText += s;
    }
    m_uiLayer->UpdateLabels(uiText);
}

// Release sample's D3D objects.
void D3D12xGPU::ReleaseD3DObjects()
{
    m_scene->ReleaseD3DObjects();
    if (m_enableUI)
    {
        m_uiLayer.reset();
    }
    m_fence.Reset();

    ResetComPtrArray(&m_renderTargets);
    m_commandQueue.Reset();
    m_swapChain.Reset();
    m_device.Reset();

#ifdef USE_DXGI_1_6
    ComPtr<IDXGIFactory7> spDxgiFactory7;
    if (m_adapterChangeRegistrationCookie != 0 && SUCCEEDED(m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&spDxgiFactory7))))
    {
        ThrowIfFailed(spDxgiFactory7->UnregisterAdaptersChangedEvent(m_adapterChangeRegistrationCookie));
        m_adapterChangeRegistrationCookie = 0;
        CloseHandle(m_adapterChangeEvent);
        m_adapterChangeEvent = NULL;
    }
#endif
    m_dxgiFactory.Reset();

#if defined(_DEBUG)
    {
        ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
    }
#endif
}

void D3D12xGPU::OnKeyUp(UINT8 key)
{
    m_scene->KeyUp(key);
    switch (key)
    {
    case VK_CONTROL:
        m_bCtrlKeyIsPressed = false;
        break;
    default:
        break;
    }
}

void D3D12xGPU::OnKeyDown(UINT8 key)
{
    m_scene->KeyDown(key);
    switch (key)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        if (m_bCtrlKeyIsPressed)
        {
            SelectGPUPreference(key - '0');
        }
        else if (m_manualAdapterSelection)
        {
            SelectAdapter(key - '0');
        }
        break;
    case 'A':
        m_manualAdapterSelection = !m_manualAdapterSelection;
        if (!m_manualAdapterSelection && m_activeAdapter != 0)
        {
            SelectAdapter(0);
        }
        break;
    case VK_CONTROL:
        m_bCtrlKeyIsPressed = true;
        break;
    default:
        break;
    }
}

void D3D12xGPU::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    // Determine if the swap buffers and other resources need to be resized or not.
    if ((width != m_width || height != m_height) && !minimized)
    {
        UpdateForSizeChange(width, height);

        if (!m_swapChain)
        {
            return;
        }

        try
        {
            // Flush all current GPU commands.
            WaitForGpu(m_commandQueue.Get());

            // Release the resources holding references to the swap chain (requirement of
            // IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
            // current fence value.
            ReleaseSizeDependentResources();
            for (UINT i = 0; i < FrameCount; i++)
            {
                m_fenceValues[i] = m_fenceValues[m_frameIndex];
            }

            // Resize the swap chain to the desired dimensions.
            DXGI_SWAP_CHAIN_DESC1 desc = {};
            ThrowIfFailed(m_swapChain->GetDesc1(&desc));
            ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.Format, desc.Flags));

            BOOL fullscreenState;
            ThrowIfFailed(m_swapChain->GetFullscreenState(&fullscreenState, nullptr));
            m_windowedMode = !fullscreenState;

            // Reset the frame index to the current back buffer index.
            m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
            m_scene->SetFrameIndex(m_frameIndex);

            LoadSizeDependentResources();

            m_windowVisible = !minimized;
        }
        catch (HrException& e)
        {
            if ((e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET))
            {
                m_width = width;
                m_height = height;
                RecreateD3Dresources();
            }
            else
            {
                throw;
            }
        }
    }
}

void D3D12xGPU::OnUpdate()
{
    m_timer.Tick();

    CalculateFrameStats();
    m_scene->Update(m_timer.GetElapsedSeconds());

    if (m_enableUI)
    {
        UpdateUI();
    }
}

// Render the scene.
void D3D12xGPU::OnRender()
{
    if (m_windowVisible)
    {
        try
        {
            // Check for any adapter changes, such as a new adapter being available.
            if (QueryForAdapterEnumerationChanges())
            {
                // Dxgi factory needs to be recreated on a change.
                ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

#ifdef USE_DXGI_1_6
                ComPtr<IDXGIFactory7> spDxgiFactory7;
                if (SUCCEEDED(m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&spDxgiFactory7))))
                {
                    m_adapterChangeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                    if (m_adapterChangeEvent == nullptr)
                    {
                        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
                    }
                    ThrowIfFailed(spDxgiFactory7->RegisterAdaptersChangedEvent(m_adapterChangeEvent, &m_adapterChangeRegistrationCookie));
                }
#endif

                // Check if the application should switch to a different adapter.
                ThrowIfFailed(ValidateActiveAdapter());
            }

            // UILayer will transition backbuffer to a present state.
            bool bSetBackbufferReadyForPresent = !m_enableUI;
            m_scene->Render(m_commandQueue.Get(), bSetBackbufferReadyForPresent);

            if (m_enableUI)
            {
                m_uiLayer->Render(m_frameIndex);
            }

            // Present and update the frame index for the next frame.
            PIXBeginEvent(m_commandQueue.Get(), 0, L"Presenting to screen");
            // When using sync interval 0, it is recommended to always pass the tearing flag when it is supported.
            ThrowIfFailed(m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
            PIXEndEvent(m_commandQueue.Get());

            MoveToNextFrame();
        }
        catch (HrException& e)
        {
            if (e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET)
            {
                RecreateD3Dresources();
            }
            else
            {
                throw;
            }
        }
    }
}

// Tears down D3D resources and reinitializes them.
void D3D12xGPU::RecreateD3Dresources()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        WaitForGpu(m_commandQueue.Get());
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    ReleaseD3DObjects();
    OnInit();
}

void D3D12xGPU::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    try
    {
        WaitForGpu(m_commandQueue.Get());
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    CloseHandle(m_fenceEvent);
    if (m_adapterChangeEvent)
    {
        CloseHandle(m_adapterChangeEvent);
    }
}

void D3D12xGPU::SelectAdapter(UINT index)
{
    if (index != m_activeAdapter && index < m_gpuAdapterDescs.size() && m_gpuAdapterDescs[index].supportsDx12FL11)
    {
        m_activeAdapter = index;
        RecreateD3Dresources();
    }
}

// Looks up an adapter index for a given adapter luid.
// Returns false if no such adapter is currently enumerated.
bool D3D12xGPU::RetrieveAdapterIndex(UINT* adapterIndex, LUID adapterLuid)
{
    for (UINT i = 0; i < m_gpuAdapterDescs.size(); i++)
    {
        if (memcmp(&m_gpuAdapterDescs[i].desc.AdapterLuid, &adapterLuid, sizeof(adapterLuid)) == 0)
        {
            *adapterIndex = i;
            return true;
        }
    }
    return false;
}

void D3D12xGPU::SelectGPUPreference(UINT index)
{
    if (static_cast<DXGI_GPU_PREFERENCE>(index) != m_activeGpuPreference && index < m_gpuPreferenceToName.size())
    {
        // Regenerate adapter list with the new preference mode selected.
        m_activeGpuPreference = static_cast<DXGI_GPU_PREFERENCE>(index);

        // Verify the active adapter still meets the new gpu preference setting. 
        if (FAILED(ValidateActiveAdapter()))
        {
            // It does not, recreate the resources on a matching adapter.
            RecreateD3Dresources();
        }
    }
}

// Rechecks active adapter availability and it matching application settings.
// Returns an error, if the adapter is no longer valid.
HRESULT D3D12xGPU::ValidateActiveAdapter()
{
    EnumerateGPUadapters();

    if (!RetrieveAdapterIndex(&m_activeAdapter, m_activeAdapterLuid))
    {
        // The adapter is no longer available, default to 0.
        m_activeAdapter = 0;
        return DXGI_ERROR_DEVICE_RESET;
    }

    // Enforce adapter 0 being used, unless manual adapter selection is enabled.
    if (!m_manualAdapterSelection && m_activeAdapter != 0)
    {
        // A different adapter has become adapter 0, switch to it.
        m_activeAdapter = 0;
        return DXGI_ERROR_DEVICE_RESET;
    }

    return S_OK;
}

// Returns whether there have been adapter enumeration changes in the system
bool D3D12xGPU::QueryForAdapterEnumerationChanges()
{
    bool bChangeInAdapterEnumeration = false;
    if (m_adapterChangeEvent)
    {
#ifdef USE_DXGI_1_6
        // If QueryInterface for IDXGIFactory7 succeeded, then use RegisterAdaptersChangedEvent notifications.
        DWORD waitResult = WaitForSingleObject(m_adapterChangeEvent, 0);
        bChangeInAdapterEnumeration = (waitResult == WAIT_OBJECT_0);

        if (bChangeInAdapterEnumeration)
        {
            // Before recreating the factory, unregister the adapter event
            ComPtr<IDXGIFactory7> spDxgiFactory7;
            if (SUCCEEDED(m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&spDxgiFactory7))))
            {
                ThrowIfFailed(spDxgiFactory7->UnregisterAdaptersChangedEvent(m_adapterChangeRegistrationCookie));
                m_adapterChangeRegistrationCookie = 0;
                CloseHandle(m_adapterChangeEvent);
                m_adapterChangeEvent = NULL;
            }
        }
#endif
    }
    else
    {
        // Otherwise, IDXGIFactory7 doesn't exist, so continue using the polling solution of IsCurrent.
        bChangeInAdapterEnumeration = !m_dxgiFactory->IsCurrent();
    }

    return bChangeInAdapterEnumeration;
}

// Retrieves information about available GPU adapters.
void D3D12xGPU::EnumerateGPUadapters()
{
    m_gpuAdapterDescs.clear();
    
    ComPtr<IDXGIAdapter1> adapter;
#ifdef USE_DXGI_1_6
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapterByGpuPreference(adapterIndex, m_activeGpuPreference, IID_PPV_ARGS(&adapter)); ++adapterIndex)
#else
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
#endif
    {
        DxgiAdapterInfo adapterInfo;
        ThrowIfFailed(adapter->GetDesc1(&adapterInfo.desc));
        adapterInfo.supportsDx12FL11 = SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr));
        m_gpuAdapterDescs.push_back(move(adapterInfo));
    }
}

// Acquire selected adapter.
// If adapter cannot be found, *ppAdapter will be set to nullptr.
void D3D12xGPU::GetGPUAdapter(
    UINT adapterIndex,
    IDXGIAdapter1** ppAdapter)
{
    ComPtr<IDXGIAdapter1> adapter;
    *ppAdapter = nullptr;
#ifdef USE_DXGI_1_6
    if (m_dxgiFactory->EnumAdapterByGpuPreference(adapterIndex, m_activeGpuPreference, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND)
#else
    if (m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
#endif
    {
        DXGI_ADAPTER_DESC1 desc;
        ThrowIfFailed(adapter->GetDesc1(&desc));

        // Check to see if the adapter supports Direct3D 12.
        ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr));

        *ppAdapter = adapter.Detach();
    }
}

// Code computes the average frames per second, and also the 
// average time it takes to render one frame.
void D3D12xGPU::CalculateFrameStats()
{
    static int frameCnt = 0;
    static double elapsedTime = 0.0f;
    double totalTime = m_timer.GetTotalSeconds();
    frameCnt++;

    // Compute averages over one second period.
    if ((totalTime - elapsedTime) >= 1.0f)
    {
        float diff = static_cast<float>(totalTime - elapsedTime);
        m_fps = static_cast<float>(frameCnt) / diff; // Normalize to an exact second.

        frameCnt = 0;
        elapsedTime = totalTime;

        // Display FPS on the window title bar if UI is disabled.
        if (!m_enableUI)
        {
            wstring fpsStr = to_wstring(m_fps);
            wstring windowText = L"    fps: " + fpsStr;
            SetCustomWindowText(windowText.c_str());
        }
    }
}

// Wait for pending GPU work to complete.
void D3D12xGPU::WaitForGpu(ID3D12CommandQueue* pCommandQueue)
{
    // Schedule a Signal command in the queue.
    ThrowIfFailed(pCommandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12xGPU::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }
    m_scene->SetFrameIndex(m_frameIndex);

    // Set the fence value for the next frame.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}