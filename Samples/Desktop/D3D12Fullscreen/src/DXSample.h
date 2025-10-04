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

#include "DXSampleHelper.h"
#include "Win32Application.h"

class DXSample
{
public:
    DXSample(UINT width, UINT height, std::wstring name);
    virtual ~DXSample();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized) = 0;
    virtual void OnDestroy() = 0;
    
    // Samples override the event handlers to handle specific messages.
    virtual void OnKeyDown(UINT8 /*key*/) {}
    virtual void OnKeyUp(UINT8 /*key*/) {}
    virtual void OnWindowMoved(int /*x*/, int /*y*/) {}
    virtual void OnMouseMove(UINT /*x*/, UINT /*y*/) {}
    virtual void OnLeftButtonDown(UINT /*x*/, UINT /*y*/) {}
    virtual void OnLeftButtonUp(UINT /*x*/, UINT /*y*/) {}
    virtual void OnDisplayChanged() {}
    
    // Accessors.
    UINT GetWidth() const           { return m_width; }
    UINT GetHeight() const          { return m_height; }
    const WCHAR* GetTitle() const   { return m_title.c_str(); }
    bool GetTearingSupport() const  { return m_tearingSupport; }
    RECT GetWindowsBounds() const   { return m_windowBounds; }
    virtual IDXGISwapChain* GetSwapchain() { return nullptr; }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void SetWindowBounds(int left, int top, int right, int bottom);
    std::wstring GetAssetFullPath(LPCWSTR assetName);

protected:
    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

    void SetCustomWindowText(LPCWSTR text);
    void CheckTearingSupport();

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    // Window bounds
    RECT m_windowBounds;
    
    // Whether or not tearing is available for fullscreen borderless windowed mode.
    bool m_tearingSupport;

    // Adapter info.
    bool m_useWarpDevice;

    // Override to be able to start without UI, to avoid 11On12 usage
    bool m_enableUI;

private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;
};
