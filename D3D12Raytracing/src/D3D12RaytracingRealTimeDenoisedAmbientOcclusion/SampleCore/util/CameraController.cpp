//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//


#include "../stdafx.h"
#include "CameraController.h"
#include "GameInput.h"

using namespace DirectX;
using namespace GameCore;

CameraController::CameraController(Camera& camera) : m_camera(camera)
{
    m_HorizontalLookSensitivity = 2.0f;
    m_VerticalLookSensitivity = 2.0f;
	m_MoveSpeed = 5.0f;
	m_StrafeSpeed = 5.0f;
    m_MoveSpeed *= 20;
	m_StrafeSpeed *= 50;
	m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = -1.0f;

    m_FineMovement = false;
    m_FineRotation = false;
    m_Momentum = false;

    m_LastYaw = 0.0f;
    m_LastPitch = 0.0f;
    m_LastForward = 0.0f;
    m_LastStrafe = 0.0f;
    m_LastAscent = 0.0f;

	m_boundaryMin = -XMVectorSplatInfinity();
	m_boundaryMax = XMVectorSplatInfinity();
}

void CameraController::SetBoundaries(const XMVECTOR& _min, XMVECTOR& _max)
{
	m_boundaryMin = _min;
	m_boundaryMax = _max;
}

// Returns true if camera was modified.
bool CameraController::Update(float deltaTime)
{
    Camera prevCameraState = m_camera;

	float timeScale = deltaTime;

    if (GameInput::IsFirstPressed(GameInput::kLThumbClick) || GameInput::IsFirstPressed(GameInput::kKey_lshift))
        m_FineMovement = !m_FineMovement;

    if (GameInput::IsFirstPressed(GameInput::kRThumbClick))
        m_FineRotation = !m_FineRotation;

    float speedScale = (m_FineMovement ? 0.1f : 1.0f) * timeScale;
    float panScale = (m_FineRotation ? 0.5f : 1.0f) * timeScale;

    float yaw = GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogRightStickX) * m_HorizontalLookSensitivity * panScale;
    float pitch = GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogRightStickY) * m_VerticalLookSensitivity * panScale;
    float forward = m_MoveSpeed * speedScale * (
        GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogLeftStickY) +
        (GameInput::IsPressed(GameInput::kKey_w) ? 1.f : 0.0f) +
        (GameInput::IsPressed(GameInput::kKey_s) ? -1.f: 0.0f)
      );
    float strafe = m_StrafeSpeed * speedScale * (
        GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogLeftStickX) +
        (GameInput::IsPressed(GameInput::kKey_d) ? 1.f: 0.0f) +
        (GameInput::IsPressed(GameInput::kKey_a) ? -1.f: 0.0f)
      );
    float ascent = m_StrafeSpeed * speedScale * (
        GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogRightTrigger) -
        GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogLeftTrigger) +
        (GameInput::IsPressed(GameInput::kKey_e) ? 1.f: 0.0f) +
        (GameInput::IsPressed(GameInput::kKey_q) ? -1.f: 0.0f)
      );

    if (m_Momentum)
    {
        ApplyMomentum(m_LastYaw, yaw, deltaTime);
        ApplyMomentum(m_LastPitch, pitch, deltaTime);
        ApplyMomentum(m_LastForward, forward, deltaTime);
        ApplyMomentum(m_LastStrafe, strafe, deltaTime);
        ApplyMomentum(m_LastAscent, ascent, deltaTime);
    }

    // Don't apply momentum to mouse inputs.
	if (GameInput::IsPressed(GameInput::kMouse0))
	{
		yaw += GameInput::GetAnalogInput(GameInput::kAnalogMouseX) * m_MouseSensitivityX;
		pitch += GameInput::GetAnalogInput(GameInput::kAnalogMouseY) * m_MouseSensitivityY;
	}
 	else if (GameInput::IsPressed(GameInput::kMouse1))
	{
		yaw += -1 * GameInput::GetAnalogInput(GameInput::kAnalogMouseX) * m_MouseSensitivityX;
    	pitch += -1 * GameInput::GetAnalogInput(GameInput::kAnalogMouseY) * m_MouseSensitivityY;
	}

	m_camera.RotateAroundYAxis(yaw);
	m_camera.RotatePitch(pitch);
	m_camera.TranslateRightUpForward(strafe, ascent, forward);

	// Confine camera within a boundary.
	{
		XMVECTOR eye = m_camera.Eye();
		XMVECTOR eyeToAt = m_camera.At() - eye;
		eye = XMVectorMin(eye, m_boundaryMax);
		eye = XMVectorMax(eye, m_boundaryMin);
		m_camera.Set(eye, eye + eyeToAt, m_camera.Up());

#if PRINT_OUT_CAMERA_CONFIG
        std::wstringstream wstr;

        wstr << L"camera.position.eye = { " << XMVectorGetX(eye) << L"f, " << XMVectorGetY(eye) << L"f, " << XMVectorGetZ(eye) << L"f, 1};\n";
        wstr << L"camera.position.at = { " << XMVectorGetX(m_camera.At()) << L"f, " << XMVectorGetY(m_camera.At()) << L"f, " << XMVectorGetZ(m_camera.At()) << L"f, 1};\n";
        wstr << L"camera.position.up = { " << XMVectorGetX(m_camera.Up()) << L"f, " << XMVectorGetY(m_camera.Up()) << L"f, " << XMVectorGetZ(m_camera.Up()) << L"f, 0};\n";

        OutputDebugStringW(wstr.str().c_str());
#endif
	}

    return memcmp(&prevCameraState, &m_camera, sizeof(m_camera)) != 0;
}

void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
    float blendedValue;
    if (fabsf(newValue) > fabsf(oldValue))
        blendedValue = lerp(newValue, oldValue, powf(0.6f, deltaTime * 60.0f));
    else
        blendedValue = lerp(newValue, oldValue, powf(0.8f, deltaTime * 60.0f));
    oldValue = blendedValue;
    newValue = blendedValue;
}
