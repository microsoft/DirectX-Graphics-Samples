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
// Author(s):  Jack Elliott
//

#pragma once
#include <../Core/VectorMath.h>
#include <../Core/EngineTuning.h>
#include <../Core/Color.h>
#include <../Core/Math/Random.h>
#include <algorithm>
#include <cstdlib>

NumVar VerticalSpeedScale("Application/Moving Object/Vertical Speed", 0.3f, 0, 10.0f, 0.1f);
NumVar HorizontalSpeedScale("Application/Moving Object/Horizontal Speed", 0.2f, 0, 10.0f, 0.1f);

using namespace Math;

#define PI 3.1415926535897932384626433832795f

static inline Vector3 RandNormal()
{
    float theta = g_RNG.NextFloat(2.0f * PI);
    float x = cos(theta);
    float y = sin(theta);
    float z = g_RNG.NextFloat(-1.0f, 1.0f);
    return Vector3(x, y, z);
}

static inline Color RandColor()
{
    return Color(g_RNG.NextFloat(), g_RNG.NextFloat(), g_RNG.NextFloat());
}

class Animateable
{
public:

    Animateable()
    {
    }

    void InitAsProjectile()
    {
        m_acceleration = Vector3( 0.0f, -0.0008f, 0.0f );
        m_velocity = Vector3( g_RNG.NextFloat(-1.0f, 1.0f) * HorizontalSpeedScale, VerticalSpeedScale, g_RNG.NextFloat(-1.0f, 1.0f) * HorizontalSpeedScale );
        m_diffuseColor = RandColor();

        OrthogonalTransform& curr = m_worldStates[0];

        curr.SetRotation(Quaternion(RandNormal(), Scalar(g_RNG.NextFloat(2.0f * PI))));
        m_rotationRate = Quaternion(RandNormal(), Scalar(0.1f));

        SaveState();
    }

    void InitAsSpinner(Vector3 pos, Quaternion rotationRate)
    {
        m_acceleration = Vector3(kZero);
        m_velocity = Vector3(kZero);
        m_diffuseColor = RandColor();

        OrthogonalTransform& curr = m_worldStates[0];
        curr = OrthogonalTransform(pos);

        m_rotationRate = rotationRate;

        SaveState();
    }

    void Update()
    {

        SaveState();

        m_velocity += m_acceleration;

        OrthogonalTransform& curr = m_worldStates[0];
        curr.SetTranslation( curr.GetTranslation() + m_velocity );
        curr.SetRotation( curr.GetRotation() * m_rotationRate );

        m_lifetime++;
    }

    Math::OrthogonalTransform GetWorldMatrix(uint32_t index)
    {
        index = std::min(index, uint32_t(_countof(m_worldStates) - 1));
        return m_worldStates[index];
    }

    void SaveState()
    {
        m_worldStates[2] = m_worldStates[1];
        m_worldStates[1] = m_worldStates[0];
    }

    uint32_t m_lifetime = 0;

    OrthogonalTransform m_worldStates[3];

    Math::Vector3 m_velocity;
    Math::Vector3 m_acceleration;
    Math::Quaternion m_rotationRate;
    Color m_diffuseColor;

    static const uint32_t m_AnimatedObjectLifeSpan = 1500;
};