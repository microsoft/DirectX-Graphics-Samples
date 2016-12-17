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

using Microsoft::WRL::ComPtr;

// Constant buffer definition.
struct SceneConstantBuffer
{
	DirectX::XMFLOAT4 velocity;
	DirectX::XMFLOAT4 offset;
	DirectX::XMFLOAT4 color;
	DirectX::XMMATRIX projection;

	// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
	// to be array-indexed.
	float padding[36];
};

// Container for resources that are shareable across GPU nodes.
class CrossNodeResources
{
public:
	CrossNodeResources(IDXGIFactory4* pFactory, ID3D12Device* pDevice);

	void LoadPipeline();
	void LoadAssets();

	void UpdateConstantBuffer(UINT8* data, UINT dataSize, UINT64 frameId);
	void ResizeSwapChain();

	inline ID3D12Device* GetDevice()                             { return m_device.Get(); }
	inline ID3D12CommandQueue* GetCommandQueue(UINT nodeIndex)   { return m_queues[nodeIndex].Get(); }
	inline IDXGISwapChain3* GetSwapChain()                       { return m_swapChain.Get(); }
	inline ID3D12RootSignature* GetSceneRootSignature()          { return m_sceneRootSignature.Get(); }
	inline ID3D12PipelineState* GetScenePipelineState()          { return m_scenePipelineState.Get(); }
	inline ID3D12RootSignature* GetPostRootSignature()           { return m_postRootSignature.Get(); }
	inline ID3D12PipelineState* GetPostPipelineState()           { return m_postPipelineState.Get(); }

	inline D3D12_GPU_VIRTUAL_ADDRESS GetSceneConstantBufferGpuVirtualAddress()
	{
		return m_sceneConstantBuffer->GetGPUVirtualAddress();
	}

private:
	// Objects that are visible across all GPU nodes.
	ComPtr<IDXGIFactory4> m_factory;
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12RootSignature> m_sceneRootSignature;
	ComPtr<ID3D12PipelineState> m_scenePipelineState;
	ComPtr<ID3D12RootSignature> m_postRootSignature;
	ComPtr<ID3D12PipelineState> m_postPipelineState;

	// We use an Upload heap for the constant buffers.
	// Upload heaps reside in system memory and can be accessed by any available node.
	ComPtr<ID3D12Resource> m_sceneConstantBuffer;
	UINT8* m_mappedConstantBuffer;

	// The command queues are actually tied to specific GPU nodes, but are placed here
	// for convenience since all queues are needed for swap chain creation and resizing.
	ComPtr<ID3D12CommandQueue> m_queues[Settings::MaxNodeCount];
};
