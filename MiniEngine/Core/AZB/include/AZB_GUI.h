#pragma once
//===============================================================================
// desc: A UI superclass that utilises ImGui to create a functional UI for education and performance profiling
// auth: Aliyaan Zulfiqar
//===============================================================================
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "implot.h"
//===============================================================================

class GUI
{
public:
	GUI() {}

	// Get ImGui started!
	void Init(void* Hwnd, ID3D12Device* pDevice, int numFramesInFlight, const DXGI_FORMAT& renderTargetFormat);

	// Run ImGui render loop!
	void Run();

	// Shutdown ImGui safely
	void Terminate();

	ID3D12DescriptorHeap* m_pSrvDescriptorHeap = nullptr;

	//tmp 
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

};