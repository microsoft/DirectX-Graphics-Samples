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
#include "ShadowCamera.h"
#include "DirectXMath.h"

using namespace Math;

void ShadowCamera::UpdateMatrix(
    Vector3 LightDirection, Vector3 ShadowCenter, Vector3 ShadowBounds,
    uint32_t BufferWidth, uint32_t BufferHeight, uint32_t BufferPrecision )
{
    SetLookDirection( LightDirection, Vector3(kZUnitVector) );

    // Converts world units to texel units so we can quantize the camera position to whole texel units
    Vector3 RcpDimensions = Recip(ShadowBounds);
    Vector3 QuantizeScale = Vector3((float)BufferWidth, (float)BufferHeight, (float)((1 << BufferPrecision) - 1)) * RcpDimensions;

    //
    // Recenter the camera at the quantized position
    //

    // Transform to view space
    ShadowCenter = ~GetRotation() * ShadowCenter;
    // Scale to texel units, truncate fractional part, and scale back to world units
    ShadowCenter = Floor( ShadowCenter * QuantizeScale ) / QuantizeScale;
    // Transform back into world space
    ShadowCenter = GetRotation() * ShadowCenter;

    SetPosition( ShadowCenter );


    SetProjMatrix(Matrix4::MakeScale(Vector3(2.0f, 2.0f, 1.0f) * RcpDimensions));

    Update();

    // Transform from clip space to texture space
    m_ShadowMatrix = Matrix4( AffineTransform( Matrix3::MakeScale( 0.5f, -0.5f, 1.0f ), Vector3(0.5f, 0.5f, 0.0f) ) ) * m_ViewProjMatrix;
}


void ShadowCamera::UpdateMatrixImproved(
    Vector3 LightDirection, Vector3 ShadowCenter, Vector3 ViewSpaceInfo,
    uint32_t BufferWidth, uint32_t BufferHeight, uint32_t BufferPrecision)
{
    SetLookDirection(LightDirection, Vector3(kZUnitVector));

    SetPosition(ShadowCenter);

    Matrix4 ortho(XMMatrixOrthographicRH((float)ViewSpaceInfo.GetX(), (float)ViewSpaceInfo.GetY(), 0.5f * (float)ViewSpaceInfo.GetZ(), 0.5f * (float) -ViewSpaceInfo.GetZ()));

    SetProjMatrix(ortho);

    Update();

    // Transform from clip space to texture space
    m_ShadowMatrix = Matrix4(AffineTransform(Matrix3::MakeScale(0.5f, -0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.0f))) * m_ViewProjMatrix;
}
