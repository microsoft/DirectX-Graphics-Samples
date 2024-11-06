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

#include "Vector.h"

namespace Math
{
    class Quaternion
    {
    public:
        INLINE Quaternion() { m_vec = XMQuaternionIdentity(); }
        INLINE Quaternion( const Vector3& axis, const Scalar& angle ) { m_vec = XMQuaternionRotationAxis( axis, angle ); }
        INLINE Quaternion( float pitch, float yaw, float roll) { m_vec = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll); }
        INLINE explicit Quaternion( const XMMATRIX& matrix ) { m_vec = XMQuaternionRotationMatrix( matrix ); }	
        INLINE explicit Quaternion( FXMVECTOR vec ) { m_vec = vec; }
        INLINE explicit Quaternion( EIdentityTag ) { m_vec = XMQuaternionIdentity(); }

        INLINE operator XMVECTOR() const { return m_vec; }

        INLINE Quaternion operator~ ( void ) const { return Quaternion(XMQuaternionConjugate(m_vec)); }
        INLINE Quaternion operator- ( void ) const { return Quaternion(XMVectorNegate(m_vec)); }

        INLINE Quaternion operator* ( Quaternion rhs ) const { return Quaternion(XMQuaternionMultiply(rhs, m_vec)); }
        INLINE Vector3 operator* ( Vector3 rhs ) const { return Vector3(XMVector3Rotate(rhs, m_vec)); }

        INLINE Quaternion& operator= ( Quaternion rhs ) { m_vec = rhs; return *this; }
        INLINE Quaternion& operator*= ( Quaternion rhs ) { *this = *this * rhs; return *this; }

    protected:
        XMVECTOR m_vec;
    };

    INLINE Quaternion Normalize(Quaternion q) { return Quaternion(XMQuaternionNormalize(q)); }
    INLINE Quaternion Slerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(XMQuaternionSlerp(a, b, t))); }
    INLINE Quaternion Lerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(XMVectorLerp(a, b, t))); }
}
