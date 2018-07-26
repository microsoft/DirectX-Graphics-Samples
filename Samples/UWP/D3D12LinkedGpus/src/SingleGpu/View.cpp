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
#include "View.h"

using namespace Windows::Foundation;
using namespace Windows::System;

inline int ConvertToPixels(float dimension, float dpi)
{
    return static_cast<int>(dimension * dpi / 96.0f + 0.5f);
}

View::View(UINT_PTR pSample) :
    m_pSample(reinterpret_cast<DXSample*>(pSample)),
    m_windowClosed(false),
    m_windowResizing(false),
    m_windowVisible(true),
    m_logicalWidth(0),
    m_logicalHeight(0)
{
}

void View::Initialize(CoreApplicationView^ applicationView)
{
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &View::OnActivated);

    // For simplicity, this sample ignores CoreApplication's Suspend and Resume
    // events which a typical app should subscribe to.
}

void View::SetWindow(CoreWindow^ window)
{
    m_logicalWidth = window->Bounds.Width;
    m_logicalHeight = window->Bounds.Height;

    auto dispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;
    dispatcher->AcceleratorKeyActivated +=
        ref new TypedEventHandler<CoreDispatcher^, AcceleratorKeyEventArgs^>(this, &View::OnAcceleratorKeyActivated);

    window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &View::OnKeyDown);
    window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &View::OnKeyUp);
    window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &View::OnSizeChanged);
    window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &View::OnVisibilityChanged);
    window->Closed += ref new TypedEventHandler<CoreWindow ^, CoreWindowEventArgs ^>(this, &View::OnClosed);
    window->PointerMoved += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &View::OnPointerMoved);
    window->PointerPressed += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &View::OnPointerPressed);
    window->PointerReleased += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &View::OnPointerReleased);

    try
    {
        window->ResizeStarted += ref new TypedEventHandler<CoreWindow^, Object^>(this, &View::OnResizeStarted);
        window->ResizeCompleted += ref new TypedEventHandler<CoreWindow^, Object^>(this, &View::OnResizeCompleted);
    }
    catch (Exception^ e)
    {
        // Requires Windows 10 Creators Update (10.0.15063) or later.
    }

    // For simplicity, this sample ignores a number of events on CoreWindow that a
    // typical app should subscribe to.

    DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
    displayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &View::OnDpiChanged);
    displayInformation->DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &View::OnDisplayContentsInvalidated);
}

void View::Load(String^ /*entryPoint*/)
{
}

void View::Run()
{
    auto applicationView = ApplicationView::GetForCurrentView();
    applicationView->Title = ref new String(m_pSample->GetTitle());

    m_pSample->OnInit();

    while (!m_windowClosed)
    {
        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

        m_pSample->OnUpdate();
        m_pSample->OnRender();
    }

    m_pSample->OnDestroy();
}

void View::Uninitialize()
{
}

void View::OnActivated(CoreApplicationView^ /*applicationView*/, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();
}

void View::OnKeyDown(CoreWindow^ /*window*/, KeyEventArgs^ args)
{
    if (static_cast<UINT>(args->VirtualKey) < 256)
    {
        m_pSample->OnKeyDown(static_cast<UINT8>(args->VirtualKey));
    }
}

void View::OnAcceleratorKeyActivated(CoreDispatcher^, AcceleratorKeyEventArgs^ args)
{
    if (args->EventType == CoreAcceleratorKeyEventType::SystemKeyDown
        && args->VirtualKey == VirtualKey::Enter
        && args->KeyStatus.IsMenuKeyDown
        && !args->KeyStatus.WasKeyDown)
    {
        // Implements the classic ALT+ENTER fullscreen toggle
        auto applicationView = ApplicationView::GetForCurrentView();

        if (applicationView->IsFullScreenMode)
        {
            applicationView->ExitFullScreenMode();
        }
        else
        {
            applicationView->TryEnterFullScreenMode();
        }

        args->Handled = true;
    }
}

void View::OnKeyUp(CoreWindow^ /*window*/, KeyEventArgs^ args)
{
    if (static_cast<UINT>(args->VirtualKey) < 256)
    {
        m_pSample->OnKeyUp(static_cast<UINT8>(args->VirtualKey));
    }
}

void View::OnSizeChanged(CoreWindow^ /*window*/, WindowSizeChangedEventArgs^ args)
{
    m_logicalWidth = args->Size.Width;
    m_logicalHeight = args->Size.Height;

    if (!m_windowResizing)
    {
        UpdateWindowSize();
    }
}

void View::OnResizeStarted(CoreWindow^ /*window*/, Platform::Object^ /*args*/)
{
    m_windowResizing = true;
}

void View::OnResizeCompleted(CoreWindow^ /*window*/, Platform::Object^ /*args*/)
{
    m_windowResizing = false;
    UpdateWindowSize();
}

void View::OnDpiChanged(DisplayInformation^ /*sender*/, Object^ /*args*/)
{
    UpdateWindowSize();
}

void View::OnVisibilityChanged(CoreWindow^ /*window*/, VisibilityChangedEventArgs^ args)
{
    m_windowVisible = args->Visible;
    UpdateWindowSize();
}

void View::OnClosed(CoreWindow^ /*window*/, CoreWindowEventArgs^ /*args*/)
{
    m_windowClosed = true;
}

void View::OnPointerMoved(CoreWindow^ /*window*/, PointerEventArgs^ args)
{
    DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
    float dpi = displayInformation->LogicalDpi;
    m_pSample->OnMouseMove(ConvertToPixels(args->CurrentPoint->Position.X, dpi), ConvertToPixels(args->CurrentPoint->Position.Y, dpi));
}

void View::OnPointerPressed(CoreWindow^ /*window*/, PointerEventArgs^ args)
{
    // For simplicity, we only consider left button press here.
    if(args->CurrentPoint->Properties->IsLeftButtonPressed)
    {
        DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
        float dpi = displayInformation->LogicalDpi;
        m_pSample->OnLeftButtonDown(ConvertToPixels(args->CurrentPoint->Position.X, dpi), ConvertToPixels(args->CurrentPoint->Position.Y, dpi));
    }
}

void View::OnPointerReleased(CoreWindow^ /*window*/, PointerEventArgs^ args)
{
    // For simplicity, we only consider left button press here.
    if(args->CurrentPoint->Properties->IsLeftButtonPressed)
    {
        DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
        float dpi = displayInformation->LogicalDpi;
        m_pSample->OnLeftButtonUp(ConvertToPixels(args->CurrentPoint->Position.X, dpi), ConvertToPixels(args->CurrentPoint->Position.Y, dpi));
    }
}

void View::UpdateWindowSize()
{
    CoreWindow^ window = CoreWindow::GetForCurrentThread();
    Windows::Foundation::Rect windowBounds = window->Bounds;

    DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
    float dpi = displayInformation->LogicalDpi;
    
    m_pSample->SetWindowBounds(
        ConvertToPixels(windowBounds.Left, dpi),
        ConvertToPixels(windowBounds.Top, dpi),
        ConvertToPixels(windowBounds.Right, dpi),
        ConvertToPixels(windowBounds.Bottom, dpi));
        
    m_pSample->OnSizeChanged(ConvertToPixels(m_logicalWidth, dpi), ConvertToPixels(m_logicalHeight, dpi), !m_windowVisible);
}

void View::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
    m_pSample->OnDisplayChanged();
}