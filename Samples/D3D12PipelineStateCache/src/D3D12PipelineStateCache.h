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

#pragma once
#include "DXSample.h"
#include "SimpleCamera.h"
#include "StepTimer.h"
#include "PSOLibrary.h"
#include "DynamicConstantBuffer.h"

using namespace Microsoft::WRL;
using namespace DirectX;

class D3D12PipelineStateCache : public DXSample
{
public:
	D3D12PipelineStateCache(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual bool OnEvent(MSG msg);

private:
	static const UINT FrameCount = 2;
	static const float IntermediateClearColor[4];

	struct VertexPositionColor
	{
		XMFLOAT4 position;
		XMFLOAT3 color;
	};

	struct VertexPositionUV
	{
		XMFLOAT4 position;
		XMFLOAT2 uv;
	};

	struct DrawConstantBuffer
	{
		XMMATRIX worldViewProjection;
	};

	// Indices in the root parameter table.
	enum RootParameters : UINT32
	{
		RootParameterUberShaderCB = 0,
		RootParameterCB,
		RootParameterSRV,
		RootParametersCount
	};

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> m_intermediateRenderTarget;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_srvDescriptorSize;
	UINT m_frameIndex;

	// Asset objects.
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_vertexIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_cubeVbv;
	D3D12_VERTEX_BUFFER_VIEW m_quadVbv;
	D3D12_INDEX_BUFFER_VIEW m_cubeIbv;
	bool m_enabledEffects[EffectPipelineTypeCount];

	// Synchronization objects.
	HANDLE m_swapChainEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];
	HANDLE m_fenceEvent;

	UINT m_drawIndex;
	UINT m_maxDrawsPerFrame;

	SimpleCamera m_camera;
	XMMATRIX m_projectionMatrix;
	StepTimer m_timer;
	PSOLibrary m_psoLibrary;
	DynamicConstantBuffer m_dynamicCB;

	inline float GetRandomColor() { return (rand() % 100) / 100.0f; }

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void ToggleEffect(EffectPipelineType type);
	void OnKeyUp(WPARAM key);
	void UpdateWindowTextPso();
	void WaitForGpu();
	void MoveToNextFrame();
};
