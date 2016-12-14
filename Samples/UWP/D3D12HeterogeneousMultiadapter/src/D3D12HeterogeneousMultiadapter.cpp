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
#include "D3D12HeterogeneousMultiadapter.h"

const float D3D12HeterogeneousMultiadapter::TriangleHalfWidth = 0.025f;
const float D3D12HeterogeneousMultiadapter::TriangleDepth = 1.0f;
const float D3D12HeterogeneousMultiadapter::ClearColor[4] = { 0.0f, 0.2f, 0.3f, 1.0f };

D3D12HeterogeneousMultiadapter::D3D12HeterogeneousMultiadapter(int width, int height, LPCWSTR name) :
	DXSample(width, height, name),
	m_frameIndex(0),
	m_triangleCount(MaxTriangleCount / 2),
	m_psLoopCount(0),
	m_blurPSLoopCount(0),
	m_currentTimesIndex(0),
	m_drawTimeMovingAverage(0),
	m_blurTimeMovingAverage(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_currentPresentFenceValue(1),
	m_currentRenderFenceValue(1),
	m_currentCrossAdapterFenceValue(1),
	m_workloadConstantBufferData(),
	m_blurWorkloadConstantBufferData(),
	m_crossAdapterTextureSupport(false),
	m_rtvDescriptorSizes{},
	m_srvDescriptorSizes{},
	m_drawTimes{},
	m_blurTimes{},
	m_frameFenceValues{}
{
	m_constantBufferData.resize(MaxTriangleCount);
}

void D3D12HeterogeneousMultiadapter::OnInit()
{
	LoadPipeline();
	LoadAssets();
	UpdateWindowTitle();
}

// Enumerate adapters to use for heterogeneous multiadaper.
_Use_decl_annotations_
HRESULT D3D12HeterogeneousMultiadapter::GetHardwareAdapters(IDXGIFactory2* pFactory, IDXGIAdapter1** ppPrimaryAdapter, IDXGIAdapter1** ppSecondaryAdapter)
{
	if (pFactory == nullptr)
	{
		return E_POINTER;
	}

	// Adapter 0 is the adapter that Presents frames to the display. It is assigned as
	// the "secondary" adapter because it is the adapter that performs the second set
	// of operations (the blur effect) in this sample.
	// Adapter 1 is an additional GPU that the app can take advantage of, but it does
	// not own the presentation step. It is assigned as the "primary" adapter because
	// it is the adapter that performs the first set of operations (rendering triangles)
	// in this sample.

	ThrowIfFailed(pFactory->EnumAdapters1(0, ppSecondaryAdapter));
	ThrowIfFailed(pFactory->EnumAdapters1(1, ppPrimaryAdapter));

	return S_OK;
}

// Load the rendering pipeline dependencies.
void D3D12HeterogeneousMultiadapter::LoadPipeline()
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

	ComPtr<IDXGIAdapter1> primaryAdapter;
	ComPtr<IDXGIAdapter1> secondaryAdapter;
	if (m_useWarpDevice)
	{
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&primaryAdapter)));
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&secondaryAdapter)));
	}
	else
	{
		ThrowIfFailed(GetHardwareAdapters(factory.Get(), &primaryAdapter, &secondaryAdapter));
	}

	DXGI_ADAPTER_DESC1 desc;
	primaryAdapter->GetDesc1(&desc);
	if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
	{
		// There is actually only one physical GPU on the system.
		// Reduce the starting triangle count to make the sample run better.
		m_triangleCount = MaxTriangleCount / 50;
	}

	IDXGIAdapter1* ppAdapters[] = { primaryAdapter.Get(), secondaryAdapter.Get() };
	for (UINT i = 0; i < GraphicsAdaptersCount; i++)
	{
		ThrowIfFailed(D3D12CreateDevice(ppAdapters[i], D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_devices[i])));
		ThrowIfFailed(ppAdapters[i]->GetDesc1(&m_adapterDescs[i]));
	}

	// Describe and create the command queues and get their timestamp frequency.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	for (UINT i = 0; i < GraphicsAdaptersCount; i++)
	{
		ThrowIfFailed(m_devices[i]->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_directCommandQueues[i])));
		ThrowIfFailed(m_directCommandQueues[i]->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies[i]));
	}

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ThrowIfFailed(m_devices[Primary]->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_copyCommandQueue)));

	// Describe and create the swap chain on the secondary device because that's where we present from.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForCoreWindow(
		m_directCommandQueues[Secondary].Get(),		// Swap chain needs the queue so that it can force a flush on it.
		reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
		&swapChainDesc,
		nullptr,
		&swapChain
		));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heaps.
		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;

			if (i == Secondary)
			{
				// Add space for the intermediate render target.
				rtvHeapDesc.NumDescriptors++;
			}

			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(m_devices[i]->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeaps[i])));
		}

		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_devices[Primary]->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

		// Describe and create a shader resource view (SRV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
		cbvSrvUavHeapDesc.NumDescriptors = FrameCount + 1;	// +1 for the intermediate blur render target.
		cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_devices[Secondary]->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));

		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			m_rtvDescriptorSizes[i] = m_devices[i]->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_srvDescriptorSizes[i] = m_devices[i]->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}

	// Create query heaps and result buffers.
	{
		// Two timestamps for each frame.
		const UINT resultCount = 2 * FrameCount;
		const UINT resultBufferSize = resultCount * sizeof(UINT64);

		D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
		timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		timestampHeapDesc.Count = resultCount;

		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			ThrowIfFailed(m_devices[i]->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_timestampResultBuffers[i])));

			ThrowIfFailed(m_devices[i]->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_timestampQueryHeaps[i])));
		}
	}

	// Create frame resources.
	{
		const CD3DX12_CLEAR_VALUE clearValue(swapChainDesc.Format, ClearColor);
		const CD3DX12_RESOURCE_DESC renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			swapChainDesc.Format,
			swapChainDesc.Width,
			swapChainDesc.Height,
			1u, 1u,
			swapChainDesc.SampleDesc.Count,
			swapChainDesc.SampleDesc.Quality,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0u);

		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeaps[i]->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV and a command allocator for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				if (i == Secondary)
				{
					ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[i][n])));
				}
				else
				{
					ThrowIfFailed(m_devices[i]->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
						D3D12_HEAP_FLAG_NONE,
						&renderTargetDesc,
						D3D12_RESOURCE_STATE_COMMON,
						&clearValue,
						IID_PPV_ARGS(&m_renderTargets[i][n])));
				}
				
				m_devices[i]->CreateRenderTargetView(m_renderTargets[i][n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSizes[i]);

				ThrowIfFailed(m_devices[i]->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_directCommandAllocators[i][n])));

				if (i == Primary)
				{
					ThrowIfFailed(m_devices[i]->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_copyCommandAllocators[n])));
				}
			}
		}

		// Create cross-adapter shared resources on the primary adapter, and open the shared handles on the secondary adapter.
		{
			// Check whether shared row-major textures can be directly sampled by the
			// secondary adapter. Support of this feature (or the lack thereof) will
			// determine our sharing strategy for the resource in question.
			D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
			ThrowIfFailed(m_devices[Secondary]->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options), sizeof(options)));
			m_crossAdapterTextureSupport = options.CrossAdapterRowMajorTextureSupported;

			UINT64 textureSize = 0;
			D3D12_RESOURCE_DESC crossAdapterDesc;

			if (m_crossAdapterTextureSupport)
			{
				// If cross-adapter row-major textures are supported by the adapter,
				// then they can be sampled directly.
				crossAdapterDesc = renderTargetDesc;
				crossAdapterDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
				crossAdapterDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				D3D12_RESOURCE_ALLOCATION_INFO textureInfo = m_devices[Primary]->GetResourceAllocationInfo(0, 1, &crossAdapterDesc);
				textureSize = textureInfo.SizeInBytes;
			}
			else
			{
				// If cross-adapter row-major textures are not supported by the adapter,
				// then they will be shared as buffers and then copied to a destination
				// texture on the secondary adapter.

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
				m_devices[Primary]->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &layout, nullptr, nullptr, nullptr);
				textureSize = Align(layout.Footprint.RowPitch * layout.Footprint.Height);

				// Create a buffer with the same layout as the render target texture.
				crossAdapterDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);
			}

			// Create a heap that will be shared by both adapters.
			CD3DX12_HEAP_DESC heapDesc(
				textureSize * FrameCount,
				D3D12_HEAP_TYPE_DEFAULT,
				0,
				D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);

			ThrowIfFailed(m_devices[Primary]->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_crossAdapterResourceHeaps[Primary])));

			HANDLE heapHandle = nullptr;
			ThrowIfFailed(m_devices[Primary]->CreateSharedHandle(
				m_crossAdapterResourceHeaps[Primary].Get(),
				nullptr,
				GENERIC_ALL,
				nullptr,
				&heapHandle));

			HRESULT openSharedHandleResult = m_devices[Secondary]->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&m_crossAdapterResourceHeaps[Secondary]));

			// We can close the handle after opening the cross-adapter shared resource.
			CloseHandle(heapHandle);

			ThrowIfFailed(openSharedHandleResult);

			// Create placed resources for each frame per adapter in the shared heap.
			for (UINT n = 0; n < FrameCount; n++)
			{
				ThrowIfFailed(m_devices[Primary]->CreatePlacedResource(
					m_crossAdapterResourceHeaps[Primary].Get(),
					textureSize * n,
					&crossAdapterDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&m_crossAdapterResources[Primary][n])));

				ThrowIfFailed(m_devices[Secondary]->CreatePlacedResource(
					m_crossAdapterResourceHeaps[Secondary].Get(),
					textureSize * n,
					&crossAdapterDesc,
					m_crossAdapterTextureSupport ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_COPY_SOURCE,
					nullptr,
					IID_PPV_ARGS(&m_crossAdapterResources[Secondary][n])));

				if (!m_crossAdapterTextureSupport)
				{
					// If the primary adapter's render target must be shared as a buffer,
					// create a texture resource to copy it into on the secondary adapter.
					ThrowIfFailed(m_devices[Secondary]->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
						D3D12_HEAP_FLAG_NONE,
						&renderTargetDesc,
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
						nullptr,
						IID_PPV_ARGS(&m_secondaryAdapterTextures[n])));
				}
			}
		}

		// Create an intermediate render target and view on the secondary adapter.
		{
			const D3D12_RESOURCE_DESC intermediateRenderTargetDesc = m_renderTargets[Primary][0]->GetDesc();

			ThrowIfFailed(m_devices[Secondary]->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&intermediateRenderTargetDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				nullptr,
				IID_PPV_ARGS(&m_intermediateBlurRenderTarget)));

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeaps[Secondary]->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSizes[Secondary]);
			m_devices[Secondary]->CreateRenderTargetView(m_intermediateBlurRenderTarget.Get(), nullptr, rtvHandle);
		}
		
		// Create SRVs for the shared resources and intermediate render target on the secondary adapter.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
			for (UINT n = 0; n < FrameCount; n++)
			{
				ID3D12Resource* pSrvResource = m_crossAdapterTextureSupport ? m_crossAdapterResources[Secondary][n].Get() : m_secondaryAdapterTextures[n].Get();
				m_devices[Secondary]->CreateShaderResourceView(pSrvResource, nullptr, srvHandle);
				srvHandle.Offset(m_srvDescriptorSizes[Secondary]);
			}

			m_devices[Secondary]->CreateShaderResourceView(m_intermediateBlurRenderTarget.Get(), nullptr, srvHandle);
		}
	}
}

// Load the sample assets.
void D3D12HeterogeneousMultiadapter::LoadAssets()
{
	// Create the root signatures.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_devices[Primary]->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_devices[Primary]->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_devices[Secondary]->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
		// is executed on the GPU so we can use the default range behavior:
		// D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

		CD3DX12_ROOT_PARAMETER1 blurRootParameters[3];
		blurRootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
		blurRootParameters[1].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_PIXEL);
		blurRootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC staticPointSampler(0);
		staticPointSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticPointSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_STATIC_SAMPLER_DESC staticLinearSampler(1);
		staticLinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticLinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_STATIC_SAMPLER_DESC staticSamplers[] = { staticPointSampler, staticLinearSampler };
		rootSignatureDesc.Init_1_1(_countof(blurRootParameters), blurRootParameters, _countof(staticSamplers), staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_devices[Secondary]->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_blurRootSignature)));
	}

	// Create the pipeline states, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> vertexShaderBlur;
		ComPtr<ID3DBlob> pixelShaderBlurU;
		ComPtr<ID3DBlob> pixelShaderBlurV;
		ComPtr<ID3DBlob> error;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VShader", "vs_5_0", compileFlags, 0, &vertexShader, &error));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PShader", "ps_5_0", compileFlags, 0, &pixelShader, &error));

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"blurShaders.hlsl").c_str(), nullptr, nullptr, "VSSimpleBlur", "vs_5_0", compileFlags, 0, &vertexShaderBlur, &error));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"blurShaders.hlsl").c_str(), nullptr, nullptr, "PSSimpleBlurU", "ps_5_0", compileFlags, 0, &pixelShaderBlurU, &error));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"blurShaders.hlsl").c_str(), nullptr, nullptr, "PSSimpleBlurV", "ps_5_0", compileFlags, 0, &pixelShaderBlurV, &error));

		// Define the vertex input layouts.
		const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		const D3D12_INPUT_ELEMENT_DESC blurInputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Describe and create the graphics pipeline state objects (PSOs).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_devices[Primary]->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

		psoDesc.InputLayout = { blurInputElementDescs, _countof(blurInputElementDescs) };
		psoDesc.pRootSignature = m_blurRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlur.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlurU.Get());
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		ThrowIfFailed(m_devices[Secondary]->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_blurPipelineStates[0])));

		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlurV.Get());
		ThrowIfFailed(m_devices[Secondary]->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_blurPipelineStates[1])));
	}

	// Create the command lists.
	ThrowIfFailed(m_devices[Primary]->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCommandAllocators[Primary][m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_directCommandLists[Primary])));
	ThrowIfFailed(m_devices[Primary]->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copyCommandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_copyCommandList)));
	ThrowIfFailed(m_copyCommandList->Close());

	ThrowIfFailed(m_devices[Secondary]->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCommandAllocators[Secondary][m_frameIndex].Get(), m_blurPipelineStates[0].Get(), IID_PPV_ARGS(&m_directCommandLists[Secondary])));

	// Note: ComPtr's are CPU objects but these resources need to stay in scope until
	// the command list that references them has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resources are not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> vertexBufferUpload;
	ComPtr<ID3D12Resource> fullscreenQuadVertexBufferUpload;

	// Create the vertex buffer for the primary adapter.
	{
		// Define the geometry for a triangle.
		Vertex triangleVertices[] =
		{
			{ { 0.0f, TriangleHalfWidth, TriangleDepth } },
			{ { TriangleHalfWidth, -TriangleHalfWidth, TriangleDepth } },
			{ { -TriangleHalfWidth, -TriangleHalfWidth, TriangleDepth } }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		ThrowIfFailed(m_devices[Primary]->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		ThrowIfFailed(m_devices[Primary]->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBufferUpload)));

		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<UINT8*>(triangleVertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(m_directCommandLists[Primary].Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		m_directCommandLists[Primary]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
	}

	// Create the vertex buffer for the secondary adapter.
	{
		// Define the geometry for a fullscreen triangle.
		VertexPositionUV quadVertices[] =
		{
			{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },	// Bottom left.
			{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },	// Top left.
			{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },	// Bottom right.
			{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },		// Top right.
		};

		const UINT vertexBufferSize = sizeof(quadVertices);

		ThrowIfFailed(m_devices[Secondary]->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_fullscreenQuadVertexBuffer)));

		ThrowIfFailed(m_devices[Secondary]->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&fullscreenQuadVertexBufferUpload)));

		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<UINT8*>(quadVertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(m_directCommandLists[Secondary].Get(), m_fullscreenQuadVertexBuffer.Get(), fullscreenQuadVertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		m_directCommandLists[Secondary]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_fullscreenQuadVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		m_fullscreenQuadVertexBufferView.BufferLocation = m_fullscreenQuadVertexBuffer->GetGPUVirtualAddress();
		m_fullscreenQuadVertexBufferView.StrideInBytes = sizeof(VertexPositionUV);
		m_fullscreenQuadVertexBufferView.SizeInBytes = sizeof(quadVertices);
	}

	// Create the depth stencil view.
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		const CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

		ThrowIfFailed(m_devices[Primary]->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_depthStencil)
			));

		m_devices[Primary]->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create the constant buffers.
	{
		{
			const UINT64 constantBufferSize = sizeof(SceneConstantBuffer) * MaxTriangleCount * FrameCount;

			ThrowIfFailed(m_devices[Primary]->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_constantBuffer)));

			// Setup constant buffer data.
			for (UINT n = 0; n < MaxTriangleCount; n++)
			{
				m_constantBufferData[n].velocity = XMFLOAT4(GetRandomFloat(0.01f, 0.02f), 0.0f, 0.0f, 0.0f);
				m_constantBufferData[n].offset = XMFLOAT4(GetRandomFloat(-5.0f, -1.5f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
				m_constantBufferData[n].color = XMFLOAT4(GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), 1.0f);
				XMStoreFloat4x4(&m_constantBufferData[n].projection, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.01f, 20.0f)));
			}

			// Map and initialize the constant buffer. We don't unmap this until the
			// app closes. Keeping things mapped for the lifetime of the resource is okay.
			CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
			ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
			memcpy(m_pCbvDataBegin, &m_constantBufferData[0], constantBufferSize / FrameCount);
		}

		{
			const UINT64 workloadConstantBufferSize = sizeof(WorkloadConstantBufferData) * FrameCount;

			ThrowIfFailed(m_devices[Primary]->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(workloadConstantBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_workloadConstantBuffer)));

			// Setup constant buffer data.
			m_workloadConstantBufferData.loopCount = m_psLoopCount;

			// Map and initialize the constant buffer. We don't unmap this until the
			// app closes. Keeping things mapped for the lifetime of the resource is okay.
			CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
			ThrowIfFailed(m_workloadConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pWorkloadCbvDataBegin)));
			memcpy(m_pWorkloadCbvDataBegin, &m_workloadConstantBufferData, workloadConstantBufferSize / FrameCount);
		}

		{
			const UINT64 blurWorkloadConstantBufferSize = sizeof(WorkloadConstantBufferData) * FrameCount;

			ThrowIfFailed(m_devices[Secondary]->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(blurWorkloadConstantBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_blurWorkloadConstantBuffer)));

			// Setup constant buffer data.
			m_blurWorkloadConstantBufferData.loopCount = m_blurPSLoopCount;

			// Map and initialize the constant buffer. We don't unmap this until the
			// app closes. Keeping things mapped for the lifetime of the resource is okay.
			CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
			ThrowIfFailed(m_blurWorkloadConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pBlurWorkloadCbvDataBegin)));
			memcpy(m_pBlurWorkloadCbvDataBegin, &m_blurWorkloadConstantBufferData, blurWorkloadConstantBufferSize / FrameCount);
		}

		{
			ThrowIfFailed(m_devices[Secondary]->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(BlurConstantBufferData)),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_blurConstantBuffer)));

			// Map the constant buffer.
			CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
			ThrowIfFailed(m_blurConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pBlurCbvDataBegin)));

			// Setup constant buffer data.
			m_pBlurCbvDataBegin[0].offset = 0.5f;
			m_pBlurCbvDataBegin[0].textureDimensions.x = static_cast<float>(m_width);
			m_pBlurCbvDataBegin[0].textureDimensions.y = static_cast<float>(m_height);

			// Unmap the constant buffer because we don't update this again.
			// If we ever do, it should be buffered by the number of frames like other constant buffers.
			const CD3DX12_RANGE emptyRange(0, 0);
			m_blurConstantBuffer->Unmap(0, &emptyRange);
			m_pBlurCbvDataBegin = nullptr;
		}
	}

	// Close the command lists and execute them to begin the vertex buffer copies into the default heaps.
	for (UINT i = 0; i < GraphicsAdaptersCount; i++)
	{
		ThrowIfFailed(m_directCommandLists[i]->Close());
		ID3D12CommandList* ppCommandLists[] = { m_directCommandLists[i].Get() };
		m_directCommandQueues[i]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	// We use a cross-adapter fence for handling Signals and Waits between adapters.
	// We use regular fences for things that don't need to be cross adapter because they don't need the additional overhead associated with being cross-adapter.
	{
		// Fence used to control CPU pacing.
		ThrowIfFailed(m_devices[Secondary]->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_frameFence)));

		// Fence used by the primary adapter to signal its copy queue that it has completed rendering.
		// When this is signaled, the primary adapter's copy queue can begin copying to the cross-adapter shared resource.
		ThrowIfFailed(m_devices[Primary]->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_renderFence)));

		// Cross-adapter shared fence used by both adapters.
		// Used by the primary adapter to signal the secondary adapter that it has completed copying to the cross-adapter shared resource.
		// When this is signaled, the secondary adapter can begin its work.
		ThrowIfFailed(m_devices[Primary]->CreateFence(0, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, IID_PPV_ARGS(&m_crossAdapterFences[Primary]))); 

		// For now, require GENERIC_ALL access.
		HANDLE fenceHandle = nullptr;
		ThrowIfFailed(m_devices[Primary]->CreateSharedHandle(
			m_crossAdapterFences[Primary].Get(),
			nullptr,
			GENERIC_ALL,
			nullptr,
			&fenceHandle));

		HRESULT openSharedHandleResult = m_devices[Secondary]->OpenSharedHandle(fenceHandle, IID_PPV_ARGS(&m_crossAdapterFences[Secondary]));

		// We can close the handle after opening the cross-adapter shared fence.
		CloseHandle(fenceHandle);

		ThrowIfFailed(openSharedHandleResult);

		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			// Create an event handle to use for frame synchronization.
			m_fenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvents == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

			// Wait for the command list to execute; we are reusing the same command 
			// list in our main loop but for now, we just want to wait for setup to 
			// complete before continuing.
			WaitForGpu(static_cast<GraphicsAdapter>(i));
		}
	}
}

// Get a random float value between min and max.
float D3D12HeterogeneousMultiadapter::GetRandomFloat(float min, float max)
{
	float scale = static_cast<float>(rand()) / RAND_MAX;
	float range = max - min;
	return scale * range + min;
}

// Update frame-based values.
void D3D12HeterogeneousMultiadapter::OnUpdate()
{
	// Add the oldest timestamp data to our moving average counters.
	// Use the oldest timestamp index to limit CPU waits.
	{
		// The oldest frame is the current frame index and it will always be complete due to the wait in MoveToNextFrame().
		const UINT oldestFrameIndex = m_frameIndex;
		assert(m_frameFenceValues[oldestFrameIndex] <= m_frameFence->GetCompletedValue());

		// Get the timestamp values from the result buffers.
		D3D12_RANGE readRange = {};
		const D3D12_RANGE emptyRange = {};

		UINT64* ppMovingAverage[] = { m_drawTimes, m_blurTimes };
		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			readRange.Begin = 2 * oldestFrameIndex * sizeof(UINT64);
			readRange.End = readRange.Begin + 2 * sizeof(UINT64);

			void* pData = nullptr;
			ThrowIfFailed(m_timestampResultBuffers[i]->Map(0, &readRange, &pData));

			const UINT64* pTimestamps = reinterpret_cast<UINT64*>(static_cast<UINT8*>(pData) + readRange.Begin);
			const UINT64 timeStampDelta = pTimestamps[1] - pTimestamps[0];

			// Unmap with an empty range (written range).
			m_timestampResultBuffers[i]->Unmap(0, &emptyRange);

			// Calculate the GPU execution time in microseconds.
			const UINT64 gpuTimeUS =  (timeStampDelta * 1000000) / m_directCommandQueueTimestampFrequencies[i];
			ppMovingAverage[i][m_currentTimesIndex] = gpuTimeUS;
		}

		// Move to the next index.
		m_currentTimesIndex = (m_currentTimesIndex + 1) % MovingAverageFrameCount;
	}

	// Dynamically change the workload on the primary adapter. This is a VERY naive implementation.
	// The point here is to show that applications have a choice with how to spend their extra cycles.
	// Note: If copies take longer then you should take that into account as well.
	{
		static UINT64 framesSinceLastUpdate = 0;
		framesSinceLastUpdate++;
		if (framesSinceLastUpdate > MovingAverageFrameCount)
		{
			// Calculate the average draw and blur times for last few frames.
			m_drawTimeMovingAverage = 0;
			m_blurTimeMovingAverage = 0;
			for (UINT i = 0; i < MovingAverageFrameCount; i++)
			{
				m_drawTimeMovingAverage += m_drawTimes[i];
				m_blurTimeMovingAverage += m_blurTimes[i];
			}

			m_drawTimeMovingAverage /= MovingAverageFrameCount;
			m_blurTimeMovingAverage /= MovingAverageFrameCount;
			framesSinceLastUpdate = 0;

			// Adjust the shader blur time to be at least 20ms/frame.
			// Note: This is just done to show that we can reach ~100% utilization of both adapters.
			if (AllowShaderDynamicWorkload)
			{
				const UINT64 desiredBlurPSTimeUS = 20000;	// 20 ms
				if (m_blurTimeMovingAverage < desiredBlurPSTimeUS || m_blurPSLoopCount != 0)
				{
					// Adjust the PS blur time based on the moving average.
					const float timeDelta = (static_cast<float>(desiredBlurPSTimeUS) - static_cast<float>(m_blurTimeMovingAverage)) / static_cast<float>(m_blurTimeMovingAverage);
					if (timeDelta < -.05f || timeDelta > .01f)
					{
						const float stepSize = max(1.0f, m_blurPSLoopCount);
						m_blurPSLoopCount += static_cast<INT>(stepSize * timeDelta);
					}
				}
			}

			// Adjust the render time to be greater than the blur time.
			{
				const UINT64 desiredDrawPSTimeUS = m_blurTimeMovingAverage + static_cast<UINT64>(m_blurTimeMovingAverage * .10f);
				const float timeDelta = (static_cast<float>(desiredDrawPSTimeUS) - static_cast<float>(m_drawTimeMovingAverage)) / static_cast<float>(m_drawTimeMovingAverage);
				if (timeDelta < -.10f || timeDelta > .01f)
				{
					if (AllowDrawDynamicWorkload)
					{
						// Adjust the number of triangles drawn.
						const float stepSize = max(1.0f, m_triangleCount);
						m_triangleCount = min(m_triangleCount + static_cast<INT>(stepSize * timeDelta), MaxTriangleCount);
					}
					else if (AllowShaderDynamicWorkload)
					{
						// Adjust the number of the PS loop count based on the moving average.
						const float stepSize = max(1.0f, m_psLoopCount);
						m_psLoopCount += static_cast<INT>(stepSize * timeDelta);
					}
				}
			}
		}

		// Conditionally update the window's title.
		if (framesSinceLastUpdate % WindowTextUpdateFrequency == 0)
		{
			UpdateWindowTitle();
		}
	}

	// Update the workloads.
	{
		WorkloadConstantBufferData* pWorkloadDst = m_pWorkloadCbvDataBegin + m_frameIndex;
		WorkloadConstantBufferData* pWorkloadSrc = &m_workloadConstantBufferData;
		pWorkloadSrc->loopCount = m_psLoopCount;
		memcpy(pWorkloadDst, pWorkloadSrc, sizeof(WorkloadConstantBufferData));

		WorkloadConstantBufferData* pBlurWorkloadDst = m_pBlurWorkloadCbvDataBegin + m_frameIndex;
		WorkloadConstantBufferData* pBlurWorkloadSrc = &m_blurWorkloadConstantBufferData;
		pBlurWorkloadSrc->loopCount = m_blurPSLoopCount;
		memcpy(pBlurWorkloadDst, pBlurWorkloadSrc, sizeof(WorkloadConstantBufferData));
	}

	// Update the triangles.
	{
		const float offsetBounds = 2.5f;

		for (UINT n = 0; n < m_triangleCount; n++)
		{
			// Animate the triangles.
			m_constantBufferData[n].offset.x += m_constantBufferData[n].velocity.x;
			if (m_constantBufferData[n].offset.x > offsetBounds)
			{
				m_constantBufferData[n].velocity.x = GetRandomFloat(0.01f, 0.02f);
				m_constantBufferData[n].offset.x = -offsetBounds;
			}
		}

		SceneConstantBuffer* dst = m_pCbvDataBegin + (m_frameIndex * MaxTriangleCount);
		memcpy(dst, &m_constantBufferData[0], m_triangleCount * sizeof(SceneConstantBuffer));
	}
}

// Render the scene.
void D3D12HeterogeneousMultiadapter::OnRender()
{
	// Record all the commands we need to render the scene into the command lists.
	PopulateCommandLists();

	// Execute the command lists.
	{
		{
			ID3D12CommandList* ppRenderCommandLists[] = { m_directCommandLists[Primary].Get() };
			m_directCommandQueues[Primary]->ExecuteCommandLists(_countof(ppRenderCommandLists), ppRenderCommandLists);

			// Signal the copy queue to indicate render is complete.
			ThrowIfFailed(m_directCommandQueues[Primary]->Signal(m_renderFence.Get(), m_currentRenderFenceValue));
		}

		{
			// GPU Wait for the primary adapter to finish rendering.
			ThrowIfFailed(m_copyCommandQueue->Wait(m_renderFence.Get(), m_currentRenderFenceValue));
			m_currentRenderFenceValue++;

			ID3D12CommandList* ppCopyCommandLists[] = { m_copyCommandList.Get() };
			m_copyCommandQueue->ExecuteCommandLists(_countof(ppCopyCommandLists), ppCopyCommandLists);

			// Signal the secondary adapter to indicate the copy is complete.
			ThrowIfFailed(m_copyCommandQueue->Signal(m_crossAdapterFences[Primary].Get(), m_currentCrossAdapterFenceValue));
		}

		{
			// GPU Wait for the primary adapter to finish copying.
			ThrowIfFailed(m_directCommandQueues[Secondary]->Wait(m_crossAdapterFences[Secondary].Get(), m_currentCrossAdapterFenceValue));
			m_currentCrossAdapterFenceValue++;

			ID3D12CommandList* ppBlurCommandLists[] = { m_directCommandLists[Secondary].Get() };
			m_directCommandQueues[Secondary]->ExecuteCommandLists(_countof(ppBlurCommandLists), ppBlurCommandLists);
		}
	}

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	// Signal the frame is complete.
	ThrowIfFailed(m_directCommandQueues[Secondary]->Signal(m_frameFence.Get(), m_currentPresentFenceValue));
	m_frameFenceValues[m_frameIndex] = m_currentPresentFenceValue;
	m_currentPresentFenceValue++;

	MoveToNextFrame();
}

void D3D12HeterogeneousMultiadapter::OnDestroy()
{
	// Ensure that the GPUs are no longer referencing resources that are about to be
	// cleaned up by the destructor.
	for (UINT i = 0; i < GraphicsAdaptersCount; i++)
	{
		WaitForGpu(static_cast<GraphicsAdapter>(i));
		CloseHandle(m_fenceEvents[i]);
	}
}

// Fill the command list with all the render commands and dependent state.
void D3D12HeterogeneousMultiadapter::PopulateCommandLists()
{
	// Command list to render target the triangles on the primary adapter.
	{
		const GraphicsAdapter adapter = Primary;

		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ThrowIfFailed(m_directCommandAllocators[adapter][m_frameIndex]->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		ThrowIfFailed(m_directCommandLists[adapter]->Reset(m_directCommandAllocators[adapter][m_frameIndex].Get(), m_pipelineState.Get()));

		// Get a timestamp at the start of the command list.
		const UINT timestampHeapIndex = 2 * m_frameIndex;
		m_directCommandLists[adapter]->EndQuery(m_timestampQueryHeaps[adapter].Get(), D3D12_QUERY_TYPE_TIMESTAMP, timestampHeapIndex);

		// Set necessary state.
		m_directCommandLists[adapter]->SetGraphicsRootSignature(m_rootSignature.Get());

		m_directCommandLists[adapter]->RSSetViewports(1, &m_viewport);
		m_directCommandLists[adapter]->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the render target will be used as a render target.
		m_directCommandLists[adapter]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[adapter][m_frameIndex].Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeaps[adapter]->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSizes[adapter]);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		m_directCommandLists[adapter]->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

		// Record commands.
		m_directCommandLists[adapter]->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);
		m_directCommandLists[adapter]->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// Draw the triangles.
		m_directCommandLists[adapter]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_directCommandLists[adapter]->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_directCommandLists[adapter]->SetGraphicsRootConstantBufferView(1, m_workloadConstantBuffer->GetGPUVirtualAddress() + (m_frameIndex * sizeof(WorkloadConstantBufferData)));

		const D3D12_GPU_VIRTUAL_ADDRESS cbVirtualAddress = m_constantBuffer->GetGPUVirtualAddress();
		for (UINT n = 0; n < m_triangleCount; n++)
		{
			const D3D12_GPU_VIRTUAL_ADDRESS cbLocation = cbVirtualAddress + (m_frameIndex * MaxTriangleCount * sizeof(SceneConstantBuffer)) + (n * sizeof(SceneConstantBuffer));
			m_directCommandLists[adapter]->SetGraphicsRootConstantBufferView(0, cbLocation);
			m_directCommandLists[adapter]->DrawInstanced(3, 1, 0, 0);
		}

		// Indicate that the render target will now be used to copy.
		m_directCommandLists[adapter]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[adapter][m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));

		// Get a timestamp at the end of the command list and resolve the query data.
		m_directCommandLists[adapter]->EndQuery(m_timestampQueryHeaps[adapter].Get(), D3D12_QUERY_TYPE_TIMESTAMP, timestampHeapIndex + 1);
		m_directCommandLists[adapter]->ResolveQueryData(m_timestampQueryHeaps[adapter].Get(), D3D12_QUERY_TYPE_TIMESTAMP, timestampHeapIndex, 2, m_timestampResultBuffers[adapter].Get(), timestampHeapIndex * sizeof(UINT64));

		ThrowIfFailed(m_directCommandLists[adapter]->Close());
	}

	// Command list to copy the render target to the shared heap on the primary adapter.
	{
		const GraphicsAdapter adapter = Primary;

		// Reset the copy command allocator and command list.
		ThrowIfFailed(m_copyCommandAllocators[m_frameIndex]->Reset());
		ThrowIfFailed(m_copyCommandList->Reset(m_copyCommandAllocators[m_frameIndex].Get(), nullptr));

		// Copy the intermediate render target to the cross-adapter shared resource.
		// Transition barriers are not required since there are fences guarding against
		// concurrent read/write access to the shared heap.
		if (m_crossAdapterTextureSupport)
		{
			// If cross-adapter row-major textures are supported by the adapter,
			// simply copy the texture into the cross-adapter texture.
			m_copyCommandList->CopyResource(m_crossAdapterResources[adapter][m_frameIndex].Get(), m_renderTargets[adapter][m_frameIndex].Get());
		}
		else
		{
			// If cross-adapter row-major textures are not supported by the adapter,
			// the texture will be copied over as a buffer so that the texture row
			// pitch can be explicitly managed.

			// Copy the intermediate render target into the shared buffer using the
			// memory layout prescribed by the render target.
			D3D12_RESOURCE_DESC renderTargetDesc = m_renderTargets[adapter][m_frameIndex]->GetDesc();
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;

			m_devices[adapter]->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);

			CD3DX12_TEXTURE_COPY_LOCATION dest(m_crossAdapterResources[adapter][m_frameIndex].Get(), renderTargetLayout);
			CD3DX12_TEXTURE_COPY_LOCATION src(m_renderTargets[adapter][m_frameIndex].Get(), 0);
			CD3DX12_BOX box(0, 0, m_width, m_height);

			m_copyCommandList->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
		}

		ThrowIfFailed(m_copyCommandList->Close());
	}

	// Command list to blur the render target and present.
	{
		const GraphicsAdapter adapter = Secondary;

		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ThrowIfFailed(m_directCommandAllocators[adapter][m_frameIndex]->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		ThrowIfFailed(m_directCommandLists[adapter]->Reset(m_directCommandAllocators[adapter][m_frameIndex].Get(), m_blurPipelineStates[0].Get()));

		if (!m_crossAdapterTextureSupport)
		{
			// Copy the buffer in the shared heap into a texture that the secondary
			// adapter can sample from.
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_secondaryAdapterTextures[m_frameIndex].Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_COPY_DEST);
			m_directCommandLists[adapter]->ResourceBarrier(1, &barrier);

			// Copy the shared buffer contents into the texture using the memory
			// layout prescribed by the texture.
			D3D12_RESOURCE_DESC secondaryAdapterTexture = m_secondaryAdapterTextures[m_frameIndex]->GetDesc();
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;

			m_devices[adapter]->GetCopyableFootprints(&secondaryAdapterTexture, 0, 1, 0, &textureLayout, nullptr, nullptr, nullptr);

			CD3DX12_TEXTURE_COPY_LOCATION dest(m_secondaryAdapterTextures[m_frameIndex].Get(), 0);
			CD3DX12_TEXTURE_COPY_LOCATION src(m_crossAdapterResources[adapter][m_frameIndex].Get(), textureLayout);
			CD3DX12_BOX box(0, 0, m_width, m_height);

			m_directCommandLists[adapter]->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			m_directCommandLists[adapter]->ResourceBarrier(1, &barrier);
		}

		// Get a timestamp at the start of the command list.
		const UINT timestampHeapIndex = 2 * m_frameIndex;
		m_directCommandLists[adapter]->EndQuery(m_timestampQueryHeaps[adapter].Get(), D3D12_QUERY_TYPE_TIMESTAMP, timestampHeapIndex);

		// Set necessary state.
		m_directCommandLists[adapter]->SetGraphicsRootSignature(m_blurRootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get() };
		m_directCommandLists[adapter]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		m_directCommandLists[adapter]->RSSetViewports(1, &m_viewport);
		m_directCommandLists[adapter]->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the intermediate render target will be used as a render target.
		m_directCommandLists[adapter]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_intermediateBlurRenderTarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// Record commands.
		m_directCommandLists[adapter]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_directCommandLists[adapter]->IASetVertexBuffers(0, 1, &m_fullscreenQuadVertexBufferView);
		m_directCommandLists[adapter]->SetGraphicsRootConstantBufferView(0, m_blurConstantBuffer->GetGPUVirtualAddress());
		m_directCommandLists[adapter]->SetGraphicsRootConstantBufferView(2, m_blurWorkloadConstantBuffer->GetGPUVirtualAddress() + (m_frameIndex * sizeof(WorkloadConstantBufferData)));

		// Draw the fullscreen quad - Blur pass #1.
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), m_frameIndex, m_srvDescriptorSizes[adapter]);
			m_directCommandLists[adapter]->SetGraphicsRootDescriptorTable(1, srvHandle);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeaps[adapter]->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSizes[adapter]);
			m_directCommandLists[adapter]->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

			m_directCommandLists[adapter]->DrawInstanced(4, 1, 0, 0);
		}

		// Draw the fullscreen quad - Blur pass #2.
		{
			m_directCommandLists[adapter]->SetPipelineState(m_blurPipelineStates[1].Get());

			// Indicate that the back buffer will be used as a render target and the
			// intermediate render target will be used as a SRV.
			D3D12_RESOURCE_BARRIER barriers[] = {
				CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[adapter][m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
				CD3DX12_RESOURCE_BARRIER::Transition(m_intermediateBlurRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			};

			m_directCommandLists[adapter]->ResourceBarrier(_countof(barriers), barriers);

			CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), FrameCount, m_srvDescriptorSizes[adapter]);
			m_directCommandLists[adapter]->SetGraphicsRootDescriptorTable(1, srvHandle);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeaps[adapter]->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSizes[adapter]);
			m_directCommandLists[adapter]->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

			m_directCommandLists[adapter]->DrawInstanced(4, 1, 0, 0);
		}

		// Indicate that the back buffer will now be used to present.
		m_directCommandLists[adapter]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[adapter][m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// Get a timestamp at the end of the command list and resolve the query data.
		m_directCommandLists[adapter]->EndQuery(m_timestampQueryHeaps[adapter].Get(), D3D12_QUERY_TYPE_TIMESTAMP, timestampHeapIndex + 1);
		m_directCommandLists[adapter]->ResolveQueryData(m_timestampQueryHeaps[adapter].Get(), D3D12_QUERY_TYPE_TIMESTAMP, timestampHeapIndex, 2, m_timestampResultBuffers[adapter].Get(), timestampHeapIndex * sizeof(UINT64));

		ThrowIfFailed(m_directCommandLists[adapter]->Close());
	}
}

void D3D12HeterogeneousMultiadapter::UpdateWindowTitle()
{
	std::wstringstream stringStream;

	stringStream << L"[" << m_triangleCount << L" triangles]";
	stringStream << L" [Render, " << m_adapterDescs[Primary].Description << ": " << m_drawTimeMovingAverage << L"us" << L" (PS loop count : " << m_psLoopCount<< ")]";
	stringStream << L" [Blur, " << m_adapterDescs[Secondary].Description << ": " << m_blurTimeMovingAverage << L"us" << L" (PS loop count : " << m_blurPSLoopCount << ")]";

	SetCustomWindowText(stringStream.str().c_str());
}

// Wait for pending GPU work to complete.
void D3D12HeterogeneousMultiadapter::WaitForGpu(GraphicsAdapter adapter)
{
	// Schedule a Signal command in the queue.
	// Note we just re-use the cross adapter fence for convenience.
	ThrowIfFailed(m_directCommandQueues[adapter]->Signal(m_crossAdapterFences[adapter].Get(), m_currentCrossAdapterFenceValue));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_crossAdapterFences[adapter]->SetEventOnCompletion(m_currentCrossAdapterFenceValue, m_fenceEvents[adapter]));
	WaitForSingleObject(m_fenceEvents[adapter], INFINITE);
	m_currentCrossAdapterFenceValue++;
}

// Prepare to render the next frame.
void D3D12HeterogeneousMultiadapter::MoveToNextFrame()
{
	// Get the current the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	const UINT64 completedFenceValue = m_frameFence->GetCompletedValue();
	if (completedFenceValue < m_frameFenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_frameFence->SetEventOnCompletion(m_frameFenceValues[m_frameIndex], m_fenceEvents[Secondary]));
		WaitForSingleObject(m_fenceEvents[Secondary], INFINITE);
	}
}
