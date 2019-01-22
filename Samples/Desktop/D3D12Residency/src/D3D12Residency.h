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
#include "d3dx12Residency.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

// Demonstrate how to use the D3DX12Residency library.
class D3D12Residency : public DXSample
{
public:
	D3D12Residency(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	static const UINT FrameCount = 3;
	static const UINT TextureWidth = 512;
	static const UINT TextureHeight = 512;
	static const UINT TexturePixelSize = 4;	// The number of bytes used to represent a pixel in the texture.

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGIAdapter3> m_adapter;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize;
	UINT m_srvDescriptorSize;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	// Resource residency management.
	static const UINT NumTextures = 1024 * 8;				// Should make for ~8GB of VRAM.
	static const UINT CommandListSubmissionsPerFrame = 1;
	static const UINT MaxResidencyLatency = FrameCount * CommandListSubmissionsPerFrame;

	struct ManagedCommandList
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ComPtr<ID3D12GraphicsCommandList> commandList;
		std::shared_ptr<D3DX12Residency::ResidencySet> residencySet;	// The set of resources that must be resident on the GPU to render this command list.
		UINT64 fenceValue = 0;
	};

	struct ManagedTexture
	{
		ComPtr<ID3D12Resource> texture;						// The D3D12 texture resource that will be managed by the residency library.
		D3DX12Residency::ManagedObject trackingHandle;		// The residency library works in units of ManagedObjects. They can be treated opaquely like a handle.
		UINT64 size = 0;
		UINT index = 0;
	};

	UINT64 m_totalAllocations;
	UINT m_textureIndex;
	D3DX12Residency::ResidencyManager m_residencyManager;
	std::queue<std::shared_ptr<ManagedCommandList>> m_commandListPool;

	// Thread and texture loading management.
	std::vector<std::future<void>> m_threads;
	bool m_cancel;
	std::mutex m_mutex;
	std::queue<std::shared_ptr<ManagedTexture>> m_incompleteTextures;	// Textures that are not initialized yet.
	std::vector<std::shared_ptr<ManagedTexture>> m_loadedTextures;		// Textures that are ready to be used.
	UINT m_loadedTextureCount;

	inline std::shared_ptr<ManagedTexture> GetNextIncompleteTexture()
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_incompleteTextures.size() == 0)
		{
			return nullptr;
		}
		else
		{
			auto pTexture = m_incompleteTextures.front();
			m_incompleteTextures.pop();
			return pTexture;
		}
	}

	inline void StoreCompletedTexture(std::shared_ptr<ManagedTexture> pTexture)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_loadedTextures[pTexture->index] = pTexture;
		m_loadedTextureCount++;
	}

	inline UINT LoadedTextureCount()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_loadedTextureCount;
	}

	void LoadPipeline();
	void LoadAssets();
	void LoadTexturesAsync();
	void PopulateCommandList(std::shared_ptr<ManagedCommandList> pManagedCommandList);
	void FlushGpu();
};
