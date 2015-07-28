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
#include "StepTimer.h"
#include "FrameResource.h"
#include "SimpleCamera.h"

using namespace DirectX;
using namespace std;
using namespace Microsoft::WRL;

class D3D12DynamicIndexing : public DXSample
{
public:
	D3D12DynamicIndexing(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual bool OnEvent(MSG msg);

private:
	static const UINT FrameCount = 3;
	static const UINT CityRowCount = 15;
	static const UINT CityColumnCount = 8;
	static const UINT CityMaterialCount = CityRowCount * CityColumnCount;
	static const UINT CityMaterialTextureWidth = 64;
	static const UINT CityMaterialTextureHeight = 64;
	static const UINT CityMaterialTextureChannelCount = 4;
	static const bool UseBundles = true;
	static const float CitySpacingInterval;

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> m_depthStencil;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	D3D12_RECT m_scissorRect;

	// App resources.
	UINT m_numIndices;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_cityDiffuseTexture;
	ComPtr<ID3D12Resource> m_cityMaterialTextures[CityMaterialCount];
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	StepTimer m_timer;
	UINT m_cbvSrvDescriptorSize;
	UINT m_rtvDescriptorSize;
	SimpleCamera m_camera;

	// Frame resources.
	vector<FrameResource*> m_frameResources;
	FrameResource* m_pCurrentFrameResource;
	UINT m_currentFrameResourceIndex;

	// Synchronization objects.
	UINT m_frameIndex;
	UINT m_frameCounter;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	void LoadPipeline();
	void LoadAssets();
	void CreateFrameResources();
	void PopulateCommandList(FrameResource* pFrameResource);
};
