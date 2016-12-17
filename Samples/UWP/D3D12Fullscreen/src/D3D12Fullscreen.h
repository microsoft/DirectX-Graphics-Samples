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

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

// Illustrate how to handle window resizing and fullscreen mode.
class D3D12Fullscreen : public DXSample
{
public:
	D3D12Fullscreen(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual void OnDestroy();
	virtual void OnKeyDown(UINT8 key);

private:
	static const UINT FrameCount = 2;
	static const float QuadWidth;
	static const float QuadHeight;
	static const float LetterboxColor[4];
	static const float ClearColor[4];

	struct SceneVertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct PostVertex
	{
		XMFLOAT4 position;
		XMFLOAT2 uv;
	};

	struct SceneConstantBuffer
	{
		XMFLOAT4X4 transform;
		XMFLOAT4 offset;
		UINT padding[44];
	}; 
	
	struct Resolution
	{
		UINT Width;
		UINT Height;
	};

	static const Resolution m_resolutionOptions[];
	static const UINT m_resolutionOptionsCount;
	static UINT m_resolutionIndex; // Index of the current scene rendering resolution from m_resolutionOptions.

	// Pipeline objects.
	CD3DX12_VIEWPORT m_sceneViewport;
	CD3DX12_VIEWPORT m_postViewport;
	CD3DX12_RECT m_sceneScissorRect;
	CD3DX12_RECT m_postScissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> m_intermediateRenderTarget;
	ComPtr<ID3D12CommandAllocator> m_sceneCommandAllocators[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_postCommandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_sceneRootSignature;
	ComPtr<ID3D12RootSignature> m_postRootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
	ComPtr<ID3D12PipelineState> m_scenePipelineState;
	ComPtr<ID3D12PipelineState> m_postPipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_sceneCommandList;
	ComPtr<ID3D12GraphicsCommandList> m_postCommandList;
	UINT m_rtvDescriptorSize;
	UINT m_cbvSrvDescriptorSize;

	// App resources.
	ComPtr<ID3D12Resource> m_sceneVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_sceneVertexBufferView;
	ComPtr<ID3D12Resource> m_postVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_postVertexBufferView;
	ComPtr<ID3D12Resource> m_sceneConstantBuffer;
	SceneConstantBuffer m_sceneConstantBufferData;
	UINT8* m_pCbvDataBegin;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	// Track the state of the window.
	// If it's minimized the app may decide not to render frames.
	bool m_windowVisible;
	bool m_windowedMode;

	void LoadPipeline();
	void LoadAssets();
	void LoadSizeDependentResources();
	void LoadSceneResolutionDependentResources();
	void PopulateCommandLists();
	void WaitForGpu();
	void MoveToNextFrame();
	void UpdatePostViewAndScissor();
	void UpdateTitle();
};
