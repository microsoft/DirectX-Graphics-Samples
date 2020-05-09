//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//

#pragma once

#include "Camera.h"

namespace GameCore
{
    class CameraController
    {
    public:
        CameraController(Camera& camera);

        bool Update(float dt);

        void SlowMovement(bool enable) { m_FineMovement = enable; }
        void SlowRotation(bool enable) { m_FineRotation = enable; }
        void EnableMomentum(bool enable) { m_Momentum = enable; }
		void SetBoundaries(const DirectX::XMVECTOR& _min, DirectX::XMVECTOR& _max);

    private:
        CameraController& operator=(const CameraController&) {return *this;}

        void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

        Camera& m_camera;
        float m_HorizontalLookSensitivity;
        float m_VerticalLookSensitivity;
        float m_MoveSpeed;
        float m_StrafeSpeed;
        float m_MouseSensitivityX;
        float m_MouseSensitivityY;

        bool m_FineMovement;
        bool m_FineRotation;
        bool m_Momentum;

        float m_LastYaw;
        float m_LastPitch;
        float m_LastForward;
        float m_LastStrafe;
        float m_LastAscent;

		DirectX::XMVECTOR m_boundaryMin;
		DirectX::XMVECTOR m_boundaryMax;
    };
}
