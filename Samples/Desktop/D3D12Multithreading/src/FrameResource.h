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

#include "stdafx.h"
#include "Camera.h"
#include "DXSampleHelper.h"
#include "D3D12Multithreading.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class FrameResource
{
public:
	ID3D12CommandList* m_batchSubmit[NumContexts * 2 + CommandListCount];

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CommandListCount];
	ComPtr<ID3D12GraphicsCommandList> m_commandLists[CommandListCount];

	ComPtr<ID3D12CommandAllocator> m_shadowCommandAllocators[NumContexts];
	ComPtr<ID3D12GraphicsCommandList> m_shadowCommandLists[NumContexts];

	ComPtr<ID3D12CommandAllocator> m_sceneCommandAllocators[NumContexts];
	ComPtr<ID3D12GraphicsCommandList> m_sceneCommandLists[NumContexts];

	UINT64 m_fenceValue;

private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateShadowMap;
	ComPtr<ID3D12Resource> m_shadowTexture;
	D3D12_CPU_DESCRIPTOR_HANDLE m_shadowDepthView;
	ComPtr<ID3D12Resource> m_shadowConstantBuffer;
	ComPtr<ID3D12Resource> m_sceneConstantBuffer;
	SceneConstantBuffer* mp_shadowConstantBufferWO;		// WRITE-ONLY pointer to the shadow pass constant buffer.
	SceneConstantBuffer* mp_sceneConstantBufferWO;		// WRITE-ONLY pointer to the scene pass constant buffer.
	D3D12_GPU_DESCRIPTOR_HANDLE m_nullSrvHandle;	// Null SRV for out of bounds behavior.
	D3D12_GPU_DESCRIPTOR_HANDLE m_shadowDepthHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_shadowCbvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_sceneCbvHandle;

public:
	FrameResource(ID3D12Device* pDevice, ID3D12PipelineState* pPso, ID3D12PipelineState* pShadowMapPso, ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, D3D12_VIEWPORT* pViewport, UINT frameResourceIndex);
	~FrameResource();

	void Bind(ID3D12GraphicsCommandList* pCommandList, BOOL scenePass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle);
	void Init();
	void SwapBarriers();
	void Finish();
	void WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[NumLights], LightState lights[NumLights]);
};
