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
#include "CrossNodeResources.h"
#include "Fence.h"

// Precompiled shaders.
#include "SceneVS.hlsl.h"
#include "ScenePS.hlsl.h"
#include "PostVS.hlsl.h"
#include "PostPS.hlsl.h"

CrossNodeResources::CrossNodeResources(IDXGIFactory4* pFactory, ID3D12Device* pDevice)
{
	pFactory->QueryInterface(IID_PPV_ARGS(&m_factory));
	pDevice->QueryInterface(IID_PPV_ARGS(&m_device));

	LoadPipeline();
	LoadAssets();
}

void CrossNodeResources::LoadPipeline()
{
	// Describe and create the command queues.
	// Each GPU node needs its own command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	std::vector<IUnknown*> ppQueues(Settings::BackBufferCount);
	std::vector<UINT> creationNodes(Settings::BackBufferCount);
	for (UINT n = 0; n < Settings::BackBufferCount; n++)
	{
		UINT nodeIndex = n % Settings::NodeCount;
		creationNodes[n] = 1 << nodeIndex;
		queueDesc.NodeMask = creationNodes[n];

		if (!m_queues[nodeIndex])
		{
			ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[nodeIndex])));
		}
		ppQueues[n] = m_queues[nodeIndex].Get();
	}

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = Settings::BackBufferCount;
	swapChainDesc.Width = Settings::Width;
	swapChainDesc.Height = Settings::Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// It is recommended to always use the tearing flag when it is available.
	swapChainDesc.Flags = Settings::TearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(m_factory->CreateSwapChainForCoreWindow(
		ppQueues[0],		// Swap chain needs the queue so that it can force a flush on it.
		reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
		&swapChainDesc,
		nullptr,
		&swapChain
		));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	if (Settings::NodeCount > 1)
	{
		// Set up the swap chain to allow back buffers to live on multiple GPU nodes.
		ThrowIfFailed(m_swapChain->ResizeBuffers1(
			swapChainDesc.BufferCount,
			swapChainDesc.Width,
			swapChainDesc.Height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			swapChainDesc.Flags,
			creationNodes.data(),
			ppQueues.data()));
	}
}

void CrossNodeResources::LoadAssets()
{
	// Create the root signatures.
	// Root signatures may be shared across GPU nodes.
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
			ThrowIfFailed(m_device->CreateRootSignature(Settings::SharedNodeMask, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_sceneRootSignature)));
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
			ThrowIfFailed(m_device->CreateRootSignature(Settings::SharedNodeMask, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_postRootSignature)));
		}
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	// Pipeline states may be shared across GPU nodes.
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
		psoDesc.NodeMask = Settings::SharedNodeMask;

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
		postPsoDesc.NodeMask = Settings::SharedNodeMask;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&postPsoDesc, IID_PPV_ARGS(&m_postPipelineState)));
	}

	// Create and map the constant buffers.
	// Upload heaps live in system memory and can be made visible to all GPU nodes.
	{
		const UINT constantBufferDataSize = Settings::TriangleCount * Settings::SceneConstantBufferFrames * sizeof(SceneConstantBuffer);

		D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		uploadHeapProps.VisibleNodeMask = Settings::SharedNodeMask;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_sceneConstantBuffer)));

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		ThrowIfFailed(m_sceneConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		ZeroMemory(m_mappedConstantBuffer, constantBufferDataSize);
	}
}

void CrossNodeResources::UpdateConstantBuffer(UINT8* data, UINT dataSize, UINT64 frameId)
{
	UINT index = frameId % Settings::SceneConstantBufferFrames;
	UINT offset = Settings::TriangleCount * index * sizeof(SceneConstantBuffer);

	memcpy(m_mappedConstantBuffer + offset, data, dataSize);
}

void CrossNodeResources::ResizeSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc = {};
	m_swapChain->GetDesc(&desc);

	if (Settings::NodeCount > 1)
	{
		std::vector<IUnknown*> ppQueues(Settings::BackBufferCount);
		std::vector<UINT> creationNodes(Settings::BackBufferCount);
		for (UINT n = 0; n < Settings::BackBufferCount; n++)
		{
			UINT nodeIndex = n % Settings::NodeCount;
			creationNodes[n] = 1 << nodeIndex;
			ppQueues[n] = m_queues[nodeIndex].Get();
		}

		ThrowIfFailed(m_swapChain->ResizeBuffers1(Settings::BackBufferCount, Settings::Width, Settings::Height, desc.BufferDesc.Format, desc.Flags, creationNodes.data(), ppQueues.data()));
	}
	else
	{
		ThrowIfFailed(m_swapChain->ResizeBuffers(Settings::BackBufferCount, Settings::Width, Settings::Height, desc.BufferDesc.Format, desc.Flags));
	}
}
