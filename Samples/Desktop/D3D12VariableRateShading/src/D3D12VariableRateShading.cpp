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
#include "D3D12VariableRateShading.h"
#include "UILayer.h"
#include <dxgidebug.h>
#include "VariableRateShadingScene.h"

using namespace std;

std::wstring GetShadingRateString(const D3D12_VARIABLE_SHADING_RATE_TIER& tier, const D3D12_SHADING_RATE& shadingRate)
{
    if (tier == D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
    {
        return L"Not supported";
    }
    else
    {
        switch (shadingRate)
        {
        case D3D12_SHADING_RATE_1X1: return L"1x1";
        case D3D12_SHADING_RATE_1X2: return L"1x2";
        case D3D12_SHADING_RATE_2X1: return L"2x1";
        case D3D12_SHADING_RATE_2X2: return L"2x2";
        case D3D12_SHADING_RATE_2X4: return L"2x4";
        case D3D12_SHADING_RATE_4X2: return L"4x2";
        case D3D12_SHADING_RATE_4X4: return L"4x4";
        default: return L"NA";
        }
    }
}

D3D12_SHADING_RATE GetShadingRate(UINT8 key)
{
    switch (key)
    {
    case 1: return D3D12_SHADING_RATE_1X1;
    case 2: return D3D12_SHADING_RATE_1X2;
    case 3: return D3D12_SHADING_RATE_2X1;
    case 4: return D3D12_SHADING_RATE_2X2;
    case 5: return D3D12_SHADING_RATE_2X4;
    case 6: return D3D12_SHADING_RATE_4X2;
    case 7: return D3D12_SHADING_RATE_4X4;

    default: return D3D12_SHADING_RATE_1X1;
    }
}

D3D12VariableRateShading* D3D12VariableRateShading::s_app = nullptr;

D3D12VariableRateShading::D3D12VariableRateShading(UINT width, UINT height, wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_bCtrlKeyIsPressed(false),
    m_bShiftKeyIsPressed(false),
    m_fps(0.0f),
    m_fenceValues{},
    m_windowVisible(true),
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
}

D3D12VariableRateShading::~D3D12VariableRateShading()
{
}

void D3D12VariableRateShading::OnInit()
{
    LoadPipeline();
    LoadAssets();
    LoadSizeDependentResources();
}

// Load the rendering pipeline dependencies.
void D3D12VariableRateShading::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(factory.Get(), &hardwareAdapter);

    ThrowIfFailed(D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    ));
    NAME_D3D12_OBJECT(m_device);

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
    // DXGI does not allow creating a swapchain targeting a window which has fullscreen styles(no border + topmost).
    // Temporarily remove the topmost property for creating the swapchain.
    bool prevIsFullscreen = Win32Application::IsFullscreen();
    if (prevIsFullscreen)
    {
        Win32Application::SetWindowZorderToTopMost(false);
    }
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    if (prevIsFullscreen)
    {
        Win32Application::SetWindowZorderToTopMost(true);
    }

    // With tearing support enabled we will handle ALT+Enter key presses in the
    // window message loop rather than let DXGI handle it by calling SetFullscreenState.
    factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER);

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
void D3D12VariableRateShading::LoadAssets()
{
    if (!m_scene)
    {
        m_scene = make_unique<VariableRateShadingScene>(FrameCount, this);
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
void D3D12VariableRateShading::LoadSizeDependentResources()
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
void D3D12VariableRateShading::ReleaseSizeDependentResources()
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

void D3D12VariableRateShading::UpdateUI()
{
    vector<wstring> labels;
    {
        wstringstream wLabel;
        wLabel.precision(1);
        wLabel << std::fixed;
        wLabel << L"FPS: ";
        wLabel.precision(2);
        wLabel << m_fps << L" (" << (1.0f / m_fps) * 1000.0f << L"ms)\n";
        labels.push_back(wLabel.str());
    }

    {
        wstringstream wLabel;
        wLabel.precision(2);
        wLabel << std::fixed;
        wLabel << L"\nRefraction pass (" << m_scene->GetRefractionPassGPUTimeInMs() << L"ms):\n";
        wLabel << L" Shading rate (press [1-7] to select): " << GetShadingRateString(m_scene->m_shadingRateTier, m_scene->m_refractionShadingRate) << L"\n";
        wLabel << L" Refraction scale (press [+/-] to change): " << m_scene->m_refractionScale << L"\n";
        labels.push_back(wLabel.str());
    }

    {
        wstringstream wLabel;
        wLabel.precision(2);
        wLabel << std::fixed;
        wLabel << L"\nScene pass (" << m_scene->GetScenePassGPUTimeInMs() << L"ms):\n";
        wLabel << L" Shading rate (press SHIFT + [1-7] to select): " << GetShadingRateString(m_scene->m_shadingRateTier, m_scene->m_sceneShadingRate) << L"\n";
        labels.push_back(wLabel.str());
    }

    {
        wstringstream wLabel;
        wLabel.precision(2);
        wLabel << std::fixed;
        wLabel << L"\nPostprocess pass (" << m_scene->GetPostprocessPassGPUTimeInMs() << L"ms):\n";
        wLabel << L" Shading rate (press CTRL + [1-7] to select): " << GetShadingRateString(m_scene->m_shadingRateTier, m_scene->m_postprocessShadingRate) << L"\n";
        wLabel.precision(3);
        wLabel << L" Fog density (press CTRL + [+/-] to change): " << m_scene->m_fogDensity << L"\n";
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
void D3D12VariableRateShading::ReleaseD3DObjects()
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

void D3D12VariableRateShading::OnKeyUp(UINT8 key)
{
    m_scene->KeyUp(key);
    switch (key)
    {
    case VK_CONTROL:
        m_bCtrlKeyIsPressed = false;
        break;
    case VK_SHIFT:
        m_bShiftKeyIsPressed = false;
        break;
    default:
        break;
    }
}

void D3D12VariableRateShading::OnKeyDown(UINT8 key)
{
    static const float refractionScaleStepSize = 0.01f;
    static const float fogDensityStepSize = 0.005f;

    m_scene->KeyDown(key);
    switch (key)
    {
    case VK_F1:
    {
        // Preset: No coarse pixel shading (Max detail).
        m_scene->m_refractionShadingRate = D3D12_SHADING_RATE_1X1;
        m_scene->m_sceneShadingRate = D3D12_SHADING_RATE_1X1;
        m_scene->m_postprocessShadingRate = D3D12_SHADING_RATE_1X1;
        break;
    }

    case VK_F2:
    {
        // Preset: Balanced coarse shading A (Reduce details where they are least noticable and improve performance).
        m_scene->m_refractionShadingRate = D3D12_SHADING_RATE_4X4;
        m_scene->m_sceneShadingRate = D3D12_SHADING_RATE_1X1;
        m_scene->m_postprocessShadingRate = D3D12_SHADING_RATE_1X1;
        break;
    }

    case VK_F3:
    {
        // Preset: Balanced coarse shading B (Further reduce details and improve performance).
        m_scene->m_refractionShadingRate = D3D12_SHADING_RATE_4X4;
        m_scene->m_sceneShadingRate = D3D12_SHADING_RATE_2X2;
        m_scene->m_postprocessShadingRate = D3D12_SHADING_RATE_1X1;
        break;
    }

    case VK_F4:
    {
        // Preset: Balanced coarse shading C (Further reduce details and improve performance).
        m_scene->m_refractionShadingRate = D3D12_SHADING_RATE_4X4;
        m_scene->m_sceneShadingRate = D3D12_SHADING_RATE_2X2;
        m_scene->m_postprocessShadingRate = D3D12_SHADING_RATE_2X2;
        break;
    }

    case VK_F5:
    {
        // Preset: Coarse pixel shading (Max performance).
        m_scene->m_refractionShadingRate = D3D12_SHADING_RATE_4X4;
        m_scene->m_sceneShadingRate = D3D12_SHADING_RATE_4X4;
        m_scene->m_postprocessShadingRate = D3D12_SHADING_RATE_4X4;
        break;
    }

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
    {
        const D3D12_SHADING_RATE shadingRate = GetShadingRate(key - '0');
        if (m_bShiftKeyIsPressed)
        {
            m_scene->m_sceneShadingRate = shadingRate;
        }
        else if (m_bCtrlKeyIsPressed)
        {
            m_scene->m_postprocessShadingRate = shadingRate;
        }
        else
        {
            m_scene->m_refractionShadingRate = shadingRate;
        }
        break;
    }

    case VK_CONTROL:
        m_bCtrlKeyIsPressed = true;
        break;
    case VK_SHIFT:
        m_bShiftKeyIsPressed = true;
        break;

    case VK_ADD:
    case VK_OEM_PLUS:
        if (m_bCtrlKeyIsPressed)
        {
            m_scene->m_fogDensity += fogDensityStepSize;
            m_scene->m_fogDensity = min(0.15f, m_scene->m_fogDensity);
        }
        else
        {
            m_scene->m_refractionScale += refractionScaleStepSize;
            m_scene->m_refractionScale = min(0.3f, m_scene->m_refractionScale);
        }
        break;
    case VK_SUBTRACT:
    case VK_OEM_MINUS:
        if (m_bCtrlKeyIsPressed)
        {
            m_scene->m_fogDensity -= fogDensityStepSize;
            m_scene->m_fogDensity = max(0.0f, m_scene->m_fogDensity);
        }
        else
        {
            m_scene->m_refractionScale -= refractionScaleStepSize;
            m_scene->m_refractionScale = max(0.0f, m_scene->m_refractionScale);
        }
        break;

    default:
        break;
    }
}

void D3D12VariableRateShading::OnSizeChanged(UINT width, UINT height, bool minimized)
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
                RecreateD3DResources();
            }
            else
            {
                throw;
            }
        }
    }
}

void D3D12VariableRateShading::OnUpdate()
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
void D3D12VariableRateShading::OnRender()
{
    if (m_windowVisible)
    {
        PIXBeginEvent(m_commandQueue.Get(), 0, L"Frame");

        try
        {
            // UILayer will transition backbuffer to a present state.
            const bool bSetBackbufferReadyForPresent = !m_enableUI;
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

            PIXEndEvent(m_commandQueue.Get());
        }
        catch (HrException& e)
        {
            if (e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET)
            {
                RecreateD3DResources();
            }
            else
            {
                throw;
            }
        }
    }
}

// Tears down D3D resources and reinitializes them.
void D3D12VariableRateShading::RecreateD3DResources()
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

void D3D12VariableRateShading::OnDestroy()
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
}

// Code computes the average frames per second, and also the 
// average time it takes to render one frame.
void D3D12VariableRateShading::CalculateFrameStats()
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
void D3D12VariableRateShading::WaitForGpu(ID3D12CommandQueue* pCommandQueue)
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
void D3D12VariableRateShading::MoveToNextFrame()
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