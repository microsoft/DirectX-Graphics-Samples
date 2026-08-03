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

#include "stdafx.h"
#include "D3D12RaytracingClusteredGeometry.h"

// Agility SDK loader hooks. The system d3d12.dll uses these exports to find the
// matching D3D12Core.dll at runtime. The DLL is copied into bin\<cfg>\D3D12\
// by ExperimentalD3D12.props.
//
// The new experimental D3D12Core (post-17:37 build, fixing the NVIDIA caps
// regression) loads via the release-SDK loader path, not the preview one --
// so we export 721 directly rather than D3D12_PREVIEW_SDK_VERSION (which
// today happens to be the same number but routes through the preview-only
// loader paths). When a real Agility SDK NuGet package shipping DXR2 ships,
// swap this back to D3D12_SDK_VERSION.
extern "C" { __declspec(dllexport) extern const UINT  D3D12SDKVersion = 721; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath    = ".\\D3D12\\"; }

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    SampleLog::Init();
    SampleLog::LogF(L"=== D3D12RaytracingClusteredGeometry starting (sdk=%u, preview=%u) ===\n",
                    (unsigned)D3D12_SDK_VERSION, (unsigned)D3D12_PREVIEW_SDK_VERSION);

    D3D12RaytracingClusteredGeometry sample(1280, 720, L"D3D12 Raytracing - Clustered Geometry");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}
