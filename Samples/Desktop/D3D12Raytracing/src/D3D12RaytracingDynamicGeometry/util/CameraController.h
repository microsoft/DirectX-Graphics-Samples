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

#pragma once

#include "Camera.h"

namespace GameCore
{
    class CameraController
    {
    public:
        CameraController( Camera& camera );

        void Update( float dt );

        void SlowMovement( bool enable ) { m_FineMovement = enable; }
        void SlowRotation( bool enable ) { m_FineRotation = enable; }

        void EnableMomentum( bool enable ) { m_Momentum = enable; }

		const DirectX::XMVECTOR& GetWorldEast() const { return m_WorldEast; }
		const DirectX::XMVECTOR& GetWorldUp() const { return m_WorldUp; }
        const DirectX::XMVECTOR& GetWorldNorth() const { return m_WorldNorth; }
        float GetCurrentHeading() { return m_CurrentHeading; }
        float GetCurrentPitch() { return m_CurrentPitch; }

        void SetCurrentHeading(float heading) { m_CurrentHeading = heading; }
        void SetCurrentPitch(float pitch) { m_CurrentPitch = pitch; }


    private:
        CameraController& operator=( const CameraController& ) {return *this;}

        void ApplyMomentum( float& oldValue, float& newValue, float deltaTime );

        DirectX::XMVECTOR m_WorldUp;
        DirectX::XMVECTOR m_WorldNorth;
        DirectX::XMVECTOR m_WorldEast;
        Camera& m_TargetCamera;
        float m_HorizontalLookSensitivity;
        float m_VerticalLookSensitivity;
        float m_MoveSpeed;
        float m_StrafeSpeed;
        float m_MouseSensitivityX;
        float m_MouseSensitivityY;

        float m_CurrentHeading;
        float m_CurrentPitch;

        bool m_FineMovement;
        bool m_FineRotation;
        bool m_Momentum;

        float m_LastYaw;
        float m_LastPitch;
        float m_LastForward;
        float m_LastStrafe;
        float m_LastAscent;
    };
}
