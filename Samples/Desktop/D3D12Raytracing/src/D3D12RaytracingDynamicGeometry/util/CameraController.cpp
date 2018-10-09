//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "stdafx.h"
#include "CameraController.h"
#include "GameInput.h"

using namespace DirectX;
using namespace GameCore;

CameraController::CameraController( Camera& camera) : m_TargetCamera( camera )
{
    m_WorldUp = XMVectorSet(0, 1, 0, 0);
    m_WorldNorth = XMVectorSet(0, 0, 1, 0);
    m_WorldEast = XMVectorSet(1, 0, 0, 0);

    m_HorizontalLookSensitivity = 2.0f;
    m_VerticalLookSensitivity = 2.0f;
    m_MoveSpeed = 1000.0f;
    m_StrafeSpeed = 1000.0f;
    m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = 1.0f;

    m_CurrentPitch = sinf(XMVectorGetX(XMVector3Dot(camera.Forward(), m_WorldUp)));

    XMVECTOR forward = XMVector3Normalize(camera.Forward());
    m_CurrentHeading = atan2f(-XMVectorGetX(XMVector3Dot(forward, m_WorldEast)), XMVectorGetX(XMVector3Dot(forward, m_WorldNorth)));

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

    m_CurrentPitch += pitch;
    m_CurrentPitch = XMMin( XM_PIDIV2, m_CurrentPitch);
    m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

    m_CurrentHeading -= yaw;
    if (m_CurrentHeading > XM_PI)
        m_CurrentHeading -= XM_2PI;
    else if (m_CurrentHeading <= -XM_PI)
        m_CurrentHeading += XM_2PI; 


    XMMATRIX rotation = XMMATRIX(m_WorldEast, m_WorldUp, -m_WorldNorth, XMVectorSet(0,0,0,1)) * XMMatrixRotationY(m_CurrentHeading) * XMMatrixRotationX(m_CurrentPitch);
    XMMATRIX translation = XMMatrixTranslationFromVector(XMVectorSet( strafe, ascent, -forward, 0) + m_TargetCamera.Eye());
	m_TargetCamera.SetViewMatrix(rotation * translation);
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
