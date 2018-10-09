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

using namespace DirectX;
using namespace GameCore;

Camera* Camera::mCamera = nullptr;

Camera::Camera()
{
    Reset();
    mCamera = this;
}

Camera::~Camera()
{
    mCamera = nullptr;
}

Camera* Camera::get()
{
    return mCamera;
}

void Camera::GetViewProj(XMMATRIX *view, XMMATRIX *proj, float fovInDegrees, UINT screenWidth, UINT screenHeight, bool rhCoords)
{
    float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    float fovAngleY = fovInDegrees * XM_PI / 180.0f;

    if (aspectRatio < 1.0f)
    {
        fovAngleY /= aspectRatio;
    }

	if (rhCoords)
	{
		*view = XMMatrixLookAtRH(m_eye, m_at, m_up);
		*proj = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, ZMin, ZMax);
	}
	else
	{
		*view = XMMatrixLookAtLH(m_eye, m_at, m_up);
		*proj = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, ZMin, ZMax);
	}
}

void Camera::RotateAroundYAxis(float angleRad)
{
    XMMATRIX rotation = XMMatrixRotationY(angleRad);

    m_eye = m_at + XMVector3TransformCoord(m_eye - m_at, rotation);
    m_up = XMVector3TransformCoord(m_up, rotation);
}

void Camera::RotateYaw(float angleRad)
{
    XMMATRIX rotation = XMMatrixRotationAxis(m_up, angleRad);

    m_at = m_eye + XMVector3TransformCoord(m_at - m_eye, rotation);
}

void Camera::RotatePitch(float angleRad)
{
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(m_at - m_eye, m_up));
	XMMATRIX rotation = XMMatrixRotationAxis(right, angleRad);

	m_at = m_eye + XMVector3TransformCoord(m_at - m_eye, rotation);
	m_up = XMVector3TransformCoord(m_up, rotation);
}

void Camera::TranslateForward(float translation)
{
	XMVECTOR forwardVec = Forward();
	m_eye += translation * forwardVec;
	m_at += translation * forwardVec;
}

void Camera::TranslateRight(float translation)
{
    XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(m_at - m_eye, m_up));
	m_eye += translation * rightVec;
	m_at += translation * rightVec;
}

void Camera::TranslateUp(float translation)
{
	m_eye += translation * m_up;
	m_at += translation * m_up;
}

void Camera::TranslateRightUpForward(float right, float up, float forward)
{
	XMVECTOR forwardVec = Forward();
	XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(-forwardVec, m_up));
	XMVECTOR translationVec = right * rightVec + up * m_up + forward * forwardVec;
	m_eye += XMVectorSetW(translationVec, 1);
	m_at += XMVectorSetW(translationVec, 1);
}

void Camera::SetViewMatrix(const XMMATRIX& transform)
{
	m_up = XMVector4Transform(XMVectorSet(0,1,0,0), transform);
	float forwardLength = XMVectorGetX(XMVector3Length(Forward()));
	m_eye = XMVectorSetW(XMVector3TransformCoord(XMVectorZero(), transform), 1);
	m_at = XMVector4Transform(XMVectorSet(0,forwardLength,0,0), transform);
}

void Camera::Reset()
{
	// ToDo
    m_eye = XMVectorSet(0.0f, 8.0f, -30.0f, 0.0f);
    m_at = XMVectorSet(0.0f, 8.0f, 0.0f, 0.0f);
    m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
}

void Camera::Set(const XMVECTOR& eye, const XMVECTOR& at, const XMVECTOR& up)
{
    m_eye = XMVectorSetW(eye, 1);
	m_at = XMVectorSetW(at, 0);
	m_up = XMVectorSetW(up, 0);
}
