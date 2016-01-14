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

//
// Structure to save off descriptor sizes for various descriptor heap types.
// These values never change, and need only be queried by the D3D runtime once.
//
struct DescriptorInfo
{
	UINT32 RtvDescriptorSize;
	UINT32 SamplerDescriptorSize;
	UINT32 SrvUavCbvDescriptorSize;
};

struct BitmapFrameInfo
{
	// For block compressed formats, this is the width and height of a single
	// block of texture data. This will commonly be 4x4 texels.
	// For non-block compressed formats, these will always be 1, and represent
	// a single texel.
	UINT BlockWidth;
	UINT BlockHeight;

	// For block compressed formats, this is the width and height of the texture,
	// in blocks. The resolution of the texture, in texels, would be equal to:
	// (BlockWidth * WidthInBlocks) x (BlockHeight * HeightInBlocks)
	// For non-block compressed formats, these will be equal to the texture
	// resolution, in texels.
	UINT WidthInBlocks;
	UINT HeightInBlocks;

	// This is the DXGI_FORMAT of the texture.
	DXGI_FORMAT DxgiFormat;

	// These values are only used when loading image formats other than DDS, and
	// are used to provide WIC with the necessary pixel formats for performing
	// conversion information as the data is copied from disk.
	GUID SourcePixelFormat;
	GUID TargetPixelFormat;
};

class DX12Framework
{
	friend class RenderContext;
	friend class PagingContext;

private:
	//
	// Device state management
	//
	HRESULT RecreateDeviceDependentState();

	HRESULT CreateDeviceIndependentStateInternal();
	HRESULT CreateDeviceDependentStateInternal();
	HRESULT CreateResourceDeviceState(Resource* pResource, UINT NumMips, DXGI_FORMAT Format, UINT Width, UINT Height);

	void DestroyDeviceIndependentStateInternal();
	void DestroyDeviceDependentStateInternal();
	void DestroyResource(Resource* pResource);
	void DestroyResourceDeviceState(Resource* pResource);

	HRESULT CreateSwapChainResources();
	void DestroySwapChainResources();

	HRESULT QueryCaps();

	//
	// Resource management
	//
	HRESULT GetResourceInformation(IWICBitmapDecoder* pDecoder, UINT& NumMips, DXGI_FORMAT& Format, UINT& Width, UINT& Height);
	UINT8 ReferenceResource(Resource* pResource, RenderFrame* pFrame, UINT8 RequestedMip);
	void TrimMip(Resource* pResource, UINT8 Mip);
	HRESULT GetDdsFrameInfo(IWICDdsFrameDecode* pFrame, BitmapFrameInfo* pFormatInfo);
	HRESULT GetBitmapFrameInfo(IWICBitmapFrameDecode* pFrame, BitmapFrameInfo* pFormatInfo);
	HRESULT LoadMip(Resource* pResource, UINT32 Mip);
	HRESULT GenerateMip(UINT ImageIndex, WICRect* pRect, UINT RowPitch, UINT BufferSizeInBytes, _In_reads_bytes_(BufferSizeInBytes) UINT* pBuffer);
	void RemoveResourceCommitment(Resource* pResource);
	void AddResourceCommitment(Resource* pResource);


	//
	// Dynamic buffers
	//
	HRESULT InitDynamicBuffer(DynamicBuffer* pBuffer);
	void DestroyDynamicBuffer(DynamicBuffer* pBuffer);
	HRESULT AllocateVersionedBuffer(Buffer** ppBuffer);
	void FreeVersionedBuffer(Buffer* ppBuffer);

	void RenameDynamicBuffer(DynamicBuffer* pBuffer);
	void RetireVersionedBuffer(Buffer* pBuffer);

	//
	// Dynamic heaps
	//
	HRESULT InitDynamicDescriptorHeap(DynamicDescriptorHeap* pHeap);
	void DestroyDynamicDescriptorHeap(DynamicDescriptorHeap* pHeap);
	HRESULT AllocateVersionedDescriptorHeap(DescriptorHeap** ppHeap);
	void FreeVersionedDescriptorHeap(DescriptorHeap* ppHeap);

	void RenameDynamicDescriptorHeap(DynamicDescriptorHeap* pDynamicHeap);
	void RetireVersionedDescriptorHeap(DescriptorHeap* pHeap);

	//
	// Frame management
	//
	void PrepareFrame(RenderFrame* pFrame, const Camera* pCamera);
	void PrepareRendering();
	HRESULT RenderInternal();

	HRESULT OnSizeChanged();

	HRESULT Init();

	void SetShader(RenderFrame* pFrame, const Shader* pShader);
	inline void ResetShader(RenderFrame* pFrame)
	{
		const Shader* pCurrentShader = m_pCurrentShader;
		m_pCurrentShader = nullptr;
		SetShader(pFrame, pCurrentShader);
	}

protected:
	IWICImagingFactory* m_pWICFactory = nullptr;
	IDXGIFactory2* m_pDXGIFactory = nullptr;
	IDXGIAdapter3* m_pDXGIAdapter = nullptr;
	ID3D12Device* m_pDevice = nullptr;
	ID3D12DescriptorHeap* m_pRtvHeap = nullptr;
	IDXGISwapChain3* m_pDXGISwapChain = nullptr;
	ID3D12Resource* m_pRenderTargets[SWAPCHAIN_BUFFER_COUNT];
	ID3D12Resource* m_pStagingSurface = nullptr;
	void* m_pStagingSurfaceData = nullptr;

	//
	// 11On12 interface for UI
	//
	ID3D11Device* m_p11Device = nullptr;
	ID3D11DeviceContext* m_p11Context = nullptr;
	ID3D11On12Device* m_p11On12Device = nullptr;
	ID2D1Factory3* m_pD2DFactory = nullptr;
	ID2D1Device2* m_pD2DDevice = nullptr;
	ID2D1DeviceContext2* m_pD2DContext = nullptr;
	IDWriteFactory2* m_pDWriteFactory = nullptr;

	ID3D11Resource* m_pWrappedBackBuffers[SWAPCHAIN_BUFFER_COUNT];
	ID2D1Bitmap1* m_pD2DRenderTargets[SWAPCHAIN_BUFFER_COUNT];

	HWND m_Hwnd = nullptr;
	LONG m_WindowWidth = 0;
	LONG m_WindowHeight = 0;

	RenderContext m_RenderContext;
	PagingContext m_PagingContext;

	PagingWorkerThread* m_pWorkerThread = nullptr;

	DescriptorInfo m_DescriptorInfo;

	D3D12_FEATURE_DATA_D3D12_OPTIONS m_Options;
	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT m_GpuVaSupport;

	DXGI_QUERY_VIDEO_MEMORY_INFO m_LocalVideoMemoryInfo;
	DXGI_QUERY_VIDEO_MEMORY_INFO m_NonLocalVideoMemoryInfo;

	LIST_ENTRY m_ResourceListHead;
	LIST_ENTRY m_DynamicBufferListHead;
	LIST_ENTRY m_DynamicDescriptorHeapListHead;
	LIST_ENTRY m_UnreferencedResourceListHead;
	LIST_ENTRY m_UncommittedListHead;
	LIST_ENTRY m_CommitmentListHeads[MAX_MIP_COUNT];

	TextureShader m_TextureShader;
	ColorShader m_ColorShader;
	const Shader* m_pCurrentShader = nullptr;

	DWORD m_ThreadContextWaitHandleIndex = 0;
	UINT64 m_LocalBudgetOverride = 0;

	//
	// Camera
	//
	const Camera* m_pSceneCamera = nullptr;

	//
	// Perf stats
	//
	UINT m_StatIndex = 0;
	LARGE_INTEGER m_PerformanceFrequency = {};
	LARGE_INTEGER m_LastFrameCounter = {};
	float m_StatTimeBetweenFrames[STATISTIC_COUNT];
	float m_StatRenderScene[STATISTIC_COUNT];
	float m_StatRenderUI[STATISTIC_COUNT];
	UINT m_PreviousPresentCount = 0;
	UINT m_PreviousRefreshCount = 0;
	UINT m_GlitchCount = 0;

	bool m_bUseSharedStagingSurface = false;
	bool m_bPresentOnVsync = true;

	HRESULT m_SimulatedRenderResult = S_OK;
	UINT m_NewAdapterIndex = 0xFFFFFFFF;

protected:
	DX12Framework();
	virtual ~DX12Framework();

	virtual HRESULT CreateDeviceIndependentState() = 0;
	virtual HRESULT CreateDeviceDependentState() = 0;
	virtual void DestroyDeviceIndependentState() = 0;
	virtual void DestroyDeviceDependentState() = 0;

	virtual HRESULT LoadAssets() = 0;

public:
	//
	// Rendering
	//
	void DrawRectangle(const RectF* pDest, float Stroke, const ColorF* pColor);
	void FillRectangle(const RectF* pDest, const ColorF* pColor);
	void FillRectangle(const RectF* pDest, const ColorF* pColor, Resource* pResource);

	virtual HRESULT RenderScene(const RectF& ViewportBounds) = 0;
	virtual HRESULT RenderUI() = 0;

	HRESULT Run();
	HRESULT UpdateVideoMemoryInfo();
	void LoadConfig(int argc, LPCSTR argv[]);
	virtual bool HandleMessage(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	//
	// Resource loading
	//
	HRESULT CreateResource(LPCWSTR pFileName, Resource** ppResource);

	//
	// Worker Thread
	//
	inline void NotifyPagingWork(Resource* pResource)
	{
		m_pWorkerThread->EnqueueResource(pResource);
	}
	HRESULT PageInNextLevelOfDetail(Resource* pResource);
	bool TrimToTarget(ResourceTrimPass TrimLimit, UINT64 TargetUsage);
	inline bool TrimToBudget(ResourceTrimPass TrimLimit)
	{
		return TrimToTarget(TrimLimit, m_LocalVideoMemoryInfo.Budget);
	}

	//
	// Camera
	//
	inline void SetSceneCamera(const Camera* pCamera)
	{
		m_pSceneCamera = pCamera;
	}

	//
	// Budget managemnt
	//
	inline void SetLocalBudgetOverride(UINT64 Value)
	{
		m_LocalBudgetOverride = Value;
		SetEvent(m_pWorkerThread->m_hWakeEvents[EWR_BudgetNotification]);
	}

	inline UINT64 GetLocalBudgetOverride() const
	{
		return m_LocalBudgetOverride;
	}

	//
	// Statistics
	//
	inline UINT GetGlitchCount() const
	{
		return m_GlitchCount;
	}

	//
	// Data Access
	//
	inline ID3D12Device* GetDevice()
	{
		return m_pDevice;
	}

	inline IDXGIAdapter3* GetAdapter()
	{
		return m_pDXGIAdapter;
	}

	inline ID3D12DescriptorHeap* GetRtvDescriptorHeap()
	{
		return m_pRtvHeap;
	}

	inline const DXGI_QUERY_VIDEO_MEMORY_INFO& GetLocalVideoMemoryInfo() const
	{
		return m_LocalVideoMemoryInfo;
	}

	inline const DXGI_QUERY_VIDEO_MEMORY_INFO& GetNonLocalVideoMemoryInfo() const
	{
		return m_NonLocalVideoMemoryInfo;
	}

	inline DWORD GetThreadContextWaitHandleIndex() const
	{
		return m_ThreadContextWaitHandleIndex;
	}

	bool IsUnderBudget() const
	{
		return m_LocalVideoMemoryInfo.CurrentUsage <= m_LocalVideoMemoryInfo.Budget;
	}

	bool IsOverBudget() const
	{
		return m_LocalVideoMemoryInfo.CurrentUsage > m_LocalVideoMemoryInfo.Budget;
	}

	bool IsWithinBudgetThreshold(UINT64 Size) const
	{
		return m_LocalVideoMemoryInfo.CurrentUsage + Size <= m_LocalVideoMemoryInfo.Budget;
	}
};
