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

#include "stdafx.h"
#include "FrameResource.h"
#include "SquidRoom.h"

FrameResource::FrameResource(ID3D12Device* pDevice, ID3D12PipelineState* pPso, ID3D12PipelineState* pShadowMapPso, ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, D3D12_VIEWPORT* pViewport, UINT frameResourceIndex) :
	m_fenceValue(0),
	m_pipelineState(pPso),
	m_pipelineStateShadowMap(pShadowMapPso)
{
	for (UINT i = 0; i < CommandListCount; i++)
	{
		ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
		ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandLists[i])));

		NAME_D3D12_OBJECT_INDEXED(m_commandLists, i);

		// Close these command lists; don't record into them for now.
		ThrowIfFailed(m_commandLists[i]->Close());
	}

	for (UINT i = 0; i < NumContexts; i++)
	{
		// Create command list allocators for worker threads. One alloc is 
		// for the shadow pass command list, and one is for the scene pass.
		ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_shadowCommandAllocators[i])));
		ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[i])));

		ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_shadowCommandAllocators[i].Get(), m_pipelineStateShadowMap.Get(), IID_PPV_ARGS(&m_shadowCommandLists[i])));
		ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_sceneCommandAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_sceneCommandLists[i])));

		NAME_D3D12_OBJECT_INDEXED(m_shadowCommandLists, i);
		NAME_D3D12_OBJECT_INDEXED(m_sceneCommandLists, i);

		// Close these command lists; don't record into them for now. We will 
		// reset them to a recording state when we start the render loop.
		ThrowIfFailed(m_shadowCommandLists[i]->Close());
		ThrowIfFailed(m_sceneCommandLists[i]->Close());
	}

	// Describe and create the shadow map texture.
	CD3DX12_RESOURCE_DESC shadowTexDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT>(pViewport->Width), 
		static_cast<UINT>(pViewport->Height), 
		1,
		1,
		DXGI_FORMAT_R32_TYPELESS,
		1, 
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE clearValue;		// Performance tip: Tell the runtime at resource creation the desired clear value.
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&shadowTexDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_shadowTexture)));

	NAME_D3D12_OBJECT(m_shadowTexture);

	// Get a handle to the start of the descriptor heap then offset 
	// it based on the frame resource index.
	const UINT dsvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(pDsvHeap->GetCPUDescriptorHandleForHeapStart(), 1 + frameResourceIndex, dsvDescriptorSize); // + 1 for the shadow map.

	// Describe and create the shadow depth view and cache the CPU 
	// descriptor handle.
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	pDevice->CreateDepthStencilView(m_shadowTexture.Get(), &depthStencilViewDesc, depthHandle);
	m_shadowDepthView = depthHandle;

	// Get a handle to the start of the descriptor heap then offset it 
	// based on the existing textures and the frame resource index. Each 
	// frame has 1 SRV (shadow tex) and 2 CBVs.
	const UINT nullSrvCount = 2;								// Null descriptors at the start of the heap.
	const UINT textureCount = _countof(SampleAssets::Textures);	// Diffuse + normal textures near the start of the heap.  Ideally, track descriptor heap contents/offsets at a higher level.
	const UINT cbvSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(pCbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(pCbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
	m_nullSrvHandle = cbvSrvGpuHandle;
	cbvSrvCpuHandle.Offset(nullSrvCount + textureCount + (frameResourceIndex * FrameCount), cbvSrvDescriptorSize);
	cbvSrvGpuHandle.Offset(nullSrvCount + textureCount + (frameResourceIndex * FrameCount), cbvSrvDescriptorSize);

	// Describe and create a shader resource view (SRV) for the shadow depth 
	// texture and cache the GPU descriptor handle. This SRV is for sampling 
	// the shadow map from our shader. It uses the same texture that we use 
	// as a depth-stencil during the shadow pass.
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shadowSrvDesc.Texture2D.MipLevels = 1;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	pDevice->CreateShaderResourceView(m_shadowTexture.Get(), &shadowSrvDesc, cbvSrvCpuHandle);
	m_shadowDepthHandle = cbvSrvGpuHandle;

	// Increment the descriptor handles.
	cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
	cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);

	// Create the constant buffers.
	const UINT constantBufferSize = (sizeof(SceneConstantBuffer) + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1); // must be a multiple 256 bytes
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_shadowConstantBuffer)));
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_sceneConstantBuffer)));

	// Map the constant buffers and cache their heap pointers.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_shadowConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mp_shadowConstantBufferWO)));
	ThrowIfFailed(m_sceneConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mp_sceneConstantBufferWO)));

	// Create the constant buffer views: one for the shadow pass and
	// another for the scene pass.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.SizeInBytes = constantBufferSize;

	// Describe and create the shadow constant buffer view (CBV) and 
	// cache the GPU descriptor handle.
	cbvDesc.BufferLocation = m_shadowConstantBuffer->GetGPUVirtualAddress();
	pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
	m_shadowCbvHandle = cbvSrvGpuHandle;

	// Increment the descriptor handles.
	cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
	cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);

	// Describe and create the scene constant buffer view (CBV) and 
	// cache the GPU descriptor handle.
	cbvDesc.BufferLocation = m_sceneConstantBuffer->GetGPUVirtualAddress();
	pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
	m_sceneCbvHandle = cbvSrvGpuHandle;

	// Batch up command lists for execution later.
	const UINT batchSize = _countof(m_sceneCommandLists) + _countof(m_shadowCommandLists) + 3;
	m_batchSubmit[0] = m_commandLists[CommandListPre].Get();
	memcpy(m_batchSubmit + 1, m_shadowCommandLists, _countof(m_shadowCommandLists) * sizeof(ID3D12CommandList*));
	m_batchSubmit[_countof(m_shadowCommandLists) + 1] = m_commandLists[CommandListMid].Get();
	memcpy(m_batchSubmit + _countof(m_shadowCommandLists) + 2, m_sceneCommandLists, _countof(m_sceneCommandLists) * sizeof(ID3D12CommandList*));
	m_batchSubmit[batchSize - 1] = m_commandLists[CommandListPost].Get();
}

FrameResource::~FrameResource()
{
	for (int i = 0; i < CommandListCount; i++)
	{
		m_commandAllocators[i] = nullptr;
		m_commandLists[i] = nullptr;
	}

	m_shadowConstantBuffer = nullptr;
	m_sceneConstantBuffer = nullptr;

	for (int i = 0; i < NumContexts; i++)
	{
		m_shadowCommandLists[i] = nullptr;
		m_shadowCommandAllocators[i] = nullptr;

		m_sceneCommandLists[i] = nullptr;
		m_sceneCommandAllocators[i] = nullptr;
	}

	m_shadowTexture = nullptr;
}

// Builds and writes constant buffers from scratch to the proper slots for 
// this frame resource.
void FrameResource::WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[NumLights], LightState lights[NumLights])
{
	SceneConstantBuffer sceneConsts = {}; 
	SceneConstantBuffer shadowConsts = {};
	
	// Scale down the world a bit.
	::XMStoreFloat4x4(&sceneConsts.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));
	::XMStoreFloat4x4(&shadowConsts.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));

	// The scene pass is drawn from the camera.
	pSceneCamera->Get3DViewProjMatrices(&sceneConsts.view, &sceneConsts.projection, 90.0f, pViewport->Width, pViewport->Height);

	// The light pass is drawn from the first light.
	lightCams[0].Get3DViewProjMatrices(&shadowConsts.view, &shadowConsts.projection, 90.0f, pViewport->Width, pViewport->Height);

	for (int i = 0; i < NumLights; i++)
	{
		memcpy(&sceneConsts.lights[i], &lights[i], sizeof(LightState));
		memcpy(&shadowConsts.lights[i], &lights[i], sizeof(LightState));
	}

	// The shadow pass won't sample the shadow map, but rather write to it.
	shadowConsts.sampleShadowMap = FALSE;

	// The scene pass samples the shadow map.
	sceneConsts.sampleShadowMap = TRUE;

	shadowConsts.ambientColor = sceneConsts.ambientColor = { 0.1f, 0.2f, 0.3f, 1.0f };

	memcpy(mp_sceneConstantBufferWO, &sceneConsts, sizeof(SceneConstantBuffer));
	memcpy(mp_shadowConstantBufferWO, &shadowConsts, sizeof(SceneConstantBuffer));
}

void FrameResource::Init()
{
	// Reset the command allocators and lists for the main thread.
	for (int i = 0; i < CommandListCount; i++)
	{
		ThrowIfFailed(m_commandAllocators[i]->Reset());
		ThrowIfFailed(m_commandLists[i]->Reset(m_commandAllocators[i].Get(), m_pipelineState.Get()));
	}

	// Clear the depth stencil buffer in preparation for rendering the shadow map.
	m_commandLists[CommandListPre]->ClearDepthStencilView(m_shadowDepthView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Reset the worker command allocators and lists.
	for (int i = 0; i < NumContexts; i++)
	{
		ThrowIfFailed(m_shadowCommandAllocators[i]->Reset());
		ThrowIfFailed(m_shadowCommandLists[i]->Reset(m_shadowCommandAllocators[i].Get(), m_pipelineStateShadowMap.Get()));

		ThrowIfFailed(m_sceneCommandAllocators[i]->Reset());
		ThrowIfFailed(m_sceneCommandLists[i]->Reset(m_sceneCommandAllocators[i].Get(), m_pipelineState.Get()));
	}
}

void FrameResource::SwapBarriers()
{
	// Transition the shadow map from writeable to readable.
	m_commandLists[CommandListMid]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowTexture.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void FrameResource::Finish()
{
	m_commandLists[CommandListPost]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

// Sets up the descriptor tables for the worker command list to use 
// resources provided by frame resource.
void FrameResource::Bind(ID3D12GraphicsCommandList* pCommandList, BOOL scenePass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle)
{
	if (scenePass)
	{
		// Scene pass. We use constant buf #2 and depth stencil #2
		// with rendering to the render target enabled.
		pCommandList->SetGraphicsRootDescriptorTable(2, m_shadowDepthHandle);		// Set the shadow texture as an SRV.
		pCommandList->SetGraphicsRootDescriptorTable(1, m_sceneCbvHandle);
		
		assert(pRtvHandle != nullptr);
		assert(pDsvHandle != nullptr);

		pCommandList->OMSetRenderTargets(1, pRtvHandle, FALSE, pDsvHandle);
	}
	else
	{
		// Shadow pass. We use constant buf #1 and depth stencil #1
		// with rendering to the render target disabled.
		pCommandList->SetGraphicsRootDescriptorTable(2, m_nullSrvHandle);			// Set a null SRV for the shadow texture.
		pCommandList->SetGraphicsRootDescriptorTable(1, m_shadowCbvHandle);

		pCommandList->OMSetRenderTargets(0, nullptr, FALSE, &m_shadowDepthView);	// No render target needed for the shadow pass.
	}
}
