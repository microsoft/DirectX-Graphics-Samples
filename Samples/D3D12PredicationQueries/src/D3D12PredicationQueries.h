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

using namespace Microsoft::WRL;
using namespace DirectX;

// Illustrate how queries are used in conjunction with predication.
class D3D12PredicationQueries : public DXSample
{
public:
	D3D12PredicationQueries(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	static const UINT FrameCount = 2;
	static const UINT CbvCountPerFrame = 2;

	// Vertex definition.
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	// Constant buffer definition.
	struct ConstantBufferData
	{
		XMFLOAT4 offset;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
		// to be array-indexed.
		FLOAT padding[60];
	};

	// Each geometry gets its own constant buffer.
	ConstantBufferData m_constantBufferData[CbvCountPerFrame];
	UINT8* m_pCbvDataBegin;

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
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12QueryHeap> m_queryHeap;
	UINT m_rtvDescriptorSize;
	UINT m_cbvSrvDescriptorSize;
	UINT m_frameIndex;

	// Synchronization objects.
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];
	HANDLE m_fenceEvent;

	// Asset objects.
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_queryState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_constantBuffer;
	ComPtr<ID3D12Resource> m_depthStencil;
	ComPtr<ID3D12Resource> m_queryResult;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
};
