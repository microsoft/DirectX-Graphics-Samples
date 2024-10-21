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

//===============================================================================
// desc: This is the core "game" application, where the game loop is contained. We can also access the Graphics Pipeline from here!
// modified: Aliyaan Zulfiqar
//===============================================================================

/*
   Change Log:
   [AZB] 16/10/24: Implemented ImGui and custom UI class into main program
   [AZB] 21/10/24: Implemented mouse accessor to enable swapping of input focus between ImGui and application
*/

#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "Display.h"
#include "SystemTime.h"
#include "GameInput.h"
#include "BufferManager.h"
#include "CommandContext.h"
#include "PostEffects.h"
#include "Util/CommandLineArg.h"
#include <shellapi.h>

#pragma comment(lib, "runtimeobject.lib") 

//
// [AZB]: Custom includes and macro mods
//

// [AZB]: Container file for code modifications and other helper tools. Contains the global "AZB_MOD" macro.
#include "AZB_Utils.h"




// [AZB]: These will only be included if the global modificiation macro is defined as true (=1)
#if AZB_MOD
#include "AZB_GUI.h"

// [AZB]: Set extern bool here, ensuring a single declaration and definiton.
bool g_bMouseExclusive = true;

// [AZB]: Temporary global UI class
GUI* AZB_GUI = new GUI();


// [AZB]: Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace GameCore
{
    using namespace Graphics;

    bool gIsSupending = false;

    void InitializeApplication( IGameApp& game )
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        CommandLineArgs::Initialize(argc, argv);
        Graphics::Initialize(game.RequiresRaytracingSupport());
        SystemTime::Initialize();
        GameInput::Initialize();
        EngineTuning::Initialize();

        game.Startup();
    }


    bool UpdateApplication(IGameApp& game)
    {
        EngineProfiling::Update();

        float DeltaTime = Graphics::GetFrameTime();

        // [AZB]: Set an input option to toggle between exclusive and non-exclusive mouse access for Mini EngineImGui control and Application control
#if AZB_MOD


        // [AZB]: The app will start in exclusive mode, but as this input gets repeated we need to check which one we're currently set to in order to correctly toggle
        if (g_bMouseExclusive)
        {
            // [AZB]: This allows the mouse to disappear when controlling the in-engine camera, and reappear when using ImGui. L.ALT + M
            if (GameInput::IsPressed(GameInput::kKey_lcontrol) && GameInput::IsFirstReleased(GameInput::kKey_m))
            {
                // [AZB]: Call bespoke function to unacquire mouse
                GameInput::ReleaseMouseExclusivity();

                // [AZB]: Update flag
                g_bMouseExclusive = false;
            }
        }
        // [AZB]: Only check ImGui when the flag is set to false
        else
        {
            // [AZB]: We have to re-enable exclusive access from ImGui's side!
            if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl) && ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_M))
            {
                // [AZB]: Update flag
                g_bMouseExclusive = true;
            }
        }

#endif

        GameInput::Update(DeltaTime);
        EngineTuning::Update(DeltaTime);

        game.Update(DeltaTime);
        game.RenderScene();

        PostEffects::Render();

        GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
        UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        UiContext.ClearColor(g_OverlayBuffer);
        UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
        UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
        game.RenderUI(UiContext);

        UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
        UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
        EngineTuning::Display(UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f);

        UiContext.Finish();

        // [AZB]: Run our ImGui windows and render them correctly within the MiniEngine's pipeline
#if AZB_MOD

        // [AZB]: Run our UI!
        AZB_GUI->Run();

        // [AZB]: Submit ImGui draw calls within engine context
        ImGui::Render();

        // [AZB]: Setup ImGui buffer using the GraphicsContext API
        GraphicsContext& ImGuiContext = GraphicsContext::Begin(L"Render ImGui");
        ImGuiContext.TransitionResource(g_ImGuiBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        ImGuiContext.ClearColor(g_ImGuiBuffer);
        // [AZB]: Using the overlay buffer render target - can't use the one from g_imGuiBuffer
        ImGuiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
        ImGuiContext.SetViewportAndScissor(0, 0, g_ImGuiBuffer.GetWidth(), g_ImGuiBuffer.GetHeight());

        ImGuiContext.GetCommandList()->SetDescriptorHeaps(1, &AZB_GUI->m_pSrvDescriptorHeap);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ImGuiContext.GetCommandList());

        // [AZB]: This will execute and then close the command list and do some other super optimal context flushing
        ImGuiContext.Finish();
#endif

        // [AZB]:Present finished frame
        Display::Present();

        return !game.IsDone();
    }


    // Default implementation to be overridden by the application
    bool IGameApp::IsDone( void )
    {
        return GameInput::IsFirstPressed(GameInput::kKey_escape);
    }

    HWND g_hWnd = nullptr;

    LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );


    void TerminateApplication(IGameApp& game)
    {
        g_CommandManager.IdleGPU();

        game.Cleanup();

        GameInput::Shutdown();
    }

    int RunApplication( IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow )
    {
        if (!XMVerifyCPUSupport())
            return 1;

        Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
        ASSERT_SUCCEEDED(InitializeWinRT);

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

        ShowWindow( g_hWnd, nCmdShow/*SW_SHOWDEFAULT*/ );


// [AZB]: Custom init steps and game loop setup
#if AZB_MOD 
        // [AZB]: Set up ImGui Context here, initalising our UI class
        AZB_GUI->Init(g_hWnd, g_Device, SWAP_CHAIN_BUFFER_COUNT, SWAP_CHAIN_FORMAT);
#endif

        // [AZB]: Original game Loop
        do
        {
            MSG msg = {};
            bool done = false;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                    done = true;
            }

            if (done)
                break;
        }
        while (UpdateApplication(app));	// Returns false to quit loop

        TerminateApplication(app);
        Graphics::Shutdown();
        return 0;
    }

    //--------------------------------------------------------------------------------------
    // Called every time the application receives a message
    //--------------------------------------------------------------------------------------
    LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
    {
        // [AZB]: Helps ImGui deal with input
#if AZB_MOD
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;
#endif
        switch( message )
        {
        case WM_SIZE:
            Display::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW( hWnd, message, wParam, lParam );
        }

        return 0;
    }

}
