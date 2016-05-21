#pragma once

#include "pch.h"

#include "GameEngineImpl.h"
#include <concrt.h>

namespace ModelViewer
{
	// Main entry point for our app. Connects the app with the Windows shell and handles application lifecycle events.
	ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
	{
	public:
		App();

		// IFrameworkView methods.
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
		virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

	protected:
		// Application lifecycle event handlers.
		void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);

		// Window event handlers.
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);


		void OnKeyDown(
			_In_ Windows::UI::Core::CoreWindow^ sender,
			_In_ Windows::UI::Core::KeyEventArgs^ args
			);

		void OnKeyUp(
			_In_ Windows::UI::Core::CoreWindow^ sender,
			_In_ Windows::UI::Core::KeyEventArgs^ args
			);

		void OnMouseMoved(
			_In_ Windows::Devices::Input::MouseDevice^ mouseDevice,
			_In_ Windows::Devices::Input::MouseEventArgs^ args
			);

		void OnPointerPressed(
			_In_ Windows::UI::Core::CoreWindow^ sender,
			_In_ Windows::UI::Core::PointerEventArgs^ args
			);


	private:

		bool m_windowClosed;
		bool m_windowVisible;

		GameEngineImpl gameEngineImpl;


		char kb_map[256];

		bool m_tracking = true;

	};
}

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
