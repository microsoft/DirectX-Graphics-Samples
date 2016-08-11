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
#include "D3D12PipelineStateCache.h"

FILE* CreateConsoleAndPrintDemoInformation()
{
	AllocConsole();
	FILE *pStreamOut = nullptr;
	freopen_s(&pStreamOut, "CONOUT$", "w", stdout);

	cout << "=== D3D12 Pipeline State Cache Sample ===" << endl << endl;
	cout << "\t Input Keys:" << endl;
	cout << "\t\t U : Toggle Uber Shader usage (rendering will appear dim when in use)" << endl;
	cout << "\t\t L : Toggle Pipeline State library enable (disk cache)" << endl;
	cout << "\t\t M : Toggle PSO caching mechanism" << endl;
	cout << "\t\t C : Clear app PSO's and disk cache" << endl;
	cout << "\t\t 1 : Toggle Blit effect" << endl;
	cout << "\t\t 2 : Toggle Invert effect" << endl;
	cout << "\t\t 3 : Toggle Grayscale effect" << endl;
	cout << "\t\t 4 : Toggle Edge Detect effect" << endl;
	cout << "\t\t 5 : Toggle Blur effect" << endl;
	cout << "\t\t 6 : Toggle Warp effect" << endl;
	cout << "\t\t 7 : Toggle Pixelate effect" << endl;
	cout << "\t\t 8 : Toggle Distort effect" << endl;
	cout << "\t\t 9 : Toggle Wave effect" << endl;

	return pStreamOut;
}

void DestroyConsole(FILE* pStream)
{
	fclose(pStream);
	FreeConsole();
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	D3D12PipelineStateCache sample(1280, 720, L"D3D12 Pipeline State Object Cache Sample");

	FILE* pStreamOut = CreateConsoleAndPrintDemoInformation();
	int result = Win32Application::Run(&sample, hInstance, nCmdShow);
	DestroyConsole(pStreamOut);

	return result;
}
