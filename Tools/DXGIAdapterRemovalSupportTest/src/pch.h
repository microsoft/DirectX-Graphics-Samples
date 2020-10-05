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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <atlbase.h>

#include <d3dkmthk.h>
#include <dxgi1_6.h>
#include <psapi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <ntddvdeo.h>
#include <devpkey.h>

#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <memory>
#include <algorithm>
#include <thread>

#include "TraceSession.h"
#include "PnpListener.h"

template <typename T>
struct ScopeExit
{
    ScopeExit(T &&f) : f(std::forward<T>(f)) {}
    ~ScopeExit() { f(); }
    T f;
};

template <typename T>
inline ScopeExit<T> MakeScopeExit(T &&f)
{
    return ScopeExit<T>(std::forward<T>(f));
};

struct OutputColor
{
    OutputColor()
    {
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hStdout, &csbi);
        StdoutBaseColor = csbi.wAttributes & 0xFFFFFFF8; // Filter out just the FOREGROUND_* colors
    }

    void SetRedColor()
    {
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY | StdoutBaseColor);
    }

    void SetGreenColor()
    {
        SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY | StdoutBaseColor);
    }

    void SetYellowColor()
    {
        SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | StdoutBaseColor);
    }

    void SetDefaultColor()
    {
        SetConsoleTextAttribute(hStdout, csbi.wAttributes);
    }

private:
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdout;
    WORD StdoutBaseColor;
};
extern OutputColor g_OutClr;

class ScopedOutputColor
{
public:
    enum COLOR
    {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW
    };
    ScopedOutputColor(COLOR clr)
    {
        switch (clr)
        {
        case COLOR_RED: g_OutClr.SetRedColor(); break;
        case COLOR_GREEN: g_OutClr.SetGreenColor(); break;
        case COLOR_YELLOW: g_OutClr.SetYellowColor(); break;
        }
    }
    ~ScopedOutputColor()
    {
        g_OutClr.SetDefaultColor();
    }
};

