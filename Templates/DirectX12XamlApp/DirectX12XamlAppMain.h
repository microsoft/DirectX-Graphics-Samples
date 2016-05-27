#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"

// Renders Direct3D content on the screen.
namespace DirectX12XamlApp
{
	class DirectX12XamlAppMain
	{
	public:
		DirectX12XamlAppMain();
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void StartTracking()					{ m_sceneRenderer->StartTracking(); }
		void TrackingUpdate(float positionX)	{ m_pointerLocationX = positionX; }
		void StopTracking()						{ m_sceneRenderer->StopTracking(); }
		bool IsTracking()						{ return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
	};
}