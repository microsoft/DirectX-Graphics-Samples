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

#include "DXSample.h"

class DXSample;

class Win32Application
{
public:
    static int Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow);
    static void ToggleFullscreenWindow(IDXGISwapChain* pOutput = nullptr);
    static void SetWindowZorderToTopMost(bool setToTopMost);
    static HWND GetHwnd() { return m_hwnd; }
    static bool IsFullscreen() { return m_fullscreenMode; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
    static bool m_fullscreenMode;
    static const UINT m_windowStyle = WS_OVERLAPPEDWINDOW;
    static RECT m_windowRect;
};