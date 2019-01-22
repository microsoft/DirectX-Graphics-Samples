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
#include "GpuNode.h"

GpuNode::GpuNode(UINT nodeIndex, std::shared_ptr<CrossNodeResources> crossNodeResources) :
	m_frameIndex(0),
	m_nodeIndex(nodeIndex),
	m_nodeMask(1 << nodeIndex),
	m_crossNodeResources(crossNodeResources),
	m_sceneRenderTargetIndex(0)
{
	m_sceneCommandAllocators.resize(Settings::FrameCount);
	m_postCommandAllocators.resize(Settings::FrameCount);
	m_postRenderTargets.resize(Settings::BackBuffersPerNode);

	ThrowIfFailed(crossNodeResources->GetCommandQueue(nodeIndex)->QueryInterface(IID_PPV_ARGS(&m_graphicsQueue)));
	m_sceneFence = std::make_shared<LinearFence>(m_graphicsQueue.Get(), Settings::FrameCount);
	m_postFence = std::make_shared<LinearFence>(m_graphicsQueue.Get(), Settings::FrameCount);

	if (Settings::NodeCount > 1)
	{
		m_nodeSync = std::make_unique<NodeSynchronization>();
	}

	LoadPipeline();
	LoadAssets();
}

void GpuNode::LoadPipeline()
{
	auto pDevice = m_crossNodeResources->GetDevice();

	// Scene descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = Settings::SceneHistoryCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = m_nodeMask;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_sceneRtvHeap)));

		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = m_nodeMask;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_sceneDsvHeap)));

		// Describe and create a constant buffer view (CBV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = Settings::TriangleCount * Settings::SceneConstantBufferFrames;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = m_nodeMask;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_sceneCbvHeap)));
	}

	// Post-process descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap for 
		// the final output.
		D3D12_DESCRIPTOR_HEAP_DESC postRtvHeapDesc = {};
		postRtvHeapDesc.NumDescriptors = Settings::BackBuffersPerNode;
		postRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		postRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		postRtvHeapDesc.NodeMask = m_nodeMask;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&postRtvHeapDesc, IID_PPV_ARGS(&m_postRtvHeap)));

		// Describe and create a shader resource view (SRV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = Settings::SceneHistoryCount;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.NodeMask = m_nodeMask;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_postSrvHeap)));

		// Describe and create a sampler descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = 1;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		samplerHeapDesc.NodeMask = m_nodeMask;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_postSamplerHeap)));
	}

	// Create command allocators for each frame.
	for (UINT n = 0; n < Settings::FrameCount; n++)
	{
		ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[n])));
		ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_postCommandAllocators[n])));
	}
}

void GpuNode::LoadAssets()
{
	auto pDevice = m_crossNodeResources->GetDevice();

	// Single-use command allocator/list for resource initialization.
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandList(m_nodeMask, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	// Create the command lists.
	{
		ThrowIfFailed(pDevice->CreateCommandList(
			m_nodeMask,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_sceneCommandAllocators[m_frameIndex].Get(),
			m_crossNodeResources->GetScenePipelineState(),
			IID_PPV_ARGS(&m_sceneCommandList)));

		ThrowIfFailed(pDevice->CreateCommandList(
			m_nodeMask,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_postCommandAllocators[m_frameIndex].Get(),
			m_crossNodeResources->GetPostPipelineState(),
			IID_PPV_ARGS(&m_postCommandList)));

		// Command lists are created in the 'recording' state. We always call Reset
		// as the first thing when populating command lists, so we need to close
		// these here for the first frame.
		ThrowIfFailed(m_sceneCommandList->Close());
		ThrowIfFailed(m_postCommandList->Close());
	}

	ComPtr<ID3D12Resource> triangleVertexBufferUpload;
	ComPtr<ID3D12Resource> quadVertexBufferUpload;

	// Create the triangle vertex buffer.
	{
		// Define the geometry for a triangle.
		SceneVertex vertices[] =
		{
			{ { 0.0f, Settings::TriangleHalfWidth, Settings::TriangleDepth } },
			{ { Settings::TriangleHalfWidth, -Settings::TriangleHalfWidth, Settings::TriangleDepth } },
			{ { -Settings::TriangleHalfWidth, -Settings::TriangleHalfWidth, Settings::TriangleDepth } }
		};

		const UINT vertexBufferSize = sizeof(vertices);

		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		heapProps.CreationNodeMask = m_nodeMask;
		heapProps.VisibleNodeMask = m_nodeMask;

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_sceneVertexBuffer)));

		D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		uploadHeapProps.CreationNodeMask = m_nodeMask;
		uploadHeapProps.VisibleNodeMask = m_nodeMask;		// Upload heap is single-use and doesn't need to be shared.

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&triangleVertexBufferUpload)));

		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<UINT8*>(vertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(commandList.Get(), m_sceneVertexBuffer.Get(), triangleVertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sceneVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		m_sceneVertexBufferView.BufferLocation = m_sceneVertexBuffer->GetGPUVirtualAddress();
		m_sceneVertexBufferView.StrideInBytes = sizeof(SceneVertex);
		m_sceneVertexBufferView.SizeInBytes = sizeof(vertices);
	}

	// Create the quad vertex buffer.
	{
		// Define the geometry for a quad.
		PostVertex vertices[] =
		{
			{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f } },	// Bottom Left
			{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },	// Top Left
			{ { 1.0f, -1.0f, 0.0f },{ 1.0f, 1.0f } },	// Bottom Right
			{ { 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },	// Top Right
		};

		const UINT vertexBufferSize = sizeof(vertices);

		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		heapProps.CreationNodeMask = m_nodeMask;
		heapProps.VisibleNodeMask = m_nodeMask;

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_postVertexBuffer)));

		D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		uploadHeapProps.CreationNodeMask = m_nodeMask;
		uploadHeapProps.VisibleNodeMask = m_nodeMask;		// Upload heap is single-use and doesn't need to be shared.

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&quadVertexBufferUpload)));

		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<UINT8*>(vertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(commandList.Get(), m_postVertexBuffer.Get(), quadVertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_postVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		m_postVertexBufferView.BufferLocation = m_postVertexBuffer->GetGPUVirtualAddress();
		m_postVertexBufferView.StrideInBytes = sizeof(PostVertex);
		m_postVertexBufferView.SizeInBytes = sizeof(vertices);
	}

	// Create constant buffer views.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_sceneCbvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.SizeInBytes = sizeof(SceneConstantBuffer);
		cbvDesc.BufferLocation = m_crossNodeResources->GetSceneConstantBufferGpuVirtualAddress();

		for (UINT frame = 0; frame < Settings::SceneConstantBufferFrames; frame++)
		{
			for (UINT n = 0; n < Settings::TriangleCount; n++)
			{
				pDevice->CreateConstantBufferView(&cbvDesc, cpuHandle);

				cpuHandle.Offset(Settings::CbvSrvDescriptorSize);
				cbvDesc.BufferLocation += cbvDesc.SizeInBytes;
			}
		}
	}

	// Describe and create a sampler.
	{
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		pDevice->CreateSampler(&samplerDesc, m_postSamplerHeap->GetCPUDescriptorHandleForHeapStart());
	}

	ThrowIfFailed(commandList->Close());

	// Submit the initialization work to the GPU.
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Wait for the graphics queue to finish executing the commands.
	Fence fence(m_graphicsQueue.Get());
	fence.FlushGpuQueue();

	LoadSizeDependentResources();
}

void GpuNode::LoadSizeDependentResources()
{
	auto pDevice = m_crossNodeResources->GetDevice();
	auto pSwapChain = m_crossNodeResources->GetSwapChain();

	// Single-use command allocator/list for resource initialization.
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandList(m_nodeMask, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	// Create RTVs for back buffers assigned to this node.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_postRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < Settings::BackBuffersPerNode; n++)
	{
		UINT backBufferIndex = (n * Settings::NodeCount) + m_nodeIndex;

		// Create an RTV for the swap chain back buffer assigned to this node.
		ThrowIfFailed(pSwapChain->GetBuffer(backBufferIndex, IID_PPV_ARGS(&m_postRenderTargets[n])));

		pDevice->CreateRenderTargetView(m_postRenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(Settings::RtvDescriptorSize);
	}

	m_frameIndex = 0;
	m_sceneRenderTargetIndex = m_nodeIndex;

	// Create the render targets used to draw the scene.
	// These will be sampled from during the post-processing step.
	{
		D3D12_RESOURCE_DESC renderTargetDesc = m_postRenderTargets[0]->GetDesc();
		D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo(m_nodeMask, 1, &renderTargetDesc);
		const UINT64 alignedRenderTargetSize = AlignResource(info.SizeInBytes, info.Alignment);

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = alignedRenderTargetSize * Settings::SceneHistoryCount;
		heapDesc.Alignment = info.Alignment;
		heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, m_nodeMask, Settings::SharedNodeMask);
		heapDesc.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;

		ThrowIfFailed(pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_sceneRenderTargetHeap)));

		CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R8G8B8A8_UNORM, Settings::ClearColor);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_sceneRtvHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_postSrvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		D3D12_RESOURCE_BARRIER barriers[Settings::SceneHistoryCount];

		for (UINT n = 0; n < Settings::SceneHistoryCount; n++)
		{
			// The post-processing pass expects that a previous post-processing pass has
			// transitioned the next frame's dependencies into the COPY_DEST state.
			// Since these resources were created with the PIXEL_SHADER_RESOURCE state,
			// we need to transition them to COPY_DEST.

			UINT firstResource = (m_nodeIndex + Settings::SceneHistoryCount - (Settings::NodeCount - 1)) % Settings::SceneHistoryCount;
			UINT lastResource = (m_nodeIndex + Settings::SceneHistoryCount - 1) % Settings::SceneHistoryCount;
			bool initCopyDest = false;
			if (firstResource <= lastResource)
			{
				initCopyDest = m_nodeSync && (firstResource <= n && n <= lastResource);
			}
			else
			{
				initCopyDest = m_nodeSync && (n <= lastResource || firstResource <= n);
			}

			ThrowIfFailed(pDevice->CreatePlacedResource(
				m_sceneRenderTargetHeap.Get(),
				alignedRenderTargetSize * n,
				&renderTargetDesc,
				initCopyDest ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&clearValue,
				IID_PPV_ARGS(&m_sceneRenderTargets[n])));

			barriers[n] = CD3DX12_RESOURCE_BARRIER::Aliasing(nullptr, m_sceneRenderTargets[n].Get());

			pDevice->CreateRenderTargetView(m_sceneRenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(Settings::RtvDescriptorSize);

			pDevice->CreateShaderResourceView(m_sceneRenderTargets[n].Get(), &srvDesc, srvHandle);
			srvHandle.Offset(Settings::CbvSrvDescriptorSize);
		}

		commandList->ResourceBarrier(_countof(barriers), barriers);
	}

	// Create the depth stencil and its view.
	{
		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, m_nodeMask, m_nodeMask);
		D3D12_CLEAR_VALUE depthOptimizedClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Settings::Width, Settings::Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_sceneDepthStencil)
			));

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		pDevice->CreateDepthStencilView(m_sceneDepthStencil.Get(), &depthStencilDesc, m_sceneDsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	ThrowIfFailed(commandList->Close());

	// Submit the resource barrier changes to the GPU.
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Wait for the graphics queue to finish executing the commands.
	Fence fence(m_graphicsQueue.Get());
	fence.FlushGpuQueue();
}

// Get references to scene render targets and post-processing fences on other nodes.
void GpuNode::LinkSharedResources(std::shared_ptr<GpuNode>* ppNodes, UINT nodeCount)
{
	if (m_nodeSync)
	{
		// We need to store a reference to the other nodes' render targets so that
		// this node can copy rendered scenes to them.
		for (UINT nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
		{
			if (nodeIndex == m_nodeIndex)
			{
				// This node already has references to its own render targets.
				continue;
			}

			for (UINT n = 0; n < Settings::SceneHistoryCount; n++)
			{
				ThrowIfFailed(ppNodes[nodeIndex]->m_sceneRenderTargets[n].As(&m_nodeSync->sceneRenderTargets[nodeIndex][n]));
			}

			// Cross-node copies are only done when all nodes are finished with their
			// post-processing pass (which includes transitioning resources into the
			// correct states so that they may be copied into).
			// Keep a reference to the other nodes' fences so this node can wait to
			// perform the copy until the resource barriers are correctly set.
			m_nodeSync->postFences[nodeIndex] = ppNodes[nodeIndex]->m_postFence;
		}
	}
}

void GpuNode::OnUpdate(SceneConstantBuffer* pBuffers, UINT bufferCount, UINT64 frameId)
{
	m_crossNodeResources->UpdateConstantBuffer(reinterpret_cast<UINT8*>(pBuffers), bufferCount * sizeof(SceneConstantBuffer), frameId);
}

// Fill the command list with all the render commands and dependent state.
void GpuNode::RenderScene(UINT64 frameId, UINT simulatedGpuLoad)
{
	UINT cbvDescriptorTableOffset = Settings::TriangleCount * (frameId % Settings::SceneConstantBufferFrames);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_sceneCbvHeap->GetGPUDescriptorHandleForHeapStart(), cbvDescriptorTableOffset, Settings::CbvSrvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_sceneRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_sceneRenderTargetIndex, Settings::RtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_sceneDsvHeap->GetCPUDescriptorHandleForHeapStart());

	m_sceneFence->Next();

	// Record the rendering commands.
	ThrowIfFailed(m_sceneCommandAllocators[m_frameIndex]->Reset());
	ThrowIfFailed(m_sceneCommandList->Reset(m_sceneCommandAllocators[m_frameIndex].Get(), m_crossNodeResources->GetScenePipelineState()));

	// Set necessary state.
	m_sceneCommandList->SetGraphicsRootSignature(m_crossNodeResources->GetSceneRootSignature());
	m_sceneCommandList->SetGraphicsRoot32BitConstant(1, simulatedGpuLoad, 0);
	
	ID3D12DescriptorHeap* ppHeaps[] = { m_sceneCbvHeap.Get() };
	m_sceneCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_sceneCommandList->RSSetViewports(1, &Settings::Viewport);
	m_sceneCommandList->RSSetScissorRects(1, &Settings::ScissorRect);
	m_sceneCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_sceneRenderTargets[m_sceneRenderTargetIndex].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_sceneCommandList->ResourceBarrier(1, &barrier);
	m_sceneCommandList->ClearRenderTargetView(rtvHandle, Settings::ClearColor, 0, nullptr);
	m_sceneCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	m_sceneCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_sceneCommandList->IASetVertexBuffers(0, 1, &m_sceneVertexBufferView);

	for (UINT m = 0; m < Settings::TriangleCount; m++)
	{
		m_sceneCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		m_sceneCommandList->DrawInstanced(3, 1, 0, 0);

		cbvHandle.Offset(Settings::CbvSrvDescriptorSize);
	}

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = m_nodeSync ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_sceneCommandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_sceneCommandList->Close());

	ID3D12CommandList* ppSceneCommandLists[] = { m_sceneCommandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppSceneCommandLists), ppSceneCommandLists);

	m_sceneFence->Signal();
}

void GpuNode::RenderPost(UINT64 frameId)
{
	UINT backBufferIndex = m_crossNodeResources->GetSwapChain()->GetCurrentBackBufferIndex() / Settings::NodeCount;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_postRtvHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, Settings::RtvDescriptorSize);

	m_postFence->Next();

	ThrowIfFailed(m_postCommandAllocators[m_frameIndex]->Reset());
	ThrowIfFailed(m_postCommandList->Reset(m_postCommandAllocators[m_frameIndex].Get(), m_crossNodeResources->GetPostPipelineState()));

	if (m_nodeSync)
	{
		// If the rendered scenes need to be copied across nodes, we need to wait
		// until the other nodes are done with their post processing passes (which
		// includes transitioning the state of the next render targets in the
		// history to the COPY_DEST state).
		if (frameId > 0)
		{
			for (UINT node = 0; node < Settings::NodeCount; node++)
			{
				if (node != m_nodeIndex)
				{
					Fence::GpuWait(m_graphicsQueue.Get(), m_nodeSync->postFences[node].get());
				}
			}
		}

		// Copy the render target from the scene pass to all other nodes.
		for (UINT n = 0; n < Settings::NodeCount; n++)
		{
			// We only keep references to the render targets that we need to copy to.
			if (m_nodeSync->sceneRenderTargets[n][m_sceneRenderTargetIndex])
			{
				m_postCommandList->CopyResource(
					m_nodeSync->sceneRenderTargets[n][m_sceneRenderTargetIndex].Get(),
					m_sceneRenderTargets[m_sceneRenderTargetIndex].Get());
			}
		}

		// Transition all render target resources into the PIXEL_SHADER_RESOURCE
		// state to prepare for rendering the post-process effect.
		D3D12_RESOURCE_BARRIER barriers[Settings::MaxNodeCount];

		for (UINT index = 0; index < Settings::NodeCount - 1; index++)
		{
			UINT offset = Settings::SceneHistoryCount - Settings::NodeCount + 1 + index;
			UINT renderTargetIndex = (m_sceneRenderTargetIndex + offset) % Settings::SceneHistoryCount;

			barriers[index] = CD3DX12_RESOURCE_BARRIER::Transition(
				m_sceneRenderTargets[renderTargetIndex].Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

		barriers[Settings::NodeCount - 1] = CD3DX12_RESOURCE_BARRIER::Transition(
			m_sceneRenderTargets[m_sceneRenderTargetIndex].Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		m_postCommandList->ResourceBarrier(Settings::NodeCount, barriers);
	}

	m_postCommandList->SetGraphicsRootSignature(m_crossNodeResources->GetPostRootSignature());

	ID3D12DescriptorHeap* ppPostHeaps[] = { m_postSrvHeap.Get(), m_postSamplerHeap.Get() };
	m_postCommandList->SetDescriptorHeaps(_countof(ppPostHeaps), ppPostHeaps);

	m_postCommandList->RSSetViewports(1, &Settings::Viewport);
	m_postCommandList->RSSetScissorRects(1, &Settings::ScissorRect);

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_postRenderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_postCommandList->ResourceBarrier(1, &barrier);

	m_postCommandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	m_postCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_postCommandList->IASetVertexBuffers(0, 1, &m_postVertexBufferView);

	m_postCommandList->SetGraphicsRootDescriptorTable(0, m_postSrvHeap->GetGPUDescriptorHandleForHeapStart());
	m_postCommandList->SetGraphicsRootDescriptorTable(1, m_postSamplerHeap->GetGPUDescriptorHandleForHeapStart());
	m_postCommandList->SetGraphicsRoot32BitConstant(2, frameId % Settings::SceneHistoryCount, 0);
	m_postCommandList->SetGraphicsRoot32BitConstant(2, static_cast<UINT>(min(frameId + 1, Settings::SceneHistoryCount)), 1);

	m_postCommandList->DrawInstanced(4, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	D3D12_RESOURCE_BARRIER barriers[Settings::MaxNodeCount];
	barriers[0] = barrier;

	if (m_nodeSync)
	{
		// Transition render target resources rendered by the next nodes into the
		// COPY_DEST state to prepare them to be copied to during those nodes'
		// post-process passes.
		for (UINT offset = 1; offset < Settings::NodeCount; offset++)
		{
			UINT renderTargetIndex = (m_sceneRenderTargetIndex + offset) % Settings::SceneHistoryCount;

			barriers[offset] = CD3DX12_RESOURCE_BARRIER::Transition(
				m_sceneRenderTargets[renderTargetIndex].Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_COPY_DEST);
		}
	}

	m_postCommandList->ResourceBarrier(Settings::NodeCount, barriers);

	ThrowIfFailed(m_postCommandList->Close());

	ID3D12CommandList* ppPostCommandLists[] = { m_postCommandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppPostCommandLists), ppPostCommandLists);
}

void GpuNode::Present(UINT syncInterval, bool windowedMode)
{
	// When using sync interval 0, it is recommended to always pass the tearing
	// flag when it is supported, even if not presenting to a fullscreen window.
	// This flag cannot be used if the app is in "exclusive" fullscreen mode as
	// a result of calling SetFullscreenState.

	UINT presentFlags = (syncInterval == 0 && Settings::TearingSupport && windowedMode) ? DXGI_PRESENT_ALLOW_TEARING : 0;

	ThrowIfFailed(m_crossNodeResources->GetSwapChain()->Present(syncInterval, presentFlags));
}

void GpuNode::ReleaseBackBuffers()
{
	// Release the resources holding references to the swap chain (requirement of
	// IDXGISwapChain::ResizeBuffers).

	for (UINT n = 0; n < Settings::BackBuffersPerNode; n++)
	{
		m_postRenderTargets[n].Reset();
	}
}

void GpuNode::WaitForGpu()
{
	m_postFence->FlushGpuQueue();
}

void GpuNode::MoveToNextFrame()
{
	m_postFence->Signal();

	m_frameIndex = (m_frameIndex + 1) % Settings::FrameCount;

	// We render to every nth render target.
	m_sceneRenderTargetIndex = (m_sceneRenderTargetIndex + Settings::NodeCount) % Settings::SceneHistoryCount;
}
