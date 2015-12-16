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
// The number of segments in the graph, representing the update frequency. The number
// of graph points should be one greater than the number of segments, accounting for
// the fact that each segment has two sides.
//
// e.g. 'GRAPH_SEGMENTS' represents the number of gaps, where 'NUM_GRAPH_POINTS'
// represents the number of pipes (edges) of the segments, with shared edges combined:
//
//     1       2       3       4       5       6       7       8
// 1       2       3       4       5       6       7       8       9
// |       |       |       |       |       |       |       |       |
// |       |       |       |       |       |       |       |       |
//
#define GRAPH_SEGMENTS 64
#define NUM_GRAPH_POINTS (GRAPH_SEGMENTS + 1)

//
// Houses the resource information and the visual bounds for each loaded asset.
//
struct Image
{
	RectF Bounds;
	Resource* pResource;
};

class D3D12MemoryManagement : public DX12Framework
{
private:
	std::vector<Image> m_Images;

	Camera m_ViewportCamera;
	Camera m_DebugCamera;
	Camera* m_pCapturedCamera;
	Camera* m_pSceneCamera;

	int m_MouseX = 0;
	int m_MouseY = 0;
	bool m_bMouseDown = false;

	UINT64 m_LastGraphTick = 0;
	bool m_bRenderStats = false;
	UINT32 m_CurrentGraphPoint = 0;
	UINT64 m_GraphPoints[NUM_GRAPH_POINTS];

	bool m_bDrawMipColors = false;
	bool m_bSimulateDeviceRemoved = false;
	bool m_bFullscreen = false;
	RECT m_WindowRect;

	//
	// D2D UI rendering
	//
	ID2D1SolidColorBrush* m_pTextBrush = nullptr;
	IDWriteTextFormat* m_pTextFormat = nullptr;

protected:
	virtual HRESULT CreateDeviceIndependentState() override;
	virtual HRESULT CreateDeviceDependentState() override;
	virtual void DestroyDeviceIndependentState() override;
	virtual void DestroyDeviceDependentState() override;

	virtual HRESULT LoadAssets();
	virtual HRESULT RenderScene(const RectF& ViewportBounds);
	virtual HRESULT RenderUI();
	virtual bool HandleMessage(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	HRESULT GenerateMemoryGraphGeometry(const RectF& Bounds, UINT GraphSizeMB, ID2D1PathGeometry** ppPathGeometry);
	void RenderMemoryGraph();

	void CalculateImagePagingData(
		const RectF* pViewportBounds,
		const Image* pImage,
		UINT8* pVisibleMip,
		UINT8* pPrefetchMip);

public:
	D3D12MemoryManagement();
	virtual ~D3D12MemoryManagement();
};
