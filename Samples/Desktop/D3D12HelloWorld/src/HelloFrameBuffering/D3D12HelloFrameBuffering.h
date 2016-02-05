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

class D3D12HelloFrameBuffering : public DXSample
{
public:
	D3D12HelloFrameBuffering(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	// In this sample we overload the meaning of FrameCount to mean both the maximum
	// number of frames that will be queued to the GPU at a time, as well as the number
	// of back buffers in the DXGI swap chain. For the majority of applications, this
	// is convenient and works well. However, there will be certain cases where an
	// application may want to queue up more frames than there are back buffers
	// available.
	// It should be noted that excessive buffering of frames dependent on user input
	// may result in noticeable latency in your app.
	static const UINT FrameCount = 2;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void MoveToNextFrame();
	void WaitForGpu();
};
