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

#include "stdafx.h"
#include "GameInput.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#define USE_XINPUT
#include <XInput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#define USE_KEYBOARD_MOUSE
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


extern HWND g_hWnd;

#else

using namespace Windows::Gaming::Input;
using namespace Windows::Foundation::Collections;

#define USE_KEYBOARD_MOUSE

struct DIMOUSESTATE2
{
    LONG lX, lY, lZ;
    BYTE rgbButtons[8];
};

#endif

namespace
{
    bool s_Buttons[2][GameInput::kNumDigitalInputs];
    float s_HoldDuration[GameInput::kNumDigitalInputs] = { 0.0f };
    float s_Analogs[GameInput::kNumAnalogInputs];
    float s_AnalogsTC[GameInput::kNumAnalogInputs];

#ifdef USE_KEYBOARD_MOUSE

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    IDirectInput8A* s_DI;
    IDirectInputDevice8A* s_Keyboard;
    IDirectInputDevice8A* s_Mouse;
#endif

    DIMOUSESTATE2 s_MouseState;
    unsigned char s_Keybuffer[256];
    unsigned char s_DXKeyMapping[GameInput::kNumKeys]; // map DigitalInput enum to DX key codes 

#endif

#ifdef USE_XINPUT
    float FilterAnalogInput( int val, int deadZone )
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
#else
    float FilterAnalogInput( float val, float deadZone )
    {
        if (val < -deadZone)
            return (val + deadZone) / (1.0f - deadZone);
        else if (val > deadZone)
            return (val - deadZone) / (1.0f - deadZone);
        else
            return 0.0f;
    }
#endif

#ifdef USE_KEYBOARD_MOUSE
    void KbmBuildKeyMapping()
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        s_DXKeyMapping[GameInput::kKey_escape] = 1;
        s_DXKeyMapping[GameInput::kKey_1] = 2;
        s_DXKeyMapping[GameInput::kKey_2] = 3;
        s_DXKeyMapping[GameInput::kKey_3] = 4;
        s_DXKeyMapping[GameInput::kKey_4] = 5;
        s_DXKeyMapping[GameInput::kKey_5] = 6;
        s_DXKeyMapping[GameInput::kKey_6] = 7;
        s_DXKeyMapping[GameInput::kKey_7] = 8;
        s_DXKeyMapping[GameInput::kKey_8] = 9;
        s_DXKeyMapping[GameInput::kKey_9] = 10;
        s_DXKeyMapping[GameInput::kKey_0] = 11;
        s_DXKeyMapping[GameInput::kKey_minus] = 12;
        s_DXKeyMapping[GameInput::kKey_equals] = 13;
        s_DXKeyMapping[GameInput::kKey_back] = 14;
        s_DXKeyMapping[GameInput::kKey_tab] = 15;
        s_DXKeyMapping[GameInput::kKey_q] = 16;
        s_DXKeyMapping[GameInput::kKey_w] = 17;
        s_DXKeyMapping[GameInput::kKey_e] = 18;
        s_DXKeyMapping[GameInput::kKey_r] = 19;
        s_DXKeyMapping[GameInput::kKey_t] = 20;
        s_DXKeyMapping[GameInput::kKey_y] = 21;
        s_DXKeyMapping[GameInput::kKey_u] = 22;
        s_DXKeyMapping[GameInput::kKey_i] = 23;
        s_DXKeyMapping[GameInput::kKey_o] = 24;
        s_DXKeyMapping[GameInput::kKey_p] = 25;
        s_DXKeyMapping[GameInput::kKey_lbracket] = 26;
        s_DXKeyMapping[GameInput::kKey_rbracket] = 27;
        s_DXKeyMapping[GameInput::kKey_return] = 28;
        s_DXKeyMapping[GameInput::kKey_lcontrol] = 29;
        s_DXKeyMapping[GameInput::kKey_a] = 30;
        s_DXKeyMapping[GameInput::kKey_s] = 31;
        s_DXKeyMapping[GameInput::kKey_d] = 32;
        s_DXKeyMapping[GameInput::kKey_f] = 33;
        s_DXKeyMapping[GameInput::kKey_g] = 34;
        s_DXKeyMapping[GameInput::kKey_h] = 35;
        s_DXKeyMapping[GameInput::kKey_j] = 36;
        s_DXKeyMapping[GameInput::kKey_k] = 37;
        s_DXKeyMapping[GameInput::kKey_l] = 38;
        s_DXKeyMapping[GameInput::kKey_semicolon] = 39;
        s_DXKeyMapping[GameInput::kKey_apostrophe] = 40;
        s_DXKeyMapping[GameInput::kKey_grave] = 41;
        s_DXKeyMapping[GameInput::kKey_lshift] = 42;
        s_DXKeyMapping[GameInput::kKey_backslash] = 43;
        s_DXKeyMapping[GameInput::kKey_z] = 44;
        s_DXKeyMapping[GameInput::kKey_x] = 45;
        s_DXKeyMapping[GameInput::kKey_c] = 46;
        s_DXKeyMapping[GameInput::kKey_v] = 47;
        s_DXKeyMapping[GameInput::kKey_b] = 48;
        s_DXKeyMapping[GameInput::kKey_n] = 49;
        s_DXKeyMapping[GameInput::kKey_m] = 50;
        s_DXKeyMapping[GameInput::kKey_comma] = 51;
        s_DXKeyMapping[GameInput::kKey_period] = 52;
        s_DXKeyMapping[GameInput::kKey_slash] = 53;
        s_DXKeyMapping[GameInput::kKey_rshift] = 54;
        s_DXKeyMapping[GameInput::kKey_multiply] = 55;
        s_DXKeyMapping[GameInput::kKey_lalt] = 56;
        s_DXKeyMapping[GameInput::kKey_space] = 57;
        s_DXKeyMapping[GameInput::kKey_capital] = 58;
        s_DXKeyMapping[GameInput::kKey_f1] = 59;
        s_DXKeyMapping[GameInput::kKey_f2] = 60;
        s_DXKeyMapping[GameInput::kKey_f3] = 61;
        s_DXKeyMapping[GameInput::kKey_f4] = 62;
        s_DXKeyMapping[GameInput::kKey_f5] = 63;
        s_DXKeyMapping[GameInput::kKey_f6] = 64;
        s_DXKeyMapping[GameInput::kKey_f7] = 65;
        s_DXKeyMapping[GameInput::kKey_f8] = 66;
        s_DXKeyMapping[GameInput::kKey_f9] = 67;
        s_DXKeyMapping[GameInput::kKey_f10] = 68;
        s_DXKeyMapping[GameInput::kKey_numlock] = 69;
        s_DXKeyMapping[GameInput::kKey_scroll] = 70;
        s_DXKeyMapping[GameInput::kKey_numpad7] = 71;
        s_DXKeyMapping[GameInput::kKey_numpad8] = 72;
        s_DXKeyMapping[GameInput::kKey_numpad9] = 73;
        s_DXKeyMapping[GameInput::kKey_subtract] = 74;
        s_DXKeyMapping[GameInput::kKey_numpad4] = 75;
        s_DXKeyMapping[GameInput::kKey_numpad5] = 76;
        s_DXKeyMapping[GameInput::kKey_numpad6] = 77;
        s_DXKeyMapping[GameInput::kKey_add] = 78;
        s_DXKeyMapping[GameInput::kKey_numpad1] = 79;
        s_DXKeyMapping[GameInput::kKey_numpad2] = 80;
        s_DXKeyMapping[GameInput::kKey_numpad3] = 81;
        s_DXKeyMapping[GameInput::kKey_numpad0] = 82;
        s_DXKeyMapping[GameInput::kKey_decimal] = 83;
        s_DXKeyMapping[GameInput::kKey_f11] = 87;
        s_DXKeyMapping[GameInput::kKey_f12] = 88;
        s_DXKeyMapping[GameInput::kKey_numpadenter] = 156;
        s_DXKeyMapping[GameInput::kKey_rcontrol] = 157;
        s_DXKeyMapping[GameInput::kKey_divide] = 181;
        s_DXKeyMapping[GameInput::kKey_sysrq] = 183;
        s_DXKeyMapping[GameInput::kKey_ralt] = 184;
        s_DXKeyMapping[GameInput::kKey_pause] = 197;
        s_DXKeyMapping[GameInput::kKey_home] = 199;
        s_DXKeyMapping[GameInput::kKey_up] = 200;
        s_DXKeyMapping[GameInput::kKey_pgup] = 201;
        s_DXKeyMapping[GameInput::kKey_left] = 203;
        s_DXKeyMapping[GameInput::kKey_right] = 205;
        s_DXKeyMapping[GameInput::kKey_end] = 207;
        s_DXKeyMapping[GameInput::kKey_down] = 208;
        s_DXKeyMapping[GameInput::kKey_pgdn] = 209;
        s_DXKeyMapping[GameInput::kKey_insert] = 210;
        s_DXKeyMapping[GameInput::kKey_delete] = 211;
        s_DXKeyMapping[GameInput::kKey_lwin] = 219;
        s_DXKeyMapping[GameInput::kKey_rwin] = 220;
        s_DXKeyMapping[GameInput::kKey_apps] = 221;
#else
#define WinRTKey(name) (unsigned char)Windows::System::VirtualKey::name
        s_DXKeyMapping[GameInput::kKey_escape] = WinRTKey(Escape);
        s_DXKeyMapping[GameInput::kKey_1] = WinRTKey(Number1);
        s_DXKeyMapping[GameInput::kKey_2] = WinRTKey(Number2);
        s_DXKeyMapping[GameInput::kKey_3] = WinRTKey(Number3);
        s_DXKeyMapping[GameInput::kKey_4] = WinRTKey(Number4);
        s_DXKeyMapping[GameInput::kKey_5] = WinRTKey(Number5);
        s_DXKeyMapping[GameInput::kKey_6] = WinRTKey(Number6);
        s_DXKeyMapping[GameInput::kKey_7] = WinRTKey(Number7);
        s_DXKeyMapping[GameInput::kKey_8] = WinRTKey(Number8);
        s_DXKeyMapping[GameInput::kKey_9] = WinRTKey(Number9);
        s_DXKeyMapping[GameInput::kKey_0] = WinRTKey(Number0);
        s_DXKeyMapping[GameInput::kKey_minus] = WinRTKey(Subtract);
        s_DXKeyMapping[GameInput::kKey_equals] = WinRTKey(Add);
        s_DXKeyMapping[GameInput::kKey_back] = WinRTKey(Back);
        s_DXKeyMapping[GameInput::kKey_tab] = WinRTKey(Tab);
        s_DXKeyMapping[GameInput::kKey_q] = WinRTKey(Q);
        s_DXKeyMapping[GameInput::kKey_w] = WinRTKey(W);
        s_DXKeyMapping[GameInput::kKey_e] = WinRTKey(E);
        s_DXKeyMapping[GameInput::kKey_r] = WinRTKey(R);
        s_DXKeyMapping[GameInput::kKey_t] = WinRTKey(T);
        s_DXKeyMapping[GameInput::kKey_y] = WinRTKey(Y);
        s_DXKeyMapping[GameInput::kKey_u] = WinRTKey(U);
        s_DXKeyMapping[GameInput::kKey_i] = WinRTKey(I);
        s_DXKeyMapping[GameInput::kKey_o] = WinRTKey(O);
        s_DXKeyMapping[GameInput::kKey_p] = WinRTKey(P);
        s_DXKeyMapping[GameInput::kKey_lbracket] = 219;
        s_DXKeyMapping[GameInput::kKey_rbracket] = 221;
        s_DXKeyMapping[GameInput::kKey_return] = WinRTKey(Enter);
        s_DXKeyMapping[GameInput::kKey_lcontrol] = WinRTKey(Control);  // No L/R
        s_DXKeyMapping[GameInput::kKey_a] = WinRTKey(A);
        s_DXKeyMapping[GameInput::kKey_s] = WinRTKey(S);
        s_DXKeyMapping[GameInput::kKey_d] = WinRTKey(D);
        s_DXKeyMapping[GameInput::kKey_f] = WinRTKey(F);
        s_DXKeyMapping[GameInput::kKey_g] = WinRTKey(G);
        s_DXKeyMapping[GameInput::kKey_h] = WinRTKey(H);
        s_DXKeyMapping[GameInput::kKey_j] = WinRTKey(J);
        s_DXKeyMapping[GameInput::kKey_k] = WinRTKey(K);
        s_DXKeyMapping[GameInput::kKey_l] = WinRTKey(L);
        s_DXKeyMapping[GameInput::kKey_semicolon] = 186;
        s_DXKeyMapping[GameInput::kKey_apostrophe] = 222;
        s_DXKeyMapping[GameInput::kKey_grave] = 192; // ` or ~
        s_DXKeyMapping[GameInput::kKey_lshift] = WinRTKey(LeftShift);
        s_DXKeyMapping[GameInput::kKey_backslash] = 220;
        s_DXKeyMapping[GameInput::kKey_z] = WinRTKey(Z);
        s_DXKeyMapping[GameInput::kKey_x] = WinRTKey(X);
        s_DXKeyMapping[GameInput::kKey_c] = WinRTKey(C);
        s_DXKeyMapping[GameInput::kKey_v] = WinRTKey(V);
        s_DXKeyMapping[GameInput::kKey_b] = WinRTKey(B);
        s_DXKeyMapping[GameInput::kKey_n] = WinRTKey(N);
        s_DXKeyMapping[GameInput::kKey_m] = WinRTKey(M);
        s_DXKeyMapping[GameInput::kKey_comma] = 188;
        s_DXKeyMapping[GameInput::kKey_period] = 190;
        s_DXKeyMapping[GameInput::kKey_slash] = 191;
        s_DXKeyMapping[GameInput::kKey_rshift] = WinRTKey(RightShift);
        s_DXKeyMapping[GameInput::kKey_multiply] = WinRTKey(Multiply);
        s_DXKeyMapping[GameInput::kKey_lalt] = 255; // Only a modifier
        s_DXKeyMapping[GameInput::kKey_space] = WinRTKey(Space);
        s_DXKeyMapping[GameInput::kKey_capital] = WinRTKey(CapitalLock);
        s_DXKeyMapping[GameInput::kKey_f1] = WinRTKey(F1);
        s_DXKeyMapping[GameInput::kKey_f2] = WinRTKey(F2);
        s_DXKeyMapping[GameInput::kKey_f3] = WinRTKey(F3);
        s_DXKeyMapping[GameInput::kKey_f4] = WinRTKey(F4);
        s_DXKeyMapping[GameInput::kKey_f5] = WinRTKey(F5);
        s_DXKeyMapping[GameInput::kKey_f6] = WinRTKey(F6);
        s_DXKeyMapping[GameInput::kKey_f7] = WinRTKey(F7);
        s_DXKeyMapping[GameInput::kKey_f8] = WinRTKey(F8);
        s_DXKeyMapping[GameInput::kKey_f9] = WinRTKey(F9);
        s_DXKeyMapping[GameInput::kKey_f10] = WinRTKey(F10);
        s_DXKeyMapping[GameInput::kKey_numlock] = WinRTKey(NumberKeyLock);
        s_DXKeyMapping[GameInput::kKey_scroll] = WinRTKey(Scroll);
        s_DXKeyMapping[GameInput::kKey_numpad7] = WinRTKey(NumberPad7);
        s_DXKeyMapping[GameInput::kKey_numpad8] = WinRTKey(NumberPad8);
        s_DXKeyMapping[GameInput::kKey_numpad9] = WinRTKey(NumberPad9);
        s_DXKeyMapping[GameInput::kKey_subtract] = WinRTKey(Subtract);
        s_DXKeyMapping[GameInput::kKey_numpad4] = WinRTKey(NumberPad4);
        s_DXKeyMapping[GameInput::kKey_numpad5] = WinRTKey(NumberPad5);
        s_DXKeyMapping[GameInput::kKey_numpad6] = WinRTKey(NumberPad6);
        s_DXKeyMapping[GameInput::kKey_add] = WinRTKey(Add);
        s_DXKeyMapping[GameInput::kKey_numpad1] = WinRTKey(NumberPad1);
        s_DXKeyMapping[GameInput::kKey_numpad2] = WinRTKey(NumberPad2);
        s_DXKeyMapping[GameInput::kKey_numpad3] = WinRTKey(NumberPad3);
        s_DXKeyMapping[GameInput::kKey_numpad0] = WinRTKey(NumberPad0);
        s_DXKeyMapping[GameInput::kKey_decimal] = WinRTKey(Decimal);
        s_DXKeyMapping[GameInput::kKey_f11] = WinRTKey(F11);
        s_DXKeyMapping[GameInput::kKey_f12] = WinRTKey(F12);
        s_DXKeyMapping[GameInput::kKey_numpadenter] = WinRTKey(Enter); // No distinction
        s_DXKeyMapping[GameInput::kKey_rcontrol] = WinRTKey(Control);  // No L/R
        s_DXKeyMapping[GameInput::kKey_divide] = WinRTKey(Divide);
        s_DXKeyMapping[GameInput::kKey_sysrq] = 255; // Ignored
        s_DXKeyMapping[GameInput::kKey_ralt] = 255; // Only a modifier
        s_DXKeyMapping[GameInput::kKey_pause] = WinRTKey(Pause);
        s_DXKeyMapping[GameInput::kKey_home] = WinRTKey(Home);
        s_DXKeyMapping[GameInput::kKey_up] = WinRTKey(Up);
        s_DXKeyMapping[GameInput::kKey_pgup] = WinRTKey(PageUp);
        s_DXKeyMapping[GameInput::kKey_left] = WinRTKey(Left);
        s_DXKeyMapping[GameInput::kKey_right] = WinRTKey(Right);
        s_DXKeyMapping[GameInput::kKey_end] = WinRTKey(End);
        s_DXKeyMapping[GameInput::kKey_down] = WinRTKey(Down);
        s_DXKeyMapping[GameInput::kKey_pgdn] = WinRTKey(PageDown);
        s_DXKeyMapping[GameInput::kKey_insert] = WinRTKey(Insert);
        s_DXKeyMapping[GameInput::kKey_delete] = WinRTKey(Delete);
        s_DXKeyMapping[GameInput::kKey_lwin] = WinRTKey(LeftWindows);
        s_DXKeyMapping[GameInput::kKey_rwin] = WinRTKey(RightWindows);
        s_DXKeyMapping[GameInput::kKey_apps] = WinRTKey(Application);
#endif
    }

    void KbmZeroInputs()
    {
        memset(&s_MouseState, 0, sizeof(DIMOUSESTATE2));
        memset(s_Keybuffer, 0, sizeof(s_Keybuffer));
    }

    void KbmInitialize()
    {
        KbmBuildKeyMapping();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&s_DI, nullptr)))
            ThrowIfFalse(false, L"DirectInput8 initialization failed.");

        if (FAILED(s_DI->CreateDevice(GUID_SysKeyboard, &s_Keyboard, nullptr)))
            ThrowIfFalse(false, L"Keyboard CreateDevice failed.");
        if (FAILED(s_Keyboard->SetDataFormat(&c_dfDIKeyboard)))
            ThrowIfFalse(false, L"Keyboard SetDataFormat failed.");
        if (FAILED(s_Keyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            ThrowIfFalse(false, L"Keyboard SetCooperativeLevel failed.");

        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData = 10;
        if (FAILED(s_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
            ThrowIfFalse(false, L"Keyboard set buffer size failed.");

        if (FAILED(s_DI->CreateDevice(GUID_SysMouse, &s_Mouse, nullptr)))
            ThrowIfFalse(false, L"Mouse CreateDevice failed.");
        if (FAILED(s_Mouse->SetDataFormat(&c_dfDIMouse2)))
            ThrowIfFalse(false, L"Mouse SetDataFormat failed.");
        if (FAILED(s_Mouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
            ThrowIfFalse(false, L"Mouse SetCooperativeLevel failed.");
#endif

        KbmZeroInputs();
    }

    void KbmShutdown()
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
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
#endif
    }

    void KbmUpdate()
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        HWND foreground = GetForegroundWindow();
        bool visible = IsWindowVisible(foreground) != 0;

        if (foreground != g_hWnd // wouldn't be able to acquire
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
#endif
    }

#endif

}

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_TV_TITLE)
void GameInput::SetKeyState(Windows::System::VirtualKey key, bool IsDown)
{
    s_Keybuffer[(unsigned char)key] = IsDown ? 0x80 : 0x00;
    //DEBUGPRINT("%d key is %s", (unsigned int)key, IsDown ? L"down" : L"up");
}
#endif

void GameInput::Initialize()
{
    // For Windows 8
    //	XInputEnable(TRUE);

    ZeroMemory( s_Buttons, sizeof(s_Buttons) );
    ZeroMemory( s_Analogs, sizeof(s_Analogs) );

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

#ifdef USE_XINPUT
    XINPUT_STATE newInputState;
    if (ERROR_SUCCESS == XInputGetState( 0, &newInputState ))
    {
        if (newInputState.Gamepad.wButtons & (1 << 0)) s_Buttons[0][kDPadUp] = true;
        if (newInputState.Gamepad.wButtons & (1 << 1)) s_Buttons[0][kDPadDown] = true;
        if (newInputState.Gamepad.wButtons & (1 << 2)) s_Buttons[0][kDPadLeft] = true;
        if (newInputState.Gamepad.wButtons & (1 << 3)) s_Buttons[0][kDPadRight] = true;
        if (newInputState.Gamepad.wButtons & (1 << 4)) s_Buttons[0][kStartButton] = true;
        if (newInputState.Gamepad.wButtons & (1 << 5)) s_Buttons[0][kBackButton] = true;
        if (newInputState.Gamepad.wButtons & (1 << 6)) s_Buttons[0][kLThumbClick] = true;
        if (newInputState.Gamepad.wButtons & (1 << 7)) s_Buttons[0][kRThumbClick] = true;
        if (newInputState.Gamepad.wButtons & (1 << 8)) s_Buttons[0][kLShoulder] = true;
        if (newInputState.Gamepad.wButtons & (1 << 9)) s_Buttons[0][kRShoulder] = true;
        if (newInputState.Gamepad.wButtons & (1 << 12)) s_Buttons[0][kAButton] = true;
        if (newInputState.Gamepad.wButtons & (1 << 13)) s_Buttons[0][kBButton] = true;
        if (newInputState.Gamepad.wButtons & (1 << 14)) s_Buttons[0][kXButton] = true;
        if (newInputState.Gamepad.wButtons & (1 << 15)) s_Buttons[0][kYButton] = true;

        s_Analogs[ kAnalogLeftTrigger ]		= newInputState.Gamepad.bLeftTrigger / 255.0f;
        s_Analogs[ kAnalogRightTrigger ]	= newInputState.Gamepad.bRightTrigger / 255.0f;
        s_Analogs[ kAnalogLeftStickX ]		= FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
        s_Analogs[ kAnalogLeftStickY ]		= FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
        s_Analogs[ kAnalogRightStickX ]		= FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
        s_Analogs[ kAnalogRightStickY ]		= FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
    }
#else

    IVectorView<Gamepad^>^ gamepads = Gamepad::Gamepads;
    if (gamepads->Size != 0)
    {
        IGamepad^ gamepad = gamepads->GetAt(0);
        GamepadReading reading = gamepad->GetCurrentReading();
        uint32_t Buttons = (uint32_t)reading.Buttons;
        if (Buttons & (uint32_t)GamepadButtons::DPadUp) s_Buttons[0][kDPadUp] = true;
        if (Buttons & (uint32_t)GamepadButtons::DPadDown) s_Buttons[0][kDPadDown] = true;
        if (Buttons & (uint32_t)GamepadButtons::DPadLeft) s_Buttons[0][kDPadLeft] = true;
        if (Buttons & (uint32_t)GamepadButtons::DPadRight) s_Buttons[0][kDPadRight] = true;
        if (Buttons & (uint32_t)GamepadButtons::Menu) s_Buttons[0][kStartButton] = true;
        if (Buttons & (uint32_t)GamepadButtons::View) s_Buttons[0][kBackButton] = true;
        if (Buttons & (uint32_t)GamepadButtons::LeftThumbstick) s_Buttons[0][kLThumbClick] = true;
        if (Buttons & (uint32_t)GamepadButtons::RightThumbstick) s_Buttons[0][kRThumbClick] = true;
        if (Buttons & (uint32_t)GamepadButtons::LeftShoulder) s_Buttons[0][kLShoulder] = true;
        if (Buttons & (uint32_t)GamepadButtons::RightShoulder) s_Buttons[0][kRShoulder] = true;
        if (Buttons & (uint32_t)GamepadButtons::A) s_Buttons[0][kAButton] = true;
        if (Buttons & (uint32_t)GamepadButtons::B) s_Buttons[0][kBButton] = true;
        if (Buttons & (uint32_t)GamepadButtons::X) s_Buttons[0][kXButton] = true;
        if (Buttons & (uint32_t)GamepadButtons::Y) s_Buttons[0][kYButton] = true;

        static const float kAnalogStickDeadZone = 0.18f;

        s_Analogs[ kAnalogLeftTrigger ]		= (float)reading.LeftTrigger;
        s_Analogs[ kAnalogRightTrigger ]	= (float)reading.RightTrigger;
        s_Analogs[ kAnalogLeftStickX ]		= FilterAnalogInput((float)reading.LeftThumbstickX, kAnalogStickDeadZone );
        s_Analogs[ kAnalogLeftStickY ]		= FilterAnalogInput((float)reading.LeftThumbstickY, kAnalogStickDeadZone );
        s_Analogs[ kAnalogRightStickX ]		= FilterAnalogInput((float)reading.RightThumbstickX, kAnalogStickDeadZone );
        s_Analogs[ kAnalogRightStickY ]		= FilterAnalogInput((float)reading.RightThumbstickY, kAnalogStickDeadZone );
    }

#endif

#ifdef USE_KEYBOARD_MOUSE
    KbmUpdate();

    for (uint32_t i = 0; i < kNumKeys; ++i)
    {
        s_Buttons[0][i] = (s_Keybuffer[s_DXKeyMapping[i]] & 0x80) != 0;
    }

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
