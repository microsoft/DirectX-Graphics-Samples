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

// Illustrate how small texture resources can be packed into 64K pages for more
// efficient memory usage.
class D3D12SmallResources : public DXSample
{
public:
	D3D12SmallResources(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual void OnKeyDown(UINT8 key);

private:
	static const UINT FrameCount = 2;
	static const UINT GridWidth = 11;
	static const UINT GridHeight = 7;
	static const UINT TextureCount = GridWidth * GridHeight;
	static const UINT TextureWidth = 32;
	static const UINT TextureHeight = 32;
	static const UINT TexturePixelSizeInBytes = 4;

	// Vertex definition.
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<IDXGIAdapter3> m_adapter;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_copyCommandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandQueue> m_copyQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_srvDescriptorSize;
	UINT m_frameIndex;

	// Synchronization objects.
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];
	HANDLE m_fenceEvent;

	// Asset objects.
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12GraphicsCommandList> m_copyCommandList;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	std::vector<ComPtr<ID3D12Resource>> m_textures;
	ComPtr<ID3D12Heap> m_textureHeap;		// Only used when the resources being drawn are placed resources.
	bool m_usePlacedResources;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	void LoadPipeline();
	void LoadAssets();
	std::vector<UINT8> GenerateTexture();
	void CreateTextures();
	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
};
