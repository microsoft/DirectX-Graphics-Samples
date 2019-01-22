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

// Demonstrate how to use the D3DX12AffinityLayer library to render a scene on multiple
// GPUs using alternate frame rendering (AFR). Each GPU node available to the application
// will render a frame in round-robin fashion.
//
// The rendering consists of two passes:
// - The scene pass draws a number of triangles of different colors. The triangles
//   animate across the screen as the frames progress.
// - The post-processing pass takes the render targets of the previous 6 scene passes and
//   blends them together to produce a motion blur effect.
//
// When this sample runs on a system with linked GPUs, each node will take turns
// rendering the scene pass and share that frame's resulting render target with the
// other linked nodes.
class D3D12LinkedGpusAffinity : public DXSample
{
public:
	D3D12LinkedGpusAffinity(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual void OnKeyDown(UINT8 key);
	virtual void OnDestroy();

private:
	ComPtr<CD3DX12AffinityDevice> m_device;
	ComPtr<CDXGIAffinitySwapChain> m_swapChain;
	ComPtr<CD3DX12AffinityCommandQueue> m_graphicsQueue;
	std::vector<ComPtr<CD3DX12AffinityCommandAllocator>> m_sceneCommandAllocators;
	std::vector<ComPtr<CD3DX12AffinityCommandAllocator>> m_postCommandAllocators;
	ComPtr<CD3DX12AffinityDescriptorHeap> m_sceneRtvHeap;
	ComPtr<CD3DX12AffinityDescriptorHeap> m_sceneDsvHeap;
	ComPtr<CD3DX12AffinityDescriptorHeap> m_sceneCbvHeap;
	ComPtr<CD3DX12AffinityDescriptorHeap> m_postRtvHeap;
	ComPtr<CD3DX12AffinityDescriptorHeap> m_postSrvHeap;
	ComPtr<CD3DX12AffinityDescriptorHeap> m_postSamplerHeap;

	// Objects used for rendering the scene.
	ComPtr<CD3DX12AffinityGraphicsCommandList> m_sceneCommandList;
	ComPtr<CD3DX12AffinityRootSignature> m_sceneRootSignature;
	ComPtr<CD3DX12AffinityPipelineState> m_scenePipelineState;
	ComPtr<CD3DX12AffinityResource> m_sceneVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_sceneVertexBufferView;
	ComPtr<CD3DX12AffinityResource> m_sceneConstantBuffer;
	UINT8* m_mappedConstantBuffer;
	ComPtr<CD3DX12AffinityHeap> m_sceneRenderTargetHeap;
	std::vector<ComPtr<CD3DX12AffinityResource>> m_sceneRenderTargets;
	ComPtr<CD3DX12AffinityResource> m_sceneDepthStencil;

	// Objects used for post-processing.
	ComPtr<CD3DX12AffinityGraphicsCommandList> m_postCommandList;
	ComPtr<CD3DX12AffinityRootSignature> m_postRootSignature;
	ComPtr<CD3DX12AffinityPipelineState> m_postPipelineState;
	ComPtr<CD3DX12AffinityResource> m_postVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_postVertexBufferView;
	std::vector<ComPtr<CD3DX12AffinityResource>> m_postRenderTargets;

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
	UINT m_nodeIndex;
	UINT m_frameIndex;
	UINT m_sceneRenderTargetIndex;
	UINT m_syncInterval;
	bool m_windowVisible;
	bool m_windowedMode;

	// Additional work must be done to synchronize dependencies in the post-processing
	// pass when rendering with multiple nodes.
	bool m_syncSceneRenderTargets;

	void LoadPipeline();
	void LoadAssets();
	void LoadSizeDependentResources();
	float GetRandomFloat(float min, float max);
	void UpdateWindowTitle();
	void RenderScene();
	void RenderPost();
	void WaitForGpus();
	void MoveToNextFrame();

	static inline UINT64 AlignResource(UINT64 size, UINT64 alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
};
