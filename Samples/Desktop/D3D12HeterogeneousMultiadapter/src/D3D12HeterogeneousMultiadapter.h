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

class D3D12HeterogeneousMultiadapter : public DXSample
{
public:
	D3D12HeterogeneousMultiadapter(int width, int height, LPCWSTR name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	static const bool AllowDrawDynamicWorkload = false;		// Allow the sample to change the number of triangles drawn, in an attempt to balance the workload between adapters.
	static const bool AllowShaderDynamicWorkload = true;	// Allow the sample to change PS complexity (simulated), in an attempt to balance the workload between adapters.

	static const UINT FrameCount = 3;
	static const float ClearColor[4];
	static const UINT MovingAverageFrameCount = 20;
	static const UINT WindowTextUpdateFrequency = 20;	// Update the window title every x frames.
	static const UINT MaxTriangleCount = 15000;			// The max number of triangles per frame.
	static const float TriangleHalfWidth;				// The x and y offsets used by the triangle vertices.
	static const float TriangleDepth;					// The z offset used by the triangle vertices.

	UINT m_frameIndex;
	UINT m_triangleCount;
	UINT m_psLoopCount;
	UINT m_blurPSLoopCount;
	UINT m_currentTimesIndex;
	UINT64 m_drawTimes[MovingAverageFrameCount];
	UINT64 m_blurTimes[MovingAverageFrameCount];
	UINT64 m_drawTimeMovingAverage;
	UINT64 m_blurTimeMovingAverage;

	// Vertex definitions.
	struct Vertex
	{
		XMFLOAT3 position;
	};

	struct VertexPositionUV
	{
		XMFLOAT4 position;
		XMFLOAT2 uv;
	};

	// Constant buffer definitions.
	struct SceneConstantBuffer
	{
		XMFLOAT4 velocity;
		XMFLOAT4 offset;
		XMFLOAT4 color;
		XMFLOAT4X4 projection;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
		// to be array-indexed.
		float padding[36];
	};

	struct BlurConstantBufferData
	{
		XMFLOAT2 textureDimensions;

		// Controls how much of the render target is blurred along X axis [0.0. 1.0]. 
		// E.g. 1 = all of the RT is blurred, 0.5 = half of the RT is blurred, 0.0 = none of the RT is blurred
		FLOAT offset;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
		// to be array-indexed.
		float padding[61];
	};

	struct WorkloadConstantBufferData
	{
		UINT loopCount;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
		// to be array-indexed.
		float padding[63];
	};

	enum GraphicsAdapter
	{
		Primary, // Note: Not necessarily the OS's primary adapter (adapter enumerated at index 0).
		Secondary,
		GraphicsAdaptersCount
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	UINT m_rtvDescriptorSizes[GraphicsAdaptersCount];
	UINT m_srvDescriptorSizes[GraphicsAdaptersCount];
	DXGI_ADAPTER_DESC1 m_adapterDescs[GraphicsAdaptersCount];
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_devices[GraphicsAdaptersCount];
	ComPtr<ID3D12CommandAllocator> m_directCommandAllocators[GraphicsAdaptersCount][FrameCount];
	ComPtr<ID3D12CommandAllocator> m_copyCommandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_directCommandQueues[GraphicsAdaptersCount];
	ComPtr<ID3D12CommandQueue> m_copyCommandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12RootSignature> m_blurRootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_blurPipelineStates[2];
	ComPtr<ID3D12DescriptorHeap> m_rtvHeaps[GraphicsAdaptersCount];
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
	ComPtr<ID3D12GraphicsCommandList> m_directCommandLists[GraphicsAdaptersCount];
	ComPtr<ID3D12GraphicsCommandList> m_copyCommandList;

	// Synchronization objects.
	ComPtr<ID3D12Fence> m_frameFence;
	ComPtr<ID3D12Fence> m_renderFence;
	ComPtr<ID3D12Fence> m_crossAdapterFences[GraphicsAdaptersCount];
	UINT64 m_currentPresentFenceValue;
	UINT64 m_currentRenderFenceValue;
	UINT64 m_currentCrossAdapterFenceValue;
	UINT64 m_frameFenceValues[FrameCount];
	HANDLE m_fenceEvents[GraphicsAdaptersCount];

	// Asset objects.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_fullscreenQuadVertexBuffer;
	ComPtr<ID3D12Resource> m_constantBuffer;
	ComPtr<ID3D12Resource> m_workloadConstantBuffer;
	ComPtr<ID3D12Resource> m_blurWorkloadConstantBuffer;
	ComPtr<ID3D12Resource> m_blurConstantBuffer;
	ComPtr<ID3D12Resource> m_depthStencil;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_fullscreenQuadVertexBufferView;
	std::vector<SceneConstantBuffer> m_constantBufferData;
	SceneConstantBuffer* m_pCbvDataBegin;
	BlurConstantBufferData* m_pBlurCbvDataBegin;
	WorkloadConstantBufferData m_workloadConstantBufferData;
	WorkloadConstantBufferData* m_pWorkloadCbvDataBegin;
	WorkloadConstantBufferData m_blurWorkloadConstantBufferData;
	WorkloadConstantBufferData* m_pBlurWorkloadCbvDataBegin;
	ComPtr<ID3D12Heap> m_crossAdapterResourceHeaps[GraphicsAdaptersCount];
	ComPtr<ID3D12Resource> m_crossAdapterResources[GraphicsAdaptersCount][FrameCount];
	BOOL m_crossAdapterTextureSupport;
	ComPtr<ID3D12Resource> m_secondaryAdapterTextures[FrameCount];			// Only used if cross adapter texture support is unavailable.
	ComPtr<ID3D12Resource> m_renderTargets[GraphicsAdaptersCount][FrameCount];
	ComPtr<ID3D12Resource> m_intermediateBlurRenderTarget;
	ComPtr<ID3D12QueryHeap> m_timestampQueryHeaps[GraphicsAdaptersCount];
	ComPtr<ID3D12Resource> m_timestampResultBuffers[GraphicsAdaptersCount];
	UINT64 m_directCommandQueueTimestampFrequencies[GraphicsAdaptersCount];

	HRESULT GetHardwareAdapters(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppPrimaryAdapter, _Outptr_result_maybenull_ IDXGIAdapter1** ppSecondaryAdapter);
	void LoadPipeline();
	void LoadAssets();
	float GetRandomFloat(float min, float max);
	void PopulateCommandLists();
	void UpdateWindowTitle();
	void WaitForGpu(GraphicsAdapter adapter);
	void MoveToNextFrame();

	static inline UINT Align(UINT size, UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
};
