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
#include "D3D12RaytracingProceduralGeometry.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // ToDo
	D3D12RaytracingProceduralGeometry sample(1280, 720, L"D3D12 Raytracing - Procedural Geometry");
    //D3D12RaytracingProceduralGeometry sample(1034,1282, L"D3D12 Raytracing - Procedural Geometry");
    //D3D12RaytracingProceduralGeometry sample(1280/2, 720/2, L"D3D12 Raytracing - Procedural Geometry");
    //D3D12RaytracingProceduralGeometry sample(1280/4, 720/4, L"D3D12 Raytracing - Procedural Geometry");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}
