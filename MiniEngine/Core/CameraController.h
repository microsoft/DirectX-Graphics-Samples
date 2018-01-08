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

#include "GameCore.h"
#include "VectorMath.h"

namespace Math
{
    class Camera;
}

namespace GameCore
{
    using namespace Math;

    class CameraController
    {
    public:
        // Assumes worldUp is not the X basis vector
        CameraController( Camera& camera, Vector3 worldUp );

        void Update( float dt );

        void SlowMovement( bool enable ) { m_FineMovement = enable; }
        void SlowRotation( bool enable ) { m_FineRotation = enable; }

        void EnableMomentum( bool enable ) { m_Momentum = enable; }

    private:
        CameraController& operator=( const CameraController& ) {return *this;}

        void ApplyMomentum( float& oldValue, float& newValue, float deltaTime );

        Vector3 m_WorldUp;
        Vector3 m_WorldNorth;
        Vector3 m_WorldEast;
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
