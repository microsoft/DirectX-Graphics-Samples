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

using namespace DirectX;

class Camera
{
public:
	Camera();
	void Initialize(PointF position, float zoom);

	inline void SetOrthographicProjection(const RectF& Rect)
	{
		m_projectionRect = Rect;
	}

	inline float GetZoom() const
	{
		return m_zoom;
	}

	XMMATRIX GetViewProjectionMatrix() const;
	RectF GenerateViewportBounds() const;

	void OnMouseMove(PointF delta);
	void OnMouseWheel(short delta);

private:
	PointF m_position;
	float m_zoom;
	RectF m_projectionRect;

	bool m_mouseDown;
};
