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
#include "Fence.h"

using Microsoft::WRL::ComPtr;

// The purpose of this sample is to provide the reference application for the
// LinkedGpusAffinity project. By diffing the two projects, you can see the changes
// involved to add alternate frame rendering support via the D3DX12AffinityLayer.
class D3D12SingleGpu : public DXSample
{
public:
	D3D12SingleGpu(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual void OnKeyDown(UINT8 key);
	virtual void OnDestroy();

private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12CommandQueue> m_graphicsQueue;
	std::vector<ComPtr<ID3D12CommandAllocator>> m_sceneCommandAllocators;
	std::vector<ComPtr<ID3D12CommandAllocator>> m_postCommandAllocators;
	ComPtr<ID3D12DescriptorHeap> m_sceneRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_sceneDsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_sceneCbvHeap;
	ComPtr<ID3D12DescriptorHeap> m_postRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_postSrvHeap;
	ComPtr<ID3D12DescriptorHeap> m_postSamplerHeap;

	// Objects used for rendering the scene.
	ComPtr<ID3D12GraphicsCommandList> m_sceneCommandList;
	ComPtr<ID3D12RootSignature> m_sceneRootSignature;
	ComPtr<ID3D12PipelineState> m_scenePipelineState;
	ComPtr<ID3D12Resource> m_sceneVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_sceneVertexBufferView;
	ComPtr<ID3D12Resource> m_sceneConstantBuffer;
	UINT8* m_mappedConstantBuffer;
	ComPtr<ID3D12Heap> m_sceneRenderTargetHeap;
	std::vector<ComPtr<ID3D12Resource>> m_sceneRenderTargets;
	ComPtr<ID3D12Resource> m_sceneDepthStencil;

	// Objects used for post-processing.
	ComPtr<ID3D12GraphicsCommandList> m_postCommandList;
	ComPtr<ID3D12RootSignature> m_postRootSignature;
	ComPtr<ID3D12PipelineState> m_postPipelineState;
	ComPtr<ID3D12Resource> m_postVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_postVertexBufferView;
	std::vector<ComPtr<ID3D12Resource>> m_postRenderTargets;

	// A fence to indicate when the scene pass is complete.
	std::shared_ptr<LinearFence> m_sceneFence;

	// A fence to regulate submission of post-processing render passes.
	// Up to 'Settings::FrameCount' command lists can be in flight on the node at once.
	std::shared_ptr<LinearFence> m_postFence;

	// Vertex definitions.
	struct SceneVertex
	{
		DirectX::XMFLOAT3 position;
	};

	struct PostVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
	};

	// Constant buffer definition.
	struct SceneConstantBuffer
	{
		DirectX::XMFLOAT4 velocity;
		DirectX::XMFLOAT4 offset;
		DirectX::XMFLOAT4 color;
		DirectX::XMMATRIX projection;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow
		// multiple buffers to be array-indexed.
		float padding[36];
	};

	// Data about each of the triangles in the scene.
	std::vector<SceneConstantBuffer> m_sceneData;

	UINT64 m_frameId;
	UINT m_simulatedGpuLoad;
	UINT m_frameIndex;
	UINT m_sceneRenderTargetIndex;
	UINT m_syncInterval;
	bool m_windowVisible;
	bool m_windowedMode;

	void LoadPipeline();
	void LoadAssets();
	void LoadSizeDependentResources();
	float GetRandomFloat(float min, float max);
	void UpdateWindowTitle();
	void RenderScene();
	void RenderPost();
	void WaitForGpu();
	void MoveToNextFrame();

	static inline UINT64 AlignResource(UINT64 size, UINT64 alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
};
