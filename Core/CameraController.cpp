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

#include "pch.h"
#include "CameraController.h"
#include "Camera.h"
#include "GameInput.h"

using namespace Math;
using namespace GameCore;

FlyingFPSCamera::FlyingFPSCamera( Camera& camera, Vector3 worldUp ) : CameraController( camera )
{
    m_WorldUp = Normalize(worldUp);
    m_WorldNorth = Normalize(Cross(m_WorldUp, Vector3(kXUnitVector)));
    m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

    m_HorizontalLookSensitivity = 2.0f;
    m_VerticalLookSensitivity = 2.0f;
    m_MoveSpeed = 1000.0f;
    m_StrafeSpeed = 1000.0f;
    m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = 1.0f;

    m_CurrentPitch = Sin(Dot(camera.GetForwardVec(), m_WorldUp));

    Vector3 forward = Normalize(Cross(m_WorldUp, camera.GetRightVec()));
    m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

    m_FineMovement = false;
    m_FineRotation = false;
    m_Momentum = true;

    m_LastYaw = 0.0f;
    m_LastPitch = 0.0f;
    m_LastForward = 0.0f;
    m_LastStrafe = 0.0f;
    m_LastAscent = 0.0f;
}

namespace Graphics
{
    extern EnumVar DebugZoom;
}

void FlyingFPSCamera::Update( float deltaTime )
{
    (deltaTime);

    float timeScale = Graphics::DebugZoom == 0 ? 1.0f : Graphics::DebugZoom == 1 ? 0.5f : 0.25f;

    if (GameInput::IsFirstPressed(GameInput::kLThumbClick) || GameInput::IsFirstPressed(GameInput::kKey_lshift))
        m_FineMovement = !m_FineMovement;

    if (GameInput::IsFirstPressed(GameInput::kRThumbClick))
        m_FineRotation = !m_FineRotation;

    float speedScale = (m_FineMovement ? 0.1f : 1.0f) * timeScale;
    float panScale = (m_FineRotation ? 0.5f : 1.0f) * timeScale;

    float yaw = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogRightStickX ) * m_HorizontalLookSensitivity * panScale;
    float pitch = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogRightStickY ) * m_VerticalLookSensitivity * panScale;
    float forward =	m_MoveSpeed * speedScale * (
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

    Matrix3 orientation = Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * Matrix3::MakeYRotation( m_CurrentHeading ) * Matrix3::MakeXRotation( m_CurrentPitch );
    Vector3 position = orientation * Vector3( strafe, ascent, -forward ) + m_TargetCamera.GetPosition();
    m_TargetCamera.SetTransform( AffineTransform( orientation, position ) );
    m_TargetCamera.Update();
}

void FlyingFPSCamera::SetHeadingPitchAndPosition(float heading, float pitch, const Vector3& position)
{
    m_CurrentHeading = heading;
    if (m_CurrentHeading > XM_PI)
        m_CurrentHeading -= XM_2PI;
    else if (m_CurrentHeading <= -XM_PI)
        m_CurrentHeading += XM_2PI; 

    m_CurrentPitch = pitch;
    m_CurrentPitch = XMMin( XM_PIDIV2, m_CurrentPitch);
    m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

    Matrix3 orientation =
        Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * 
        Matrix3::MakeYRotation( m_CurrentHeading ) *
        Matrix3::MakeXRotation( m_CurrentPitch );

    m_TargetCamera.SetTransform( AffineTransform( orientation, position ) );
    m_TargetCamera.Update();
}


void CameraController::ApplyMomentum( float& oldValue, float& newValue, float deltaTime )
{
    float blendedValue;
    if (Abs(newValue) > Abs(oldValue))
        blendedValue = Lerp(newValue, oldValue, Pow(0.6f, deltaTime * 60.0f));
    else
        blendedValue = Lerp(newValue, oldValue, Pow(0.8f, deltaTime * 60.0f));
    oldValue = blendedValue;
    newValue = blendedValue;
}

OrbitCamera::OrbitCamera( Camera& camera, Math::BoundingSphere focus, Vector3 worldUp ) : CameraController( camera )
{
    m_ModelBounds = focus;
    m_WorldUp = Normalize(worldUp);

    m_JoystickSensitivityX = 2.0f;
    m_JoystickSensitivityY = 2.0f;

    m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = 1.0f;

    m_CurrentPitch = 0.0f;
    m_CurrentHeading = 0.0f;
    m_CurrentCloseness = 0.5f;

    m_Momentum = true;

    m_LastYaw = 0.0f;
    m_LastPitch = 0.0f;
}

void OrbitCamera::Update( float deltaTime )
{
    (deltaTime);

    float timeScale = Graphics::DebugZoom == 0 ? 1.0f : Graphics::DebugZoom == 1 ? 0.5f : 0.25f;

    float yaw = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogLeftStickX ) * timeScale * m_JoystickSensitivityX;
    float pitch = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogLeftStickY ) * timeScale * m_JoystickSensitivityY;
    float closeness = GameInput::GetTimeCorrectedAnalogInput( GameInput::kAnalogRightStickY ) * timeScale;

    if (m_Momentum)
    {
        ApplyMomentum(m_LastYaw, yaw, deltaTime);
        ApplyMomentum(m_LastPitch, pitch, deltaTime);
    }

    // don't apply momentum to mouse inputs
    yaw += GameInput::GetAnalogInput(GameInput::kAnalogMouseX) * m_MouseSensitivityX;
    pitch += GameInput::GetAnalogInput(GameInput::kAnalogMouseY) * m_MouseSensitivityY;
    closeness += GameInput::GetAnalogInput(GameInput::kAnalogMouseScroll) * 0.1f;

    m_CurrentPitch += pitch;
    m_CurrentPitch = XMMin( XM_PIDIV2, m_CurrentPitch);
    m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

    m_CurrentHeading -= yaw;
    if (m_CurrentHeading > XM_PI)
        m_CurrentHeading -= XM_2PI;
    else if (m_CurrentHeading <= -XM_PI)
        m_CurrentHeading += XM_2PI; 

    m_CurrentCloseness += closeness;
    m_CurrentCloseness = Clamp(m_CurrentCloseness, 0.0f, 1.0f);

    Matrix3 orientation = Matrix3::MakeYRotation( m_CurrentHeading ) * Matrix3::MakeXRotation( m_CurrentPitch );
    Vector3 position = orientation.GetZ() * (m_ModelBounds.GetRadius() * Lerp(3.0f, 1.0f, m_CurrentCloseness) + m_TargetCamera.GetNearClip());
    m_TargetCamera.SetTransform(AffineTransform(orientation, position + m_ModelBounds.GetCenter()));
    m_TargetCamera.Update();
}
