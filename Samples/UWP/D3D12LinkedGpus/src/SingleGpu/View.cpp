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

View::View(UINT_PTR pSample) :
	m_pSample(reinterpret_cast<DXSample*>(pSample)),
	m_windowClosed(false)
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
	window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &View::OnKeyDown);
	window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &View::OnKeyUp);
	window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &View::OnSizeChanged);
	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &View::OnVisibilityChanged);
	window->Closed += ref new TypedEventHandler<CoreWindow ^, CoreWindowEventArgs ^>(this, &View::OnClosed);

	// For simplicity, this sample ignores a number of events on CoreWindow that a
	// typical app should subscribe to.

	DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
	displayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &View::OnDpiChanged);
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

void View::OnKeyUp(CoreWindow^ /*window*/, KeyEventArgs^ args)
{
	if (static_cast<UINT>(args->VirtualKey) < 256)
	{
		m_pSample->OnKeyUp(static_cast<UINT8>(args->VirtualKey));
	}
}

void View::OnSizeChanged(CoreWindow^ /*window*/, WindowSizeChangedEventArgs^ args)
{
	UpdateWindowSize(args->Size.Width, args->Size.Height);
}

void View::OnDpiChanged(DisplayInformation^ /*sender*/, Object^ /*args*/)
{
	CoreWindow^ window = CoreWindow::GetForCurrentThread();
	UpdateWindowSize(window->Bounds.Width, window->Bounds.Height);
}

void View::OnVisibilityChanged(CoreWindow^ window, VisibilityChangedEventArgs^ args)
{
	UpdateWindowSize(window->Bounds.Width, window->Bounds.Height, args->Visible);
}

void View::OnClosed(CoreWindow^ /*window*/, CoreWindowEventArgs^ /*args*/)
{
	m_windowClosed = true;
}

inline UINT ConvertToPixels(float dimension, float dpi)
{
	return static_cast<UINT>(dimension * dpi / 96.0f);
}

void View::UpdateWindowSize(float width, float height, bool visible)
{
	DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
	float dpi = displayInformation->LogicalDpi;
	m_pSample->OnSizeChanged(ConvertToPixels(width, dpi), ConvertToPixels(height, dpi), !visible);
}
