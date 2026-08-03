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
bool Win32Application::m_fullscreenMode = false;
RECT Win32Application::m_windowRect;
using Microsoft::WRL::ComPtr;

int Win32Application::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
    try
    {
        // Parse the command line parameters
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        pSample->ParseCommandLineArgs(argv, argc);
        LocalFree(argv);

        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"DXSampleClass";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        m_hwnd = CreateWindow(
            windowClass.lpszClassName,
            pSample->GetTitle(),
            m_windowStyle,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,        // We have no parent window.
            nullptr,        // We aren't using menus.
            hInstance,
            pSample);

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        pSample->OnInit();

        auto desc = pSample->GetDeviceResources()->GetAdapterDescription();
        bool isSoftwareAdapter = (wcsstr(desc, L"WARP") != nullptr) ||
                                 (wcsstr(desc, L"Basic Render") != nullptr);
        ShowWindow(m_hwnd, pSample->ShouldMaximizeWindowOnLaunch()
                               ? SW_MAXIMIZE
                               : SW_NORMAL);

        // On software adapters (WARP / Basic Render), spin up the async
        // display worker: the sample's OnRender (slow on a CPU rasterizer
        // -- >1 s/frame at high res) runs on a dedicated thread that
        // writes into an offscreen RT, and the UI thread's WM_PAINT just
        // blits the latest offscreen into the back buffer and Presents.
        // This is what keeps the message pump responsive on slow frames
        // and prevents Windows "(Not Responding)" / DWM ghost-mode
        // bouncing when the user maximizes the window.
        //
        // On hardware adapters: not used.  WM_PAINT calls OnUpdate +
        // OnRender directly as before.
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
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        pSample->OnDestroy();

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }
    catch (std::exception& e)
    {
        OutputDebugString(L"Application hit a problem: ");
        OutputDebugStringA(e.what());
        OutputDebugString(L"\nTerminating.\n");

        // Mirror to log file so unattended runs (e.g. --screenshot mode) leave a trace.
        SampleLog::Write(L"Application hit a problem: ");
        wchar_t wbuf[1024]; size_t cv;
        mbstowcs_s(&cv, wbuf, e.what(), _TRUNCATE);
        SampleLog::Write(wbuf);
        SampleLog::Write(L"\nTerminating.\n");

        pSample->OnDestroy();
        return EXIT_FAILURE;
    }
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
        SetWindowLong(m_hwnd, GWL_STYLE, m_windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

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

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
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
            pSample->SetWindowBounds(windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);

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
            pSample->SetWindowBounds(windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);

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
