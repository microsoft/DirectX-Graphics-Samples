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

#include "Win32Application.h"
#include <stdexcept>

HWND Win32Application::Create(HINSTANCE instance, WNDPROC windowProc, void* sample, UINT width, UINT height)
{
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = L"DxrPartitionedTlasWindow";
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (!RegisterClassExW(&windowClass))
        throw std::runtime_error("RegisterClassExW failed");

    RECT size{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, FALSE);
    HWND window =
        CreateWindowExW(0, windowClass.lpszClassName, L"DXR 2 PTLAS", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        size.right - size.left, size.bottom - size.top, nullptr, nullptr, instance, sample);
    if (!window)
        throw std::runtime_error("CreateWindowExW failed");
    return window;
}

void Win32Application::ToggleFullscreen(HWND window)
{
    // Borderless fullscreen keeps the flip-model swapchain in windowed mode,
    // so the same VSync/tearing path works in both window configurations.
    static bool fullscreen = false;
    static WINDOWPLACEMENT placement{sizeof(WINDOWPLACEMENT)};
    if (!fullscreen)
    {
        MONITORINFO monitor{sizeof(MONITORINFO)};
        if (!GetWindowPlacement(window, &placement) ||
            !GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor))
            throw std::runtime_error("Could not query fullscreen window bounds");
        SetWindowLongW(window, GWL_STYLE, GetWindowLongW(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(window, HWND_TOP, monitor.rcMonitor.left, monitor.rcMonitor.top,
                     monitor.rcMonitor.right - monitor.rcMonitor.left, monitor.rcMonitor.bottom - monitor.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
    }
    else
    {
        SetWindowLongW(window, GWL_STYLE, GetWindowLongW(window, GWL_STYLE) | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &placement);
        SetWindowPos(window, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
    }
    fullscreen = !fullscreen;
}

void Win32Application::Run(const std::function<void()>& render, const std::function<bool()>& minimized)
{
    MSG message{};
    while (message.message != WM_QUIT)
    {
        if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        else if (!minimized())
            render();
        else
            WaitMessage();
    }
}
