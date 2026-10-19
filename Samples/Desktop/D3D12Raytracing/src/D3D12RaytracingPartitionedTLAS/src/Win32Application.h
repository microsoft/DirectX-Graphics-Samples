//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <windows.h>
#include <functional>

class Win32Application
{
public:
    static HWND Create(HINSTANCE instance, WNDPROC windowProc, void* sample, UINT width, UINT height);
    static void Run(const std::function<void()>& render, const std::function<bool()>& minimized);
    static void ToggleFullscreen(HWND window);
};
