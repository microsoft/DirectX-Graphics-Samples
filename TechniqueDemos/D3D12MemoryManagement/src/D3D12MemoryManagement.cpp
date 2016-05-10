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

//
// The size and padding between images rendered on screen.
//
#define RENDER_TILE_SIZE 256
#define RENDER_TILE_PADDING 8

//
// The number of rows of images to display at any one time.
//
#define NUM_ROWS 4

//
// Constants defining the size and formatting of the memory graph shown in the top-right corner.
//
#define GRAPH_PADDING 16.0f
#define GRAPH_WIDTH 512.0f
#define GRAPH_HEIGHT 96.0f
#define GRAPH_HEIGHT_RATIO 90
#define GRAPH_WIDTH_PER_SEGMENT (GRAPH_WIDTH / GRAPH_SEGMENTS)
#define GRAPH_NOTCH_COUNT 4

//
// The default distance from the camera (unscaled) to prioritize prefetched mipmaps.
//
#define PREFETCH_DISTANCE 600.0f

//
// Helper function to calculate an average for a numbe rof statistic points.
//
static float AverageStatistics(float* pStats, UINT StatCount)
{
	float Total = 0.0f;
	for (UINT i = 0; i < StatCount; ++i)
	{
		Total += pStats[i];
	}
	return Total / StatCount;
}

//
// Initializes the specified image object to reference the input resource,
// and calculates the image bounds based on its image index. The image bounds
// are then cached and used for visibility and rendering calculations.
//
void InitializeImage(Image* pImage, Resource* pResource, UINT32 ImageIndex)
{
	UINT32 TileWidth = RENDER_TILE_SIZE;
	UINT32 TileHeight = RENDER_TILE_SIZE;

	UINT32 Row = ImageIndex % NUM_ROWS;
	UINT32 Col = ImageIndex / NUM_ROWS;

	UINT32 HeightOffset = ((NUM_ROWS - 1) * RENDER_TILE_SIZE + (NUM_ROWS - 1) * RENDER_TILE_PADDING) / 2;

	D3D12_RESOURCE_DESC Desc = pResource->pDeviceState->pD3DResource->GetDesc();
	float ScaleX = 1.0f;
	float ScaleY = 1.0f;
	if (Desc.Width < 256)
	{
		ScaleX = Desc.Width / 256.0f;
	}
	if (Desc.Height < 256)
	{
		ScaleY = Desc.Height / 256.0f;
	}

	if (Desc.Width > Desc.Height)
	{
		ScaleY *= (float)Desc.Height / Desc.Width;
	}
	else
	{
		ScaleX *= (float)Desc.Width / Desc.Height;
	}

	float X = Col * ((float)TileWidth + RENDER_TILE_PADDING);
	float Y = Row * ((float)TileHeight + RENDER_TILE_PADDING) - HeightOffset;
	float Width = TileWidth * ScaleX;
	float Height = TileHeight * ScaleY;

	float HalfWidth = Width / 2;
	float HalfHeight = Height / 2;

	RectF DestRect =
	{
		X - HalfWidth,
		Y - HalfHeight,
		X + HalfWidth,
		Y + HalfHeight
	};

	pImage->pResource = pResource;
	pImage->Bounds = DestRect;
}

D3D12MemoryManagement::D3D12MemoryManagement()
{
	m_ViewportCamera.Initialize(PointF{ 600.0f, 0.0f }, 0.6f);
	SetSceneCamera(&m_ViewportCamera);

	m_DebugCamera = m_ViewportCamera;
	m_pCapturedCamera = &m_ViewportCamera;
	m_pSceneCamera = &m_ViewportCamera;
	ZeroMemory(m_GraphPoints, sizeof(m_GraphPoints));
#if(_DEBUG)
	m_bRenderStats = true;
	m_bDrawMipColors = true;
#endif
}

D3D12MemoryManagement::~D3D12MemoryManagement()
{
}

HRESULT D3D12MemoryManagement::CreateDeviceDependentState()
{
	//
	// D2D brushes must be created when the device is recreated.
	//
	HRESULT hr;
	hr = m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pTextBrush);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create solid color brush for text, hr=0x%.8x", hr);
		return hr;
	}

	return S_OK;
}

HRESULT D3D12MemoryManagement::CreateDeviceIndependentState()
{
	HRESULT hr;

	//
	// Text formats are not dependent on a graphics device, so they can be created
	// once and saved.
	//
	hr = m_pDWriteFactory->CreateTextFormat(
		L"Verdana",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		11.0f,
		L"en-us",
		&m_pTextFormat);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create text format, hr=0x%.8x", hr);
		return hr;
	}

	m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	return S_OK;
}

void D3D12MemoryManagement::DestroyDeviceDependentState()
{
	SafeRelease(m_pTextBrush);
}

void D3D12MemoryManagement::DestroyDeviceIndependentState()
{
	SafeRelease(m_pTextFormat);
}

//
// LoadAssets goes through the texture asset directory and attempts to load all files
// as images for the application to stream in. Texture assets are located in
// Assets\Textures, relative to the sample binary.
//
HRESULT D3D12MemoryManagement::LoadAssets()
{
	WCHAR FileName[MAX_PATH];
	GetWorkingDir(FileName, MAX_PATH);

	wcscat_s(FileName, L"Assets\\Textures\\*");
	WCHAR* pEnd = FileName + wcslen(FileName) - 1;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(FileName, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		//
		// Does the directory not exist?
		//
		LOG_ERROR("Failed to load texture assets");
		return E_FAIL;
	}

	UINT ImageIndex = 0;

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//
			// Ignore directories.
			//
			continue;
		}
		else
		{
			//
			// Try to create the resource from the file. This may not be a valid image file,
			// and if this is the case, we will fail to create the resource, and continue
			// with the next file.
			//
			*pEnd = L'\0';
			wcscat_s(FileName, ffd.cFileName);

			Resource* pResource = nullptr;

			HRESULT hr = CreateResource(FileName, &pResource);
			if (FAILED(hr))
			{
				continue;
			}

			Image Img;
			InitializeImage(&Img, pResource, ImageIndex);
			m_Images.push_back(Img);

			++ImageIndex;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);

	for (UINT GeneratedImage = 0; GeneratedImage < MAX_GENERATED_IMAGES; ++GeneratedImage)
	{
		Resource* pResource = nullptr;
		HRESULT hr = CreateResource(L"", &pResource);
		if (FAILED(hr))
		{
			LOG_WARNING("Unable to generate image %d", GeneratedImage);
			continue;
		}

		Image Img;
		InitializeImage(&Img, pResource, ImageIndex);
		m_Images.push_back(Img);

		++ImageIndex;
	}

	return S_OK;
}

bool D3D12MemoryManagement::HandleMessage(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = DX12Framework::HandleMessage(hwnd, Message, wParam, lParam);
	if (bHandled)
	{
		return true;
	}

	switch (Message)
	{
	case WM_MOUSEWHEEL:
		{
			// Use mouse wheel to apply camera zoom.
			m_pCapturedCamera->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			return true;
		}

	case WM_LBUTTONDOWN:
		{
			// Use left mouse button and movement for panning.
			m_MouseX = (int)LOWORD(lParam);
			m_MouseY = (int)HIWORD(lParam);
			m_bMouseDown = true;

			SetCapture(m_Hwnd);
			return true;
		}

	case WM_LBUTTONUP:
		{
			m_bMouseDown = false;
			ReleaseCapture();
			return true;
		}

	case WM_MOUSEMOVE:
		{
			if (m_bMouseDown)
			{
				int xPos = (int)(short)LOWORD(lParam);
				int yPos = (int)(short)HIWORD(lParam);

				float DeltaScrollX = (float)(xPos - m_MouseX);
				float DeltaScrollY = (float)(yPos - m_MouseY);

				if (m_pCapturedCamera == &m_ViewportCamera)
				{
					m_pCapturedCamera->OnMouseMove(PointF{ DeltaScrollX, DeltaScrollY });
				}
				else
				{
					//
					// Move the debug camera relative to the viewport camera's coordinate space.
					//
					float zoom = m_pCapturedCamera->GetZoom() / m_ViewportCamera.GetZoom();
					m_pCapturedCamera->OnMouseMove(PointF{ -DeltaScrollX * zoom, -DeltaScrollY * zoom });
				}

				m_MouseX = xPos;
				m_MouseY = yPos;
			}

			return true;
		}

	//
	// On window resize, adjust the camera perspective to match the new window size.
	//
	case WM_SIZE:
		{
			RectF NewProjection =
			{
				-m_WindowWidth / 2.0f,
				-m_WindowHeight / 2.0f,
				m_WindowWidth / 2.0f,
				m_WindowHeight / 2.0f,
			};
			m_pCapturedCamera->SetOrthographicProjection(NewProjection);
			return true;
		}

	case WM_KEYDOWN:
		{
			if (wParam == GetVirtualKeyFromCharacter('s')) // Toggle 's'tats.
			{
				m_bRenderStats = !m_bRenderStats;
			}
			else if (wParam == GetVirtualKeyFromCharacter('m')) // View 'm'ip coloring.
			{
				m_bDrawMipColors = !m_bDrawMipColors;
			}
			else if (wParam == GetVirtualKeyFromCharacter('v')) // Toggle 'v'sync.
			{
				m_bPresentOnVsync = !m_bPresentOnVsync;
			}
	#if(_DEBUG)
			//
			// In debug builds, allow simulated device removed errors for testing purposes.
			//
			else if (wParam == GetVirtualKeyFromCharacter('r')) // Simulate 'r'emove device.
			{
				m_SimulatedRenderResult = DXGI_ERROR_DEVICE_REMOVED;
				if (m_NewAdapterIndex == 0xFFFFFFFF)
				{
					m_NewAdapterIndex = 0;
				}
				else
				{
					// Toggle adapters 0 and 1.
					m_NewAdapterIndex = !m_NewAdapterIndex;
				}
			}
			else if (wParam == GetVirtualKeyFromCharacter('e')) //  Simulate remove device without adapter toggle.
			{
				m_SimulatedRenderResult = DXGI_ERROR_DEVICE_REMOVED;
			}
	#endif
			else if (wParam == GetVirtualKeyFromCharacter('c')) // Toggle 'c'amera.
			{
				//
				// Toggle captured viewport - switched which viewport the mouse controls.
				//
				if (m_pCapturedCamera == &m_ViewportCamera)
				{
					m_pCapturedCamera = &m_DebugCamera;
				}
				else
				{
					m_pCapturedCamera = &m_ViewportCamera;
				}
			}
			else if (wParam == GetVirtualKeyFromCharacter('d')) // 'D'etach camera.
			{
				//
				// Toggle scene viewport - switches which camera is used as the scene camera
				// which determines prefetching and display behavior.
				//
				if (m_pSceneCamera == &m_ViewportCamera)
				{
					m_pSceneCamera = &m_DebugCamera;
				}
				else
				{
					m_pSceneCamera = &m_ViewportCamera;
					m_pCapturedCamera = &m_ViewportCamera;
				}
			}
			else if (wParam == GetVirtualKeyFromCharacter('f')) // Toggle 'f'ullscreen mode.
			{
				HRESULT hr = S_OK;
				if (m_bFullscreen)
				{
					SetWindowLongPtr(m_Hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
					SetWindowPos(
						m_Hwnd,
						nullptr,
						m_WindowRect.left,
						m_WindowRect.top,
						m_WindowRect.right - m_WindowRect.left,
						m_WindowRect.bottom - m_WindowRect.top,
						0);
				}
				else
				{
					IDXGIOutput* pDXGIOutput;
					hr = m_pDXGISwapChain->GetContainingOutput(&pDXGIOutput);
					if (SUCCEEDED(hr))
					{
						DXGI_OUTPUT_DESC Desc;
						pDXGIOutput->GetDesc(&Desc);

						GetWindowRect(m_Hwnd, &m_WindowRect);
						SetWindowLongPtr(m_Hwnd, GWL_STYLE, WS_POPUP);
						SetWindowPos(
							m_Hwnd,
							HWND_TOP,
							Desc.DesktopCoordinates.left,
							Desc.DesktopCoordinates.top,
							Desc.DesktopCoordinates.right - Desc.DesktopCoordinates.left,
							Desc.DesktopCoordinates.bottom - Desc.DesktopCoordinates.top,
							SWP_FRAMECHANGED | SWP_SHOWWINDOW);

						pDXGIOutput->Release();
					}
				}

				if (SUCCEEDED(hr))
				{
					ShowWindow(m_Hwnd, SW_SHOWNORMAL);
					m_bFullscreen = !m_bFullscreen;
				}

			}
			//
			// Budget overrides in debug builds.
			//
			else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) // '+' - Increase budget.
			{
				UINT64 CurrentOverride = GetLocalBudgetOverride();
				CurrentOverride += _128MB;
				SetLocalBudgetOverride(CurrentOverride);
			}
			else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) // '-' - Decrease budget.
			{
				UINT64 CurrentOverride = GetLocalBudgetOverride();
				if (CurrentOverride > _128MB)
				{
					CurrentOverride -= _128MB;
				}
				else
				{
					CurrentOverride = 0;
				}
				SetLocalBudgetOverride(CurrentOverride);
			}
			return true;
		}
	}
	return false;
}

void D3D12MemoryManagement::CalculateImagePagingData(const RectF* pViewportBounds, const Image* pImage, UINT8* pVisibleMip, UINT8* pPrefetchMip)
{
	float ImageScale = (pImage->Bounds.Right - pImage->Bounds.Left) * m_pSceneCamera->GetZoom();
	UINT8 RequiredMip = (UINT8)CalculateRequiredMipLevel(pImage->pResource, ImageScale);

	//
	// Determine if the resource is visible, or nearby, and if so, calculate the visible
	// or prefetchable mipmap index for this resource. This is used to determine the
	// priority which the paging thread will stream in the resources. Visible mipmaps
	// have a higher priority than prefetched ones, and prefetched mipmaps are higher
	// priority than all others.
	//
	float ScaledPrefetchDistance = PREFETCH_DISTANCE / m_pSceneCamera->GetZoom();
	bool IsVisible = RectIntersects(*pViewportBounds, pImage->Bounds);
	bool IsNearlyVisible = RectNearlyIntersects(*pViewportBounds, pImage->Bounds, ScaledPrefetchDistance);

	UINT8 VisibleMip;
	UINT8 PrefetchMip;

	if (IsVisible)
	{
		VisibleMip = RequiredMip;
		PrefetchMip = IncreaseMipQuality(RequiredMip, 1);
	}
	else if (IsNearlyVisible)
	{
		VisibleMip = UNDEFINED_MIPMAP_INDEX;
		PrefetchMip = RequiredMip;
	}
	else
	{
		VisibleMip = UNDEFINED_MIPMAP_INDEX;
		PrefetchMip = UNDEFINED_MIPMAP_INDEX;
	}

	*pVisibleMip = VisibleMip;
	*pPrefetchMip = PrefetchMip;
}

HRESULT D3D12MemoryManagement::RenderScene(const RectF& ViewportBounds)
{
	RectF SceneBounds = m_pSceneCamera->GenerateViewportBounds();

	for (auto& Img : m_Images)
	{
		Resource* pResource = Img.pResource;

		//
		// Get image visibility and prefetching information.
		//
		UINT8 VisibleMip;
		UINT8 PrefetchMip;
		CalculateImagePagingData(&SceneBounds, &Img, &VisibleMip, &PrefetchMip);

		//
		// If the visibility or prefetch values have changed, notify the paging thread
		// so it can update this resource's priority.
		//
		if (pResource->VisibleMip != VisibleMip || pResource->PrefetchMip != PrefetchMip)
		{
			pResource->VisibleMip = VisibleMip;
			pResource->PrefetchMip = PrefetchMip;
			NotifyPagingWork(pResource);
		}

		//
		// Although visibility information is calculated above using the scene camera, for debug
		// purposes, we may want to render with another camera. We will calculate the real
		// image visibility for rendering purposes using the viewport bounds instead of the scene
		// bounds.
		//
		bool IsVisible = RectIntersects(Img.Bounds, ViewportBounds);
		if (IsVisible)
		{
			static const ColorF DefaultColor = { 1, 1, 1, 1 };

			FillRectangle(&Img.Bounds, &DefaultColor, pResource);

			if (m_bDrawMipColors)
			{
				//
				// Render overlay and border around images to indicate both the visible
				// mipmap, and the mipmap that is resident. This shows the difference
				// between the requested and actual mipmap.
				//
				static const ColorF MipColors[MAX_MIP_COUNT] =
				{
					{ 1.0f, 0.0f, 0.0f, 0.5f }, // Red
					{ 1.0f, 0.5f, 0.0f, 0.5f }, // Orange
					{ 1.0f, 1.0f, 0.0f, 0.5f }, // Yellow
					{ 0.0f, 1.0f, 0.0f, 0.5f }, // Green
					{ 0.0f, 1.0f, 1.0f, 0.5f }, // Aqua
					{ 0.0f, 0.5f, 0.5f, 0.5f }, // LightBlue
					{ 0.0f, 0.0f, 1.0f, 0.5f }, // Blue
					{ 0.5f, 0.0f, 1.0f, 0.5f }, // Purple
					{ 1.0f, 0.0f, 1.0f, 0.5f }, // Magenta
					{ 0.6f, 0.3f, 0.0f, 0.5f }, // Brown
					{ 0.4f, 0.4f, 0.0f, 0.5f }, // Gold
					{ 1.0f, 1.0f, 1.0f, 0.5f }, // White
					{ 0.7f, 0.7f, 0.7f, 0.5f }, // LightGray
					{ 0.3f, 0.3f, 0.3f, 0.5f }, // DarkGray
					{ 0.0f, 0.0f, 0.0f, 0.5f }, // Black
				};

				VisibleMip = pResource->VisibleMip;
				const ColorF* pVisibleColor = &MipColors[VisibleMip];

				FillRectangle(&Img.Bounds, pVisibleColor);

				UINT8 ResidentMip = pResource->MostDetailedMipResident;
				const ColorF* pResidentColor = &MipColors[ResidentMip];

				DrawRectangle(&Img.Bounds, 2.0f, pResidentColor);
			}
		}
	}

	//
	// Render debug viewport.
	//
	{
		ColorF ViewportColor = { 1.0f, 0.0f, 0.0f, 0.25f };
		ColorF ViewportColorNoPrefetch = { 1.0f, 0.0f, 0.0f, 1.0f };

		const ColorF* pColor = &ViewportColorNoPrefetch;
		float Thickness = 2 / m_ViewportCamera.GetZoom();

		float ScaledPrefetchDistance = PREFETCH_DISTANCE / m_pSceneCamera->GetZoom();
		Thickness = ScaledPrefetchDistance;
		pColor = &ViewportColor;

		DrawRectangle(&SceneBounds, Thickness, pColor);
	}

	return S_OK;
}

//
// Renders the user interface using D2D on top of our D3D12 content.
//
HRESULT D3D12MemoryManagement::RenderUI()
{
	HRESULT hr = S_OK;

	UINT FrameIndex = m_pDXGISwapChain->GetCurrentBackBufferIndex();

	//
	// Acquire the wrapped back buffer object. When we release the wrapped object, it will
	// be placed in a presentable state, and we can call Present directly.
	//
	m_p11On12Device->AcquireWrappedResources(&m_pWrappedBackBuffers[FrameIndex], 1);

	m_pD2DContext->SetTarget(m_pD2DRenderTargets[FrameIndex]);
	m_pD2DContext->BeginDraw();
	m_pD2DContext->SetTransform(D2D1::Matrix3x2F::Identity());

	if (m_bRenderStats)
	{
		//
		// Render statistical information, such as the framerate, memory budget graph,
		// and glitch count.
		//
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		RenderMemoryGraph();

		//
		// Frame statistics (FPS, CPU time, etc).
		//
		{
			D2D1_RECT_F TextRect;

			TextRect.left = 8;
			TextRect.top = 8;
			TextRect.right = 256;
			TextRect.bottom = 256;

			float StatTimeBetweenFrames = AverageStatistics(m_StatTimeBetweenFrames, STATISTIC_COUNT);
			float StatRenderScene = AverageStatistics(m_StatRenderScene, STATISTIC_COUNT);
			float StatRenderUI = AverageStatistics(m_StatRenderUI, STATISTIC_COUNT);

			m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			wchar_t FPSString[128];
			swprintf_s(
				FPSString,
				_TRUNCATE,
				L"FPS: %d (%.2fms)\n"
				L"Glitch Count: %d\n"
				L"\n"
				L"RenderScene: %.2f ms\n"
				L"RenderUI: %.2f ms",
				(UINT)(1.0f / StatTimeBetweenFrames),
				StatTimeBetweenFrames * 1000.0f,
				GetGlitchCount(),
				StatRenderScene * 1000.0f,
				StatRenderUI * 1000.0f);

			m_pD2DContext->DrawTextW(
				FPSString,
				(UINT)wcslen(FPSString),
				m_pTextFormat,
				&TextRect,
				m_pTextBrush);
		}
	}

	hr = m_pD2DContext->EndDraw();
	if (FAILED(hr))
	{
		LOG_WARNING("D2D EndDraw failed, hr=0x%.8x", hr);
	}

	//
	// Unreference the render target after rendering completes.
	//
	m_pD2DContext->SetTarget(nullptr);

	m_p11On12Device->ReleaseWrappedResources(&m_pWrappedBackBuffers[FrameIndex], 1);

	//
	// Flush D2D based content on the D3D11On12 device to synchronize with D3D12 content.
	//
	m_p11Context->Flush();

	return hr;
}

void D3D12MemoryManagement::RenderMemoryGraph()
{
	static const D2D1_COLOR_F BackgroundColor = { 0.1f, 0.1f, 0.1f, 0.85f };
	static const D2D1_COLOR_F BorderColor = { 0.75f, 0.75f, 0.75f, 0.75f };
	static const D2D1_COLOR_F NotchColor = { 0.75f, 0.75f, 0.75f, 0.4f };
	static const D2D1_COLOR_F UsageColor = { 0.5f, 0.5f, 0.0f, 0.75f };
	static const D2D1_COLOR_F CurrentUsageColor = { 0.8f, 0.8f, 0.0f, 1.0f };
	static const D2D1_COLOR_F CurrentBudgetColor = { 0.8f, 0.0f, 0.0f, 1.0f };

	HRESULT hr;

	DXGI_QUERY_VIDEO_MEMORY_INFO LocalInfo = GetLocalVideoMemoryInfo();

	UINT64 Tick = GetTickCount64();
	if (Tick - m_LastGraphTick > 1000 / GRAPH_SEGMENTS * 10)
	{
		// Record data point.
		m_GraphPoints[m_CurrentGraphPoint % NUM_GRAPH_POINTS] = LocalInfo.CurrentUsage;
		++m_CurrentGraphPoint;
		m_LastGraphTick = Tick;
	}

	//
	// Get largest sample in point set for an upper bound.
	//
	UINT64 LargestSeenValue = 0;
	for (UINT i = 0; i < NUM_GRAPH_POINTS; ++i)
	{
		if (m_GraphPoints[i] > LargestSeenValue)
		{
			LargestSeenValue = m_GraphPoints[i];
		}
	}
	if (LargestSeenValue == 0)
	{
		return;
	}

	//
	// Calculate graph scale values.
	//
	UINT LargestSeenValueMB = (UINT)(LargestSeenValue / 1024 / 1024);

	//
	// Graph is scaled by powers of 2 scaled by 100, e.g. 100, 200, 400, 800, ...
	// Use the power of two value to scale the graph accordingly.
	//
	DWORD ScaleIndex;
	DWORD Result = BitScanReverse(&ScaleIndex, LargestSeenValueMB / 100);
	if (Result == 0)
	{
		ScaleIndex = 0;
	}
	else
	{
		++ScaleIndex;
	}

	//
	// Graph only goes up GRAPH_HEIGHT_RATIO % to the top to leave a little bit of room
	// at the top of the graph. Purely aesthetics.
	//
	UINT GraphSizeMB = (1 << ScaleIndex) * 100;
	float GraphHeightPadded = GRAPH_HEIGHT * GRAPH_HEIGHT_RATIO / 100;
	float GraphPaddingTop = GRAPH_HEIGHT - GraphHeightPadded;

	//
	// Calculate the graph rectangle bounds.
	//
	RectF GraphDest;

	GraphDest.Right = m_WindowWidth - GRAPH_PADDING;
	GraphDest.Left = GraphDest.Right - GRAPH_WIDTH;
	GraphDest.Top = GRAPH_PADDING;
	GraphDest.Bottom = GraphDest.Top + GRAPH_HEIGHT;

	//
	// Try to create the brush and path geometry for the graph. If either fails, we cannot render the graph.
	//
	ComPtr<ID2D1SolidColorBrush> pBrush;
	ComPtr<ID2D1PathGeometry> pPathGeometry;

	hr = m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create solid color brush for rendering UI, hr=0x.8x", hr);
		return;
	}

	hr = GenerateMemoryGraphGeometry(GraphDest, GraphSizeMB, &pPathGeometry);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to generate path geometry for memory graph, hr=0x.8x", hr);
		return;
	}

	//
	// Render memory graph border and background.
	//
	{
		pBrush->SetColor(BackgroundColor);
		m_pD2DContext->FillRectangle((D2D1_RECT_F&)GraphDest, pBrush.Get());

		pBrush->SetColor(BorderColor);
		m_pD2DContext->DrawRectangle((D2D1_RECT_F&)GraphDest, pBrush.Get(), 2);
	}

	//
	// Render the path geometry for the graph.
	//
	{
		pBrush->SetColor(UsageColor);
		m_pD2DContext->FillGeometry(pPathGeometry.Get(), pBrush.Get());
	}

	//
	// Render the notches on the graph for reference values.
	//
	{
		pBrush->SetColor(NotchColor);
		UINT SizePerNotch = GraphSizeMB / GRAPH_NOTCH_COUNT;

		for (int i = 0; i < GRAPH_NOTCH_COUNT + 1; ++i)
		{
			wchar_t Text[32];
			swprintf_s(Text, _TRUNCATE, L"%dMB", SizePerNotch * i);

			D2D1_RECT_F TextRect;

			TextRect.left = GraphDest.Left - 100;
			TextRect.top = GraphDest.Top + GraphPaddingTop + (GRAPH_NOTCH_COUNT - i) * (GraphHeightPadded / GRAPH_NOTCH_COUNT) - 10;
			TextRect.right = GraphDest.Left - 5;
			TextRect.bottom = TextRect.top + 20;

			m_pD2DContext->DrawTextW(
				Text,
				(UINT)wcslen(Text),
				m_pTextFormat,
				&TextRect,
				m_pTextBrush);

			//
			// Label 0MB, but do not draw the line.
			//
			if (i > 0)
			{
				D2D1_POINT_2F Point0 = { GraphDest.Left, TextRect.top + 10 };
				D2D1_POINT_2F Point1 = { GraphDest.Right, TextRect.top + 10 };
				m_pD2DContext->DrawLine(Point0, Point1, pBrush.Get(), 1);
			}
		}
	}

	//
	// Draw current usage/budget line and value.
	//
	{
		//
		// Usage
		//
		{
			pBrush->SetColor(CurrentUsageColor);
			float Top = GraphDest.Bottom - (LocalInfo.CurrentUsage / 1024 / 1024) / (float)GraphSizeMB * GraphHeightPadded;

			D2D1_POINT_2F Point0 = { GraphDest.Left, Top };
			D2D1_POINT_2F Point1 = { GraphDest.Right, Top };
			m_pD2DContext->DrawLine(Point0, Point1, pBrush.Get(), 1);

			wchar_t Text[32];
			swprintf_s(Text, L"%dMB", (UINT)(LocalInfo.CurrentUsage / 1024 / 1024));

			D2D1_RECT_F TextRect;

			TextRect.left = Point0.x;
			TextRect.top = Top;
			TextRect.right = Point1.x - 5;
			TextRect.bottom = Top + 10;

			m_pD2DContext->DrawTextW(
				Text,
				(UINT)wcslen(Text),
				m_pTextFormat,
				&TextRect,
				m_pTextBrush);
		}

		//
		// Budget
		//
		{
			pBrush->SetColor(CurrentBudgetColor);
			float Top = max(GraphDest.Bottom - (LocalInfo.Budget / 1024 / 1024) / (float)GraphSizeMB * GraphHeightPadded, GraphDest.Top);

			D2D1_POINT_2F Point0 = { GraphDest.Left, Top };
			D2D1_POINT_2F Point1 = { GraphDest.Right, Top };
			m_pD2DContext->DrawLine(Point0, Point1, pBrush.Get(), 1);

			wchar_t Text[32];
			swprintf_s(Text, L"%dMB", (UINT)(LocalInfo.Budget / 1024 / 1024));

			D2D1_RECT_F TextRect;

			TextRect.left = Point0.x;
			TextRect.top = Top - 10;
			TextRect.right = Point1.x - 5;
			TextRect.bottom = Top;

			m_pD2DContext->DrawTextW(
				Text,
				(UINT)wcslen(Text),
				m_pTextFormat,
				&TextRect,
				m_pTextBrush);
		}

	}
}

//
// Generates new graph geometry, which may change every frame.
//
HRESULT D3D12MemoryManagement::GenerateMemoryGraphGeometry(const RectF& Bounds, UINT GraphSizeMB, ID2D1PathGeometry** ppPathGeometry)
{
	HRESULT hr;

	ID2D1PathGeometry* pPathGeometry = nullptr;
	ID2D1GeometrySink* pGeometrySink = nullptr;

	hr = m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to create path geometry, hr=0x%.8x");
		goto cleanup;
	}

	hr = pPathGeometry->Open(&pGeometrySink);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to open path geometry sink, hr=0x%.8x");
		goto cleanup;
	}

	{
		float GraphHeightPadded = GRAPH_HEIGHT * GRAPH_HEIGHT_RATIO / 100;
		float XStep = GRAPH_WIDTH / GRAPH_SEGMENTS;

		D2D1_POINT_2F Point;
		Point.x = Bounds.Left;
		Point.y = Bounds.Bottom;

		pGeometrySink->BeginFigure(Point, D2D1_FIGURE_BEGIN_FILLED);

		for (UINT i = 0; i < NUM_GRAPH_POINTS; ++i)
		{
			UINT32 GraphPoint = (i + m_CurrentGraphPoint) % NUM_GRAPH_POINTS;

			Point.y = Bounds.Bottom - (m_GraphPoints[GraphPoint] / 1024 / 1024) / (float)GraphSizeMB * GraphHeightPadded;
			pGeometrySink->AddLine(Point);
			Point.x += XStep;
		}

		Point.x = Bounds.Right;
		Point.y = Bounds.Bottom;
		pGeometrySink->AddLine(Point);

		pGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
	}

	hr = pGeometrySink->Close();
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to close path geometry sink, hr=0x%.8x", hr);
		goto cleanup;
	}

	pGeometrySink->Release();
	*ppPathGeometry = pPathGeometry;

	return S_OK;

cleanup:
	SafeRelease(pGeometrySink);
	SafeRelease(pPathGeometry);

	return hr;
}
