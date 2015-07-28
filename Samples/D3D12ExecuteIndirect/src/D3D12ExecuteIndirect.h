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

class D3D12ExecuteIndirect : public DXSample
{
public:
	D3D12ExecuteIndirect(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual bool OnEvent(MSG msg);

private:
	static const UINT FrameCount = 3;
	static const UINT TriangleCount = 1024;
	static const UINT TriangleResourceCount = TriangleCount * FrameCount;
	static const UINT ComputeThreadBlockSize = 128;		// Should match the value in compute.hlsl.
	static const float TriangleHalfWidth;				// The x and y offsets used by the triangle vertices.
	static const float TriangleDepth;					// The z offset used by the triangle vertices.
	static const float CullingCutoff;					// The +/- x offset of the clipping planes in homogenous space [-1,1].

	// Vertex definition.
	struct Vertex
	{
		XMFLOAT3 position;
	};

	// Constant buffer definition.
	struct ConstantBufferData
	{
		XMFLOAT4 velocity;
		XMFLOAT4 offset;
		XMFLOAT4 color;
		XMMATRIX projection;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
		// to be array-indexed.
		float padding[36];
	};

	// Root constants for the compute shader.
	struct CSRootConstants
	{
		float xOffset;
		float zOffset;
		float cullOffset;
		float commandCount;
	};

	// Data structure to match the command signature used for ExecuteIndirect.
	struct IndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS cbv;
		D3D12_DRAW_ARGUMENTS drawArguments;
	};

	// Graphics root signature parameter offsets.
	enum GraphicsRootParameters
	{
		Cbv,
		GraphicsRootParametersCount
	};

	// Compute root signature parameter offsets.
	enum ComputeRootParameters
	{
		SrvUavTable,
		RootConstants,			// Root constants that give the shader information about the triangle vertices and culling planes.
		ComputeRootParametersCount
	};

	// CBV/SRV/UAV desciptor heap offsets.
	enum HeapOffsets
	{
		CbvOffset = 0,
		CbvSrvOffset = CbvOffset + TriangleCount,							// SRV that points to the constant buffers used by the rendering thread.
		CommandsOffset = CbvSrvOffset + 1,									// SRV that points to all of the indirect commands.
		ProcessedCommandsOffset = CommandsOffset + 1,						// UAV that records the commands we actually want to execute.
		CbvSrvUavDescriptorCountPerFrame = ProcessedCommandsOffset + 1		// 1 CBV per triangle + [2 SRVs + 1 UAV for the compute shader].
	};

	// Each triangle gets its own constant buffer per frame.
	std::vector<ConstantBufferData> m_constantBufferData;
	UINT8* m_pCbvDataBegin;

	CSRootConstants m_csRootConstants;	// Constants for the compute shader.
	bool m_enableCulling;				// Toggle whether the compute shader pre-processes the indirect commands.

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	D3D12_RECT m_cullingScissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_computeCommandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandQueue> m_computeCommandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12RootSignature> m_computeRootSignature;
	ComPtr<ID3D12CommandSignature> m_commandSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
	UINT m_rtvDescriptorSize;
	UINT m_cbvSrvUavDescriptorSize;
	UINT m_frameIndex;

	// Synchronization objects.
	ComPtr<ID3D12Fence> m_fence;
	ComPtr<ID3D12Fence> m_computeFence;
	UINT64 m_fenceValues[FrameCount];
	HANDLE m_fenceEvent;

	// Asset objects.
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_computeState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12GraphicsCommandList> m_computeCommandList;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_constantBuffer;
	ComPtr<ID3D12Resource> m_depthStencil;
	ComPtr<ID3D12Resource> m_commandBuffer;
	ComPtr<ID3D12Resource> m_processedCommandBuffers[FrameCount];
	ComPtr<ID3D12Resource> m_processedCommandBufferCounters[FrameCount];
	UINT8* m_pMappedUavCounters[FrameCount];
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	void LoadPipeline();
	void LoadAssets();
	float GetRandomFloat(float min, float max);
	void PopulateCommandLists();
	void WaitForGpu();
	void MoveToNextFrame();
};