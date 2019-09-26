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

#include "../stdafx.h"
#include "Camera.h"

using namespace DirectX;
using namespace GameCore;

Camera* Camera::s_camera = nullptr;

Camera::Camera()
{
    s_camera = this;
}

Camera::~Camera()
{
    s_camera = nullptr;
}

void Camera::GetProj(XMMATRIX *proj, UINT screenWidth, UINT screenHeight, bool rhCoords) const
{
	float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	float fovAngleY = XMConvertToRadians(fov);

	if (aspectRatio < 1.0f)
	{
		fovAngleY /= aspectRatio;
	}

	if (rhCoords)
	{
		*proj = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, ZMin, ZMax);
	}
	else
	{
		*proj = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, ZMin, ZMax);
	}
}

void Camera::GetViewProj(XMMATRIX *view, XMMATRIX *proj, UINT screenWidth, UINT screenHeight, bool rhCoords) const
{
	if (rhCoords)
	{
		*view = XMMatrixLookAtRH(m_eye, m_at, m_up);
	}
	else
	{
		*view = XMMatrixLookAtLH(m_eye, m_at, m_up);
	}

	GetProj(proj, screenWidth, screenHeight, rhCoords);
}

void Camera::RotateAroundYAxis(float angleRad)
{
    XMMATRIX rotation = XMMatrixRotationY(angleRad);

    m_eye = m_at + XMVector3TransformCoord(m_eye - m_at, rotation);
    m_up = XMVector3TransformNormal(m_up, rotation);
}

void Camera::RotateYaw(float angleRad)
{
    XMMATRIX rotation = XMMatrixRotationAxis(m_up, angleRad);

    m_at = m_eye + XMVector3TransformNormal(m_at - m_eye, rotation);
}

void Camera::RotatePitch(float angleRad)
{
	XMVECTOR right = XMVector3Cross(-(m_at - m_eye), m_up);
	XMMATRIX rotation = XMMatrixRotationAxis(right, angleRad);

	m_at = m_eye + XMVector3TransformNormal(m_at - m_eye, rotation);
	m_up = XMVector3TransformNormal(m_up, rotation);
}

void Camera::TranslateForward(float delta)
{
	XMVECTOR forward = Forward();
	m_eye += delta * forward;
	m_at += delta * forward;
}

void Camera::TranslateRight(float delta)
{
	XMVECTOR forward = Forward();
    XMVECTOR right = XMVector3Cross(-forward, m_up);
	m_eye += delta * right;
	m_at += delta * right;
}

void Camera::TranslateUp(float delta)
{
	m_eye += delta * m_up;
	m_at += delta * m_up;
}

void Camera::TranslateRightUpForward(float deltaRight, float deltaUp, float deltaForward)
{
	XMVECTOR forward = Forward();
	XMVECTOR right = XMVector3Cross(-forward, m_up);
	XMVECTOR translationVec = deltaRight * right + deltaUp * m_up + deltaForward * forward;
	m_eye += translationVec;
	m_at += translationVec;
}

void Camera::Set(const XMVECTOR& eye, const XMVECTOR& at, const XMVECTOR& up)
{
    m_eye = XMVectorSetW(eye, 1);
	m_at = XMVectorSetW(at, 1);
	m_up = XMVectorSetW(up, 0);
}
