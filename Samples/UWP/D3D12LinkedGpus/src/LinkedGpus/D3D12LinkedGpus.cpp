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
#include "D3D12LinkedGpus.h"

using namespace DirectX;

D3D12LinkedGpus::D3D12LinkedGpus(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_nodeIndex(0),
	m_frameId(0),
	m_simulatedGpuLoad(0x1000),
	m_syncInterval(0),
	m_windowVisible(true),
	m_windowedMode(true)
{
	m_sceneData.resize(Settings::TriangleCount);
}

void D3D12LinkedGpus::OnInit()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<ID3D12Device> device;
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device)
			));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device)
			));
	}

	// Query capabilities of the device to configure the sample.
	Settings::Initialize(device.Get(), m_width, m_height, m_tearingSupport);

	// Initialize resources for the sample.
	m_crossNodeResources = std::make_shared<CrossNodeResources>(factory.Get(), device.Get());

	for (UINT n = 0; n < Settings::NodeCount; n++)
	{
		m_nodes[n] = std::make_shared<GpuNode>(n, m_crossNodeResources);
	}

	for (UINT n = 0; n < Settings::NodeCount; n++)
	{
		m_nodes[n]->LinkSharedResources(m_nodes, Settings::NodeCount);
	}

	LoadSceneData();
	UpdateWindowTitle();
}

// Generate data for the scene triangles.
void D3D12LinkedGpus::LoadSceneData()
{
	for (UINT n = 0; n < Settings::TriangleCount; n++)
	{
		m_sceneData[n].velocity = XMFLOAT4(GetRandomFloat(0.005f, 0.01f), 0.0f, 0.0f, 0.0f);
		m_sceneData[n].offset = XMFLOAT4(GetRandomFloat(-6.0f, -1.5f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
		m_sceneData[n].color = XMFLOAT4(GetRandomFloat(0.4f, 0.9f), GetRandomFloat(0.4f, 0.9f), GetRandomFloat(0.4f, 0.9f), 1.0f);
		m_sceneData[n].projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.01f, 20.0f));
	}
}

// Get a random float value between min and max.
float D3D12LinkedGpus::GetRandomFloat(float min, float max)
{
	float scale = static_cast<float>(rand()) / RAND_MAX;
	float range = max - min;
	return scale * range + min;
}

void D3D12LinkedGpus::UpdateWindowTitle()
{
	WCHAR nodeText[100];
	if (Settings::NodeCount == 1)
	{
		swprintf_s(nodeText, L"Node Count = 1 | SyncInterval = %u | SimulatedLoad = %u", m_syncInterval, m_simulatedGpuLoad);
	}
	else
	{
		swprintf_s(
			nodeText,
			L"Node Count = %u | Cross Node Sharing Tier = %d | SyncInterval = %u | SimulatedLoad = %u",
			Settings::NodeCount,
			Settings::Tier2Support ? 2 : 1,
			m_syncInterval,
			m_simulatedGpuLoad);
	}
	SetCustomWindowText(nodeText);
}

// Update frame-based values.
void D3D12LinkedGpus::OnUpdate()
{
	for (UINT n = 0; n < Settings::TriangleCount; n++)
	{
		const float offsetBounds = 1.5f * m_aspectRatio;
	
		// Animate the triangles.
		m_sceneData[n].offset.x += m_sceneData[n].velocity.x;
		if (m_sceneData[n].offset.x > offsetBounds)
		{
			m_sceneData[n].velocity.x = GetRandomFloat(0.005f, 0.01f);
			m_sceneData[n].offset.x = -offsetBounds;
		}
	}

	m_nodes[m_nodeIndex]->OnUpdate(m_sceneData.data(), Settings::TriangleCount, m_frameId);
}

// Render the scene.
void D3D12LinkedGpus::OnRender()
{
	if (m_windowVisible)
	{
		auto currentNode = m_nodes[m_nodeIndex];

		// Render and present the current frame.
		currentNode->RenderScene(m_frameId, m_simulatedGpuLoad);
		currentNode->RenderPost(m_frameId);
		currentNode->Present(m_syncInterval, m_windowedMode);

		MoveToNextFrame();
	}
}

void D3D12LinkedGpus::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	// Determine if the swap buffers and other resources need to be resized or not.
	if ((width != m_width || height != m_height) && !minimized)
	{
		// Flush all current GPU commands.
		WaitForGpus();

		// Release buffers tied to the swap chain and update the global Settings
		// for the new size.
		for (UINT n = 0; n < Settings::NodeCount; n++)
		{
			m_nodes[n]->ReleaseBackBuffers();
		}
		Settings::OnSizeChanged(width, height);

		// Resize the swap chain to the desired dimensions.
		m_crossNodeResources->ResizeSwapChain();

		// Re-create and re-link render targets.
		for (UINT n = 0; n < Settings::NodeCount; n++)
		{
			m_nodes[n]->LoadSizeDependentResources();
		}
		for (UINT n = 0; n < Settings::NodeCount; n++)
		{
			m_nodes[n]->LinkSharedResources(m_nodes, Settings::NodeCount);
		}

		// Reset the node index and align the frameId so that assumptions in post-processing hold.
		m_nodeIndex = m_crossNodeResources->GetSwapChain()->GetCurrentBackBufferIndex() % Settings::NodeCount;
		m_frameId += (Settings::SceneHistoryCount - (m_frameId % Settings::SceneHistoryCount));

		// Update the m_width, m_height, and m_aspectRatio member variables.
		UpdateForSizeChange(width, height);

		// Update the scene projection matrix.
		for (UINT n = 0; n < Settings::TriangleCount; n++)
		{
			m_sceneData[n].projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.01f, 20.0f));
		}
	}

	m_windowVisible = !minimized;
}

void D3D12LinkedGpus::OnKeyDown(UINT8 key)
{
	switch (key)
	{
	// Instrument the Space Bar to toggle between fullscreen states.
	// The CoreWindow will fire a SizeChanged event once the window is in the
	// fullscreen state. At that point, the IDXGISwapChain should be resized
	// to match the new window size.
	case VK_SPACE:
	{
		auto applicationView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
		if (applicationView->IsFullScreenMode)
		{
			applicationView->ExitFullScreenMode();
		}
		else
		{
			applicationView->TryEnterFullScreenMode();
		}
		break;
	}

	case VK_LEFT:
	case VK_RIGHT:
		m_syncInterval = (m_syncInterval > 0) ? 0 : 1;
		UpdateWindowTitle();
		break;

	case VK_UP:
		if (m_simulatedGpuLoad > 0)
		{
			if (m_simulatedGpuLoad < 0x80000)
			{
				m_simulatedGpuLoad <<= 1;
			}
		}
		else
		{
			m_simulatedGpuLoad = 1;
		}
		UpdateWindowTitle();
		break;

	case VK_DOWN:
		if (m_simulatedGpuLoad != 0)
		{
			m_simulatedGpuLoad >>= 1;
		}
		UpdateWindowTitle();
		break;
	}
}

void D3D12LinkedGpus::OnDestroy()
{
	// Wait for the GPUs to be done with all their resources.
	WaitForGpus();
}

// Wait for pending GPU work to complete.
void D3D12LinkedGpus::WaitForGpus()
{
	for (UINT n = 0; n < Settings::NodeCount; n++)
	{
		m_nodes[n]->WaitForGpu();
	}
}

// Prepare to render the next frame.
void D3D12LinkedGpus::MoveToNextFrame()
{
	UINT nextNode = m_crossNodeResources->GetSwapChain()->GetCurrentBackBufferIndex() % Settings::NodeCount;

	if (Settings::NodeCount > 1 && nextNode == m_nodeIndex)
	{
		// If a WM_SIZE event was processed during Present, we will already be in the
		// correct state. Do not update the node index or advance the node's frame.
	}
	else
	{
		// Advance the frame on the current node.
		m_nodes[m_nodeIndex]->MoveToNextFrame();

		// Advance to the next node.
		m_nodeIndex = nextNode;

		m_frameId++;
	}
}
