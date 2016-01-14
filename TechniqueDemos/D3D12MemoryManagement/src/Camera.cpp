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
#include "Camera.h"

Camera::Camera() :
	m_position(),
	m_zoom(),
	m_projectionRect()
{
}

void Camera::Initialize(PointF position, float zoom)
{
	m_projectionRect.Left = -640.0f;
	m_projectionRect.Right = 640.0f;
	m_projectionRect.Top = -360.0f;
	m_projectionRect.Bottom = 360.0f;

	m_position = position;
	m_zoom = zoom;
}

XMMATRIX Camera::GetViewProjectionMatrix() const
{
	XMMATRIX Projection = XMMatrixOrthographicOffCenterLH(
		m_projectionRect.Left / m_zoom,
		m_projectionRect.Right / m_zoom,
		m_projectionRect.Bottom / m_zoom,
		m_projectionRect.Top / m_zoom,
		0.1f,
		10.0f);

	XMVECTOR Eye = XMVectorSet(m_position.X, m_position.Y, -1.0f, 0.0f);
	XMVECTOR LookAt = XMVectorSet(m_position.X, m_position.Y, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX View = XMMatrixLookAtLH(Eye, LookAt, Up);

	return View * Projection;
}

RectF Camera::GenerateViewportBounds() const
{
	RectF ViewportBounds = m_projectionRect;
	ViewportBounds.Left = ViewportBounds.Left / m_zoom + m_position.X;
	ViewportBounds.Right = ViewportBounds.Right / m_zoom + m_position.X;
	ViewportBounds.Top = ViewportBounds.Top / m_zoom + m_position.Y;
	ViewportBounds.Bottom = ViewportBounds.Bottom / m_zoom + m_position.Y;

	return ViewportBounds;
}

void Camera::OnMouseMove(PointF delta)
{
	m_position.X -= delta.X / m_zoom;
	m_position.Y -= delta.Y / m_zoom;
}

void Camera::OnMouseWheel(short delta)
{
	if (abs(delta) >= WHEEL_DELTA)
	{
		float newZoom = m_zoom;
		float deltaZoom = 1.0f - ((float)abs(delta) / WHEEL_DELTA / 10.0f);

		if (delta < 0)
		{
			newZoom *= deltaZoom;
		}
		else
		{
			newZoom /= deltaZoom;
		}

		if (newZoom < 0.1f)
		{
			newZoom = 0.1f;
		}
		if (newZoom > 100.0f)
		{
			newZoom = 100.0f;
		}

		m_zoom = newZoom;
	}
}
