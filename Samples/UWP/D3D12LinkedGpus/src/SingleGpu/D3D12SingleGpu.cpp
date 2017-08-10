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
#include "D3D12SingleGpu.h"

// Precompiled shaders.
#include "SceneVS.hlsl.h"
#include "ScenePS.hlsl.h"
#include "PostVS.hlsl.h"
#include "PostPS.hlsl.h"

using namespace DirectX;

D3D12SingleGpu::D3D12SingleGpu(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_frameIndex(0),
	m_frameId(0),
	m_simulatedGpuLoad(0x1000),
	m_syncInterval(0),
	m_windowVisible(true),
	m_windowedMode(true)
{
	m_sceneData.resize(Settings::TriangleCount);
}

void D3D12SingleGpu::OnInit()
{
	LoadPipeline();
	LoadAssets();

	UpdateWindowTitle();
}

void D3D12SingleGpu::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
			));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
			));
	}

	// Query capabilities of the device to configure the sample.
	Settings::Initialize(m_device.Get(), m_width, m_height);

	// Initialize member variables dependent on the Settings.
	m_sceneRenderTargets.resize(Settings::SceneHistoryCount);
	m_postRenderTargets.resize(Settings::BackBufferCount);
	m_sceneCommandAllocators.resize(Settings::FrameCount);
	m_postCommandAllocators.resize(Settings::FrameCount);

	// Describe and create the command queues.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_graphicsQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = Settings::BackBufferCount;
	swapChainDesc.Width = Settings::Width;
	swapChainDesc.Height = Settings::Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForCoreWindow(
		m_graphicsQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
		reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
		&swapChainDesc,
		nullptr,
		&swapChain
		));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	// Scene descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = Settings::SceneHistoryCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_sceneRtvHeap)));

		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_sceneDsvHeap)));

		// Describe and create a constant buffer view (CBV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = Settings::TriangleCount * Settings::SceneConstantBufferFrames;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_sceneCbvHeap)));
	}

	// Post-process descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap for 
		// the final output.
		D3D12_DESCRIPTOR_HEAP_DESC postRtvHeapDesc = {};
		postRtvHeapDesc.NumDescriptors = Settings::BackBufferCount;
		postRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		postRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&postRtvHeapDesc, IID_PPV_ARGS(&m_postRtvHeap)));

		// Describe and create a shader resource view (SRV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = Settings::SceneHistoryCount;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_postSrvHeap)));

		// Describe and create a sampler descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = 1;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_postSamplerHeap)));
	}

	// Create command allocators for each buffered frame.
	for (UINT n = 0; n < Settings::FrameCount; n++)
	{
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[n])));
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_postCommandAllocators[n])));
	}
}

// Generate data for the scene triangles.
void D3D12SingleGpu::LoadAssets()
{
	// Create the root signatures.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Create a root signature for rendering the triangle scene.
		{
			CD3DX12_DESCRIPTOR_RANGE1 sceneRanges[1];
			sceneRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			CD3DX12_ROOT_PARAMETER1 sceneRootParameters[2];
			sceneRootParameters[0].InitAsDescriptorTable(1, &sceneRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);
			sceneRootParameters[1].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC sceneRootSignatureDesc;
			sceneRootSignatureDesc.Init_1_1(_countof(sceneRootParameters), sceneRootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&sceneRootSignatureDesc, featureData.HighestVersion, &signature, &error));
			ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_sceneRootSignature)));
		}

		// Create a root signature for the post-process pass.
		{
			// We don't modify the SRV in the post-processing command list after
			// SetGraphicsRootDescriptorTable is executed on the GPU so we can use the default
			// range behavior: D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
			CD3DX12_DESCRIPTOR_RANGE1 postRanges[2];
			postRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Settings::SceneHistoryCount, 0);
			postRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

			CD3DX12_ROOT_PARAMETER1 postRootParameters[3];
			postRootParameters[0].InitAsDescriptorTable(1, &postRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			postRootParameters[1].InitAsDescriptorTable(1, &postRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			postRootParameters[2].InitAsConstants(2, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC postRootSignatureDesc;
			postRootSignatureDesc.Init_1_1(_countof(postRootParameters), postRootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&postRootSignatureDesc, featureData.HighestVersion, &signature, &error));
			ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_postRootSignature)));
		}
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		// Define the vertex input layout for the triangle scene.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Describe and create the graphics pipeline state objects (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_sceneRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_SceneVS, sizeof(g_SceneVS));
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_ScenePS, sizeof(g_ScenePS));
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_scenePipelineState)));

		// Define the vertex input layout for the post-process fullscreen quad.
		D3D12_INPUT_ELEMENT_DESC postInputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Describe and create the PSO for the post-process pass.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC postPsoDesc = {};
		postPsoDesc.InputLayout = { postInputElementDescs, _countof(postInputElementDescs) };
		postPsoDesc.pRootSignature = m_postRootSignature.Get();
		postPsoDesc.VS = CD3DX12_SHADER_BYTECODE(g_PostVS, sizeof(g_PostVS));
		postPsoDesc.PS = CD3DX12_SHADER_BYTECODE(g_PostPS, sizeof(g_PostPS));
		postPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		postPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		postPsoDesc.DepthStencilState.DepthEnable = FALSE;
		postPsoDesc.DepthStencilState.StencilEnable = FALSE;
		postPsoDesc.SampleMask = UINT_MAX;
		postPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		postPsoDesc.NumRenderTargets = 1;
		postPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		postPsoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&postPsoDesc, IID_PPV_ARGS(&m_postPipelineState)));
	}

	// Single-use command allocator/list for resource initialization.
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	// Create command lists for the scene and post-processing passes.
	{
		ThrowIfFailed(m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_sceneCommandAllocators[m_frameIndex].Get(),
			m_scenePipelineState.Get(),
			IID_PPV_ARGS(&m_sceneCommandList)));

		ThrowIfFailed(m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_postCommandAllocators[m_frameIndex].Get(),
			m_postPipelineState.Get(),
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

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_sceneVertexBuffer)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
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
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },	// Bottom Left
			{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },	// Top Left
			{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },	// Bottom Right
			{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },	// Top Right
		};

		const UINT vertexBufferSize = sizeof(vertices);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_postVertexBuffer)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
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

	// Create and map the constant buffers.
	{
		// UPLOAD heaps are not duplicated by the Affinity layer because:
		//   1. they live in system memory
		//   2. they are expected to contain data that will be read by all GPUs.
		// All of our constant buffers change each frame, so the state of any one frame
		// is only ever read by a single GPU. Therefore, we must ensure that a large
		// enough resource is created to persist frame data for all GPUs to read from.
		const UINT constantBufferDataSize = Settings::TriangleCount * Settings::SceneConstantBufferFrames * sizeof(SceneConstantBuffer);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_sceneConstantBuffer)));

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_sceneConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		ZeroMemory(m_mappedConstantBuffer, constantBufferDataSize);

		// Create constant buffer views.
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_sceneCbvHeap->GetCPUDescriptorHandleForHeapStart());
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.SizeInBytes = sizeof(SceneConstantBuffer);
		cbvDesc.BufferLocation = m_sceneConstantBuffer->GetGPUVirtualAddress();

		for (UINT frame = 0; frame < Settings::SceneConstantBufferFrames; frame++)
		{
			for (UINT n = 0; n < Settings::TriangleCount; n++)
			{
				m_device->CreateConstantBufferView(&cbvDesc, cpuHandle);

				cpuHandle.Offset(Settings::CbvSrvDescriptorSize);
				cbvDesc.BufferLocation += cbvDesc.SizeInBytes;
			}
		}
	}

	for (UINT n = 0; n < Settings::TriangleCount; n++)
	{
		m_sceneData[n].velocity = XMFLOAT4(GetRandomFloat(0.005f, 0.01f), 0.0f, 0.0f, 0.0f);
		m_sceneData[n].offset = XMFLOAT4(GetRandomFloat(-6.0f, -1.5f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
		m_sceneData[n].color = XMFLOAT4(GetRandomFloat(0.4f, 0.9f), GetRandomFloat(0.4f, 0.9f), GetRandomFloat(0.4f, 0.9f), 1.0f);
		m_sceneData[n].projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.01f, 20.0f));
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

		m_device->CreateSampler(&samplerDesc, m_postSamplerHeap->GetCPUDescriptorHandleForHeapStart());
	}

	m_sceneFence = std::make_shared<LinearFence>(m_graphicsQueue.Get(), Settings::FrameCount);
	m_postFence = std::make_shared<LinearFence>(m_graphicsQueue.Get(), Settings::FrameCount);

	ThrowIfFailed(commandList->Close());

	// Submit the initialization work to the GPU.
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Wait for the graphics queue to finish executing the commands.
	Fence fence(m_graphicsQueue.Get());
	fence.FlushGpuQueue();

	LoadSizeDependentResources();
}

void D3D12SingleGpu::LoadSizeDependentResources()
{
	// Single-use command allocator/list for resource initialization.
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	// Create RTVs for swap chain back buffers.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_postRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < Settings::BackBufferCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_postRenderTargets[n])));

		m_device->CreateRenderTargetView(m_postRenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(Settings::RtvDescriptorSize);
	}

	m_frameIndex = 0;
	m_sceneRenderTargetIndex = 0;

	// Create the render targets used to draw the scene.
	// These will be sampled from during the post-processing step.
	{
		D3D12_RESOURCE_DESC renderTargetDesc = m_postRenderTargets[0]->GetDesc();
		D3D12_RESOURCE_ALLOCATION_INFO info = m_device->GetResourceAllocationInfo(0, 1, &renderTargetDesc);
		const UINT64 alignedRenderTargetSize = AlignResource(info.SizeInBytes, info.Alignment);

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = alignedRenderTargetSize * Settings::SceneHistoryCount;
		heapDesc.Alignment = info.Alignment;
		heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		heapDesc.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;

		ThrowIfFailed(m_device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_sceneRenderTargetHeap)));

		CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R8G8B8A8_UNORM, Settings::ClearColor);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_sceneRtvHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_postSrvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		D3D12_RESOURCE_BARRIER barriers[Settings::SceneHistoryCount];

		for (UINT sceneIndex = 0; sceneIndex < Settings::SceneHistoryCount; sceneIndex++)
		{
			ThrowIfFailed(m_device->CreatePlacedResource(
				m_sceneRenderTargetHeap.Get(),
				alignedRenderTargetSize * sceneIndex,
				&renderTargetDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&clearValue,
				IID_PPV_ARGS(&m_sceneRenderTargets[sceneIndex])));

			barriers[sceneIndex] = CD3DX12_RESOURCE_BARRIER::Aliasing(nullptr, m_sceneRenderTargets[sceneIndex].Get());

			m_device->CreateRenderTargetView(m_sceneRenderTargets[sceneIndex].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(Settings::RtvDescriptorSize);

			m_device->CreateShaderResourceView(m_sceneRenderTargets[sceneIndex].Get(), &srvDesc, srvHandle);
			srvHandle.Offset(Settings::CbvSrvDescriptorSize);
		}

		commandList->ResourceBarrier(_countof(barriers), barriers);
	}

	// Create the depth stencil and its view.
	{
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

		ThrowIfFailed(m_device->CreateCommittedResource(
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

		m_device->CreateDepthStencilView(m_sceneDepthStencil.Get(), &depthStencilDesc, m_sceneDsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	ThrowIfFailed(commandList->Close());

	// Submit the resource barrier changes to the GPU.
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Wait for the graphics queue to finish executing the commands.
	Fence fence(m_graphicsQueue.Get());
	fence.FlushGpuQueue();
}

// Get a random float value between min and max.
float D3D12SingleGpu::GetRandomFloat(float min, float max)
{
	float scale = static_cast<float>(rand()) / RAND_MAX;
	float range = max - min;
	return scale * range + min;
}

void D3D12SingleGpu::UpdateWindowTitle()
{
	WCHAR nodeText[100];
	swprintf_s(nodeText, L"Node Count = 1 | SyncInterval = %u | SimulatedLoad = %u", m_syncInterval, m_simulatedGpuLoad);
	SetCustomWindowText(nodeText);
}

// Update frame-based values.
void D3D12SingleGpu::OnUpdate()
{
	for (UINT n = 0; n < Settings::TriangleCount; n++)
	{
		const float offsetBounds = 1.5f * m_aspectRatio;

		// Animate the triangles.
		m_sceneData[n].offset.x += m_sceneData[n].velocity.x;
		if (m_sceneData[n].offset.x > offsetBounds)
		{
			m_sceneData[n].velocity.x = GetRandomFloat(0.005f, 0.01f);
			m_sceneData[n].offset.x = -offsetBounds;
		}
	}

	UINT index = m_frameId % Settings::SceneConstantBufferFrames;
	UINT offset = Settings::TriangleCount * index * sizeof(SceneConstantBuffer);

	memcpy(m_mappedConstantBuffer + offset, m_sceneData.data(), Settings::TriangleCount * sizeof(SceneConstantBuffer));
}

// Render the scene.
void D3D12SingleGpu::OnRender()
{
	if (m_windowVisible)
	{
		// Render and present the current frame.
		RenderScene();
		RenderPost();

		// When using sync interval 0, it is recommended to always pass the tearing
		// flag when it is supported, even if not presenting to a fullscreen window.
		// This flag cannot be used if the app is in "exclusive" fullscreen mode as
		// a result of calling SetFullscreenState.

		UINT presentFlags = (m_syncInterval == 0 && m_tearingSupport && m_windowedMode) ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(m_swapChain->Present(m_syncInterval, presentFlags));

		MoveToNextFrame();
	}
}

// Fill the command list with all the render commands and dependent state.
void D3D12SingleGpu::RenderScene()
{
	UINT cbvDescriptorTableOffset = Settings::TriangleCount * (m_frameId % Settings::SceneConstantBufferFrames);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_sceneCbvHeap->GetGPUDescriptorHandleForHeapStart(), cbvDescriptorTableOffset, Settings::CbvSrvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_sceneRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_sceneRenderTargetIndex, Settings::RtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_sceneDsvHeap->GetCPUDescriptorHandleForHeapStart());

	m_sceneFence->Next();

	// Record the rendering commands.
	ThrowIfFailed(m_sceneCommandAllocators[m_frameIndex]->Reset());
	ThrowIfFailed(m_sceneCommandList->Reset(m_sceneCommandAllocators[m_frameIndex].Get(), m_scenePipelineState.Get()));

	// Set necessary state.
	m_sceneCommandList->SetGraphicsRootSignature(m_sceneRootSignature.Get());
	m_sceneCommandList->SetGraphicsRoot32BitConstant(1, m_simulatedGpuLoad, 0);

	ID3D12DescriptorHeap* ppHeaps[] = { m_sceneCbvHeap.Get() };
	m_sceneCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_sceneCommandList->RSSetViewports(1, &Settings::Viewport);
	m_sceneCommandList->RSSetScissorRects(1, &Settings::ScissorRect);
	m_sceneCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_sceneRenderTargets[m_sceneRenderTargetIndex].Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_sceneCommandList->ResourceBarrier(1, &barrier);
	m_sceneCommandList->ClearRenderTargetView(rtvHandle, Settings::ClearColor, 0, nullptr);
	m_sceneCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_sceneCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_sceneCommandList->IASetVertexBuffers(0, 1, &m_sceneVertexBufferView);

	for (UINT triangle = 0; triangle < Settings::TriangleCount; triangle++)
	{
		m_sceneCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		m_sceneCommandList->DrawInstanced(3, 1, 0, 0);

		cbvHandle.Offset(Settings::CbvSrvDescriptorSize);
	}

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_sceneCommandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_sceneCommandList->Close());

	ID3D12CommandList* ppSceneCommandLists[] = { m_sceneCommandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppSceneCommandLists), ppSceneCommandLists);

	m_sceneFence->Signal();
}

void D3D12SingleGpu::RenderPost()
{
	UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_postRtvHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, Settings::RtvDescriptorSize);

	m_postFence->Next();

	ThrowIfFailed(m_postCommandAllocators[m_frameIndex]->Reset());
	ThrowIfFailed(m_postCommandList->Reset(m_postCommandAllocators[m_frameIndex].Get(), m_postPipelineState.Get()));

	m_postCommandList->SetGraphicsRootSignature(m_postRootSignature.Get());

	ID3D12DescriptorHeap* ppPostHeaps[] = { m_postSrvHeap.Get(), m_postSamplerHeap.Get() };
	m_postCommandList->SetDescriptorHeaps(_countof(ppPostHeaps), ppPostHeaps);

	m_postCommandList->RSSetViewports(1, &Settings::Viewport);
	m_postCommandList->RSSetScissorRects(1, &Settings::ScissorRect);

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_postRenderTargets[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_postCommandList->ResourceBarrier(1, &barrier);

	m_postCommandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	m_postCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_postCommandList->IASetVertexBuffers(0, 1, &m_postVertexBufferView);

	m_postCommandList->SetGraphicsRootDescriptorTable(0, m_postSrvHeap->GetGPUDescriptorHandleForHeapStart());
	m_postCommandList->SetGraphicsRootDescriptorTable(1, m_postSamplerHeap->GetGPUDescriptorHandleForHeapStart());
	m_postCommandList->SetGraphicsRoot32BitConstant(2, m_frameId % Settings::SceneHistoryCount, 0);
	m_postCommandList->SetGraphicsRoot32BitConstant(2, static_cast<UINT>(min(m_frameId + 1, Settings::SceneHistoryCount)), 1);

	m_postCommandList->DrawInstanced(4, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	m_postCommandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_postCommandList->Close());

	ID3D12CommandList* ppPostCommandLists[] = { m_postCommandList.Get() };
	m_graphicsQueue->ExecuteCommandLists(_countof(ppPostCommandLists), ppPostCommandLists);
}

void D3D12SingleGpu::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	// Determine if the swap buffers and other resources need to be resized or not.
	if ((width != m_width || height != m_height) && !minimized)
	{
		// Flush all current GPU commands.
		WaitForGpu();

		// Release buffers tied to the swap chain and update the global Settings
		// for the new size.
		for (UINT n = 0; n < Settings::BackBufferCount; n++)
		{
			m_postRenderTargets[n].Reset();
		}
		Settings::OnSizeChanged(width, height);

		// Resize the swap chain to the desired dimensions.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		ThrowIfFailed(m_swapChain->GetDesc1(&swapChainDesc));
		ThrowIfFailed(m_swapChain->ResizeBuffers(Settings::BackBufferCount, width, height, swapChainDesc.Format, swapChainDesc.Flags));

		// Re-create render targets.
		LoadSizeDependentResources();

		// Align the frameId so that assumptions in post-processing hold.
		m_frameId += (Settings::SceneHistoryCount - (m_frameId % Settings::SceneHistoryCount));

		// Update the m_width, m_height, and m_aspectRatio member variables.
		UpdateForSizeChange(width, height);

		// Update the scene projection matrix.
		for (UINT n = 0; n < Settings::TriangleCount; n++)
		{
			m_sceneData[n].projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.01f, 20.0f));
		}
	}

	m_windowVisible = !minimized;
}

void D3D12SingleGpu::OnKeyDown(UINT8 key)
{
	switch (key)
	{
	// Instrument the Space Bar to toggle between fullscreen states.
	// The CoreWindow will fire a SizeChanged event once the window is in the
	// fullscreen state. At that point, the IDXGISwapChain should be resized
	// to match the new window size.
	case VK_SPACE:
	{
		auto applicationView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
		if (applicationView->IsFullScreenMode)
		{
			applicationView->ExitFullScreenMode();
		}
		else
		{
			applicationView->TryEnterFullScreenMode();
		}
		break;
	}

	case VK_LEFT:
	case VK_RIGHT:
		m_syncInterval = (m_syncInterval > 0) ? 0 : 1;
		UpdateWindowTitle();
		break;

	case VK_UP:
		if (m_simulatedGpuLoad > 0)
		{
			if (m_simulatedGpuLoad < 0x80000)
			{
				m_simulatedGpuLoad <<= 1;
			}
		}
		else
		{
			m_simulatedGpuLoad = 1;
		}
		UpdateWindowTitle();
		break;

	case VK_DOWN:
		if (m_simulatedGpuLoad != 0)
		{
			m_simulatedGpuLoad >>= 1;
		}
		UpdateWindowTitle();
		break;
	}
}

void D3D12SingleGpu::OnDestroy()
{
	// Wait for the GPUs to be done with all their resources.
	WaitForGpu();
}

// Wait for pending GPU work to complete.
void D3D12SingleGpu::WaitForGpu()
{
	m_postFence->FlushGpuQueue();
}

// Prepare to render the next frame.
void D3D12SingleGpu::MoveToNextFrame()
{
	m_postFence->Signal();

	// Advance to the next frame.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_sceneRenderTargetIndex = (m_sceneRenderTargetIndex + 1) % Settings::SceneHistoryCount;

	m_frameId++;
}
