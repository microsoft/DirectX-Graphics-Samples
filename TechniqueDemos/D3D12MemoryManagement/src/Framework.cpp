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

#define DEFAULT_CLIENT_WIDTH 1280
#define DEFAULT_CLIENT_HEIGHT 720

#define WINDOW_CLASS L"MemorySampleApp"

#define SWAPCHAIN_BACK_BUFFER_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM

//
// Maximum size of a single paging operation. Mipmaps greater than 32MB in size will be
// broken up into multiple transfers.
//
#define MAX_TRANSFER_SIZE _32MB

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DX12Framework* pApplication = (DX12Framework*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pApplication->HandleMessage(hwnd, Message, wParam, lParam))
	{
		return 0;
	}

	return DefWindowProc(hwnd, Message, wParam, lParam);
}

LRESULT CALLBACK WndProcPreCreate(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (Message == WM_CREATE)
	{
		//
		// On creation, assign our user data (DX12Framework*) to the hwnd, and point
		// to the new WndProc used to handle those messages.
		//
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		return 0;
	}

	return DefWindowProc(hwnd, Message, wParam, lParam);
}

DX12Framework::DX12Framework() :
	m_RenderContext(this),
	m_PagingContext(this)
{
	InitializeListHead(&m_ResourceListHead);
	InitializeListHead(&m_DynamicBufferListHead);
	InitializeListHead(&m_DynamicDescriptorHeapListHead);
	InitializeListHead(&m_UnreferencedResourceListHead);
	InitializeListHead(&m_UncommittedListHead);
	for (int i = 0; i < MAX_MIP_COUNT; ++i)
	{
		InitializeListHead(&m_CommitmentListHeads[i]);
	}

	ZeroMemory(m_StatTimeBetweenFrames, sizeof(m_StatTimeBetweenFrames));
	ZeroMemory(m_StatRenderScene, sizeof(m_StatRenderScene));
	ZeroMemory(m_StatRenderUI, sizeof(m_StatRenderUI));
	ZeroMemory(m_pRenderTargets, sizeof(m_pRenderTargets));
	ZeroMemory(m_pWrappedBackBuffers, sizeof(m_pRenderTargets));
	ZeroMemory(m_pD2DRenderTargets, sizeof(m_pRenderTargets));
}

DX12Framework::~DX12Framework()
{
}

HRESULT DX12Framework::Init()
{
	HRESULT hr;

	hr = CreateDeviceIndependentStateInternal();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create device independent state");
		return hr;
	}

	hr = CreateDeviceDependentStateInternal();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create device dependent state");
		return hr;
	}

	hr = LoadAssets();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to load sample app assets");
		return hr;
	}

	return S_OK;
}

//
// Creates all device independent state. This includes any resource or object which
// can persist across a D3D12 device removal.
//
HRESULT DX12Framework::CreateDeviceIndependentStateInternal()
{
	HRESULT hr;
	UINT dxgiFactoryFlags = 0;

#if(_DEBUG)
	{
		//
		// Try to enable the debug layer if it's present on the system. It should
		// be enabled before creating the device.
		// Install the 'Graphics Tools' optional feature in Settings to enable
		// the SDK layers.
		//
		bool bDebugLayerEnabled = false;

		ID3D12Debug* pDebugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
		{
			pDebugController->EnableDebugLayer();
			pDebugController->Release();

			bDebugLayerEnabled = true;
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}

		wprintf(L"\nDebug layer is %senabled\n\n", bDebugLayerEnabled ? L"" : L"NOT ");
	}
#endif

	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_pDXGIFactory));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create DXGI Factory, hr=0x%.8x", hr);
		return hr;
	}

	//
	// QPC information for framerate tracking.
	//
	QueryPerformanceFrequency(&m_PerformanceFrequency);
	QueryPerformanceCounter(&m_LastFrameCounter);

	//
	// Initialize WIC.
	//
	hr = CoInitialize(nullptr);
	if (FAILED(hr))
	{
		LOG_ERROR("CoInitialize failed, hr=0x%.8x", hr);
		return hr;
	}

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create IWICFactory instance, hr=0x%.8x", hr);
		return hr;
	}

	//
	// Use TLS to create a waitable event object which can be used to wait on any context.
	//
	m_ThreadContextWaitHandleIndex = TlsAlloc();
	if (m_ThreadContextWaitHandleIndex == TLS_OUT_OF_INDEXES)
	{
		LOG_ERROR("Error allocating TLS index for wait handle, Error=0x%.8x", GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HANDLE hFlushEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!hFlushEvent)
	{
		LOG_ERROR("Error creating thread flush event, Error=0x%.8x", GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	TlsSetValue(m_ThreadContextWaitHandleIndex, (void*)hFlushEvent);

	//
	// Create window class and the window itself.
	//
	static const DWORD WindowStyle = WS_OVERLAPPEDWINDOW;

	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProcPreCreate;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = WINDOW_CLASS;
	if (RegisterClassEx(&windowClass) == FALSE)
	{
		LOG_ERROR("Failed to register window class, Error=0x%.8x", GetLastError());
		HRESULT_FROM_WIN32(GetLastError());
	}

	//
	// Adjust window rectangle to ensure we create a window whose client area is the requested size.
	//
	RECT WindowRect = { 0, 0, DEFAULT_CLIENT_WIDTH, DEFAULT_CLIENT_HEIGHT };
	AdjustWindowRect(&WindowRect, WindowStyle, FALSE);

	m_Hwnd = CreateWindow(
		WINDOW_CLASS,
		L"D3D12 Memory Sample",
		WindowStyle,
		300,
		300,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top,
		nullptr,
		nullptr,
		hInstance,
		this);
	if (!m_Hwnd)
	{
		LOG_ERROR("Failed to create window, Error=0x%.8x", GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	m_WindowWidth = DEFAULT_CLIENT_WIDTH;
	m_WindowHeight = DEFAULT_CLIENT_HEIGHT;

	//
	// Initialize D2D and DWrite device independent interfaces.
	//
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), (IUnknown**)&m_pDWriteFactory);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create DWrite factory, hr=0x%.8x", hr);
		return hr;
	}

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_pD2DFactory));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create D2D factory, hr=0x%.8x", hr);
		return hr;
	}

	hr = CreateDeviceIndependentState();
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

void DX12Framework::DestroyResource(Resource* pResource)
{
	RemoveEntryList(&pResource->ListEntry);

	SafeRelease(pResource->pDecoder);
#if(_DEBUG)
	free((void*)pResource->pFileName);
#endif
	assert(pResource->pDeviceState == nullptr);

	DestroyResourceDeviceState(pResource);
	RemoveEntryList(&pResource->CommittedListEntry);
	delete pResource;
}

void DX12Framework::DestroyResourceDeviceState(Resource* pResource)
{
	ResourceDeviceState* pDeviceState = pResource->pDeviceState;

	if (pDeviceState)
	{
		SafeRelease(pDeviceState->pD3DResource);

		for (UINT i = 0; i < pDeviceState->NumHeaps; ++i)
		{
			SafeRelease(pDeviceState->Mips[0].ppHeaps[i]);
		}
		delete[] pDeviceState->Mips[0].ppHeaps;
		free(pDeviceState);
		pResource->pDeviceState = nullptr;
	}

	RemoveResourceCommitment(pResource);
	if (pResource->PrioritizationEntry.Flink != nullptr)
	{
		RemoveEntryList(&pResource->PrioritizationEntry);
		pResource->PrioritizationEntry.Flink = nullptr;
	}
}

void DX12Framework::DestroyDeviceIndependentStateInternal()
{
	DestroyDeviceIndependentState();

	LIST_ENTRY* pResourceEntry = m_ResourceListHead.Flink;
	while (pResourceEntry != &m_ResourceListHead)
	{
		Resource* pResource = CONTAINING_RECORD(pResourceEntry, Resource, ListEntry);
		pResourceEntry = pResourceEntry->Flink;

		DestroyResource(pResource);
	}

	HANDLE hFlushEvent = TlsGetValue(m_ThreadContextWaitHandleIndex);
	if (hFlushEvent)
	{
		CloseHandle(hFlushEvent);
		hFlushEvent = nullptr;
	}

	SafeRelease(m_pDXGIFactory);
	SafeRelease(m_pWICFactory);
	SafeRelease(m_pD2DFactory);
	SafeRelease(m_pDWriteFactory);

	CoUninitialize();
}

HRESULT DX12Framework::CreateDeviceDependentStateInternal()
{
	HRESULT hr;

	{
		//
		// Have the user select the desired graphics adapter.
		//
		IDXGIAdapter* pTempAdapter;
		UINT AdapterOrdinal = 0;

		printf("Available Adapters:\n");
		while (m_pDXGIFactory->EnumAdapters(AdapterOrdinal, &pTempAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC Desc;
			pTempAdapter->GetDesc(&Desc);

			printf("  [%d] %ls\n", AdapterOrdinal, Desc.Description);

			pTempAdapter->Release();
			++AdapterOrdinal;
		}

		for (;;)
		{
			printf("\nSelect Adapter: ");

			AdapterOrdinal = m_NewAdapterIndex;
			if (AdapterOrdinal == 0xFFFFFFFF)
			{
				if (scanf_s("%d", &AdapterOrdinal) == 0)
				{
					//
					// If the user did not specify valid input, just use adapter 0.
					//
					AdapterOrdinal = 0;
				}
			}

			hr = m_pDXGIFactory->EnumAdapters(AdapterOrdinal, &pTempAdapter);
			if (hr == DXGI_ERROR_NOT_FOUND)
			{
				LOG_WARNING("Invalid adapter ordinal");
				continue;
			}

			hr = pTempAdapter->QueryInterface(&m_pDXGIAdapter);
			pTempAdapter->Release();

			if (FAILED(hr))
			{
				LOG_ERROR("Failed to query IDXGIAdapter3 interface from selected adapter.");
				return hr;
			}

			break;
		}
	}

	//
	// Obtain the default video memory information for the local and non-local segment groups.
	//
	hr = m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &m_LocalVideoMemoryInfo);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to query initial video memory info for local segment group");
		return hr;
	}

	hr = m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &m_NonLocalVideoMemoryInfo);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to query initial video memory info for non-local segment group");
		return hr;
	}

	hr = D3D12CreateDevice(m_pDXGIAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create D3D12 device, hr=0x%.8x", hr);
		return hr;
	}

#if _DEBUG
	ID3D12InfoQueue* pInfoQueue = nullptr;
	if (SUCCEEDED(m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
	{
		// Suppress whole categories of messages.
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level.
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID.
		D3D12_MESSAGE_ID DenyIds[] =
		{
			// The 11On12 implementation does not use optimized clearing yet.
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		pInfoQueue->PushStorageFilter(&NewFilter);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		pInfoQueue->Release();
	}
#endif

	//
	// Cache the descriptor sizes.
	//
	m_DescriptorInfo.RtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DescriptorInfo.SamplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_DescriptorInfo.SrvUavCbvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	hr = QueryCaps();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to query device caps");
		return hr;
	}

	hr = m_RenderContext.CreateDeviceDependentState(SWAPCHAIN_BUFFER_COUNT);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize render context");
		return hr;
	}

	hr = m_PagingContext.CreateDeviceDependentState();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize paging context");
		return hr;
	}

	hr = m_TextureShader.CreateDeviceDependentState(m_pDevice, L"Assets\\Shaders\\Texture.hlsl");
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize texture shader");
		return hr;
	}

	hr = m_ColorShader.CreateDeviceDependentState(m_pDevice, L"Assets\\Shaders\\Color.hlsl");
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize color shader");
		return hr;
	}

	//
	// Create a basic Flip-Discard swapchain.
	//
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
	SwapChainDesc.BufferCount = SWAPCHAIN_BUFFER_COUNT;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.Format = SWAPCHAIN_BACK_BUFFER_FORMAT;
	SwapChainDesc.Height = m_WindowHeight;
	SwapChainDesc.Scaling = DXGI_SCALING_NONE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Width = m_WindowWidth;
	SwapChainDesc.Flags = 0;

	//
	// The swap chain is created as an IDXGISwapChain1 interface, but we then query
	// the IDXGISwapchain3 interface from it.
	//
	IDXGISwapChain1* pSwapChain;
	hr = m_pDXGIFactory->CreateSwapChainForHwnd(m_RenderContext.GetCommandQueue(), m_Hwnd, &SwapChainDesc, nullptr, nullptr, &pSwapChain);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create swap chain for hwnd, hr=0x%.8x", hr);
		return hr;
	}

	hr = pSwapChain->QueryInterface(&m_pDXGISwapChain);
	pSwapChain->Release();
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to query IDXGISwapChain3 interface, hr=0x%.8x", hr);
		return hr;
	}

	//
	// Create RTV heap for render target.
	//
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};

		HeapDesc.NumDescriptors = SWAPCHAIN_BUFFER_COUNT;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_pDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_pRtvHeap));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create RTV descriptor heap, hr=0x%.8x", hr);
			return hr;
		}
	}

	//
	// If enabled, create the shared staging resource used for transfers.
	//
	if (m_bUseSharedStagingSurface)
	{
		hr = m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(MAX_TRANSFER_SIZE),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pStagingSurface));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create staging upload buffer, hr=0x%.8x", hr);
			return hr;
		}

		CD3DX12_RANGE readRange(0, 0);
		hr = m_pStagingSurface->Map(0, &readRange, &m_pStagingSurfaceData);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to map upload buffer data, hr=0x%.8x", hr);
			return hr;
		}
	}

	//
	// Create 11On12 state to enable D2D rendering on D3D12.
	//
	IUnknown* pRenderCommandQueue = m_RenderContext.GetCommandQueue();
	hr = D3D11On12CreateDevice(m_pDevice, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, &pRenderCommandQueue, 1, 0, &m_p11Device, &m_p11Context, nullptr);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create D3D11On12 device, hr=0x%.8x", hr);
		return hr;
	}

	hr = m_p11Device->QueryInterface(&m_p11On12Device);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to query 11On12 device interface, hr=0x%.8x", hr);
		return hr;
	}

	//
	// Create D2D/DWrite components.
	//
	{
		D2D1_DEVICE_CONTEXT_OPTIONS DeviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		IDXGIDevice* pDxgiDevice;
		hr = m_p11On12Device->QueryInterface(&pDxgiDevice);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to query IDXGIDevice interface, hr=0x%.8x", hr);
			return hr;
		}

		hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pD2DDevice);
		pDxgiDevice->Release();
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create D2D device, hr=0x%.8x", hr);
			return hr;
		}

		hr = m_pD2DDevice->CreateDeviceContext(DeviceOptions, &m_pD2DContext);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create D2D context, hr=0x%.8x", hr);
			return hr;
		}
	}

	//
	// Create the resources to enable D2D to access the swap chain.
	//
	hr = CreateSwapChainResources();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create swap chain resources");
		return hr;
	}

	//
	// Initialize our paging worker thread. The worker thread will be used for streaming
	// resource mipmaps asynchronously from the rendering thread.
	//
	try
	{
		m_pWorkerThread = new PagingWorkerThread(this);
	}
	catch (std::bad_alloc&)
	{
		LOG_ERROR("Failed to allocate paging worker thread instance");
		return E_OUTOFMEMORY;
	}

	hr = m_pWorkerThread->Init();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize paging worker thread");
		return hr;
	}

	//
	// If we are reinitializing from a device removed state, the application may have
	// resources already which need to be reinitialized. Do this now.
	//
	{
		LIST_ENTRY* pResourceEntry = m_ResourceListHead.Flink;
		while (pResourceEntry != &m_ResourceListHead)
		{
			Resource* pResource = CONTAINING_RECORD(pResourceEntry, Resource, ListEntry);
			pResourceEntry = pResourceEntry->Flink;

			UINT NumMips;
			UINT Width;
			UINT Height;
			DXGI_FORMAT Format;

			hr = GetResourceInformation(pResource->pDecoder, NumMips, Format, Width, Height);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to get resource information, hr=0x%.8x", hr);
				return hr;
			}

			hr = CreateResourceDeviceState(pResource, NumMips, Format, Width, Height);
			if (FAILED(hr))
			{
				return hr;
			}
		}
	}

	hr = CreateDeviceDependentState();
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

void DX12Framework::DestroyDeviceDependentStateInternal()
{
	DestroyDeviceDependentState();

	if (m_pWorkerThread)
	{
		delete m_pWorkerThread;
		m_pWorkerThread = nullptr;
	}

	m_RenderContext.DestroyDeviceDependentState();
	m_PagingContext.DestroyDeviceDependentState();

	m_TextureShader.DestroyDeviceDependentState();
	m_ColorShader.DestroyDeviceDependentState();

	{
		LIST_ENTRY* pResourceEntry = m_ResourceListHead.Flink;
		while (pResourceEntry != &m_ResourceListHead)
		{
			Resource* pResource = CONTAINING_RECORD(pResourceEntry, Resource, ListEntry);
			pResourceEntry = pResourceEntry->Flink;

			DestroyResourceDeviceState(pResource);
		}
	}

	{
		LIST_ENTRY* pVersionedBufferEntry = m_DynamicBufferListHead.Flink;
		while (pVersionedBufferEntry != &m_DynamicBufferListHead)
		{
			Buffer* pVersionedBuffer = static_cast<Buffer*>(pVersionedBufferEntry);
			pVersionedBufferEntry = pVersionedBufferEntry->Flink;
			FreeVersionedBuffer(pVersionedBuffer);
		}
	}

	{
		LIST_ENTRY* pVersionedDescriptorHeapEntry = m_DynamicDescriptorHeapListHead.Flink;
		while (pVersionedDescriptorHeapEntry != &m_DynamicDescriptorHeapListHead)
		{
			DescriptorHeap* pVersionedDescriptorHeap = static_cast<DescriptorHeap*>(pVersionedDescriptorHeapEntry);
			pVersionedDescriptorHeapEntry = pVersionedDescriptorHeapEntry->Flink;
			FreeVersionedDescriptorHeap(pVersionedDescriptorHeap);
		}
	}

	SafeRelease(m_pDXGIAdapter);
	SafeRelease(m_pDevice);
	SafeRelease(m_pDXGISwapChain);
	SafeRelease(m_pRtvHeap);
	SafeRelease(m_pStagingSurface);

	SafeRelease(m_p11Device);
	SafeRelease(m_p11On12Device);
	SafeRelease(m_pD2DDevice);
	SafeRelease(m_pD2DContext);

	for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i)
	{
		SafeRelease(m_pRenderTargets[i]);
		SafeRelease(m_pWrappedBackBuffers[i]);
		SafeRelease(m_pD2DRenderTargets[i]);
	}

	if (m_p11Context)
	{
		m_p11Context->ClearState();
		m_p11Context->Flush();
		SafeRelease(m_p11Context);
	}
}

HRESULT DX12Framework::CreateSwapChainResources()
{
	HRESULT hr;

	float dpiX;
	float dpiY;
	m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY);

	//
	// For each buffer in the swapchain, we need to create a wrapped resource, and create
	// a D2D render target object to enable D2D rendering for the UI.
	//
	ULONG Ref;
	for (UINT n = 0; n < SWAPCHAIN_BUFFER_COUNT; n++)
	{
		hr = m_pDXGISwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n]));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to get swap chain back buffer for frame index %d, hr=0x%.8x", n, hr);
			return hr;
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), n, m_DescriptorInfo.RtvDescriptorSize);
		m_pDevice->CreateRenderTargetView(m_pRenderTargets[n], nullptr, handle);

		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		hr = m_p11On12Device->CreateWrappedResource(
			m_pRenderTargets[n],
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_pWrappedBackBuffers[n]));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed create 11On12 wrapped resource for render target %d, hr=0x%.8x", n, hr);
			return hr;
		}

		IDXGISurface* pSurface;
		hr = m_pWrappedBackBuffers[n]->QueryInterface(&pSurface);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to query IDXGISurface from wrapped back buffer, hr=0x%.8x", hr);
			return hr;
		}

		hr = m_pD2DContext->CreateBitmapFromDxgiSurface(pSurface, &bitmapProperties, &m_pD2DRenderTargets[n]);
		Ref = pSurface->Release();
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create D2D bitmap from back buffer surface, hr=0x%.8x", hr);
			return hr;
		}
	}

	return S_OK;
}

void DX12Framework::DestroySwapChainResources()
{
	for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i)
	{
		SafeRelease(m_pRenderTargets[i]);
		SafeRelease(m_pD2DRenderTargets[i]);
		SafeRelease(m_pWrappedBackBuffers[i]);
	}

	//
	// Must flush the 11 queue to ensure that the wrapped resources are destroyed prior to calling
	// ResizeBuffers, or it will fail because the objects are still live.
	//
	m_p11Context->Flush();
	m_RenderContext.Flush();
}

HRESULT DX12Framework::InitDynamicBuffer(DynamicBuffer* pBuffer)
{
	HRESULT hr;
	Buffer* pNewBuffer;

	hr = AllocateVersionedBuffer(&pNewBuffer);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to allocate buffer for dynamic buffer 0x%p, hr=0x%.8x", pBuffer, hr);
		return hr;
	}

	pBuffer->m_pBuffer = pNewBuffer;
	pBuffer->m_pCurrentAddress = pNewBuffer->pBaseAddress;

	return S_OK;
}

void DX12Framework::DestroyDynamicBuffer(DynamicBuffer* pBuffer)
{
	FreeVersionedBuffer(pBuffer->m_pBuffer);
}

void DX12Framework::FreeVersionedBuffer(Buffer* pBuffer)
{
	SafeRelease(pBuffer->pBuffer);
	SafeDelete(pBuffer);
}

HRESULT DX12Framework::InitDynamicDescriptorHeap(DynamicDescriptorHeap* pHeap)
{
	HRESULT hr;
	DescriptorHeap* pNewHeap;

	hr = AllocateVersionedDescriptorHeap(&pNewHeap);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to allocate heap for dynamic heap 0x%p, hr=0x%.8x", pHeap, hr);
		return hr;
	}

	pHeap->m_pHeap = pNewHeap;
	pHeap->m_CurrentHandleIndex = 0;
	pHeap->m_DescriptorSize = m_DescriptorInfo.SrvUavCbvDescriptorSize;

	return S_OK;
}

void DX12Framework::DestroyDynamicDescriptorHeap(DynamicDescriptorHeap* pHeap)
{
	FreeVersionedDescriptorHeap(pHeap->m_pHeap);
}

void DX12Framework::FreeVersionedDescriptorHeap(DescriptorHeap* pHeap)
{
	SafeRelease(pHeap->pHeap);
	SafeDelete(pHeap);
}

HRESULT DX12Framework::Run()
{
	HRESULT hr;

	hr = Init();
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to initialize framework, hr=0x%.8x", hr);
		return hr;
	}

	ShowWindow(m_Hwnd, SW_SHOWNORMAL);

	MSG Msg;
	for (;;)
	{
		//
		// Process any pending window messages.
		//
		while (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);

			if (Msg.message == WM_QUIT)
			{
				goto exit;
			}
		}

		//
		// Render a frame.
		//
		hr = RenderInternal();
		if (FAILED(hr))
		{
			LOG_WARNING("Failure returned from render (likely device removed), hr=0x%.8x", hr);
			m_RenderContext.Flush();
			hr = RecreateDeviceDependentState();
			if (FAILED(hr))
			{
				LOG_ERROR("Unable to recover from failure, hr=0x%.8x", hr);
				abort();
			}
		}
	}

exit:
	DestroyDeviceDependentStateInternal();
	DestroyDeviceIndependentStateInternal();

#ifdef _DEBUG
	IDXGIDebug1* pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
		pDebug->Release();
	}
#endif

	return S_OK;
}

HRESULT DX12Framework::QueryCaps()
{
	HRESULT hr;

	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_Options, sizeof(m_Options));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to acquire D3D12 options for ID3D12Device 0x%p, hr=0x%.8x", m_pDevice, hr);
		return hr;
	}

	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &m_GpuVaSupport, sizeof(m_GpuVaSupport));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to acquire GPU virtual address support for ID3D12Device 0x%p, hr=0x%.8x", m_pDevice, hr);
		return hr;
	}

	D3D12_FEATURE_DATA_FEATURE_LEVELS FeatureLevels = {};
	D3D_FEATURE_LEVEL FeatureLevelsList[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
	};
	FeatureLevels.NumFeatureLevels = ARRAYSIZE(FeatureLevelsList);
	FeatureLevels.pFeatureLevelsRequested = FeatureLevelsList;

	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &FeatureLevels, sizeof(FeatureLevels));
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to acquire feature level support for ID3D12Device 0x%p, hr=0x%.8x", m_pDevice, hr);
		return hr;
	}

	printf("\n");
	printf("--- Device details --------------------\n");
	printf("  GPU Virtual Address Info:\n");
	printf("    Max Process Size:      %I64d GB\n", (1ULL << m_GpuVaSupport.MaxGPUVirtualAddressBitsPerProcess) / 1024 / 1024 / 1024);
	printf("    Max Resource Size:     %I64d GB\n", (1ULL << m_GpuVaSupport.MaxGPUVirtualAddressBitsPerResource) / 1024 / 1024 / 1024);
	printf("  Feature Level Info:\n");
	printf("    Max Feature Level:     %s\n", GetFeatureLevelName(FeatureLevels.MaxSupportedFeatureLevel));
	printf("  Additional Info:\n");
	printf("    Resource Binding Tier: Tier %d\n", m_Options.ResourceBindingTier);
	printf("    Resource Heap Tier:    Tier %d\n", m_Options.ResourceHeapTier);
	printf("    Tiled Resource Tier:   Tier %d\n", m_Options.TiledResourcesTier);
	printf("\n");

	return S_OK;
}

HRESULT DX12Framework::UpdateVideoMemoryInfo()
{
	HRESULT hr;

	DXGI_QUERY_VIDEO_MEMORY_INFO NewLocal;
	DXGI_QUERY_VIDEO_MEMORY_INFO NewNonLocal;

	//
	// QueryVideoMemoryInfo obtains up to date budgeting information from the graphics
	// kernel about video memory usage, budget, and reservation. This sample does not
	// use the reservation mechanics, but instead focuses on the proper techniques used
	// to manage the application's video memory usage and stay under budget through
	// trimming behavior.
	//

	//
	// Each call to QueryVideoMemoryInfo takes a memory segment group. Applications
	// should design their video memory usage around the local memory group budget,
	// and the non-local budget can usually be safely ignored. Non-local memory refers
	// to memory that is not located near the GPU, and as a result is typically much
	// slower than the local mmeory. On a discrete video card, the local and non-local
	// memory groups would represent VRAM and system memory, respectively. Accessing
	// system memory via PCI-e is far slower than accessing VRAM directly on-board,
	// and should typically be avoided in high performance graphics. Although some
	// small or low bandwidth allocations can effectively be placed here, it is not
	// recommended for normal texture memory. The local memory group always refers to
	// the fastest and most ideal placement for texture data.
	//
	hr = m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &NewLocal);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to update non-local video memory info. If the process budget has been lowered, trimming will not be done, hr=0x%.8x", hr);
		return hr;
	}

	//
	// Per the above comment, obtaining the non-local memory info is not strictly
	// required, and can typically be ignored completely. In some cases, such as on
	// integrated GPUs, non-local memory does not even exist, and QueryVideoMemoryInfo
	// will return all zero data. Non-local memory's best usage is often for a fallback
	// under high memory pressure in order to make forward progress on the system.
	//
	hr = m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &NewNonLocal);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to update local video memory info. If the process budget has been lowered, trimming will not be done, hr=0x%.8x", hr);
		return hr;
	}

	if (m_LocalBudgetOverride != 0)
	{
		NewLocal.Budget = m_LocalBudgetOverride;
	}

	m_LocalVideoMemoryInfo = NewLocal;
	m_NonLocalVideoMemoryInfo = NewNonLocal;

	return S_OK;
}

HRESULT DX12Framework::GetResourceInformation(IWICBitmapDecoder* pDecoder, UINT& NumMips, DXGI_FORMAT& Format, UINT& Width, UINT& Height)
{
	HRESULT hr;
	IWICDdsDecoder* pDdsDecoder = nullptr;
	WICDdsParameters DdsParameters;

	if (pDecoder)
	{
		hr = pDecoder->QueryInterface(IID_PPV_ARGS(&pDdsDecoder));

		if (pDdsDecoder)
		{
			//
			// This is a DDS file. WIC can provide us direct access to the file information
			// which it already has stored internally.
			//
			hr = pDdsDecoder->GetParameters(&DdsParameters);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to get DDS parameters from decoder, hr=0x%.8x", hr);
				goto cleanup;
			}
		}
		else
		{
			//
			// This is a non-DDS file, which does not directly expose the same information
			// as the DDS decoder, but can be obtained from the first frame in the image.
			//
			ComPtr<IWICBitmapFrameDecode> pFrame;
			hr = pDecoder->GetFrame(0, &pFrame);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to get frame info from decoder, hr=0x%.8x", hr);
				goto cleanup;
			}

			BitmapFrameInfo BitmapFrameInfo;
			hr = GetBitmapFrameInfo(pFrame.Get(), &BitmapFrameInfo);
			if (FAILED(hr))
			{
				//
				// May have failed to convert the pixel format into a supported DXGI format.
				//
				LOG_ERROR("Failed to get bitmap frame information, hr=0x%.8x", hr);
				goto cleanup;
			}

			DdsParameters.DxgiFormat = BitmapFrameInfo.DxgiFormat;
			DdsParameters.MipLevels = 1;
			DdsParameters.Width = BitmapFrameInfo.WidthInBlocks;
			DdsParameters.Height = BitmapFrameInfo.HeightInBlocks;
		}
	}
	else
	{
		//
		// If no decoder is specified, we will generate an image for it.
		// The data will be loaded on demand as mips are fetched.
		//
		DdsParameters.MipLevels = 13;
		DdsParameters.DxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DdsParameters.Width = 1 << (DdsParameters.MipLevels - 1);
		DdsParameters.Height = 1 << (DdsParameters.MipLevels - 1);
	}

	//
	// Check format support for tiled resources.
	//
	D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport;
	FormatSupport.Format = DdsParameters.DxgiFormat;
	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
	if (FAILED(hr) || (FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_TILED) == 0)
	{
		LOG_ERROR("CheckFeatureSupport failed, or unsupported DXGI format (%d)", DdsParameters.DxgiFormat);
		hr = E_FAIL;
		goto cleanup;
	}

	NumMips = DdsParameters.MipLevels;
	Format = DdsParameters.DxgiFormat;
	Width = DdsParameters.Width;
	Height = DdsParameters.Height;

	return S_OK;

cleanup:
	SafeRelease(pDdsDecoder);

	return hr;
}

HRESULT DX12Framework::CreateResource(LPCWSTR pFileName, Resource** ppResource)
{
	HRESULT hr;

	//
	// Create a new resource object to track the state.
	//
	Resource* pResource = nullptr;
	try
	{
		pResource = new Resource();
	}
	catch (std::bad_alloc&)
	{
		LOG_ERROR("Out of memory allocating device independent resource data");
		return E_OUTOFMEMORY;
	}

	ZeroMemory(pResource, sizeof(Resource));

	IWICBitmapDecoder* pDecoder = nullptr;
	static UINT GeneratedImageIndex = 0;

	if (pFileName && *pFileName)
	{
		//
		// Open a WIC decoder to the specified file. We will use the WIC decoder
		// to open access to various mipmaps and copy the pixel data on demand.
		//
		hr = m_pWICFactory->CreateDecoderFromFilename(
			pFileName,
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&pDecoder);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to open frame decoder from file (Not image file?), hr=0x%.8x", hr);
			goto cleanup;
		}
	}
	else
	{
		//
		// This resource will be generated and does not need a decoder.
		//
		pResource->GeneratedImageIndex = GeneratedImageIndex++;
	}

	UINT NumMips;
	DXGI_FORMAT Format;
	UINT Width;
	UINT Height;

	hr = GetResourceInformation(pDecoder, NumMips, Format, Width, Height);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to get resource information, hr=0x%.8x", hr);
		goto cleanup;
	}

	//
	// Creates the device dependent resource data for the current D3D12 device.
	//
	hr = CreateResourceDeviceState(pResource, NumMips, Format, Width, Height);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create resource device state");
		goto cleanup;
	}

	//
	// Complete resource construction.
	//
#if(_DEBUG)
	pResource->pFileName = _wcsdup(pFileName);
#endif

	InitializeCriticalSection(&pResource->ReferenceLock);

	pResource->pDecoder = pDecoder;
	InsertTailList(&m_ResourceListHead, &pResource->ListEntry);
	InsertTailList(&m_UncommittedListHead, &pResource->CommittedListEntry);

	assert(pResource->pDeviceState != nullptr);

	*ppResource = pResource;

	return S_OK;

cleanup:
	SafeDelete(pResource);
	SafeRelease(pDecoder);

	return hr;
}

HRESULT DX12Framework::CreateResourceDeviceState(Resource* pResource, UINT NumMips, DXGI_FORMAT Format, UINT Width, UINT Height)
{
	HRESULT hr;

	ID3D12Resource* pTiledResource = nullptr;
	ID3D12Heap** ppHeaps = nullptr;
	ResourceDeviceState* pDeviceState = nullptr;

	//
	// Create a new reserved resource which we will map to heaps at a later time.
	// A reserved resource only consists of a virtual address, which is commonly
	// plentiful and cheap on most hardware. On some hardware, virtual address space
	// is limited, and the application may have to write additional code to recreate
	// resources with fewer mipmaps to reduce address space pressure. This sample
	// is written to assume that the application is being run on hardware with large
	// virtual address spaces.
	// By default, D3D12's TIER1 support for virtual addressing supports a 40-bit
	// virtual address space, which is equal to 1 terabyte.
	//
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		Format,
		Width,
		Height,
		1,
		(UINT16)NumMips,
		1,
		0,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);

	hr = m_pDevice->CreateReservedResource(&resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pTiledResource));
	if (FAILED(hr))
	{
		LOG_ERROR(
			"Failed to create new resource. DxgiFormat=%d, Width=%d, Height=%d, MipLevels=%d, hr=0x%.8x",
			Format,
			Width,
			Height,
			NumMips,
			hr);
		goto cleanup;
	}

	//
	// The application cannot determine tile information of a resource on its own. The
	// driver or device may have restrictions or limitations about how mipmaps are packed
	// into the tiles. Fortunately, D3D exposes a function which allows the driver to
	// inform us of the size information for each subresource (mipmap) of the resource.
	// This includes the dimensions (width, height, depth) of the subresource in tiles, as
	// well as the mip packing information, such as how many tiles are required to store
	// packed mipmaps (which is sometimes more than 1).
	//
	UINT NumTiles;
	D3D12_PACKED_MIP_INFO PackedMipInfo;
	D3D12_SUBRESOURCE_TILING SubresourceTiling[MAX_MIP_COUNT];
	UINT NumResourceTilings = MAX_MIP_COUNT;
	m_pDevice->GetResourceTiling(pTiledResource, &NumTiles, &PackedMipInfo, nullptr, &NumResourceTilings, 0, SubresourceTiling);

	//
	// Save off the number of packed and non-packed (standard) mipmaps.
	// Because packed mipmaps share the same tiles, and thus, the same physical heap memory,
	// we only need to create a single resource mip tracking structure for all packed mipmaps.
	//
	size_t NumberOfStandardMipIndices = PackedMipInfo.NumStandardMips;
	size_t NumberOfPackedMipHeapIndices = 0;
	if (PackedMipInfo.NumPackedMips > 0)
	{
		NumberOfPackedMipHeapIndices = 1;
	}

	//
	// Calculate the size of the resource's device state, and the mipmap data we'll need.
	//
	size_t SizeOfResource = sizeof(ResourceDeviceState) - sizeof(ResourceMip);
	size_t NumberOfMipIndices = NumberOfStandardMipIndices + NumberOfPackedMipHeapIndices;
	size_t SizeOfResourceMipIndices = NumberOfMipIndices * sizeof(ResourceMip);

	pDeviceState = (ResourceDeviceState*)malloc(SizeOfResource + SizeOfResourceMipIndices);
	if (pDeviceState == nullptr)
	{
		LOG_ERROR("Low memory: Could not allocate resource structure");
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}
	ZeroMemory(pDeviceState, SizeOfResource + SizeOfResourceMipIndices);

	//
	// For each mip heap, calculate the number of D3D12 heaps that will back the resource,
	// and allocate this data up front. Note that we are not creating the actual D3D12
	// heap here (this is done lazily when we load the image data). We will also
	// initialize mipmap specific tracking information.
	//
	UINT CurrentHeapCount = 0;
	for (size_t i = 0; i < NumberOfMipIndices; ++i)
	{
		MipDescription* pDesc = &pDeviceState->Mips[i].Desc;

		NumTiles = PackedMipInfo.NumTilesForPackedMips;
		if (SubresourceTiling[i].StartTileIndexInOverallResource == 0xFFFFFFFF)
		{
			pDesc->WidthInTiles = 0;
			pDesc->HeightInTiles = 0;
		}
		else
		{
			UINT WidthInTiles = SubresourceTiling[i].WidthInTiles;
			UINT HeightInTiles = SubresourceTiling[i].HeightInTiles;

			pDesc->WidthInTiles = WidthInTiles;
			pDesc->HeightInTiles = HeightInTiles;

			NumTiles = WidthInTiles * HeightInTiles;
		}
		pDesc->HeapStartIndex = CurrentHeapCount;

		//
		// Even if a resource does not fill in a full 16MB heap, it is OK, because
		// we are counting the number of 16MB or less heaps in the resource. Thus
		// even 64KB rounded up will be 1, which is the correct value.
		//
		UINT MipSize = NumTiles * TILE_SIZE;
		UINT NumHeapsInMip = (MipSize + MAX_HEAP_SIZE - 1) / MAX_HEAP_SIZE;

		CurrentHeapCount += NumHeapsInMip;
		pDeviceState->Mips[i].ReferenceFence = 0;
	}

	//
	// Allocate heap list for entire texture. Each mip is broken into chunks of
	// at most MAX_HEAP_SIZE (Currently 16MB)
	//
	try
	{
		ppHeaps = new ID3D12Heap*[CurrentHeapCount];
	}
	catch (std::bad_alloc&)
	{
		LOG_ERROR("Out of memory allocating D3D12 heap array");
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	//
	// Initialize the rest of the resource to various default values.
	//
	ZeroMemory(ppHeaps, sizeof(*ppHeaps) * CurrentHeapCount);
	for (size_t i = 0; i < NumberOfMipIndices; ++i)
	{
		pDeviceState->Mips[i].ppHeaps = ppHeaps + pDeviceState->Mips[i].Desc.HeapStartIndex;
	}

	pDeviceState->pD3DResource = pTiledResource;
	pDeviceState->NumHeaps = CurrentHeapCount;

	pResource->PackedMipTileCount = PackedMipInfo.NumTilesForPackedMips;
	pResource->NumStandardMips = PackedMipInfo.NumStandardMips;
	pResource->NumPackedMips = PackedMipInfo.NumPackedMips;
	pResource->MostDetailedMipResident = PackedMipInfo.NumStandardMips + PackedMipInfo.NumPackedMips;
	pResource->pDeviceState = pDeviceState;

	//
	// By default, the resource should not be visible or prefetched, so we will
	// initialize these values to UNDEFINED_MIPMAP_INDEX, which is not a valid
	// mipmap index.
	//
	pResource->VisibleMip = UNDEFINED_MIPMAP_INDEX;
	pResource->PrefetchMip = UNDEFINED_MIPMAP_INDEX;

	//
	// By default, there are no restrictions on which mipmaps may be used for rendering.
	//
	pResource->MipRestriction = 0;

	pResource->TrimLimit = ERTP_None;
	pResource->bIgnoreBudget = false;

	pResource->PagingEntry.Flink = nullptr;

	//
	// Notify the paging thread of this resource so it can be prioritized. Although
	// not visible, we will still attempt to page in the packed mipmaps.
	//
	NotifyPagingWork(pResource);

	return S_OK;

cleanup:
	if (ppHeaps)
	{
		delete[] ppHeaps;
	}
	SafeRelease(pTiledResource);

	if (pDeviceState)
	{
		free(pDeviceState);
	}

	return hr;
}

HRESULT DX12Framework::GetDdsFrameInfo(IWICDdsFrameDecode* pFrame, BitmapFrameInfo* pFormatInfo)
{
	HRESULT hr;

	WICDdsFormatInfo FormatInfo;
	hr = pFrame->GetFormatInfo(&FormatInfo);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to get format info from DDS frame, hr=0x%.8x", hr);
		return hr;
	}

	UINT WidthInBlocks;
	UINT HeightInBlocks;
	hr = pFrame->GetSizeInBlocks(&WidthInBlocks, &HeightInBlocks);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to get DDS frame size, hr=0x%.8x", hr);
		return hr;
	}

	pFormatInfo->BlockWidth = FormatInfo.BlockWidth;
	pFormatInfo->BlockHeight = FormatInfo.BlockHeight;
	pFormatInfo->DxgiFormat = FormatInfo.DxgiFormat;

	pFormatInfo->WidthInBlocks = WidthInBlocks;
	pFormatInfo->HeightInBlocks = HeightInBlocks;

	return S_OK;
}

HRESULT DX12Framework::GetBitmapFrameInfo(IWICBitmapFrameDecode* pFrame, BitmapFrameInfo* pFormatInfo)
{
	HRESULT hr;

	UINT WidthInBlocks;
	UINT HeightInBlocks;
	hr = pFrame->GetSize(&WidthInBlocks, &HeightInBlocks);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to get bitmap size, hr=0x%.8x", hr);
		return hr;
	}

	WICPixelFormatGUID SourcePixelFormat;
	hr = pFrame->GetPixelFormat(&SourcePixelFormat);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to get bitmap pixel format, hr=0x%.8x", hr);
		return hr;
	}

	DXGI_FORMAT DxgiFormat;

	//
	// Find the closest pixel format which we can convert the image source data into
	// for our resource.
	//
	GUID TargetPixelFormat;
	if (!GetTargetPixelFormat(&SourcePixelFormat, &TargetPixelFormat))
	{
		LOG_ERROR(
			"No target pixel format found for source format, GUID=0x%x, 0x%x, 0x%x, 0x%x",
			SourcePixelFormat.Data1,
			SourcePixelFormat.Data2,
			SourcePixelFormat.Data3,
			SourcePixelFormat.Data4);
	}

	//
	// Then find the closest matching DXGI format to that target pixel format.
	//
	DxgiFormat = GetDXGIFormatFromPixelFormat(&TargetPixelFormat);
	if (DxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		LOG_ERROR("No valid DXGI_FORMAT conversion for pixel format, hr=0x%.8x", hr);
		return hr;
	}

	pFormatInfo->BlockWidth = 1;
	pFormatInfo->BlockHeight = 1;
	pFormatInfo->DxgiFormat = DxgiFormat;
	pFormatInfo->SourcePixelFormat = SourcePixelFormat;
	pFormatInfo->TargetPixelFormat = TargetPixelFormat;

	pFormatInfo->WidthInBlocks = WidthInBlocks;
	pFormatInfo->HeightInBlocks = HeightInBlocks;

	return S_OK;
}

//
// LoadMip is in charge of creating resource data. If necessary, LoadMip will create the
// heaps (physical memory) for the mipmap, update the virtual address mappings, and copy
// the pixel data from the WIC image source.
//
HRESULT DX12Framework::LoadMip(Resource* pResource, UINT32 Mip)
{
	HRESULT hr;

	LOG_MESSAGE("Loading mip %d", Mip);

	UINT32 MipHeap = Mip;

	UINT NumMips = GetResourceMipCount(pResource);
	if (Mip >= NumMips)
	{
		LOG_WARNING("Trying to load mip %d for pResource 0x%p, but the image file only contains %d mip levels", Mip, pResource, NumMips);
		return S_FALSE;
	}

	BitmapFrameInfo MipFrameInfo = {};

	ComPtr<IWICDdsDecoder> pDdsDecoder;
	ComPtr<IWICBitmapFrameDecode> pBitmapFrame;
	ComPtr<IWICDdsFrameDecode> pDdsFrame;
	ComPtr<IWICBitmapSource> pSourceBitmap;

	//
	// Different behavior is performed depending on whether this is a generated image,
	// DDS file, or other image format, since it may contain block compressed pixel data.
	//
	if (pResource->pDecoder)
	{
		hr = pResource->pDecoder->QueryInterface(IID_PPV_ARGS(&pDdsDecoder));
		if (SUCCEEDED(hr))
		{
			hr = pDdsDecoder->GetFrame(0, Mip, 0, &pBitmapFrame);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to load decode frame for Resource 0x%p, mip %d, hr=0x%.8x", pResource, Mip, hr);
				return hr;
			}

			hr = pBitmapFrame.As(&pDdsFrame);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to query DDS frame for mip %d, hr=0x%.8x", Mip, hr);
				return hr;
			}

			hr = GetDdsFrameInfo(pDdsFrame.Get(), &MipFrameInfo);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to load frame information for mip %d, hr=0x%.8x", Mip, hr);
				return hr;
			}
		}
		else
		{
			hr = pResource->pDecoder->GetFrame(0, &pBitmapFrame);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to decode bitmap for Resource 0x%p, mip %d, hr=0x%.8x", pResource, Mip, hr);
				return hr;
			}

			hr = GetBitmapFrameInfo(pBitmapFrame.Get(), &MipFrameInfo);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to load bitmap information for mip %d, hr=0x%.8x", Mip, hr);
				return hr;
			}

			//
			// Non-DDS images may need a pixel converter to convert between the source and
			// target pixel formats.
			//
			ComPtr<IWICFormatConverter> pConverter;
			hr = m_pWICFactory->CreateFormatConverter(&pConverter);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to create pixel format converter, hr=0x%.8x", hr);
				return hr;
			}

			hr = pConverter->Initialize(
				pBitmapFrame.Get(),
				MipFrameInfo.TargetPixelFormat,
				WICBitmapDitherTypeNone,
				nullptr,
				0.0f,
				WICBitmapPaletteTypeCustom);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to initialize pixel format converter, hr=0x%.8x", hr);
				return hr;
			}

			pSourceBitmap = pConverter;
		}
	}
	else
	{
		D3D12_RESOURCE_DESC resourceDesc = pResource->pDeviceState->pD3DResource->GetDesc();
		MipFrameInfo.BlockWidth = 1;
		MipFrameInfo.BlockHeight = 1;
		MipFrameInfo.DxgiFormat = resourceDesc.Format;

		MipFrameInfo.WidthInBlocks = (UINT)(resourceDesc.Width) >> Mip;
		MipFrameInfo.HeightInBlocks = resourceDesc.Height >> Mip;
	}

	UINT NumTiles;
	UINT WidthInTiles;

	//
	// Mip maps beyond NumStandardMips are packed, and so the entire set of mipmaps
	// under it is packed into a single heap.
	//
	if (MipHeap >= pResource->PackedMipHeapIndex)
	{
		MipHeap = pResource->PackedMipHeapIndex;
		WidthInTiles = pResource->PackedMipTileCount;
		NumTiles = pResource->PackedMipTileCount;
	}
	else
	{
		WidthInTiles = pResource->pDeviceState->Mips[MipHeap].Desc.WidthInTiles;
		NumTiles = WidthInTiles * pResource->pDeviceState->Mips[MipHeap].Desc.HeightInTiles;
	}

	//
	// Round up the number of heaps needed for this mip.
	//
	UINT NumHeapsInMip = ((NumTiles * TILE_SIZE) + MAX_HEAP_SIZE - 1) / MAX_HEAP_SIZE;

	//
	// Although we have a fixed heap count that is already calculated, we still calculate
	// the required size so we don't allocate more space than necessary. We do not want
	// to create 16MB heaps for mipmaps that are much smaller, or mipmaps which only
	// excced multiples of 16MB by a small amount (e.g. 33MB == 3 heaps == 16+16+1)
	//
	UINT64 RemainingSize = NumTiles * TILE_SIZE;
	ID3D12Heap** ppHeaps = pResource->pDeviceState->Mips[MipHeap].ppHeaps;
	for (UINT i = 0; i < NumHeapsInMip; ++i)
	{
		ID3D12Heap* pHeap = ppHeaps[i];
		if (pHeap == nullptr)
		{
			D3D12_HEAP_DESC heapDesc = {};

			heapDesc.SizeInBytes = min(RemainingSize, MAX_HEAP_SIZE);
			heapDesc.Alignment = 0;
			heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			//
			// Tier 1 heaps have restrictions on the type of information that can be stored in
			// a heap. To accommodate this, we will retsrict the content to only shader resources.
			// The heap cannot store textures that are used as render targets, depth-stencil
			// output, or buffers. But this is okay, since we do not use these heaps for those
			// purposes.
			//
			heapDesc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;

			hr = m_pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&pHeap));
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to create D3D12 heap, hr=0x%.8x", hr);
				return hr;
			}

			ppHeaps[i] = pHeap;
			RemainingSize -= heapDesc.SizeInBytes;
		}
	}

	//
	// Calculate the required size of the upload buffer for transferring the contents
	// to the new heaps.
	//
	UINT64 UploadBufferSize = GetRequiredIntermediateSize(pResource->pDeviceState->pD3DResource, Mip, 1);

	ID3D12Resource* pUploadSurface;
	void* pUploadData;

	//
	// If we are using a shared staging surface, we don't have to create a new upload
	// resource for each transfer. This feature is currently experimental, and disabled
	// by default.
	//
	ComPtr<ID3D12Resource> pUploadBuffer;
	if (m_bUseSharedStagingSurface)
	{
		pUploadSurface = m_pStagingSurface;
		pUploadData = m_pStagingSurfaceData;
	}
	else
	{
		hr = m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(UploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create upload buffer, hr=0x%.8x", hr);
			return hr;
		}

		CD3DX12_RANGE readRange(0, 0);
		hr = pUploadBuffer->Map(0, &readRange, &pUploadData);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to map upload buffer, hr=0x%.8x", hr);
			return hr;
		}

		pUploadSurface = pUploadBuffer.Get();
	}

	//
	// Map the reserved resource (e.g. the virtual address we created with the resource)
	// to the heaps that back them. Page table updates for tiled resources can be costly,
	// so we will split up the mapping into one heap at a time (16MB of updates per call).
	//
	{
		static const UINT MaxTilesPerUpdate = MAX_HEAP_SIZE / TILE_SIZE;

		UINT32 NumTilesRemaining = NumTiles;

		while (NumTilesRemaining)
		{
			UINT TilesInUpdate = min(MaxTilesPerUpdate, NumTilesRemaining);

			UINT32 TileOffset = NumTiles - NumTilesRemaining;

			D3D12_TILED_RESOURCE_COORDINATE Coordinates = {};
			Coordinates.Subresource = Mip;
			Coordinates.X = TileOffset % WidthInTiles;
			Coordinates.Y = TileOffset / WidthInTiles;

			D3D12_TILE_REGION_SIZE RegionSize = {};
			RegionSize.NumTiles = TilesInUpdate;

			D3D12_TILE_RANGE_FLAGS RangeFlags = D3D12_TILE_RANGE_FLAG_NONE;
			UINT RangeOffset = 0;
			UINT RangeCount = TilesInUpdate;
			m_PagingContext.GetCommandQueue()->UpdateTileMappings(
				pResource->pDeviceState->pD3DResource,
				1,
				&Coordinates,
				&RegionSize,
				*ppHeaps,
				1,
				&RangeFlags,
				&RangeOffset,
				&RangeCount,
				D3D12_TILE_MAPPING_FLAG_NO_HAZARD);

			NumTilesRemaining -= TilesInUpdate;
			++ppHeaps;
		}
	}

	//
	// Copy the pixel data into the staging resource, and then transfer it to the
	// reserved resource via CopyTextureRegion.
	//
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layout;

	UINT NumRows;
	UINT64 RowSizeInBytes;
	UINT64 TotalBytes;
	D3D12_RESOURCE_DESC Desc = pResource->pDeviceState->pD3DResource->GetDesc();
	m_pDevice->GetCopyableFootprints(&Desc, Mip, 1, 0, &Layout, &NumRows, &RowSizeInBytes, &TotalBytes);
	UINT64 RemainingBytes = TotalBytes;

	UINT32 CurrentRow = 0;

	while (RemainingBytes > 0)
	{
		UINT32 MaxTransferHeightInBlocks;

		//
		// Begin a paging frame. Each paging operation (i.e. a copy/transfer) must be contained
		// within a paging frame so we can track and synchronize the operation on the context.
		//
		m_PagingContext.Begin();

		UINT64 BytesInTransfer;
		if (m_bUseSharedStagingSurface)
		{
			MaxTransferHeightInBlocks = static_cast<UINT32>(MAX_TRANSFER_SIZE / Layout.Footprint.RowPitch);
			BytesInTransfer = min(MAX_TRANSFER_SIZE, RemainingBytes);
		}
		else
		{
			MaxTransferHeightInBlocks = NumRows;
			BytesInTransfer = RemainingBytes;
		}

		UINT32 TransferHeightInBlocks = static_cast<UINT32>((BytesInTransfer + Layout.Footprint.RowPitch - 1) / Layout.Footprint.RowPitch);
		TransferHeightInBlocks = min(TransferHeightInBlocks, MaxTransferHeightInBlocks);

		UINT32 TransferHeightInRows = TransferHeightInBlocks * MipFrameInfo.BlockHeight;

		WICRect SourceRect;
		SourceRect.X = 0;
		SourceRect.Y = CurrentRow / MipFrameInfo.BlockHeight;
		SourceRect.Width = MipFrameInfo.WidthInBlocks;
		SourceRect.Height = TransferHeightInBlocks;

		//
		// The copy differs slightly based on whether or not this is a DDS file with block compressed data.
		//
		if (pDdsFrame)
		{
			hr = pDdsFrame->CopyBlocks(&SourceRect, Layout.Footprint.RowPitch, Layout.Footprint.RowPitch * TransferHeightInBlocks, ((BYTE*)pUploadData));
		}
		else if (pSourceBitmap.Get())
		{
			hr = pSourceBitmap->CopyPixels(&SourceRect, Layout.Footprint.RowPitch, Layout.Footprint.RowPitch * TransferHeightInBlocks, ((BYTE*)pUploadData));
		}
		else
		{
			hr = GenerateMip(pResource->GeneratedImageIndex, &SourceRect, Layout.Footprint.RowPitch, Layout.Footprint.RowPitch * TransferHeightInBlocks, (UINT*)pUploadData);
		}
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to copy frame data to upload staging buffer, hr=0x%.8x", hr);
			return hr;
		}

		//
		// Copy the texture region on the copy command queue.
		//
		Frame* pPagingFrame = m_PagingContext.GetCurrentFrame();

		D3D12_BOX SrcBox =
		{
			0,                      // UINT left;
			0,                      // UINT top;
			0,                      // UINT front;
			MipFrameInfo.WidthInBlocks * MipFrameInfo.BlockWidth, // UINT right;
			TransferHeightInRows,   // UINT bottom;
			1,                      // UINT back;
		};

		CD3DX12_TEXTURE_COPY_LOCATION Dst(pResource->pDeviceState->pD3DResource, Mip);
		CD3DX12_TEXTURE_COPY_LOCATION Src(pUploadSurface, Layout);
		Src.PlacedFootprint.Footprint.Height = TransferHeightInRows;
		Src.PlacedFootprint.Offset = 0;
		pPagingFrame->pCommandList->CopyTextureRegion(&Dst, 0, CurrentRow, 0, &Src, &SrcBox);

		//
		// Synchronize on this transfer. An application may extend this implementation
		// to support multiple operations at a time (by allowing multiple paging frames
		// to run concurrently). However, in the interest of simplifying the concepts
		// in the sample, it was opted to serialize paging operations to make things
		// easier to understand.
		//
		hr = m_PagingContext.Execute();
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to transfer content for resource 0x%p, mip %d. hr=0x%.8x", pResource, Mip, hr);
			return hr;
		}

		m_PagingContext.End();
		m_PagingContext.Flush();

		CurrentRow += TransferHeightInRows;
		RemainingBytes -= BytesInTransfer;
	}

	pResource->MostDetailedMipResident = Mip;

	AddResourceCommitment(pResource);

	return S_OK;
}

_Use_decl_annotations_
HRESULT DX12Framework::GenerateMip(UINT ImageIndex, WICRect* pRect, UINT RowPitch, UINT BufferSizeInBytes, UINT* pBuffer)
{
	const UINT RowWidth = RowPitch >> 2;
	const UINT BufferSize = BufferSizeInBytes >> 2;
	const UINT CellWidth = max(pRect->Width >> 3, 1);		// The width of a cell in the checkboard texture.
	const UINT CellHeight = max(pRect->Height >> 3, 1);		// The height of a cell in the checkerboard texture.
	const BYTE Gray = 0x88;
	const UINT Color = GetGeneratedImageColor(ImageIndex);

	if (RowWidth * pRect->Height > BufferSize)
	{
		LOG_ERROR("Buffer is too small for this mip");
		return E_INVALIDARG;
	}

	for (int y = 0; y < pRect->Height; y++)
	{
		UINT Index = y * RowWidth;
		for (int x = 0; x < pRect->Width; x++)
		{
			UINT i = x / CellWidth;
			UINT j = y / CellHeight;

			if (i % 2 == j % 2)
			{
				pBuffer[Index++] = Color;
			}
			else
			{
				// A subtle gradient from the top left corner to the bottom right.
				BYTE Shade = BYTE((i + j) << 3) + Gray;
				pBuffer[Index++] = 0xff << 24 | Shade << 16 | Shade << 8 | Shade;
			}
		}
	}

	return S_OK;
}

//
//-------------------------------------------------------------------------------------------------
// The following few functions are helper functions for simplified rendering operations.
// It should be noted that the emphasis of this sample was not to maximize rendeirng
// efficiency, but instead demonstrate memory management behavior. As a result, the
// rendering algorithms used in this application were designed around simplicity, and
// are not intended to be examples of optimized algorithms for rendering using the
// D3D12 API.
//-------------------------------------------------------------------------------------------------
//
void DX12Framework::DrawRectangle(const RectF* pDest, float Stroke, const ColorF* pColor)
{
	RectF Temp;

	// Left/Right frame.
	Temp.Top = pDest->Top - Stroke;
	Temp.Bottom = pDest->Bottom + Stroke;

	Temp.Left = pDest->Left - Stroke;
	Temp.Right = pDest->Left;
	FillRectangle(&Temp, pColor);

	Temp.Left = pDest->Right;
	Temp.Right = pDest->Right + Stroke;
	FillRectangle(&Temp, pColor);

	// Top/Bottom frame.
	Temp.Left = pDest->Left;
	Temp.Right = pDest->Right;

	Temp.Top = pDest->Top - Stroke;
	Temp.Bottom = pDest->Top;
	FillRectangle(&Temp, pColor);

	Temp.Top = pDest->Bottom;
	Temp.Bottom = pDest->Bottom + Stroke;
	FillRectangle(&Temp, pColor);
}

void DX12Framework::FillRectangle(const RectF* pDest, const ColorF* pColor)
{
	RenderFrame* pCurrentFrame = m_RenderContext.GetCurrentFrame();
	ID3D12GraphicsCommandList* pCommandList = pCurrentFrame->pCommandList;

	//
	// Set the color shader to render solid colored rectangles.
	//
	SetShader(pCurrentFrame, &m_ColorShader);

	typedef ColorShader::VertexFormat VertexType;

	VertexType Vertices[] =
	{
		{ { pDest->Left,  pDest->Top,    1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Top Left
		{ { pDest->Right, pDest->Bottom, 1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Bottom Right
		{ { pDest->Left,  pDest->Bottom, 1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Bottom Left

		{ { pDest->Right, pDest->Bottom, 1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Bottom Right
		{ { pDest->Left,  pDest->Top,    1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Top Left
		{ { pDest->Right, pDest->Top,    1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Top Right
	};

	VertexType* pVertices;
	UINT32 Offset;
	HRESULT hrVertexBuffer = pCurrentFrame->VertexBuffer.Allocate(sizeof(VertexType), 6, (void**)&pVertices, &Offset);

	if (hrVertexBuffer == E_OUTOFMEMORY)
	{
		//
		// If we ran out of space in our vertex buffer, we need to rename it in order to allocate
		// more vertices. Rename operations are guaranteed to succeed. Although in most cases
		// the system will have enough memory to simply allocate a new buffer, we are
		// capable of simply flushing the pipeline until the last frame was referenced,
		// allowing us to reuse the current buffer.
		//
		RenameDynamicBuffer(&pCurrentFrame->VertexBuffer);
		ResetShader(pCurrentFrame);
		hrVertexBuffer = pCurrentFrame->VertexBuffer.Allocate(sizeof(VertexType), 6, (void**)&pVertices, &Offset);
		assert(SUCCEEDED(hrVertexBuffer));
	}

	//
	// Copy vertices.
	//
	memcpy(pVertices, Vertices, sizeof(Vertices));

	pCommandList->DrawInstanced(6, 1, Offset / sizeof(VertexType), 0);
}

void DX12Framework::FillRectangle(const RectF* pDest, const ColorF* pColor, Resource* pResource)
{
	RenderFrame* pCurrentFrame = m_RenderContext.GetCurrentFrame();
	ID3D12GraphicsCommandList* pCommandList = pCurrentFrame->pCommandList;

	//
	// Texture has not been loaded, use default gray.
	//
	if (pResource->MostDetailedMipResident == GetResourceMipCount(pResource))
	{
		static const ColorF FillColor = { .5f, .5f, .5f, 1.0f };
		FillRectangle(pDest, &FillColor);
		return;
	}

	//
	// Reference the provided allocation, marking it with a rendering fence, which
	// we will use to track the visibility of the mipmap, so we can know when we
	// are allowed to trim it, if necessary.
	//
	UINT Mip = ReferenceResource(pResource, pCurrentFrame, pResource->VisibleMip);
	SetShader(pCurrentFrame, &m_TextureShader);

	typedef TextureShader::VertexFormat VertexType;

	VertexType Vertices[] =
	{
		{ { pDest->Left,  pDest->Top,    1.0f }, { 0.0f, 0.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Top Left
		{ { pDest->Right, pDest->Bottom, 1.0f }, { 1.0f, 1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Bottom Right
		{ { pDest->Left,  pDest->Bottom, 1.0f }, { 0.0f, 1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Bottom Left

		{ { pDest->Right, pDest->Bottom, 1.0f }, { 1.0f, 1.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Bottom Right
		{ { pDest->Left,  pDest->Top,    1.0f }, { 0.0f, 0.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Top Left
		{ { pDest->Right, pDest->Top,    1.0f }, { 1.0f, 0.0f }, { pColor->R, pColor->G, pColor->B, pColor->A } }, // Top Right
	};

	VertexType* pVertices;
	UINT32 Offset;
	HRESULT hrVertexBuffer = pCurrentFrame->VertexBuffer.Allocate(sizeof(VertexType), 6, (void**)&pVertices, &Offset);

	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	HRESULT hrSrvDescriptor = pCurrentFrame->SrvCbvHeap.Allocate(1, &GpuHandle, &CpuHandle);

	if (hrVertexBuffer == E_OUTOFMEMORY)
	{
		//
		// If we ran out of space in our vertex buffer, we need to rename it in order to allocate
		// more vertices. Rename operations are guaranteed to succeed. Although in most cases
		// the system will have enough memory to simply allocate a new buffer, we are
		// capable of simply flushing the pipeline until the last frame was referenced,
		// allowing us to reuse the current buffer.
		//
		RenameDynamicBuffer(&pCurrentFrame->VertexBuffer);
		ResetShader(pCurrentFrame);
		hrVertexBuffer = pCurrentFrame->VertexBuffer.Allocate(sizeof(VertexType), 6, (void**)&pVertices, &Offset);
		assert(SUCCEEDED(hrVertexBuffer));
	}

	if (hrSrvDescriptor == E_OUTOFMEMORY)
	{
		//
		// The same concept applies to descriptor heaps as buffers. We can guarantee
		// a successful rename operation and make forward progress at all times.
		//
		RenameDynamicDescriptorHeap(&pCurrentFrame->SrvCbvHeap);
		ResetShader(pCurrentFrame);
		hrSrvDescriptor = pCurrentFrame->SrvCbvHeap.Allocate(1, &GpuHandle, &CpuHandle);
		assert(SUCCEEDED(hrSrvDescriptor));
	}

	//
	// Copy vertices.
	//
	memcpy(pVertices, Vertices, sizeof(Vertices));

	//
	// Assign SRV.
	//
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};

		SrvDesc.Texture2D.MostDetailedMip = Mip;
		SrvDesc.Texture2D.MipLevels = GetResourceMipCount(pResource) - SrvDesc.Texture2D.MostDetailedMip;

		D3D12_RESOURCE_DESC Desc = pResource->pDeviceState->pD3DResource->GetDesc();

		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = Desc.Format;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		m_pDevice->CreateShaderResourceView(pResource->pDeviceState->pD3DResource, &SrvDesc, CpuHandle);
	}

	pCommandList->SetGraphicsRootDescriptorTable(1, GpuHandle);
	pCommandList->DrawInstanced(6, 1, Offset / sizeof(VertexType), 0);
}

HRESULT DX12Framework::RecreateDeviceDependentState()
{
	DestroyDeviceDependentStateInternal();

	return CreateDeviceDependentStateInternal();
}

bool DX12Framework::HandleMessage(HWND /*hwnd*/, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return true;
	}

	case WM_GETMINMAXINFO:
	{
		//
		// Enforce a minimum window size of 800x480, but no upper bound.
		//
		MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;
		pMinMaxInfo->ptMinTrackSize.x = 800;
		pMinMaxInfo->ptMinTrackSize.y = 480;
		pMinMaxInfo->ptMaxTrackSize.x = LONG_MAX;
		pMinMaxInfo->ptMaxTrackSize.y = LONG_MAX;
		return true;
	}

	case WM_SIZE:
	{
		if (wParam != SIZE_MINIMIZED)
		{
			//
			// Do not resize swapchain and camera projection on minimize.
			//
			OnSizeChanged();
		}
		return false;
	}
	}
	return false;
}

HRESULT DX12Framework::OnSizeChanged()
{
	RECT ClientRect;
	GetClientRect(m_Hwnd, &ClientRect);

	LONG NewWidth = ClientRect.right - ClientRect.left;
	LONG NewHeight = ClientRect.bottom - ClientRect.top;

	//
	// If the window size changed, resize our swapchain and recreate swapchain resources.
	//
	if (NewWidth != m_WindowWidth || NewHeight != m_WindowHeight)
	{
		m_WindowHeight = NewHeight;
		m_WindowWidth = NewWidth;

		DestroySwapChainResources();

		HRESULT hr = m_pDXGISwapChain->ResizeBuffers(SWAPCHAIN_BUFFER_COUNT, NewWidth, NewHeight, SWAPCHAIN_BACK_BUFFER_FORMAT, 0);
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to resize back buffer, hr=0x.8x", hr);
			return hr;
		}

		hr = CreateSwapChainResources();
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to recreate swap chain resources! hr=0x.8x", hr);
			return hr;
		}
	}

	return S_OK;
}

void DX12Framework::PrepareRendering()
{
	//
	// Reset the shader.
	//
	m_pCurrentShader = nullptr;
}

void DX12Framework::PrepareFrame(RenderFrame* pFrame, const Camera* pCamera)
{
	static const float ClearColor[] = { 0.35f, 0.35f, 0.35f, 1.0f };
	ID3D12GraphicsCommandList* pCommandList = pFrame->pCommandList;

	//
	// Setup viewport and scissor rect.
	//
	{
		D3D12_VIEWPORT Viewport;
		Viewport.TopLeftX = 0.0f;
		Viewport.TopLeftY = 0.0f;
		Viewport.Width = static_cast<FLOAT>(m_WindowWidth);
		Viewport.Height = static_cast<FLOAT>(m_WindowHeight);
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;
		pCommandList->RSSetViewports(1, &Viewport);

		D3D12_RECT ScissorRect;
		ScissorRect.left = 0;
		ScissorRect.top = 0;
		ScissorRect.right = m_WindowWidth;
		ScissorRect.bottom = m_WindowHeight;
		pCommandList->RSSetScissorRects(1, &ScissorRect);
	}

	UINT BackBufferIndex = m_pDXGISwapChain->GetCurrentBackBufferIndex();
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[BackBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), BackBufferIndex, m_DescriptorInfo.RtvDescriptorSize);
	pCommandList->OMSetRenderTargets(1, &RtvHandle, true, nullptr);
	pCommandList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);

	//
	// Reset the buffers and descriptor heaps for the new frame.
	//
	pFrame->VertexBuffer.Reset();
	pFrame->ConstantBuffer.Reset();
	pFrame->SrvCbvHeap.Reset();

	UINT32 ConstantBufferSize = CalculateConstantBufferSize(sizeof(DirectX::XMMATRIX));

	//
	// Setup camera information for this frame.
	//
	void* pViewProjection;
	UINT32 ViewProjectionOffset;
	pFrame->ConstantBuffer.Allocate(ConstantBufferSize, 1, &pViewProjection, &ViewProjectionOffset);

	*(DirectX::XMMATRIX*)pViewProjection = pCamera->GetViewProjectionMatrix();

	pFrame->CameraAddress = pFrame->ConstantBuffer.GetGPUVirtualAddress() + ViewProjectionOffset;
}

static float CalculateDeltaTime(LONGLONG CurrentTick, LONGLONG PreviousTick, LONGLONG PerformanceFrequency)
{
	return (float)(((CurrentTick - PreviousTick) / (double)PerformanceFrequency));
}

HRESULT DX12Framework::RenderInternal()
{
	HRESULT hr;

	LARGE_INTEGER PrevTick = m_LastFrameCounter;
	LARGE_INTEGER CurrentTick;
	QueryPerformanceCounter(&CurrentTick);

	m_StatTimeBetweenFrames[m_StatIndex] = CalculateDeltaTime(CurrentTick.QuadPart, PrevTick.QuadPart, m_PerformanceFrequency.QuadPart);
	m_LastFrameCounter = CurrentTick;

	//
	// Prepare for a new frame.
	//
	PrepareRendering();
	m_RenderContext.Begin();

	RenderFrame* pFrame = m_RenderContext.GetCurrentFrame();

	PrepareFrame(pFrame, m_pSceneCamera);

	RectF ViewportBounds;

	//
	// Calculate the bounds for the scene camera and render the scene.
	//
	ViewportBounds = m_pSceneCamera->GenerateViewportBounds();
	{
		QueryPerformanceCounter(&PrevTick);
		RenderScene(ViewportBounds);
		QueryPerformanceCounter(&CurrentTick);

		m_StatRenderScene[m_StatIndex] = CalculateDeltaTime(CurrentTick.QuadPart, PrevTick.QuadPart, m_PerformanceFrequency.QuadPart);
	}

	//
	// Execute the command lists generated for the scene geometry. We will draw the
	// UI in a second pass using the D3D11On12 interface using D2D for rendering.
	//
	hr = m_RenderContext.Execute();
	assert(SUCCEEDED(hr));

	//
	// Optionally simulate a failure for debugging.
	//
	if (SUCCEEDED(hr) && m_SimulatedRenderResult != S_OK)
	{
		hr = m_SimulatedRenderResult;
		m_SimulatedRenderResult = S_OK;
	}

	//
	// Render the UI.
	//
	if (SUCCEEDED(hr))
	{
		QueryPerformanceCounter(&PrevTick);

		hr = RenderUI();

		QueryPerformanceCounter(&CurrentTick);

		m_StatRenderUI[m_StatIndex] = CalculateDeltaTime(CurrentTick.QuadPart, PrevTick.QuadPart, m_PerformanceFrequency.QuadPart);
	}

	//
	// Ending the frame signals the fence on the command queue, allowing us to synchronize
	// GPU access by waiting on the fence. The fence will be completed by the graphics
	// kernel when the hardware completes the frame, and then the frame will be retired.
	//
	m_RenderContext.End();

	m_StatIndex = ((m_StatIndex + 1) % STATISTIC_COUNT);

	if (FAILED(hr))
	{
		LOG_WARNING("Rendering operations failed, likely device removed, hr=0x%.8x", hr);
		return hr;
	}

	//
	// Present and update the frame index for the next frame.
	//
	hr = m_pDXGISwapChain->Present(m_bPresentOnVsync, 0);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to present swap chain, hr=0x%.8x", hr);
		return hr;
	}

	DXGI_FRAME_STATISTICS FrameStatistics;
	hr = m_pDXGISwapChain->GetFrameStatistics(&FrameStatistics);

	if (FrameStatistics.PresentCount > m_PreviousPresentCount)
	{
		if (m_PreviousRefreshCount > 0 &&
			(FrameStatistics.PresentRefreshCount - m_PreviousRefreshCount) > (FrameStatistics.PresentCount - m_PreviousPresentCount))
		{
			++m_GlitchCount;
		}
	}
	m_PreviousPresentCount = FrameStatistics.PresentCount;
	m_PreviousRefreshCount = FrameStatistics.SyncRefreshCount;

	return S_OK;
}

void DX12Framework::SetShader(RenderFrame* pFrame, const Shader* pShader)
{
	//
	// Setup the current shader by binding the pipeline state, root signature,
	// and vertex layout information.
	//
	if (pShader != m_pCurrentShader)
	{
		ID3D12GraphicsCommandList* pCommandList = pFrame->pCommandList;

		pCommandList->SetGraphicsRootSignature(pShader->GetRootSignature());
		pCommandList->SetPipelineState(pShader->GetPipelineState());
		pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* ppHeaps[] = { pFrame->SrvCbvHeap.m_pHeap->pHeap };
		pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		pCommandList->SetGraphicsRootConstantBufferView(0, pFrame->CameraAddress);

		pFrame->VertexBuffer.Align(pShader->GetVertexSize());

		D3D12_VERTEX_BUFFER_VIEW Vbv = {};
		Vbv.BufferLocation = pFrame->VertexBuffer.GetGPUVirtualAddress();
		Vbv.StrideInBytes = pShader->GetVertexSize();
		Vbv.SizeInBytes = DYNAMIC_BUFFER_SIZE;

		pCommandList->IASetVertexBuffers(0, 1, &Vbv);

		m_pCurrentShader = pShader;
	}
}

UINT8 DX12Framework::ReferenceResource(Resource* pResource, RenderFrame* pFrame, UINT8 RequestedMip)
{
	RequestedMip = ChooseMoreDetailedMip(RequestedMip, GetResourceMipCount(pResource) - 1);

	//
	// Grab the reference lock so we can choose the mipmap to display and mark
	// it with a reference fence. The reference fence is used by the paging thread
	// to synchronize access to the mipmap's heaps when it needs to be trimmed. We
	// cannot evict physical memory while the allocation can be touched by the GPU.
	//
	EnterCriticalSection(&pResource->ReferenceLock);

	UINT8 Mip = ChooseLessDetailedMip(RequestedMip, pResource->MostDetailedMipResident);
	Mip = ChooseLessDetailedMip(Mip, pResource->MipRestriction);
	UINT8 MipHeapIndex = GetMipHeapIndexForResource(pResource, Mip);
	pResource->pDeviceState->Mips[MipHeapIndex].ReferenceFence = pFrame->CompletionFence;

	LeaveCriticalSection(&pResource->ReferenceLock);

	return Mip;
}

HRESULT DX12Framework::PageInNextLevelOfDetail(Resource* pResource)
{
	HRESULT hr = S_OK;

	UINT8 ResidentMip = pResource->MostDetailedMipResident;
	assert(ResidentMip != 0);

	UINT8 Mip = ResidentMip - 1;

	assert(IsMoreDetailedMip(ResidentMip, Mip));

	UINT32 MipHeap = GetMipHeapIndexForResource(pResource, Mip);
	ResourceMip* pResourceMip = &pResource->pDeviceState->Mips[MipHeap];

	if (*pResourceMip->ppHeaps == nullptr || Mip >= pResource->PackedMipHeapIndex)
	{
		//
		// We need to create the heap and page in the texture from disk, since this
		// mipmap has not yet been created. There is an exception for packed mipmaps,
		// where the heap has been created, but we still need to copy the pixel data
		// and possibly update the virtual address.
		//
		hr = LoadMip(pResource, Mip);
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to load mip");
			return hr;
		}
	}
	else
	{
		UINT i;

		//
		// The texture was already loaded, but we evicted it. Make it resident now.
		//
		UINT HeapCount = GetResourceMipHeapCount(*pResourceMip);
		for (i = 0; i < HeapCount; ++i)
		{
			//
			// The MakeResident API is synchronous, and the resource is considered to
			// be fully resident and usable by the GPU by the time the call returns.
			//
			ID3D12Pageable* pPageable = pResourceMip->ppHeaps[i];
			hr = m_pDevice->MakeResident(1, &pPageable);
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to make resource 0x%p mip %d resident, hr=0x%.8x", pResource, Mip, hr);
				break;
			}
		}

		if (FAILED(hr))
		{
			assert(i > 0);

			//
			// Undo the MakeResident calls above.
			//
			while (i > 0)
			{
				ID3D12Pageable* pPageable = pResourceMip->ppHeaps[i];
				HRESULT hrTemp = m_pDevice->Evict(1, &pPageable);
				if (FAILED(hrTemp))
				{
					LOG_WARNING("Failed to evict resource 0x%p mip %d, hr=0x%.8x", pResource, Mip, hrTemp);
				}
				--i;
			}

			return hr;
		}

		pResource->MostDetailedMipResident = Mip;
		pResource->MipRestriction = 0;

		//
		// Add this mipmap to the commitment lists, which is used to efficiently
		// trim more detailed mips first.
		//
		AddResourceCommitment(pResource);
	}

	return S_OK;
}

bool DX12Framework::TrimToTarget(ResourceTrimPass MaxPass, UINT64 TargetUsage)
{
	//
	// The caller of this function passes a trimming pass restriction. This restriction
	// is intended to prevent lower priority allocations from trimming higher priority
	// ones. For example, if we page in a prefetched mip, we do not want to allow that
	// to trim a visible resource. In addition to being incorrect, and artifically
	// lowering the quality for the user, it creates a recursive trimming behavior
	// by allowing the paging thread to issue trimming calls to page in the visible mip,
	// which may trim the prefetched mip.
	//
	for (ResourceTrimPass CurrentPass = ERTP_NonPrefetchable;
		CurrentPass <= MaxPass;
		CurrentPass = static_cast<ResourceTrimPass>(CurrentPass + 1))
	{
		//
		// Go through the commitment list for each mip level.
		//
		for (UINT8 Mip = 0; Mip < MAX_MIP_COUNT; ++Mip)
		{
			LIST_ENTRY* pResourceListHead = &m_CommitmentListHeads[Mip];

			LIST_ENTRY* pEntry = pResourceListHead->Flink;
			while (pEntry != pResourceListHead)
			{
				Resource* pResource = CONTAINING_RECORD(pEntry, Resource, CommittedListEntry);
				pEntry = pEntry->Flink;

				if (Mip >= GetLeastDetailedMipHeapIndex(pResource))
				{
					//
					// Skip least detailed/packed mips, we cannot evict them.
					//
					continue;
				}

				ResourceMip* pResourceMip = &pResource->pDeviceState->Mips[Mip];

				UINT64 WaitFence = 0;

				//
				// Take the reference lock so we can restrict mipmap detail for the rendering
				// thread, while simultaneously querying the reference fence that we'll need
				// to wait on in order to trim the mip.
				//
				EnterCriticalSection(&pResource->ReferenceLock);

				if (pResourceMip->ReferenceFence > m_RenderContext.GetLastCompletedFence())
				{
					WaitFence = pResourceMip->ReferenceFence;
				}

				if (CurrentPass == ERTP_Visible)
				{
					pResource->MipRestriction = DecreaseMipQuality(Mip, 1);
				}
				else if (CurrentPass == ERTP_NonVisible && IsLessDetailedMip(Mip, pResource->VisibleMip))
				{
					pResource->MipRestriction = DecreaseMipQuality(Mip, 1);
				}
				else if (CurrentPass == ERTP_NonPrefetchable && IsLessDetailedMip(Mip, pResource->PrefetchMip))
				{
					pResource->MipRestriction = DecreaseMipQuality(Mip, 1);
				}
				LeaveCriticalSection(&pResource->ReferenceLock);

				if (pResource->MipRestriction > Mip)
				{
					//
					// This mip was used in a render operation that has not been completed. We must wait
					// for the operation to complete before we trim the mipmap.
					//
					if (WaitFence > 0)
					{
						m_RenderContext.WaitForFence(WaitFence);
					}

					//
					// Trim the mipmap, and check if our budget constraints have been met.
					//
					TrimMip(pResource, Mip);

					UpdateVideoMemoryInfo();
					if (m_LocalVideoMemoryInfo.CurrentUsage < TargetUsage)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

void DX12Framework::TrimMip(Resource* pResource, UINT8 Mip)
{
	ResourceMip* pResourceMip = &pResource->pDeviceState->Mips[Mip];

	//
	// Evict all the heaps for this mipmap.
	//
	UINT HeapCount = GetResourceMipHeapCount(*pResourceMip);
	for (UINT i = 0; i < HeapCount; ++i)
	{
		ID3D12Pageable* pPageable = pResourceMip->ppHeaps[i];
		HRESULT hr = m_pDevice->Evict(1, &pPageable);
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to evict resource 0x%p mip %d, hr=0x%.8x", pResource, Mip, hr);
		}
	}

	pResource->MostDetailedMipResident = DecreaseMipQuality(Mip, 1);
	pResource->MipRestriction = 0;

	//
	// Adjust the resource commitment level so it is a lower priority when trimming.
	//
	AddResourceCommitment(pResource);

	//
	// Evicting a resource mipmap means that there is some paging work that now must
	// be done. This may simply be low priority prefetching, but may be higher priority too.
	//
	m_pWorkerThread->PrioritizeResource(pResource);
}

void DX12Framework::AddResourceCommitment(Resource* pResource)
{
	//
	// Remove the resource from any existing commitment level.
	//
	RemoveEntryList(&pResource->CommittedListEntry);

	//
	// One optimization here can be made to improve the quality of selecting resources
	// for trimming. Not all resources are the same dimensions, which means a resource's
	// mip level 0 may correspond to both a small and large texture - each will provide
	// a very different gain from evicting it. It should be a higher priority to evict
	// mipmaps which provide the most gains at the current trimming pass. This can be
	// solved by offsetting the resource in the commitment tree based on its dimensions,
	// providing a "level of detail" based not explicitly on the mip levels, but by
	// total detail level.
	//
	UINT CommittedListIndex = pResource->MostDetailedMipResident;
	InsertTailList(&m_CommitmentListHeads[CommittedListIndex], &pResource->CommittedListEntry);
}

void DX12Framework::RemoveResourceCommitment(Resource* pResource)
{
	RemoveEntryList(&pResource->CommittedListEntry);
	InsertTailList(&m_UncommittedListHead, &pResource->CommittedListEntry);
}

void DX12Framework::LoadConfig(int argc, LPCSTR argv[])
{
	//
	// Load configuration options here.
	//
	for (int i = 1; i < argc; ++i)
	{
		LPCSTR pArg = argv[i];

		if (_strcmpi(pArg, "-sharedstaging") == 0)
		{
			m_bUseSharedStagingSurface = true;
		}
	}
}
