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

CameraController::CameraController( Camera& camera) : m_TargetCamera( camera )
{
    m_HorizontalLookSensitivity = 2.0f;
    m_VerticalLookSensitivity = 2.0f;
    m_MoveSpeed = 1000.0f;
    m_StrafeSpeed = 1000.0f;
    m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = -1.0f;

    m_FineMovement = false;
    m_FineRotation = false;
    m_Momentum = true;

    m_LastYaw = 0.0f;
    m_LastPitch = 0.0f;
    m_LastForward = 0.0f;
    m_LastStrafe = 0.0f;
    m_LastAscent = 0.0f;
}

void CameraController::Update( float deltaTime )
{
    (deltaTime);

	float timeScale = 1.0f;

    if (GameInput::IsFirstPressed(GameInput::kLThumbClick) || GameInput::IsFirstPressed(GameInput::kKey_lshift))
        m_FineMovement = !m_FineMovement;

    if (GameInput::IsFirstPressed(GameInput::kRThumbClick))
        m_FineRotation = !m_FineRotation;

    float speedScale = (m_FineMovement ? 0.1f : 1.0f) * timeScale;
    float panScale = (m_FineRotation ? 0.5f : 1.0f) * timeScale;

    float yaw = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogRightStickX ) * m_HorizontalLookSensitivity * panScale;
    float pitch = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogRightStickY ) * m_VerticalLookSensitivity * panScale;
    float forward = m_MoveSpeed * speedScale * (
        GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogLeftStickY ) +
        (GameInput::IsPressed( GameInput::kKey_w ) ? deltaTime : 0.0f) +
        (GameInput::IsPressed( GameInput::kKey_s ) ? -deltaTime : 0.0f)
        );
    float strafe = m_StrafeSpeed * speedScale * (
        GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogLeftStickX  ) +
        (GameInput::IsPressed( GameInput::kKey_d ) ? deltaTime : 0.0f) +
        (GameInput::IsPressed( GameInput::kKey_a ) ? -deltaTime : 0.0f)
        );
    float ascent = m_StrafeSpeed * speedScale * (
        GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogRightTrigger ) -
        GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogLeftTrigger ) +
        (GameInput::IsPressed( GameInput::kKey_e ) ? deltaTime : 0.0f) +
        (GameInput::IsPressed( GameInput::kKey_q ) ? -deltaTime : 0.0f)
        );

    if (m_Momentum)
    {
        ApplyMomentum(m_LastYaw, yaw, deltaTime);
        ApplyMomentum(m_LastPitch, pitch, deltaTime);
        ApplyMomentum(m_LastForward, forward, deltaTime);
        ApplyMomentum(m_LastStrafe, strafe, deltaTime);
        ApplyMomentum(m_LastAscent, ascent, deltaTime);
    }

    // don't apply momentum to mouse inputs
    yaw += GameInput::GetAnalogInput(GameInput::kAnalogMouseX) * m_MouseSensitivityX;
    pitch += GameInput::GetAnalogInput(GameInput::kAnalogMouseY) * m_MouseSensitivityY;

	m_TargetCamera.RotateYaw(yaw);
	m_TargetCamera.RotatePitch(pitch);
	m_TargetCamera.TranslateRightUpForward(strafe, ascent, forward);
}

void CameraController::ApplyMomentum( float& oldValue, float& newValue, float deltaTime )
{
    float blendedValue;
    if (fabsf(newValue) > fabsf(oldValue))
        blendedValue = lerp(newValue, oldValue, powf(0.6f, deltaTime * 60.0f));
    else
        blendedValue = lerp(newValue, oldValue, powf(0.8f, deltaTime * 60.0f));
    oldValue = blendedValue;
    newValue = blendedValue;
}
