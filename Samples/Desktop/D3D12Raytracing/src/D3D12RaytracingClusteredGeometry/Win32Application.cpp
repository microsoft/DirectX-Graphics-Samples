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
#include "Win32Application.h"
#include "DXSampleHelper.h"

HWND Win32Application::m_hwnd = nullptr;
DWORD Win32Application::m_uiThreadId = 0;
std::exception_ptr Win32Application::m_windowFailure;
bool Win32Application::m_fullscreenMode = false;
RECT Win32Application::m_windowRect;
using Microsoft::WRL::ComPtr;

int Win32Application::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
    m_uiThreadId = GetCurrentThreadId();
    m_windowFailure = nullptr;
    try
    {
        // Parse the command line parameters
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        pSample->ParseCommandLineArgs(argv, argc);
        LocalFree(argv);
        const bool benchmark = pSample->IsBenchmarkMode();
        if (benchmark)
        {
            ThrowIfFalse(SetThreadDpiAwarenessContext(
                DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != nullptr,
                L"Could not set physical-pixel DPI awareness for the benchmark.\n");
        }

        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"DXSampleClass";
        RegisterClassEx(&windowClass);

        RECT windowRect = {0, 0, static_cast<LONG>(pSample->GetWidth()),
                           static_cast<LONG>(pSample->GetHeight())};
        const DWORD windowStyle = benchmark ? (WS_POPUP | WS_DISABLED) : m_windowStyle;
        const DWORD extendedStyle = benchmark ? (WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW) : 0;
        ThrowIfFalse(AdjustWindowRectEx(&windowRect, windowStyle, FALSE, extendedStyle) != FALSE,
            L"Could not calculate the sample window size.\n");

        // Create the window and store a handle to it.
        m_hwnd = CreateWindowEx(
            extendedStyle,
            windowClass.lpszClassName,
            pSample->GetTitle(),
            windowStyle,
            benchmark ? 0 : CW_USEDEFAULT,
            benchmark ? 0 : CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,        // We have no parent window.
            nullptr,        // We aren't using menus.
            hInstance,
            pSample);
        ThrowIfFalse(m_hwnd != nullptr, L"Could not create the sample window.\n");
        if (m_windowFailure)
            std::rethrow_exception(m_windowFailure);

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        pSample->OnInit();

        auto desc = pSample->GetDeviceResources()->GetAdapterDescription();
        bool isSoftwareAdapter = (wcsstr(desc, L"WARP") != nullptr) ||
                                 (wcsstr(desc, L"Basic Render") != nullptr);
        if (benchmark)
        {
            // Stay behind the user's applications and never acquire keyboard focus.
            ThrowIfFalse(SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0,
                pSample->GetWidth(), pSample->GetHeight(),
                SWP_NOACTIVATE | SWP_SHOWWINDOW) != FALSE,
                L"Could not show the non-activating benchmark window.\n");
        }
        else
        {
            ShowWindow(m_hwnd, pSample->ShouldMaximizeWindowOnLaunch() ? SW_MAXIMIZE : SW_NORMAL);
        }
        if (m_windowFailure)
            std::rethrow_exception(m_windowFailure);

        // Software rendering and presentation run on a worker so slow frames
        // do not block the UI pump. Keyboard and resize callbacks are queued
        // to that worker; hardware rendering stays in WM_PAINT.
        if (isSoftwareAdapter)
        {
            pSample->GetDeviceResources()->EnableAsyncDisplay(
                /*render*/ [pSample]() {
                    pSample->OnUpdate();
                    pSample->OnRender();
                },
                /*resize*/ [pSample](UINT w, UINT h, bool minimized) {
                    pSample->OnSizeChanged(w, h, minimized);
                });
        }

        // Main sample loop.
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            if (m_windowFailure)
                std::rethrow_exception(m_windowFailure);
            pSample->GetDeviceResources()->RethrowAsyncFailure();
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else if (benchmark && !isSoftwareAdapter)
            {
                // Benchmark progress must not depend on window exposure/WM_PAINT.
                pSample->OnUpdate();
                pSample->OnRender();
            }
        }

        pSample->GetDeviceResources()->DisableAsyncDisplay();
        pSample->GetDeviceResources()->RethrowAsyncFailure();
        pSample->OnDestroy();
        ThrowIfFailed(pSample->GetDeviceResources()->ReleaseStablePowerState(),
            L"Could not release the benchmark's stable power-state request.\n");

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }
    catch (...)
    {
        OutputDebugString(L"Application hit a problem: ");
        // Mirror to log file so unattended runs (e.g. --screenshot mode) leave a trace.
        SampleLog::Write(L"Application hit a problem: ");
        try
        {
            throw;
        }
        catch (const std::exception& e)
        {
            OutputDebugStringA(e.what());
            wchar_t wbuf[1024]; size_t cv;
            mbstowcs_s(&cv, wbuf, e.what(), _TRUNCATE);
            SampleLog::Write(wbuf);
        }
        catch (...)
        {
            OutputDebugString(L"Unknown exception");
            SampleLog::Write(L"Unknown exception");
        }
        OutputDebugString(L"\nTerminating.\n");
        SampleLog::Write(L"\nTerminating.\n");

        if (auto* deviceResources = pSample->GetDeviceResources())
            deviceResources->DisableAsyncDisplay();
        pSample->OnDestroy();
        if (auto* deviceResources = pSample->GetDeviceResources())
            deviceResources->ReleaseStablePowerState();
        return EXIT_FAILURE;
    }
}

void Win32Application::RequestExit()
{
    if (!PostThreadMessage(m_uiThreadId, WM_QUIT, 0, 0))
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
}

// Convert a styled window into a fullscreen borderless window and back again.
void Win32Application::ToggleFullscreenWindow(IDXGISwapChain* pSwapChain)
{
    if (m_fullscreenMode)
    {
        // Restore the window's attributes and size.
        SetWindowLong(m_hwnd, GWL_STYLE, m_windowStyle);

        SetWindowPos(
            m_hwnd,
            HWND_NOTOPMOST,
            m_windowRect.left,
            m_windowRect.top,
            m_windowRect.right - m_windowRect.left,
            m_windowRect.bottom - m_windowRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(m_hwnd, SW_NORMAL);
    }
    else
    {
        // Save the old window rect so we can restore it when exiting fullscreen mode.
        GetWindowRect(m_hwnd, &m_windowRect);

        // Make the window borderless so that the client area can fill the screen.
        SetWindowLong(m_hwnd, GWL_STYLE,
                      m_windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU |
                                        WS_THICKFRAME));

        RECT fullscreenWindowRect;
        try
        {
            if (pSwapChain)
            {
                // Get the settings of the display on which the app's window is currently displayed
                ComPtr<IDXGIOutput> pOutput;
                ThrowIfFailed(pSwapChain->GetContainingOutput(&pOutput));
                DXGI_OUTPUT_DESC Desc;
                ThrowIfFailed(pOutput->GetDesc(&Desc));
                fullscreenWindowRect = Desc.DesktopCoordinates;
            }
            else
            {
                // Fallback to EnumDisplaySettings implementation
                throw HrException(S_FALSE);
            }
        }
        catch (HrException& e)
        {
            UNREFERENCED_PARAMETER(e);

            // Get the settings of the primary display
            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

            fullscreenWindowRect = {
                devMode.dmPosition.x,
                devMode.dmPosition.y,
                devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
                devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
            };
        }

        SetWindowPos(
            m_hwnd,
            HWND_TOPMOST,
            fullscreenWindowRect.left,
            fullscreenWindowRect.top,
            fullscreenWindowRect.right,
            fullscreenWindowRect.bottom,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);


        ShowWindow(m_hwnd, SW_MAXIMIZE);
    }

    m_fullscreenMode = !m_fullscreenMode;
}

void Win32Application::SetWindowZorderToTopMost(bool setToTopMost)
{
    RECT windowRect;
    GetWindowRect(m_hwnd, &windowRect);

    SetWindowPos(
        m_hwnd,
        (setToTopMost) ? HWND_TOPMOST : HWND_NOTOPMOST,
        windowRect.left,
        windowRect.top,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        SWP_FRAMECHANGED | SWP_NOACTIVATE);
}

LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Exceptions cannot unwind through a Windows callback. Transfer them to
    // Run's error handler outside DispatchMessage instead.
    try
    {
        if (m_windowFailure)
            return DefWindowProc(hWnd, message, wParam, lParam);
        return HandleWindowMessage(hWnd, message, wParam, lParam);
    }
    catch (...)
    {
        m_windowFailure = std::current_exception();
        return 0;
    }
}

// Main message handler for the sample.
LRESULT Win32Application::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (pSample && pSample->IsBenchmarkMode())
    {
        switch (message)
        {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_SIZE:
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) != SC_CLOSE)
                return 0;
            break;
        case WM_WINDOWPOSCHANGING:
        {
            auto* position = reinterpret_cast<WINDOWPOS*>(lParam);
            if (!(position->flags & SWP_NOSIZE))
            {
                position->cx = static_cast<int>(pSample->GetWidth());
                position->cy = static_cast<int>(pSample->GetHeight());
            }
            break;
        }
        case WM_PAINT:
            ValidateRect(hWnd, nullptr);
            return 0;
        }
    }

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_KEYDOWN:
        if (pSample)
        {
            const UINT8 key = static_cast<UINT8>(wParam);
            // In async-display mode (WARP), dispatch via the worker's
            // pending-action queue so OnKeyDown runs on the worker thread.
            // This keeps the UI message pump responsive (the worker can
            // take seconds to process the resulting RebuildStatic on WARP;
            // running OnKeyDown synchronously here would block WM_*
            // dispatch for that duration, freezing the window).  In HW
            // mode (no worker), call directly as before -- the queue is a
            // no-op without an async worker.
            auto* dr = pSample->GetDeviceResources();
            if (dr && dr->IsAsyncDisplay())
                dr->EnqueueAsyncAction([pSample, key]() { pSample->OnKeyDown(key); });
            else
                pSample->OnKeyDown(key);
        }
        return 0;

    case WM_KEYUP:
        if (pSample)
        {
            const UINT8 key = static_cast<UINT8>(wParam);
            auto* dr = pSample->GetDeviceResources();
            if (dr && dr->IsAsyncDisplay())
                dr->EnqueueAsyncAction([pSample, key]() { pSample->OnKeyUp(key); });
            else
                pSample->OnKeyUp(key);
        }
        return 0;

    case WM_SYSKEYDOWN:
        // Handle ALT+ENTER:
        if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
        {
            if (pSample && pSample->GetDeviceResources()->IsTearingSupported())
            {
                ToggleFullscreenWindow(pSample->GetSwapchain());
                return 0;
            }
        }
        // Send all other WM_SYSKEYDOWN messages to the default WndProc.
        break;

    case WM_PAINT:
        if (pSample)
        {
            auto* dr = pSample->GetDeviceResources();
            if (dr && dr->IsAsyncDisplayActive())
            {
                // Async mode: worker thread is doing OnUpdate + OnRender
                // + Present on its own.  UI thread WM_PAINT just
                // acknowledges the paint so Windows doesn't keep
                // re-queueing it; the worker's Present is what actually
                // updates the visible content.
                ValidateRect(hWnd, nullptr);
            }
            else
            {
                pSample->OnUpdate();
                pSample->OnRender();
            }
        }
        return 0;

    case WM_SIZE:
        if (pSample)
        {
            RECT windowRect = {};
            GetWindowRect(hWnd, &windowRect);
            pSample->SetWindowBounds(windowRect.left, windowRect.top, windowRect.right,
                                     windowRect.bottom);

            RECT clientRect = {};
            GetClientRect(hWnd, &clientRect);
            const UINT clientW = clientRect.right - clientRect.left;
            const UINT clientH = clientRect.bottom - clientRect.top;
            const bool minimized = (wParam == SIZE_MINIMIZED);

            auto* dr = pSample->GetDeviceResources();
            if (dr && dr->IsAsyncDisplayActive())
            {
                // Async mode: don't call OnSizeChanged from the UI thread
                // -- that would block until the worker's in-flight render
                // finishes (5 s+ on WARP-at-4K), which is exactly the
                // stall that triggers (Not Responding) / DWM ghost mode
                // and the maximize-bouncing artifact.  Instead just stash
                // the new size; the worker picks it up between frames
                // and runs OnSizeChanged on its own thread.
                dr->RequestAsyncResize(clientW, clientH, minimized);
            }
            else
            {
                pSample->OnSizeChanged(clientW, clientH, minimized);
            }
        }
        return 0;

    case WM_MOVE:
        if (pSample)
        {
            RECT windowRect = {};
            GetWindowRect(hWnd, &windowRect);
            pSample->SetWindowBounds(windowRect.left, windowRect.top, windowRect.right,
                                     windowRect.bottom);

            int xPos = (int)(short)LOWORD(lParam);
            int yPos = (int)(short)HIWORD(lParam);
            pSample->OnWindowMoved(xPos, yPos);
        }
        return 0;

    case WM_DISPLAYCHANGE:
        if (pSample)
        {
            pSample->OnDisplayChanged();
        }
        return 0;

    case WM_MOUSEMOVE:
        if (pSample && static_cast<UINT8>(wParam) == MK_LBUTTON)
        {
            UINT x = LOWORD(lParam);
            UINT y = HIWORD(lParam);
            pSample->OnMouseMove(x, y);
        }
        return 0;

    case WM_LBUTTONDOWN:
    {
        UINT x = LOWORD(lParam);
        UINT y = HIWORD(lParam);
        pSample->OnLeftButtonDown(x, y);
    }
    return 0;

    case WM_LBUTTONUP:
    {
        UINT x = LOWORD(lParam);
        UINT y = HIWORD(lParam);
        pSample->OnLeftButtonUp(x, y);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
