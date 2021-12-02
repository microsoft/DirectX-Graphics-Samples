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
#include "Input.h"

#ifdef _GAMING_DESKTOP

// I can't find the GameInput.h header in the GDK for Desktop yet
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#define USE_KEYBOARD_MOUSE
#include <Keyboard.h>
#include <Mouse.h>

using DirectX::Keyboard;
using DirectX::Mouse;

DirectX::Keyboard g_Keyboard;
DirectX::Mouse g_Mouse;

void GameInput::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch( message )
    {
    case WM_ACTIVATEAPP:
        Keyboard::ProcessMessage(message, wParam, lParam);
        Mouse::ProcessMessage(message, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        Keyboard::ProcessMessage(message, wParam, lParam);
        break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        Mouse::ProcessMessage(message, wParam, lParam);
        break;

    default:
        break;
    }
}

#else

void GameInput::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
}

// This is what we should use on *all* platforms, but see previous comment
#include <GameInput.h>

// This should be handled by GameInput.h, but we'll borrow values from XINPUT.
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  (7849.0f / 32768.0f)
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE (8689.0f / 32768.0f)

#endif

namespace GameCore
{
    extern HWND g_hWnd;
}

namespace
{
    bool s_Buttons[2][GameInput::kNumDigitalInputs];
    float s_HoldDuration[GameInput::kNumDigitalInputs] = { 0.0f };
    float s_Analogs[GameInput::kNumAnalogInputs];
    float s_AnalogsTC[GameInput::kNumAnalogInputs];

#ifdef USE_KEYBOARD_MOUSE

    unsigned char s_Keybuffer[256];
    unsigned char s_DXKeyMapping[GameInput::kNumKeys]; // map DigitalInput enum to DX key codes 

#endif

    inline float FilterAnalogInput( int val, int deadZone )
    {
        if (val < 0)
        {
            if (val > -deadZone)
                return 0.0f;
            else
                return (val + deadZone) / (32768.0f - deadZone);
        }
        else
        {
            if (val < deadZone)
                return 0.0f;
            else
                return (val - deadZone) / (32767.0f - deadZone);
        }
    }

#ifdef USE_KEYBOARD_MOUSE
    void KbmBuildKeyMapping()
    {
        s_DXKeyMapping[GameInput::kKey_escape] = Keyboard::Keys::Escape;
        s_DXKeyMapping[GameInput::kKey_1] = Keyboard::Keys::D1;
        s_DXKeyMapping[GameInput::kKey_2] = Keyboard::Keys::D2;
        s_DXKeyMapping[GameInput::kKey_3] = Keyboard::Keys::D3;
        s_DXKeyMapping[GameInput::kKey_4] = Keyboard::Keys::D4;
        s_DXKeyMapping[GameInput::kKey_5] = Keyboard::Keys::D5;
        s_DXKeyMapping[GameInput::kKey_6] = Keyboard::Keys::D6;
        s_DXKeyMapping[GameInput::kKey_7] = Keyboard::Keys::D7;
        s_DXKeyMapping[GameInput::kKey_8] = Keyboard::Keys::D8;
        s_DXKeyMapping[GameInput::kKey_9] = Keyboard::Keys::D9;
        s_DXKeyMapping[GameInput::kKey_0] = Keyboard::Keys::D0;
        s_DXKeyMapping[GameInput::kKey_minus] = Keyboard::Keys::OemMinus;
        s_DXKeyMapping[GameInput::kKey_equals] = 13;
        s_DXKeyMapping[GameInput::kKey_back] = Keyboard::Keys::Back;
        s_DXKeyMapping[GameInput::kKey_tab] = Keyboard::Keys::Tab;
        s_DXKeyMapping[GameInput::kKey_q] = Keyboard::Keys::Q;
        s_DXKeyMapping[GameInput::kKey_w] = Keyboard::Keys::W;
        s_DXKeyMapping[GameInput::kKey_e] = Keyboard::Keys::E;
        s_DXKeyMapping[GameInput::kKey_r] = Keyboard::Keys::R;
        s_DXKeyMapping[GameInput::kKey_t] = Keyboard::Keys::T;
        s_DXKeyMapping[GameInput::kKey_y] = Keyboard::Keys::Y;
        s_DXKeyMapping[GameInput::kKey_u] = Keyboard::Keys::U;
        s_DXKeyMapping[GameInput::kKey_i] = Keyboard::Keys::I;
        s_DXKeyMapping[GameInput::kKey_o] = Keyboard::Keys::O;
        s_DXKeyMapping[GameInput::kKey_p] = Keyboard::Keys::P;
        s_DXKeyMapping[GameInput::kKey_lbracket] = Keyboard::Keys::OemOpenBrackets;
        s_DXKeyMapping[GameInput::kKey_rbracket] = Keyboard::Keys::OemCloseBrackets;
        s_DXKeyMapping[GameInput::kKey_return] = Keyboard::Keys::Enter;
        s_DXKeyMapping[GameInput::kKey_lcontrol] = Keyboard::Keys::LeftControl;
        s_DXKeyMapping[GameInput::kKey_a] = Keyboard::Keys::A;
        s_DXKeyMapping[GameInput::kKey_s] = Keyboard::Keys::S;
        s_DXKeyMapping[GameInput::kKey_d] = Keyboard::Keys::D;
        s_DXKeyMapping[GameInput::kKey_f] = Keyboard::Keys::F;
        s_DXKeyMapping[GameInput::kKey_g] = Keyboard::Keys::G;
        s_DXKeyMapping[GameInput::kKey_h] = Keyboard::Keys::H;
        s_DXKeyMapping[GameInput::kKey_j] = Keyboard::Keys::J;
        s_DXKeyMapping[GameInput::kKey_k] = Keyboard::Keys::K;
        s_DXKeyMapping[GameInput::kKey_l] = Keyboard::Keys::L;
        s_DXKeyMapping[GameInput::kKey_semicolon] = Keyboard::Keys::OemSemicolon;
        s_DXKeyMapping[GameInput::kKey_apostrophe] = 40;
        s_DXKeyMapping[GameInput::kKey_grave] = 41;
        s_DXKeyMapping[GameInput::kKey_lshift] = Keyboard::Keys::LeftShift;
        s_DXKeyMapping[GameInput::kKey_backslash] = Keyboard::Keys::OemBackslash;
        s_DXKeyMapping[GameInput::kKey_z] = Keyboard::Keys::Z;
        s_DXKeyMapping[GameInput::kKey_x] = Keyboard::Keys::X;
        s_DXKeyMapping[GameInput::kKey_c] = Keyboard::Keys::C;
        s_DXKeyMapping[GameInput::kKey_v] = Keyboard::Keys::V;
        s_DXKeyMapping[GameInput::kKey_b] = Keyboard::Keys::B;
        s_DXKeyMapping[GameInput::kKey_n] = Keyboard::Keys::N;
        s_DXKeyMapping[GameInput::kKey_m] = Keyboard::Keys::M;
        s_DXKeyMapping[GameInput::kKey_comma] = Keyboard::Keys::OemComma;
        s_DXKeyMapping[GameInput::kKey_period] = Keyboard::Keys::OemPeriod;
        s_DXKeyMapping[GameInput::kKey_slash] = 53;
        s_DXKeyMapping[GameInput::kKey_rshift] = Keyboard::Keys::RightShift;
        s_DXKeyMapping[GameInput::kKey_multiply] = Keyboard::Keys::Multiply;
        s_DXKeyMapping[GameInput::kKey_lalt] = Keyboard::Keys::LeftAlt;
        s_DXKeyMapping[GameInput::kKey_space] = Keyboard::Keys::Space;
        s_DXKeyMapping[GameInput::kKey_capital] = Keyboard::Keys::CapsLock;
        s_DXKeyMapping[GameInput::kKey_f1] = Keyboard::Keys::F1;
        s_DXKeyMapping[GameInput::kKey_f2] = Keyboard::Keys::F2;
        s_DXKeyMapping[GameInput::kKey_f3] = Keyboard::Keys::F3;
        s_DXKeyMapping[GameInput::kKey_f4] = Keyboard::Keys::F4;
        s_DXKeyMapping[GameInput::kKey_f5] = Keyboard::Keys::F5;
        s_DXKeyMapping[GameInput::kKey_f6] = Keyboard::Keys::F6;
        s_DXKeyMapping[GameInput::kKey_f7] = Keyboard::Keys::F7;
        s_DXKeyMapping[GameInput::kKey_f8] = Keyboard::Keys::F8;
        s_DXKeyMapping[GameInput::kKey_f9] = Keyboard::Keys::F9;
        s_DXKeyMapping[GameInput::kKey_f10] = Keyboard::Keys::F10;
        s_DXKeyMapping[GameInput::kKey_numlock] = Keyboard::Keys::NumLock;
        s_DXKeyMapping[GameInput::kKey_scroll] = Keyboard::Keys::Scroll;
        s_DXKeyMapping[GameInput::kKey_numpad7] = Keyboard::Keys::NumPad7;
        s_DXKeyMapping[GameInput::kKey_numpad8] = Keyboard::Keys::NumPad8;
        s_DXKeyMapping[GameInput::kKey_numpad9] = Keyboard::Keys::NumPad9;
        s_DXKeyMapping[GameInput::kKey_subtract] = Keyboard::Keys::Subtract;
        s_DXKeyMapping[GameInput::kKey_numpad4] = Keyboard::Keys::NumPad4;
        s_DXKeyMapping[GameInput::kKey_numpad5] = Keyboard::Keys::NumPad5;
        s_DXKeyMapping[GameInput::kKey_numpad6] = Keyboard::Keys::NumPad6;
        s_DXKeyMapping[GameInput::kKey_add] = Keyboard::Keys::OemPlus;
        s_DXKeyMapping[GameInput::kKey_numpad1] = Keyboard::Keys::NumPad1;
        s_DXKeyMapping[GameInput::kKey_numpad2] = Keyboard::Keys::NumPad2;
        s_DXKeyMapping[GameInput::kKey_numpad3] = Keyboard::Keys::NumPad3;
        s_DXKeyMapping[GameInput::kKey_numpad0] = Keyboard::Keys::NumPad0;
        s_DXKeyMapping[GameInput::kKey_decimal] = Keyboard::Keys::OemPeriod;
        s_DXKeyMapping[GameInput::kKey_f11] = Keyboard::Keys::F11;
        s_DXKeyMapping[GameInput::kKey_f12] = Keyboard::Keys::F12;
        s_DXKeyMapping[GameInput::kKey_numpadenter] = Keyboard::Keys::Enter; // NumPad variant?
        s_DXKeyMapping[GameInput::kKey_rcontrol] = Keyboard::Keys::RightControl;
        s_DXKeyMapping[GameInput::kKey_divide] = Keyboard::Keys::Divide;
        s_DXKeyMapping[GameInput::kKey_sysrq] = 183;
        s_DXKeyMapping[GameInput::kKey_ralt] = Keyboard::Keys::RightAlt;
        s_DXKeyMapping[GameInput::kKey_pause] = Keyboard::Keys::Pause;
        s_DXKeyMapping[GameInput::kKey_home] = Keyboard::Keys::Home;
        s_DXKeyMapping[GameInput::kKey_up] = Keyboard::Keys::Up;
        s_DXKeyMapping[GameInput::kKey_pgup] = Keyboard::Keys::PageUp;
        s_DXKeyMapping[GameInput::kKey_left] = Keyboard::Keys::Left;
        s_DXKeyMapping[GameInput::kKey_right] = Keyboard::Keys::Right;
        s_DXKeyMapping[GameInput::kKey_end] = Keyboard::Keys::End;
        s_DXKeyMapping[GameInput::kKey_down] = Keyboard::Keys::Down;
        s_DXKeyMapping[GameInput::kKey_pgdn] = Keyboard::Keys::PageDown;
        s_DXKeyMapping[GameInput::kKey_insert] = Keyboard::Keys::Insert;
        s_DXKeyMapping[GameInput::kKey_delete] = Keyboard::Keys::Delete;
        s_DXKeyMapping[GameInput::kKey_lwin] = Keyboard::Keys::LeftWindows;
        s_DXKeyMapping[GameInput::kKey_rwin] = Keyboard::Keys::RightWindows;
        s_DXKeyMapping[GameInput::kKey_apps] = Keyboard::Keys::Apps;
    }

    void KbmZeroInputs()
    {
        //memset(&s_MouseState, 0, sizeof(DIMOUSESTATE2));
        //memset(s_Keybuffer, 0, sizeof(s_Keybuffer));
    }

    void KbmInitialize()
    {
        KbmBuildKeyMapping();

        /*
        if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&s_DI, nullptr)))
            ASSERT(false, "DirectInput8 initialization failed.");

        if (FAILED(s_DI->CreateDevice(GUID_SysKeyboard, &s_Keyboard, nullptr)))
            ASSERT(false, "Keyboard CreateDevice failed.");
        if (FAILED(s_Keyboard->SetDataFormat(&c_dfDIKeyboard)))
            ASSERT(false, "Keyboard SetDataFormat failed.");
        if (FAILED(s_Keyboard->SetCooperativeLevel(GameCore::g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            ASSERT(false, "Keyboard SetCooperativeLevel failed.");

        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData = 10;
        if (FAILED(s_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
            ASSERT(false, "Keyboard set buffer size failed.");

        if (FAILED(s_DI->CreateDevice(GUID_SysMouse, &s_Mouse, nullptr)))
            ASSERT(false, "Mouse CreateDevice failed.");
        if (FAILED(s_Mouse->SetDataFormat(&c_dfDIMouse2)))
            ASSERT(false, "Mouse SetDataFormat failed.");
        if (FAILED(s_Mouse->SetCooperativeLevel(GameCore::g_hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
            ASSERT(false, "Mouse SetCooperativeLevel failed.");
        */
        KbmZeroInputs();
    }

    void KbmShutdown()
    {
        /*
        if (s_Keyboard)
        {
            s_Keyboard->Unacquire();
            s_Keyboard->Release();
            s_Keyboard = nullptr;
        }
        if (s_Mouse)
        {
            s_Mouse->Unacquire();
            s_Mouse->Release();
            s_Mouse = nullptr;
        }
        if (s_DI)
        {
            s_DI->Release();
            s_DI = nullptr;
        }
        */
    }

    void KbmUpdate()
    {
        /*
        HWND foreground = GetForegroundWindow();
        bool visible = IsWindowVisible(foreground) != 0;

        if (foreground != GameCore::g_hWnd // wouldn't be able to acquire
            || !visible)
        {
            KbmZeroInputs();
        }
        else
        {
            s_Mouse->Acquire();
            s_Mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &s_MouseState);
            s_Keyboard->Acquire();
            s_Keyboard->GetDeviceState(sizeof(s_Keybuffer), s_Keybuffer);
        }
        */
    }

#endif

}

void GameInput::Initialize()
{
    ZeroMemory(s_Buttons, sizeof(s_Buttons) );
    ZeroMemory(s_Analogs, sizeof(s_Analogs) );

#ifdef USE_KEYBOARD_MOUSE
    KbmInitialize();
#endif
}

void GameInput::Shutdown()
{
#ifdef USE_KEYBOARD_MOUSE
    KbmShutdown();
#endif
}

void GameInput::Update( float frameDelta )
{
    memcpy(s_Buttons[1], s_Buttons[0], sizeof(s_Buttons[0]));
    memset(s_Buttons[0], 0, sizeof(s_Buttons[0]));
    memset(s_Analogs, 0, sizeof(s_Analogs));

#ifdef _GAMING_DESKTOP

#define SET_BUTTON_VALUE(InputEnum, GameInputMask) \
        s_Buttons[0][InputEnum] = !!(newInputState.Gamepad.wButtons & GameInputMask);

    XINPUT_STATE newInputState;
    if (ERROR_SUCCESS == XInputGetState(0, &newInputState))
    {
        SET_BUTTON_VALUE(kDPadUp, XINPUT_GAMEPAD_DPAD_UP);
        SET_BUTTON_VALUE(kDPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
        SET_BUTTON_VALUE(kDPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
        SET_BUTTON_VALUE(kDPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
        SET_BUTTON_VALUE(kStartButton, XINPUT_GAMEPAD_START);
        SET_BUTTON_VALUE(kBackButton, XINPUT_GAMEPAD_BACK);
        SET_BUTTON_VALUE(kLThumbClick, XINPUT_GAMEPAD_LEFT_THUMB);
        SET_BUTTON_VALUE(kRThumbClick, XINPUT_GAMEPAD_RIGHT_THUMB);
        SET_BUTTON_VALUE(kLShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
        SET_BUTTON_VALUE(kRShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
        SET_BUTTON_VALUE(kAButton, XINPUT_GAMEPAD_A);
        SET_BUTTON_VALUE(kBButton, XINPUT_GAMEPAD_B);
        SET_BUTTON_VALUE(kXButton, XINPUT_GAMEPAD_X);
        SET_BUTTON_VALUE(kYButton, XINPUT_GAMEPAD_Y);

        s_Analogs[kAnalogLeftTrigger]   = newInputState.Gamepad.bLeftTrigger / 255.0f;
        s_Analogs[kAnalogRightTrigger]  = newInputState.Gamepad.bRightTrigger / 255.0f;
        s_Analogs[kAnalogLeftStickX]    = FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
        s_Analogs[kAnalogLeftStickY]    = FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
        s_Analogs[kAnalogRightStickX]   = FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
        s_Analogs[kAnalogRightStickY]   = FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
    }
#else
    IGameInputReading* pGIR = nullptr;
    if (s_pGameInput != nullptr)
        s_pGameInput->GetCurrentReading(GameInputKindGamepad, nullptr, &pGIR);
    bool IsGamepadPresent = (pGIR != nullptr);

    if (IsGamepadPresent)
    {
        GameInputGamepadState newInputState;
        pGIR->GetGamepadState(&newInputState);
        pGIR->Release();

#define SET_BUTTON_VALUE(InputEnum, GameInputMask) \
        s_Buttons[0][InputEnum] = !!(newInputState.buttons & GameInputMask);

        SET_BUTTON_VALUE(kDPadUp, GameInputGamepadDPadUp);
        SET_BUTTON_VALUE(kDPadDown, GameInputGamepadDPadDown);
        SET_BUTTON_VALUE(kDPadLeft, GameInputGamepadDPadLeft);
        SET_BUTTON_VALUE(kDPadRight, GameInputGamepadDPadRight);
        SET_BUTTON_VALUE(kStartButton, GameInputGamepadMenu);
        SET_BUTTON_VALUE(kBackButton, GameInputGamepadView);
        SET_BUTTON_VALUE(kLThumbClick, GameInputGamepadLeftThumbstick);
        SET_BUTTON_VALUE(kRThumbClick, GameInputGamepadRightThumbstick);
        SET_BUTTON_VALUE(kLShoulder, GameInputGamepadLeftShoulder);
        SET_BUTTON_VALUE(kRShoulder, GameInputGamepadRightShoulder);
        SET_BUTTON_VALUE(kAButton, GameInputGamepadA);
        SET_BUTTON_VALUE(kBButton, GameInputGamepadB);
        SET_BUTTON_VALUE(kXButton, GameInputGamepadX);
        SET_BUTTON_VALUE(kYButton, GameInputGamepadY);

        s_Analogs[kAnalogLeftTrigger]   = newInputState.leftTrigger;
        s_Analogs[kAnalogRightTrigger]  = newInputState.rightTrigger;
        s_Analogs[kAnalogLeftStickX]    = FilterAnalogInput(newInputState.leftThumbstickX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        s_Analogs[kAnalogLeftStickY]    = FilterAnalogInput(newInputState.leftThumbstickY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        s_Analogs[kAnalogRightStickX]   = FilterAnalogInput(newInputState.rightThumbstickX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        s_Analogs[kAnalogRightStickY]   = FilterAnalogInput(newInputState.rightThumbstickY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    }
#endif

#ifdef USE_KEYBOARD_MOUSE
    KbmUpdate();

    auto kbState = Keyboard::Get().GetState();

    for (uint32_t i = 0; i < kNumKeys; ++i)
    {
        s_Buttons[0][i] = kbState.IsKeyDown((Keyboard::Keys)s_DXKeyMapping[i]);
    }

    /*
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (s_MouseState.rgbButtons[i] > 0) s_Buttons[0][kMouse0 + i] = true;
    }

    s_Analogs[kAnalogMouseX] = (float)s_MouseState.lX * .0018f;
    s_Analogs[kAnalogMouseY] = (float)s_MouseState.lY * -.0018f;

    if (s_MouseState.lZ > 0)
        s_Analogs[kAnalogMouseScroll] = 1.0f;
    else if (s_MouseState.lZ < 0)
        s_Analogs[kAnalogMouseScroll] = -1.0f;
    */
#endif

    // Update time duration for buttons pressed
    for (uint32_t i = 0; i < kNumDigitalInputs; ++i)
    {
        if (s_Buttons[0][i])
        {
            if (!s_Buttons[1][i])
                s_HoldDuration[i] = 0.0f;
            else
                s_HoldDuration[i] += frameDelta;
        }
    }

    for (uint32_t i = 0; i < kNumAnalogInputs; ++i)
    {
        s_AnalogsTC[i] = s_Analogs[i] * frameDelta;
    }

}

bool GameInput::IsAnyPressed( void )
{
    return s_Buttons[0] != 0;
}

bool GameInput::IsPressed( DigitalInput di )
{
    return s_Buttons[0][di];
}

bool GameInput::IsFirstPressed( DigitalInput di )
{
    return s_Buttons[0][di] && !s_Buttons[1][di];
}

bool GameInput::IsReleased( DigitalInput di )
{
    return !s_Buttons[0][di];
}

bool GameInput::IsFirstReleased( DigitalInput di )
{
    return !s_Buttons[0][di] && s_Buttons[1][di];
}

float GameInput::GetDurationPressed( DigitalInput di )
{
    return s_HoldDuration[di];
}

float GameInput::GetAnalogInput( AnalogInput ai )
{
    return s_Analogs[ai];
}

float GameInput::GetTimeCorrectedAnalogInput( AnalogInput ai )
{
    return s_AnalogsTC[ai];
}
