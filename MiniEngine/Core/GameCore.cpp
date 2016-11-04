//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "SystemTime.h"
#include "GameInput.h"
#include "BufferManager.h"
#include "CommandContext.h"
#include "PostEffects.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	#pragma comment(lib, "runtimeobject.lib")
#else
	#include <agile.h>
	using Windows::ApplicationModel::Core::CoreApplication;
	using Windows::ApplicationModel::Core::CoreApplicationView;
	using Windows::UI::Core::CoreWindow;
	using Windows::UI::Core::CoreProcessEventsOption;
	using Windows::Foundation::TypedEventHandler;
	using Windows::ApplicationModel::Activation::IActivatedEventArgs;
#endif

namespace Graphics
{
	extern ColorBuffer g_GenMipsBuffer;
}

namespace GameCore
{
	using namespace Graphics;
	const bool TestGenerateMips = false;

	void InitializeApplication( IGameApp& game )
	{
		Graphics::Initialize();
		SystemTime::Initialize();
		GameInput::Initialize();
		EngineTuning::Initialize();

		game.Startup();
	}

	void TerminateApplication( IGameApp& game )
	{
		game.Cleanup();

		GameInput::Shutdown();
	}

	bool UpdateApplication( IGameApp& game )
	{
		EngineProfiling::Update();

		float DeltaTime = Graphics::GetFrameTime();
	
		GameInput::Update(DeltaTime);
		EngineTuning::Update(DeltaTime);
		
		game.Update(DeltaTime);
		game.RenderScene();

		PostEffects::Render();

		if (TestGenerateMips)
		{
			GraphicsContext& MipsContext = GraphicsContext::Begin();

			// Exclude from timings this copy necessary to setup the test
			MipsContext.CopySubresource(g_GenMipsBuffer, 0, g_SceneColorBuffer, 0);

			EngineProfiling::BeginBlock(L"GenerateMipMaps()", &MipsContext);
			g_GenMipsBuffer.GenerateMipMaps(MipsContext);
			EngineProfiling::EndBlock(&MipsContext);

			MipsContext.Finish();
		}

		GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
		UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		UiContext.ClearColor(g_OverlayBuffer);
		UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
		UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
		game.RenderUI(UiContext);

		EngineTuning::Display( UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f );

		UiContext.Finish();

		Graphics::Present();

		return !game.IsDone();
	}

	// Default implementation to be overridden by the application
	bool IGameApp::IsDone( void )
	{
		return GameInput::IsFirstPressed(GameInput::kKey_escape);
	}

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	IGameApp* m_game;
	Platform::Agile<Windows::UI::Core::CoreWindow> g_window;

	ref class ApplicationView sealed : public Windows::ApplicationModel::Core::IFrameworkView
	{
	public:
		ApplicationView() {}

		// IFrameworkView Methods.
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
		virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

	protected:
		// Event Handlers.
		void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnLogicalDpiChanged(Platform::Object^ sender);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);
		void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

	private:
		bool m_windowClosed;
		bool m_windowVisible;
	};

	ref class ApplicationViewSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
	{
	public:
		virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
		{
			return ref new ApplicationView();
		}
	};
	// Called by the system.  Perform application initialization here, hooking application wide events, etc.
	void ApplicationView::Initialize(CoreApplicationView^ applicationView)
	{
		applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ApplicationView::OnActivated);
	}

	// Called when we are provided a window.
	void ApplicationView::SetWindow(CoreWindow^ window)
	{
	}

	void ApplicationView::Load(Platform::String^ entryPoint)
	{
		g_window = CoreWindow::GetForCurrentThread();

		InitializeApplication(*m_game);
	}

	// Called by the system after initialization is complete.  This implements the traditional game loop.
	void ApplicationView::Run()
	{
		for (;;)
		{
			// ProcessEvents will throw if the process is exiting, allowing us to break out of the loop.  This will be
			// cleaned up when we get proper process lifetime management in a future release.
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			UpdateApplication(*m_game);
		}
	}

	void ApplicationView::Uninitialize()
	{
		TerminateApplication(*m_game);
	}

	// Called when the application is activated.  For now, there is just one activation kind - Launch.
	void ApplicationView::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void RunApplication( IGameApp& app, const wchar_t* className )
	{
		m_game = &app;
		(void)className;
		auto applicationViewSource = ref new ApplicationViewSource();
		CoreApplication::Run(applicationViewSource);
	}

#else // Win32

	HWND g_hWnd = nullptr;

	void InitWindow( const wchar_t* className );
	LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

	void RunApplication( IGameApp& app, const wchar_t* className )
	{
		//ASSERT_SUCCEEDED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));
		Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
		ASSERT_SUCCEEDED(InitializeWinRT);

		HINSTANCE hInst = GetModuleHandle(0);

		// Register class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = className;
		wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
		ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");

		// Create window
		RECT rc = { 0, 0, (LONG)g_DisplayWidth, (LONG)g_DisplayHeight };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

		ASSERT(g_hWnd != 0);

		InitializeApplication(app);

		ShowWindow( g_hWnd, SW_SHOWDEFAULT );

		do
		{
			MSG msg = {};
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT)
				break;
		}
		while (UpdateApplication(app));	// Returns false to quit loop

		Graphics::Terminate();
		TerminateApplication(app);
		Graphics::Shutdown();
	}

	//--------------------------------------------------------------------------------------
	// Called every time the application receives a message
	//--------------------------------------------------------------------------------------
	LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch( message )
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
				break;
			}

			case WM_SIZE:
				Graphics::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			default:
				return DefWindowProc( hWnd, message, wParam, lParam );
		}

		return 0;
	}
}

#endif // Win32
