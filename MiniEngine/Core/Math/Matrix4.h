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

#include "Transform.h"

namespace Math
{
    __declspec(align(16)) class Matrix4
    {
    public:
        INLINE Matrix4() {}
        INLINE Matrix4( Vector3 x, Vector3 y, Vector3 z, Vector3 w )
        {
            m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
            m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
        }

        INLINE Matrix4(const float* data)
        {
            m_mat = XMLoadFloat4x4((XMFLOAT4X4*)data);
        }

        INLINE Matrix4( Vector4 x, Vector4 y, Vector4 z, Vector4 w ) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
        INLINE Matrix4( const Matrix4& mat ) { m_mat = mat.m_mat; }
        INLINE Matrix4( const Matrix3& mat )
        {
            m_mat.r[0] = SetWToZero(mat.GetX());
            m_mat.r[1] = SetWToZero(mat.GetY());
            m_mat.r[2] = SetWToZero(mat.GetZ());
            m_mat.r[3] = CreateWUnitVector();
        }
        INLINE Matrix4( const Matrix3& xyz, Vector3 w )
        {
            m_mat.r[0] = SetWToZero(xyz.GetX());
            m_mat.r[1] = SetWToZero(xyz.GetY());
            m_mat.r[2] = SetWToZero(xyz.GetZ());
            m_mat.r[3] = SetWToOne(w);
        }
        INLINE Matrix4( const AffineTransform& xform ) { *this = Matrix4( xform.GetBasis(), xform.GetTranslation()); }
        INLINE Matrix4( const OrthogonalTransform& xform ) { *this = Matrix4( Matrix3(xform.GetRotation()), xform.GetTranslation() ); }
        INLINE explicit Matrix4( const XMMATRIX& mat ) { m_mat = mat; }
        INLINE explicit Matrix4( EIdentityTag ) { m_mat = XMMatrixIdentity(); }
        INLINE explicit Matrix4( EZeroTag ) { m_mat.r[0] = m_mat.r[1] = m_mat.r[2] = m_mat.r[3] = SplatZero(); }

        INLINE const Matrix3& Get3x3() const { return (const Matrix3&)*this; }
        INLINE void Set3x3(const Matrix3& xyz)
        {
            m_mat.r[0] = SetWToZero(xyz.GetX());
            m_mat.r[1] = SetWToZero(xyz.GetY());
            m_mat.r[2] = SetWToZero(xyz.GetZ());
        }

        INLINE Vector4 GetX() const { return Vector4(m_mat.r[0]); }
        INLINE Vector4 GetY() const { return Vector4(m_mat.r[1]); }
        INLINE Vector4 GetZ() const { return Vector4(m_mat.r[2]); }
        INLINE Vector4 GetW() const { return Vector4(m_mat.r[3]); }

        INLINE void SetX(Vector4 x) { m_mat.r[0] = x; }
        INLINE void SetY(Vector4 y) { m_mat.r[1] = y; }
        INLINE void SetZ(Vector4 z) { m_mat.r[2] = z; }
        INLINE void SetW(Vector4 w) { m_mat.r[3] = w; }

        INLINE operator XMMATRIX() const { return m_mat; }

        INLINE Vector4 operator* ( Vector3 vec ) const { return Vector4(XMVector3Transform(vec, m_mat)); }
        INLINE Vector4 operator* ( Vector4 vec ) const { return Vector4(XMVector4Transform(vec, m_mat)); }
        INLINE Matrix4 operator* ( const Matrix4& mat ) const { return Matrix4(XMMatrixMultiply(mat, m_mat)); }

        static INLINE Matrix4 MakeScale( float scale ) { return Matrix4(XMMatrixScaling(scale, scale, scale)); }
        static INLINE Matrix4 MakeScale( Vector3 scale ) { return Matrix4(XMMatrixScalingFromVector(scale)); }

    private:
        XMMATRIX m_mat;
    };
}
