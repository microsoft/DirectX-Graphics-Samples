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

class D3D12ReservedResources : public DXSample
{
public:
	D3D12ReservedResources(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual void OnKeyDown(UINT8 key);

private:
	static const UINT FrameCount = 2;
	static const UINT TextureWidth = 256;
	static const UINT TextureHeight = 256;
	static const UINT TexturePixelSizeInBytes = 4;

	// Vertex definition.
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

	// Information about the mips in the reserved resource.
	struct MipInfo
	{
		UINT heapIndex;
		bool packedMip;
		bool mapped;
		D3D12_TILED_RESOURCE_COORDINATE startCoordinate;
		D3D12_TILE_REGION_SIZE regionSize;
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_frameIndex;
	bool m_tilingSupport;

	// Synchronization objects.
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];
	HANDLE m_fenceEvent;

	// Asset objects.
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_uploadHeap;
	ComPtr<ID3D12Resource> m_reservedResource;
	std::vector<ComPtr<ID3D12Heap>> m_heaps;
	std::vector<MipInfo> m_mips;
	D3D12_PACKED_MIP_INFO m_packedMipInfo;
	UINT m_activeMip;
	bool m_activeMipChanged;

	void LoadPipeline();
	void LoadAssets();
	std::vector<UINT8> GenerateTextureData(UINT firstMip, UINT lastMip);
	void UpdateTileMapping();
	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
};
