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
#include "Camera.h"
#include "StepTimer.h"
#include "SquidRoom.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class FrameResource;

struct LightState
{
	XMFLOAT4 position;
	XMFLOAT4 direction;
	XMFLOAT4 color;
	XMFLOAT4 falloff;

	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

struct SceneConstantBuffer
{
	XMFLOAT4X4 model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4 ambientColor;
	BOOL sampleShadowMap;
	BOOL padding[3];		// Must be aligned to be made up of N float4s.
	LightState lights[NumLights];
};

class D3D12Multithreading : public DXSample
{
public:
	D3D12Multithreading(UINT width, UINT height, std::wstring name);
	virtual ~D3D12Multithreading();

	static D3D12Multithreading* Get() { return s_app; }

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual void OnKeyDown(UINT8 key);
	virtual void OnKeyUp(UINT8 key);

private:
	struct InputState
	{
		bool rightArrowPressed;
		bool leftArrowPressed;
		bool upArrowPressed;
		bool downArrowPressed;
		bool animate;
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> m_depthStencil;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
	ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateShadowMap;

	// App resources.
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	ComPtr<ID3D12Resource> m_textures[_countof(SampleAssets::Textures)];
	ComPtr<ID3D12Resource> m_textureUploads[_countof(SampleAssets::Textures)];
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	UINT m_rtvDescriptorSize;
	InputState m_keyboardInput;
	LightState m_lights[NumLights];
	Camera m_lightCameras[NumLights];
	Camera m_camera;
	StepTimer m_timer;
	StepTimer m_cpuTimer;
	int m_titleCount;
	double m_cpuTime;

	// Synchronization objects.
	HANDLE m_workerBeginRenderFrame[NumContexts];
	HANDLE m_workerFinishShadowPass[NumContexts];
	HANDLE m_workerFinishedRenderFrame[NumContexts];
	HANDLE m_threadHandles[NumContexts];
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	// Singleton object so that worker threads can share members.
	static D3D12Multithreading* s_app; 

	// Frame resources.
	FrameResource* m_frameResources[FrameCount];
	FrameResource* m_pCurrentFrameResource;
	int m_currentFrameResourceIndex;

	struct ThreadParameter
	{
		int threadIndex;
	};
	ThreadParameter m_threadParameters[NumContexts];

	void WorkerThread(int threadIndex);
	void SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList);

	void LoadPipeline();
	void LoadAssets();
	void LoadContexts();
	void BeginFrame();
	void MidFrame();
	void EndFrame();
};
