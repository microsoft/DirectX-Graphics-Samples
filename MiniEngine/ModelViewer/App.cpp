#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace ModelViewer;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Devices::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// The DirectX 12 Application template is documented at http://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x409

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
	memset(kb_map, 0, sizeof(kb_map));

	kb_map[(int)Windows::System::VirtualKey::W] = 0x11; // DIK_W;
	kb_map[(int)Windows::System::VirtualKey::A] = 0x1E; // DIK_A
	kb_map[(int)Windows::System::VirtualKey::S] = 0x1F; // DIK_S;
	kb_map[(int)Windows::System::VirtualKey::D] = 0x20; // DIK_D;
	kb_map[(int)Windows::System::VirtualKey::Escape] = 0x01; // DIK_ESCAPE;

}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	window->PointerCursor = nullptr;


	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyDown);

	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyUp);

	MouseDevice::GetForCurrentView()->MouseMoved +=
		ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &App::OnMouseMoved);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerPressed);

}

typedef struct _DIMOUSESTATE2 {
	LONG    lX;
	LONG    lY;
	LONG    lZ;
	BYTE    rgbButtons[8];
} DIMOUSESTATE2, *LPDIMOUSESTATE2;


extern _DIMOUSESTATE2 s_MouseState;


void App::OnMouseMoved(
	_In_ Windows::Devices::Input::MouseDevice^ mouseDevice,
	_In_ Windows::Devices::Input::MouseEventArgs^ args
	)
{
	s_MouseState.lX = args->MouseDelta.X*4;
	s_MouseState.lY = args->MouseDelta.Y*4;
}




extern unsigned char s_Keybuffer[256];


void App::OnKeyDown(
	_In_ CoreWindow^ /* sender */,
	_In_ KeyEventArgs^ args
	)
{
	s_Keybuffer[kb_map[(int)args->VirtualKey]] = 128;

	if (args->VirtualKey == Windows::System::VirtualKey::Escape)
	{
		CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([this]()
		{
			CoreWindow::GetForCurrentThread()->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
			m_tracking = false;
		}));

	}
}

void App::OnPointerPressed(
	_In_ Windows::UI::Core::CoreWindow^ sender,
	_In_ Windows::UI::Core::PointerEventArgs^ args
	)
{
	CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
		m_tracking = true;
	}));

}


void App::OnKeyUp(
	_In_ CoreWindow^ /* sender */,
	_In_ KeyEventArgs^ args
	)
{
	s_Keybuffer[kb_map[(int)args->VirtualKey]] = 0;
}


// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{


	GameCore::InitializeApplication(gameEngineImpl);
}

// This method is called after the window becomes active.
void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			{
				//Concurrency::critical_section::scoped_lock lck(m_render_cs);
				GameCore::UpdateApplication(gameEngineImpl);

				s_MouseState.lX = 0;
				s_MouseState.lY = 0;
				s_MouseState.lZ = 0;
			}
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}

	Graphics::Terminate();
	//TerminateApplication(gameEngineImpl);
	Graphics::Shutdown();
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{

}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// TODO: Insert your code here.
		//m_main->OnSuspending();

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	// TODO: Insert your code here.
	//m_main->OnResuming();
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	//GetDeviceResources()->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));


	//Concurrency::critical_section::scoped_lock lck (m_render_cs);

	Graphics::Resize((uint32_t)sender->Bounds.Width, (uint32_t)sender->Bounds.Height);
	//m_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	//GetDeviceResources()->SetDpi(sender->LogicalDpi);
	//m_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	//GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
	//m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	//GetDeviceResources()->ValidateDevice();
}

//std::shared_ptr<DX::DeviceResources> App::GetDeviceResources()
//{
	//if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
	//{
		// All references to the existing D3D device must be released before a new device
		// can be created.

		//m_deviceResources = nullptr;
		//m_main->OnDeviceRemoved();
	//}

	//if (m_deviceResources == nullptr)
	//{
		//m_deviceResources = std::make_shared<DX::DeviceResources>();
		//m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
		//m_main->CreateRenderers(m_deviceResources);
	//}
	//return m_deviceResources;
//}
