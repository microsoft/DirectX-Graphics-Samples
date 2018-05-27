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
#include "D3D12RaytracingDynamicGeometry.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	// ToDo
#if 0
	D3D12RaytracingDynamicGeometry sample(1280, 720, L"D3D12 Raytracing - Dynamic Geometry");
#else
	D3D12RaytracingDynamicGeometry sample(600, 300, L"D3D12 Raytracing - Dynamic Geometry");
#endif
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}
