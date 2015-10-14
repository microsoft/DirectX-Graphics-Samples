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

			MipsContext.CloseAndExecute();
		}

		// Xbox One has an separate image plane that we use for UI.  It will composite with the
		// main image plane, so we need to clear it each from (assuming it's being dynamically
		// updated.)

		GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
		UiContext.ClearColor(g_OverlayBuffer);
		UiContext.SetRenderTarget(g_OverlayBuffer);
		UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
		game.RenderUI(UiContext);

		EngineTuning::Display( UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f );

		UiContext.CloseAndExecute();

		Graphics::Present();

		if (GameInput::IsFirstPressed(GameInput::kKey_escape))
		{
			return false; // shutdown
		}
		return true;
	}

	HWND g_hWnd = nullptr;

	void InitWindow( const wchar_t* className );
	LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

	void RunApplication( IGameApp& app, const wchar_t* className )
	{
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
