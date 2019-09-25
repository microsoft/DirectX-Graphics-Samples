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
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "Sampler.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // Initialization For WICTextureLoader.
    ThrowIfFailed(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED), L"Failed to initialize WIC component");

    // ToDoF default 1080p full res
    D3D12RaytracingRealTimeDenoisedAmbientOcclusion sample(3840, 2160, L"D3D12 Raytracing - Real-Time Denoised Raytraced Ambient Occlusion");
    //D3D12RaytracingRealTimeDenoisedAmbientOcclusion sample(1920, 2160, L"D3D12 Raytracing - Real-Time Denoised Raytraced Ambient Occlusion");
    //D3D12RaytracingRealTimeDenoisedAmbientOcclusion sample(1280, 720, L"D3D12 Raytracing - Real-Time Denoised Raytraced Ambient Occlusion");
    //D3D12RaytracingRealTimeDenoisedAmbientOcclusion sample(1280, 720, L"D3D12 Raytracing - Real-Time Denoised Raytraced Ambient Occlusion");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}