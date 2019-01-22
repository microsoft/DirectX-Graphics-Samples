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

#include "DXSampleHelper.h"
#include "CrossNodeResources.h"
#include "Fence.h"

using Microsoft::WRL::ComPtr;

// Container and logic for resources consumed only by a single GPU node.
class GpuNode
{
public:
	GpuNode(UINT nodeIndex, std::shared_ptr<CrossNodeResources> crossNodeResources);
	void LoadPipeline();
	void LoadAssets();
	void LoadSizeDependentResources();
	void LinkSharedResources(std::shared_ptr<GpuNode>* ppNodes, UINT nodeCount);
	void OnUpdate(SceneConstantBuffer* pBuffers, UINT bufferCount, UINT64 frameId);
	void RenderScene(UINT64 frameId, UINT simulatedGpuLoad);
	void RenderPost(UINT64 frameId);
	void Present(UINT syncInterval, bool windowedMode);

	void ReleaseBackBuffers();
	void WaitForGpu();
	void MoveToNextFrame();

private:
	UINT m_frameIndex;
	UINT m_nodeIndex;
	UINT m_nodeMask;

	std::shared_ptr<CrossNodeResources> m_crossNodeResources;

	ComPtr<ID3D12CommandQueue> m_graphicsQueue;
	std::vector<ComPtr<ID3D12CommandAllocator>> m_sceneCommandAllocators;
	std::vector<ComPtr<ID3D12CommandAllocator>> m_postCommandAllocators;
	ComPtr<ID3D12DescriptorHeap> m_sceneRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_sceneDsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_sceneCbvHeap;
	ComPtr<ID3D12DescriptorHeap> m_postRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_postSrvHeap;
	ComPtr<ID3D12DescriptorHeap> m_postSamplerHeap;

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

	// Scene objects.
	ComPtr<ID3D12Resource> m_sceneVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_sceneVertexBufferView;
	ComPtr<ID3D12GraphicsCommandList> m_sceneCommandList;
	ComPtr<ID3D12Heap> m_sceneRenderTargetHeap;
	ComPtr<ID3D12Resource> m_sceneDepthStencil;

	// The motion blur shader samples from the number of render targets specified by
	// Settings::SceneHistoryCount. A copy of these render targets will exist on each
	// node.
	ComPtr<ID3D12Resource> m_sceneRenderTargets[Settings::SceneHistoryCount];

	// The index of the current render target used in the scene pass.
	// This value increases by Settings::NodeCount each frame.
	UINT m_sceneRenderTargetIndex;

	// Post-process objects.
	ComPtr<ID3D12Resource> m_postVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_postVertexBufferView;
	ComPtr<ID3D12GraphicsCommandList> m_postCommandList;
	std::vector<ComPtr<ID3D12Resource>> m_postRenderTargets;

	// Objects used to support synchronizing render target contents across GPU nodes.
	struct NodeSynchronization
	{
		ComPtr<ID3D12Resource> sceneRenderTargets[Settings::MaxNodeCount][Settings::SceneHistoryCount];
		std::shared_ptr<Fence> postFences[Settings::MaxNodeCount];
	};
	std::unique_ptr<NodeSynchronization> m_nodeSync;

	std::shared_ptr<LinearFence> m_sceneFence;
	std::shared_ptr<LinearFence> m_postFence;

	static inline UINT64 AlignResource(UINT64 size, UINT64 alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
};
