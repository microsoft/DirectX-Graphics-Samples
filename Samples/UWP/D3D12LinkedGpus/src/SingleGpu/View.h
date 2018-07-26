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

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;

ref class View sealed : public IFrameworkView
{
public:
    View(UINT_PTR pSample);

    virtual void Initialize(CoreApplicationView^ applicationView);
    virtual void SetWindow(CoreWindow^ window);
    virtual void Load(String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

private:
    void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args);
    void OnAcceleratorKeyActivated(CoreDispatcher^, AcceleratorKeyEventArgs^ args);
    void OnKeyDown(CoreWindow^ window, KeyEventArgs^ args);
    void OnKeyUp(CoreWindow^ window, KeyEventArgs^ args);
    void OnSizeChanged(CoreWindow^ window, WindowSizeChangedEventArgs^ args);
    void OnResizeStarted(CoreWindow^ window, Object^ args);
    void OnResizeCompleted(CoreWindow^ window, Object^ args);
    void OnDpiChanged(DisplayInformation^ sender, Object^ args);
    void OnVisibilityChanged(CoreWindow^ window, VisibilityChangedEventArgs^ args);
    void OnClosed(CoreWindow^ window, CoreWindowEventArgs^ args);
    void OnPointerMoved(CoreWindow^ window, PointerEventArgs^ args);
    void OnPointerPressed(CoreWindow^ window, PointerEventArgs^ args);
    void OnPointerReleased(CoreWindow^ window, PointerEventArgs^ args);
    void OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args);
    
    void UpdateWindowSize();

    DXSample* m_pSample;
    bool m_windowClosed;
    bool m_windowResizing;
    bool m_windowVisible;
    float m_logicalWidth;
    float m_logicalHeight;
};
