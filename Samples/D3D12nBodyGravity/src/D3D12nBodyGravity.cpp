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
#include "D3D12nBodyGravity.h"
#include <string>
#include <numeric>

#ifdef min
#undef min
#endif

// InterlockedCompareExchange returns the object's value if the 
// comparison fails.  If it is already 0, then its value won't 
// change and 0 will be returned.
#define InterlockedGetValue(object) InterlockedCompareExchange(object, 0, 0)

const float D3D12nBodyGravity::ParticleSpread = 400.0f;

namespace {
void WaitForFence (ID3D12Fence* fence, UINT64 fenceValue, HANDLE fenceEvent)
{
	if (fence->GetCompletedValue () < fenceValue) {
		ThrowIfFailed (fence->SetEventOnCompletion (fenceValue, fenceEvent));
		WaitForSingleObject (fenceEvent, INFINITE);
	}
}
}

D3D12nBodyGravity::D3D12nBodyGravity(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_frameIndex(0),
	m_viewport(),
	m_scissorRect(),
	m_rtvDescriptorSize(0),
	m_srvUavDescriptorSize(0),
	m_pConstantBufferGSData(nullptr)
{
	ZeroMemory(&m_srvIndex, sizeof(m_srvIndex));
	ZeroMemory(m_frameFenceValues, sizeof(m_frameFenceValues));

	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.right = static_cast<LONG>(width);
	m_scissorRect.bottom = static_cast<LONG>(height);

	ZeroMemory (m_computeFenceValues, sizeof (m_computeFenceValues));
	ZeroMemory (m_graphicsFenceValues, sizeof (m_graphicsFenceValues));
	ZeroMemory (m_graphicsCopyFenceValues, sizeof (m_graphicsCopyFenceValues));

	m_computeFenceValue = 0;
	m_graphicsFenceValue = 0;
	m_graphicsCopyFenceValue = 0;

	m_queryReadbackIndex = -(static_cast<int> (FrameCount));

	m_frameTimeNextEntry = 0;
	m_frameTimeEntryCount = 0;
}

void D3D12nBodyGravity::OnInit()
{
	m_camera.Init({ 0.0f, 0.0f, 1500.0f });
	m_camera.SetMoveSpeed(250.0f);

	LoadPipeline();
	LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12nBodyGravity::LoadPipeline()
{
#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

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

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_graphicsCommandQueue)));
	NAME_D3D12_OBJECT(m_graphicsCommandQueue);

	m_device->SetStablePowerState (true);
	m_graphicsCommandQueue->GetTimestampFrequency (&m_frequency);

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.BufferDesc.Width = m_width;
	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = Win32Application::GetHwnd();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.Flags = 0;

	ComPtr<IDXGISwapChain> swapChain;
	ThrowIfFailed(factory->CreateSwapChain(
		m_graphicsCommandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
		&swapChainDesc,
		&swapChain
		));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_lastFrameIndex = m_frameIndex;

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		// Describe and create a shader resource view (SRV) and unordered
		// access view (UAV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC srvUavHeapDesc = {};
		srvUavHeapDesc.NumDescriptors = DescriptorCount;
		srvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvUavHeapDesc, IID_PPV_ARGS(&m_srvUavHeap)));
		NAME_D3D12_OBJECT(m_srvUavHeap);

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_srvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	// graphics resources
	{
		for (int i = 0; i < FrameCount; ++i) {
			ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&m_graphicsAllocators[i])));
			ThrowIfFailed (m_device->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_graphicsAllocators[i].Get (), nullptr, IID_PPV_ARGS (&m_graphicsCommandLists[i])));
			ThrowIfFailed (m_device->CreateFence (0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS (&m_graphicsFences[i])));

			wchar_t buffer[64] = {};
			swprintf_s (buffer, L"m_graphicsCommandLists[%i]", i);
			SetName (m_graphicsCommandLists[i].Get (), buffer);
			m_graphicsCommandLists[i]->Close ();

			ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&m_graphicsCopyAllocators[i])));
			ThrowIfFailed (m_device->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_graphicsCopyAllocators[i].Get (), nullptr, IID_PPV_ARGS (&m_graphicsCopyCommandLists[i])));

			swprintf_s (buffer, L"m_graphicsCopyCommandLists[%i]", i);
			SetName (m_graphicsCopyCommandLists[i].Get (), buffer);
			ThrowIfFailed (m_device->CreateFence (0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS (&m_graphicsCopyFences[i])));

			m_graphicsCopyCommandLists[i]->Close ();
		}
	}

	// compute resources
	if (AsynchronousComputeEnabled)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = { D3D12_COMMAND_LIST_TYPE_COMPUTE, 0, D3D12_COMMAND_QUEUE_FLAG_NONE };
		ThrowIfFailed (m_device->CreateCommandQueue (&queueDesc, IID_PPV_ARGS (&m_computeCommandQueue)));
		NAME_D3D12_OBJECT (m_computeCommandQueue);

		for (int i = 0; i < FrameCount; ++i) {
			ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS (&m_computeAllocators[i])));
			ThrowIfFailed (m_device->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_computeAllocators[i].Get (), nullptr, IID_PPV_ARGS (&m_computeCommandLists[i])));
			ThrowIfFailed (m_device->CreateFence (0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS (&m_computeFences[i])));

			wchar_t buffer[64] = {};
			swprintf_s (buffer, L"m_computeCommandLists[%i] (compute queue)", i);
			SetName (m_computeCommandLists[i].Get (), buffer);

			m_computeCommandLists[i]->Close ();
		}
	}
	else
	{
		for (int i = 0; i < FrameCount; ++i) {
			ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&m_computeAllocators [i])));
			ThrowIfFailed (m_device->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_computeAllocators [i].Get (), nullptr, IID_PPV_ARGS (&m_computeCommandLists [i])));

			wchar_t buffer[64] = {};
			swprintf_s (buffer, L"m_computeCommandLists[%i] (graphics queue)", i);
			SetName (m_computeCommandLists[i].Get (), buffer);

			ThrowIfFailed (m_device->CreateFence (0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS (&m_computeFences [i])));

			m_computeCommandLists [i]->Close ();
		}
	}

	// timing resources
	{
		D3D12_RESOURCE_DESC cpuTimingBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (
			sizeof (UINT64));

		for (int i = 0; i < FrameCount; ++i) {
			m_device->CreateCommittedResource (
				&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&cpuTimingBufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS (&m_timeQueryReadbackBuffer [i]));
		}

		D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
		queryHeapDesc.Count = FrameCount; // Query at end of frame
		queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

		m_device->CreateQueryHeap (&queryHeapDesc,
			IID_PPV_ARGS (&m_timeQueryHeap));
		NAME_D3D12_OBJECT (m_timeQueryHeap);
	}
}

// Load the sample assets.
void D3D12nBodyGravity::LoadAssets()
{
	// Create the root signatures.
	{
		CD3DX12_DESCRIPTOR_RANGE ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[RootParametersCount];
		rootParameters[RootParameterCB].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[RootParameterSRV].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[RootParameterUAV].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

		// The rendering pipeline does not need the UAV parameter.
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters) - 1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		NAME_D3D12_OBJECT(m_rootSignature);

		// Create compute signature. Must change visibility for the SRV.
		rootParameters[RootParameterSRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		CD3DX12_ROOT_SIGNATURE_DESC computeRootSignatureDesc(_countof(rootParameters), rootParameters, 0, nullptr);
		ThrowIfFailed(D3D12SerializeRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSignature)));
		NAME_D3D12_OBJECT(m_computeRootSignature);
	}

	// Create the pipeline states, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> geometryShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> computeShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		// Load and compile shaders.
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ParticleDraw.hlsl").c_str(), nullptr, nullptr, "VSParticleDraw", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ParticleDraw.hlsl").c_str(), nullptr, nullptr, "GSParticleDraw", "gs_5_0", compileFlags, 0, &geometryShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ParticleDraw.hlsl").c_str(), nullptr, nullptr, "PSParticleDraw", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"NBodyGravityCS.hlsl").c_str(), nullptr, nullptr, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Describe the blend and depth states.
		CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.GS = CD3DX12_SHADER_BYTECODE(geometryShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = blendDesc;
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
		NAME_D3D12_OBJECT(m_pipelineState);

		// Describe and create the compute pipeline state object (PSO).
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
		computePsoDesc.pRootSignature = m_computeRootSignature.Get();
		computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());

		ThrowIfFailed(m_device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_computeState)));
		NAME_D3D12_OBJECT(m_computeState);
	}

	// Create the upload command list.
	ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&m_uploadCommandAllocator)));
	ThrowIfFailed (m_device->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_uploadCommandAllocator.Get (), m_pipelineState.Get (), IID_PPV_ARGS (&m_uploadCommandList)));
	NAME_D3D12_OBJECT (m_uploadCommandList);

	CreateVertexBuffer();
	CreateParticleBuffers();

	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> constantBufferCSUpload;

	// Create the compute shader's constant buffer.
	{
		const UINT bufferSize = sizeof(ConstantBufferCS);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_constantBufferCS)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constantBufferCSUpload)));

		NAME_D3D12_OBJECT(m_constantBufferCS);

		ConstantBufferCS constantBufferCS = {};
		constantBufferCS.param[0] = ParticleCount;
		constantBufferCS.param[1] = int(ceil(ParticleCount / 256.0f));
		constantBufferCS.paramf[0] = 0.1f;
		constantBufferCS.paramf[1] = 1.0f;

		D3D12_SUBRESOURCE_DATA computeCBData = {};
		computeCBData.pData = reinterpret_cast<UINT8*>(&constantBufferCS);
		computeCBData.RowPitch = bufferSize;
		computeCBData.SlicePitch = computeCBData.RowPitch;

		UpdateSubresources<1>(m_uploadCommandList.Get(), m_constantBufferCS.Get(), constantBufferCSUpload.Get(), 0, 0, 1, &computeCBData);
		m_uploadCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_constantBufferCS.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	}

	// Create the geometry shader's constant buffer.
	{
		const UINT constantBufferGSSize = sizeof(ConstantBufferGS) * FrameCount;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferGSSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBufferGS)
			));

		NAME_D3D12_OBJECT(m_constantBufferGS);

		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_constantBufferGS->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBufferGSData)));
		ZeroMemory(m_pConstantBufferGSData, constantBufferGSSize);
	}

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(m_uploadCommandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_uploadCommandList.Get() };
	m_graphicsCommandQueue->ExecuteCommandLists(1, ppCommandLists);

	m_frameFenceValue = 0;
	for (int i = 0; i < FrameCount; ++i) {
		m_frameFenceValues[i] = m_frameFenceValue;
		m_device->CreateFence (m_frameFenceValues[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_frameFences[i]));
		m_frameFenceEvents[i] = CreateEvent (nullptr, FALSE, FALSE, nullptr);

		m_graphicsFenceValues[i] = 0;
		ThrowIfFailed(m_device->CreateFence(m_graphicsFenceValues[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_graphicsFences[i])));
		m_graphicsFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		m_graphicsCopyFenceValues[i] = 0;
		ThrowIfFailed (m_device->CreateFence (m_graphicsCopyFenceValues[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_graphicsCopyFences[i])));
		m_graphicsCopyFenceEvents[i] = CreateEvent (nullptr, FALSE, FALSE, nullptr);

		m_computeFenceValues[i] = 0;
		ThrowIfFailed (m_device->CreateFence (m_computeFenceValues[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_computeFences[i])));
		m_computeFenceEvents[i] = CreateEvent (nullptr, FALSE, FALSE, nullptr);
	}

	{
		m_uploadFenceValue = 0;
		ThrowIfFailed (m_device->CreateFence (m_uploadFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_uploadFence)));

		m_uploadEvent = CreateEvent (nullptr, FALSE, FALSE, nullptr);

		m_graphicsCommandQueue->Signal (m_uploadFence.Get (), 1);
		WaitForFence (m_uploadFence.Get (), 1, m_uploadEvent);
	}
}

// Create the particle vertex buffer.
void D3D12nBodyGravity::CreateVertexBuffer()
{
	std::vector<ParticleVertex> vertices;
	vertices.resize(ParticleCount);
	for (UINT i = 0; i < ParticleCount; i++)
	{
		vertices[i].color = XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f);
	}
	const UINT bufferSize = ParticleCount * sizeof(ParticleVertex);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBufferUpload)));

	NAME_D3D12_OBJECT(m_vertexBuffer);

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<UINT8*>(&vertices[0]);
	vertexData.RowPitch = bufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources<1>(m_uploadCommandList.Get(), m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
	m_uploadCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	m_vertexBufferView.StrideInBytes = sizeof(ParticleVertex);
}

// Random percent value, from -1 to 1.
float D3D12nBodyGravity::RandomPercent()
{
	float ret = static_cast<float>((rand() % 10000) - 5000);
	return ret / 5000.0f;
}

void D3D12nBodyGravity::LoadParticles(_Out_writes_(numParticles) Particle* pParticles, const XMFLOAT3& center, const XMFLOAT4& velocity, float spread, UINT numParticles)
{
	srand(0);
	for (UINT i = 0; i < numParticles; i++)
	{
		XMFLOAT3 delta(spread, spread, spread);

		while (XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&delta))) > spread * spread)
		{
			delta.x = RandomPercent() * spread;
			delta.y = RandomPercent() * spread;
			delta.z = RandomPercent() * spread;
		}

		pParticles[i].position.x = center.x + delta.x;
		pParticles[i].position.y = center.y + delta.y;
		pParticles[i].position.z = center.z + delta.z;
		pParticles[i].position.w = 10000.0f * 10000.0f;

		pParticles[i].velocity = velocity;
	}
}

// Create the position and velocity buffer shader resources.
void D3D12nBodyGravity::CreateParticleBuffers()
{
	// Initialize the data in the buffers.
	std::vector<Particle> data;
	data.resize(ParticleCount);
	const UINT dataSize = ParticleCount * sizeof(Particle);

	// Split the particles into two groups.
	float centerSpread = ParticleSpread * 0.50f;
	LoadParticles(&data[0], XMFLOAT3(centerSpread, 0, 0), XMFLOAT4(0, 0, -20, 1 / 100000000.0f), ParticleSpread, ParticleCount / 2);
	LoadParticles(&data[ParticleCount / 2], XMFLOAT3(-centerSpread, 0, 0), XMFLOAT4(0, 0, 20, 1 / 100000000.0f), ParticleSpread, ParticleCount / 2);

	D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

		// Create two buffers in the GPU, each with a copy of the particles data.
		// The compute shader will update one of them while the rendering thread 
		// renders the other. When rendering completes, the threads will swap 
		// which buffer they work on.

		ThrowIfFailed(m_device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
	IID_PPV_ARGS(&m_particleBuffer0)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
	IID_PPV_ARGS(&m_particleBuffer1)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
	IID_PPV_ARGS(&m_particleBuffer0Upload)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
	IID_PPV_ARGS(&m_particleBuffer1Upload)));

	ThrowIfFailed (m_device->CreateCommittedResource (
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS (&m_particleBufferForDraw)));


	SetName(m_particleBuffer0.Get(), L"m_particleBuffer0");
	SetName(m_particleBuffer1.Get(), L"m_particleBuffer1");

	D3D12_SUBRESOURCE_DATA particleData = {};
	particleData.pData = reinterpret_cast<UINT8*>(&data[0]);
	particleData.RowPitch = dataSize;
	particleData.SlicePitch = particleData.RowPitch;

	UpdateSubresources<1>(m_uploadCommandList.Get(), m_particleBuffer0.Get(), m_particleBuffer0Upload.Get(), 0, 0, 1, &particleData);
	UpdateSubresources<1>(m_uploadCommandList.Get(), m_particleBuffer1.Get(), m_particleBuffer1Upload.Get(), 0, 0, 1, &particleData);
	m_uploadCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_particleBuffer0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	m_uploadCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_particleBuffer1.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = ParticleCount;
	srvDesc.Buffer.StructureByteStride = sizeof(Particle);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle0(m_srvUavHeap->GetCPUDescriptorHandleForHeapStart(), SrvParticlePosVelo0, m_srvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle1(m_srvUavHeap->GetCPUDescriptorHandleForHeapStart(), SrvParticlePosVelo1, m_srvUavDescriptorSize);
	m_device->CreateShaderResourceView(m_particleBuffer0.Get(), &srvDesc, srvHandle0);
	m_device->CreateShaderResourceView(m_particleBuffer1.Get(), &srvDesc, srvHandle1);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = ParticleCount;
	uavDesc.Buffer.StructureByteStride = sizeof(Particle);
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle0(m_srvUavHeap->GetCPUDescriptorHandleForHeapStart(), UavParticlePosVelo0, m_srvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle1(m_srvUavHeap->GetCPUDescriptorHandleForHeapStart(), UavParticlePosVelo1, m_srvUavDescriptorSize);
	m_device->CreateUnorderedAccessView(m_particleBuffer0.Get(), nullptr, &uavDesc, uavHandle0);
	m_device->CreateUnorderedAccessView(m_particleBuffer1.Get(), nullptr, &uavDesc, uavHandle1);
}

// Update frame-based values.
void D3D12nBodyGravity::OnUpdate()
{
	// Wait for the previous Present to complete.
	// WaitForSingleObjectEx(m_swapChainEvent, 100, FALSE);

	m_timer.Tick(NULL);
	m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()));

	ConstantBufferGS constantBufferGS = {};
	XMStoreFloat4x4(&constantBufferGS.worldViewProjection, XMMatrixMultiply(m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix(0.8f, m_aspectRatio, 1.0f, 5000.0f)));
	XMStoreFloat4x4(&constantBufferGS.inverseView, XMMatrixInverse(nullptr, m_camera.GetViewMatrix()));

	UINT8* destination = m_pConstantBufferGSData + sizeof(ConstantBufferGS) * m_frameIndex;
	memcpy(destination, &constantBufferGS, sizeof(ConstantBufferGS));
}

// Render the scene.
void D3D12nBodyGravity::OnRender()
{
	// Wait for graphics fence to finish
	if (AsynchronousComputeEnabled) {
		PIXBeginEvent (m_computeCommandQueue.Get (), 0, L"Simulate");
		m_computeCommandQueue->Wait (m_graphicsCopyFences [m_lastFrameIndex].Get (), m_graphicsCopyFenceValues [m_lastFrameIndex]);
	} else {
		PIXBeginEvent (m_graphicsCommandQueue.Get (), 0, L"Simulate");
	}
	RecordComputeCommandList ();

	// Close and execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_computeCommandLists[m_frameIndex].Get () };

	if (AsynchronousComputeEnabled) {
		m_computeCommandQueue->ExecuteCommandLists (1, ppCommandLists);
		m_computeFenceValues [m_frameIndex] = m_computeFenceValue;
		m_computeCommandQueue->Signal (m_computeFences [m_frameIndex].Get (), m_computeFenceValue);
		PIXEndEvent (m_computeCommandQueue.Get ());
	} else {
		m_graphicsCommandQueue->ExecuteCommandLists (1, ppCommandLists);
		PIXEndEvent (m_graphicsCommandQueue.Get ());
	}

	++m_computeFenceValue;

	RecordCopyCommandList ();

	ppCommandLists[0] = { m_graphicsCopyCommandLists[m_frameIndex].Get () };

	// Wait for compute fence to finish
	if (AsynchronousComputeEnabled) {
		m_graphicsCommandQueue->Wait (m_computeFences [m_frameIndex].Get (), m_computeFenceValues [m_frameIndex]);
	}

	// Execute copy
	m_graphicsCommandQueue->ExecuteCommandLists (1, ppCommandLists);
	if (AsynchronousComputeEnabled) {
		m_graphicsCommandQueue->Signal (m_graphicsCopyFences [m_frameIndex].Get (), m_graphicsCopyFenceValue);
	}
	
	PIXBeginEvent (m_graphicsCommandQueue.Get (), 0, L"Render");
	++m_graphicsCopyFenceValue;
	RecordRenderCommandList ();

	// Execute the rendering
	ppCommandLists[0] = { m_graphicsCommandLists[m_frameIndex].Get() };
	m_graphicsCommandQueue->ExecuteCommandLists(1, ppCommandLists);
	if (AsynchronousComputeEnabled) {
		m_graphicsCommandQueue->Signal (m_graphicsFences [m_frameIndex].Get (), m_graphicsFenceValue);
	}
	PIXEndEvent (m_graphicsCommandQueue.Get ());
	++m_graphicsFenceValue;

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(0, 0));

	MoveToNextFrame();
}

void D3D12nBodyGravity::RecordCopyCommandList ()
{
	ThrowIfFailed (m_graphicsCopyAllocators[m_frameIndex]->Reset ());
	ThrowIfFailed (m_graphicsCopyCommandLists[m_frameIndex]->Reset (m_graphicsCopyAllocators[m_frameIndex].Get (), m_pipelineState.Get ()));

	ID3D12GraphicsCommandList* commandList = m_graphicsCopyCommandLists[m_frameIndex].Get ();

	ID3D12Resource* particleBuffer = (m_srvIndex == 0) ? m_particleBuffer0.Get () : m_particleBuffer1.Get ();
	D3D12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition (
		particleBuffer,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->ResourceBarrier (1, barriers);
	commandList->CopyResource (m_particleBufferForDraw.Get (), particleBuffer);

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition (
		particleBuffer,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition (
		m_particleBufferForDraw.Get (),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
		);

	commandList->ResourceBarrier (2, barriers);
	ThrowIfFailed (commandList->Close ());
}

// Fill the command list with all the render commands and dependent state.
void D3D12nBodyGravity::RecordRenderCommandList ()
{
	// Command list allocators can only be reset when the associated
	// command lists have finished execution on the GPU; apps should use
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_graphicsAllocators[m_frameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command
	// list, that command list can then be reset at any time and must be before
	// re-recording.
	ThrowIfFailed(m_graphicsCommandLists[m_frameIndex]->Reset(m_graphicsAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

	ID3D12GraphicsCommandList* commandList = m_graphicsCommandLists[m_frameIndex].Get ();

	// Set necessary state.
	commandList->SetPipelineState(m_pipelineState.Get());
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	commandList->SetGraphicsRootConstantBufferView(RootParameterCB, m_constantBufferGS->GetGPUVirtualAddress() + m_frameIndex * sizeof(ConstantBufferGS));

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvUavHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.1f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Render the particles.
	const UINT srvIndex = (m_srvIndex == 0 ? SrvParticlePosVelo0 : SrvParticlePosVelo1);

		D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = m_viewport.Width;
	viewport.Height = m_viewport.Height;
		viewport.MinDepth = D3D12_MIN_DEPTH;
		viewport.MaxDepth = D3D12_MAX_DEPTH;
	commandList->RSSetViewports(1, &viewport);

		CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(), srvIndex, m_srvUavDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(RootParameterSRV, srvHandle);
	PIXBeginEvent (commandList, 0, L"Draw particles");
	commandList->DrawInstanced(ParticleCount, 1, 0, 0);
	PIXEndEvent (commandList);

	commandList->RSSetViewports(1, &m_viewport);

	// Indicate that the back buffer will now be used to present.
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	D3D12_RESOURCE_BARRIER barriers[1];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition (
		m_particleBufferForDraw.Get (),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_COPY_DEST);

	commandList->ResourceBarrier (1, barriers);

	// Issue query
	commandList->EndQuery (m_timeQueryHeap.Get (), D3D12_QUERY_TYPE_TIMESTAMP,
		m_frameIndex);
	commandList->ResolveQueryData (m_timeQueryHeap.Get (), D3D12_QUERY_TYPE_TIMESTAMP,
		m_frameIndex, 1, m_timeQueryReadbackBuffer[m_frameIndex].Get (), 0);

	ThrowIfFailed(commandList->Close());
}

void D3D12nBodyGravity::RecordComputeCommandList()
{
	ID3D12CommandAllocator* pCommandAllocator = m_computeAllocators[m_frameIndex].Get ();
	ID3D12GraphicsCommandList* pCommandList = m_computeCommandLists[m_frameIndex].Get ();

	// Prepare for the next frame.
	ThrowIfFailed (pCommandAllocator->Reset ());
	ThrowIfFailed (pCommandList->Reset (pCommandAllocator, m_computeState.Get ()));

	// Run the particle simulation multiple steps per frame
	for (int i = 0; i < 4; ++i) {
		Simulate ();
		}

	ThrowIfFailed (pCommandList->Close ());

		// Swap the indices to the SRV and UAV.
	m_srvIndex = 1 - m_srvIndex;
}

// Run the particle simulation using the compute shader.
void D3D12nBodyGravity::Simulate()
{
	ID3D12GraphicsCommandList* pCommandList = m_computeCommandLists[m_frameIndex].Get();

	UINT srvIndex;
	UINT uavIndex;
	ID3D12Resource *pUavResource;
	if (m_srvIndex == 0)
	{
		srvIndex = SrvParticlePosVelo0;
		uavIndex = UavParticlePosVelo1;
		pUavResource = m_particleBuffer1.Get();
	}
	else
	{
		srvIndex = SrvParticlePosVelo1;
		uavIndex = UavParticlePosVelo0;
		pUavResource = m_particleBuffer0.Get();
	}

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pUavResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	pCommandList->SetPipelineState(m_computeState.Get());
	pCommandList->SetComputeRootSignature(m_computeRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvUavHeap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(), srvIndex, m_srvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(), uavIndex, m_srvUavDescriptorSize);

	pCommandList->SetComputeRootConstantBufferView(RootParameterCB, m_constantBufferCS->GetGPUVirtualAddress());
	pCommandList->SetComputeRootDescriptorTable(RootParameterSRV, srvHandle);
	pCommandList->SetComputeRootDescriptorTable(RootParameterUAV, uavHandle);

	pCommandList->Dispatch(static_cast<int>(ceil(ParticleCount / 256.0f)), 1, 1);

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pUavResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void D3D12nBodyGravity::OnDestroy()
{
	m_device->SetStablePowerState (false);

	// Ensure the GPU is done rendering.
	// Wait for all fences
	for (int i = 0; i < FrameCount; ++i) {
		if (AsynchronousComputeEnabled) {
			WaitForFence (m_computeFences[i].Get (),
				m_computeFenceValues[i], m_computeFenceEvents[i]);
			WaitForFence (m_graphicsFences[i].Get (),
				m_graphicsFenceValues[i], m_graphicsFenceEvents[i]);
			WaitForFence (m_graphicsCopyFences[i].Get (),
				m_graphicsCopyFenceValues[i], m_graphicsCopyFenceEvents[i]);
		}

		WaitForFence (m_frameFences[i].Get (),
			m_frameFenceValues[i], m_frameFenceEvents[i]);
	}

	// Close handles to fence events.
	for (int i = 0; i < FrameCount; ++i) {
		CloseHandle (m_computeFenceEvents[i]);
		CloseHandle (m_graphicsFenceEvents[i]);
		CloseHandle (m_graphicsCopyFenceEvents[i]);
		CloseHandle (m_frameFenceEvents[i]);
	}
}

void D3D12nBodyGravity::OnKeyDown(UINT8 key)
{
	m_camera.OnKeyDown(key);
}

void D3D12nBodyGravity::OnKeyUp(UINT8 key)
{
	m_camera.OnKeyUp(key);
}

// Cycle through the frame resources. This method blocks execution if the 
// next frame resource in the queue has not yet had its previous contents 
// processed by the GPU.
void D3D12nBodyGravity::MoveToNextFrame ()
{
	// Assign the current fence value to the current frame.
	m_frameFenceValues[m_frameIndex] = m_frameFenceValue;

	// Signal and increment the fence value.
	ThrowIfFailed (m_graphicsCommandQueue->Signal (m_frameFences[m_frameIndex].Get (), m_frameFenceValue));
	++m_frameFenceValue;

	// Update the frame index.
	m_lastFrameIndex = m_frameIndex;
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex ();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	WaitForFence (m_frameFences[m_frameIndex].Get (),
		m_frameFenceValues[m_frameIndex], m_frameFenceEvents[m_frameIndex]);

	// Update query
	++m_queryReadbackIndex;

	// This is trailing behind, assuming frame count = 4
	// frameIndex:     0  1  2  3
	// readbackIndex: -3 -2 -1  0
	//                          \- Trigger readback of frame 0 now
	//							We just waited for it to finish, so this is
	//							safe, and we have not issued the query for
	//							the next frame yet which will overwrite slot 0

	if (m_queryReadbackIndex >= FrameCount) {
		m_queryReadbackIndex = 0;
	}

	if (m_queryReadbackIndex >= 0) {
		void* mapping = nullptr;
		m_timeQueryReadbackBuffer[m_queryReadbackIndex]->Map (0, &CD3DX12_RANGE (0, sizeof (UINT64)), &mapping);
		::memcpy (m_queryResults + m_queryReadbackIndex, mapping, sizeof (UINT64));
		m_timeQueryReadbackBuffer[m_queryReadbackIndex]->Unmap (0, nullptr);

		// Time is now previous to current
		int previousQueryIndex = m_queryReadbackIndex - 1;
		if (previousQueryIndex < 0) {
			previousQueryIndex += FrameCount;
		}

		const double diffMs = static_cast<double>(m_queryResults[m_queryReadbackIndex] - m_queryResults[previousQueryIndex])
			/ static_cast<double> (m_frequency) * 1000;

		m_frameTimes[m_frameTimeNextEntry] = diffMs;
		++m_frameTimeNextEntry;
		if (m_frameTimeNextEntry >= m_frameTimes.size ()) {
			m_frameTimeNextEntry = 0;
		}
		++m_frameTimeEntryCount;

		const auto validEntryCount = std::min (static_cast<size_t> (m_frameTimeEntryCount), m_frameTimes.size ());
		const auto sum = std::accumulate (m_frameTimes.begin (),
			m_frameTimes.begin () + validEntryCount, 0.0);

		const auto averageDiffMs = sum / validEntryCount;

		auto titleString = std::to_string (averageDiffMs) + " ms (" + std::to_string (1000.0 / averageDiffMs) + " fps)";
		if (AsynchronousComputeEnabled) {
			titleString += " - Asynchronous Compute ENABLED";
		}
		else {
			titleString += " - Asynchronous Compute DISABLED";
		}

		SetWindowTextA (Win32Application::GetHwnd (), titleString.c_str ());
	}
}
