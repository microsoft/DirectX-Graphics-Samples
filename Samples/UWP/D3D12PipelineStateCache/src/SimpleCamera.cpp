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
#include "SimpleCamera.h"

SimpleCamera::SimpleCamera():
	m_initialPosition(0, 0, 0),
	m_position(m_initialPosition),
	m_yaw(XM_PI),
	m_pitch(0.0f),
	m_lookDirection(0, 0, -1),
	m_upDirection(0, 1, 0),
	m_moveSpeed(20.0f),
	m_turnSpeed(XM_PIDIV2),
	m_keysPressed{}
{
}

void SimpleCamera::Init(XMFLOAT3 position)
{
	m_initialPosition = position;
	Reset();
}

void SimpleCamera::SetMoveSpeed(float unitsPerSecond)
{
	m_moveSpeed = unitsPerSecond;
}

void SimpleCamera::SetTurnSpeed(float radiansPerSecond)
{
	m_turnSpeed = radiansPerSecond;
}

void SimpleCamera::Reset()
{
	m_position = m_initialPosition;
	m_yaw = XM_PI;
	m_pitch = 0.0f;
	m_lookDirection = { 0, 0, -1 };
}

void SimpleCamera::Update(float elapsedSeconds)
{
	// Calculate the move vector in camera space.
	XMFLOAT3 move(0, 0, 0);

	if (m_keysPressed.a)
		move.x -= 1.0f;
	if (m_keysPressed.d)
		move.x += 1.0f;
	if (m_keysPressed.w)
		move.z -= 1.0f;
	if (m_keysPressed.s)
		move.z += 1.0f;

	if (fabs(move.x) > 0.1f && fabs(move.z) > 0.1f)
	{
		XMVECTOR vector = XMVector3Normalize(XMLoadFloat3(&move));
		move.x = XMVectorGetX(vector);
		move.z = XMVectorGetZ(vector);
	}

	float moveInterval = m_moveSpeed * elapsedSeconds;
	float rotateInterval = m_turnSpeed * elapsedSeconds;

	if (m_keysPressed.left)
		m_yaw += rotateInterval;
	if (m_keysPressed.right)
		m_yaw -= rotateInterval;
	if (m_keysPressed.up)
		m_pitch += rotateInterval;
	if (m_keysPressed.down)
		m_pitch -= rotateInterval;

	// Prevent looking too far up or down.
	m_pitch = min(m_pitch, XM_PIDIV4);
	m_pitch = max(-XM_PIDIV4, m_pitch);

	// Move the camera in model space.
	float x = move.x * -cosf(m_yaw) - move.z * sinf(m_yaw);
	float z = move.x * sinf(m_yaw) - move.z * cosf(m_yaw);
	m_position.x += x * moveInterval;
	m_position.z += z * moveInterval;

	// Determine the look direction.
	float r = cosf(m_pitch);
	m_lookDirection.x = r * sinf(m_yaw);
	m_lookDirection.y = sinf(m_pitch);
	m_lookDirection.z = r * cosf(m_yaw);
}

XMMATRIX SimpleCamera::GetViewMatrix()
{
	return XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
}

XMMATRIX SimpleCamera::GetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
}

void SimpleCamera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keysPressed.w = true;
		break;
	case 'A':
		m_keysPressed.a = true;
		break;
	case 'S':
		m_keysPressed.s = true;
		break;
	case 'D':
		m_keysPressed.d = true;
		break;
	case VK_LEFT:
		m_keysPressed.left = true;
		break;
	case VK_RIGHT:
		m_keysPressed.right = true;
		break;
	case VK_UP:
		m_keysPressed.up = true;
		break;
	case VK_DOWN:
		m_keysPressed.down = true;
		break;
	case VK_ESCAPE:
		Reset();
		break;
	}
}

void SimpleCamera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keysPressed.w = false;
		break;
	case 'A':
		m_keysPressed.a = false;
		break;
	case 'S':
		m_keysPressed.s = false;
		break;
	case 'D':
		m_keysPressed.d = false;
		break;
	case VK_LEFT:
		m_keysPressed.left = false;
		break;
	case VK_RIGHT:
		m_keysPressed.right = false;
		break;
	case VK_UP:
		m_keysPressed.up = false;
		break;
	case VK_DOWN:
		m_keysPressed.down = false;
		break;
	}
}
