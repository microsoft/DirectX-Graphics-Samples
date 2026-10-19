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

#include "D3D12RaytracingPartitionedTLAS.h"
#include <cwchar>
#include <fstream>
#include <stdexcept>

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_PREVIEW_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    try
    {
        D3D12RaytracingPartitionedTLAS sample;
        sample.ParseCommandLineArgs();
        sample.Run(instance, showCommand);
        return 0;
    }
    catch (const std::exception& error)
    {
        std::ofstream("dxr2_error.log") << error.what() << '\n';
        OutputDebugStringA(error.what());
        if (std::wcsstr(GetCommandLineW(), L"--frames") == nullptr)
            MessageBoxA(nullptr, error.what(), "DXR 2 PTLAS error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
