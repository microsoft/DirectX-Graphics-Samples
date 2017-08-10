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

#include "Common.h"

namespace Math
{
    class Scalar
    {
    public:
        INLINE Scalar() {}
        INLINE Scalar( const Scalar& s ) { m_vec = s; }
        INLINE Scalar( float f ) { m_vec = XMVectorReplicate(f); }
        INLINE explicit Scalar( FXMVECTOR vec ) { m_vec = vec; }
        INLINE explicit Scalar( EZeroTag ) { m_vec = SplatZero(); }
        INLINE explicit Scalar( EIdentityTag ) { m_vec = SplatOne(); }

        INLINE operator XMVECTOR() const { return m_vec; }
        INLINE operator float() const { return XMVectorGetX(m_vec); }

    private:
        XMVECTOR m_vec;
    };

    INLINE Scalar operator- ( Scalar s ) { return Scalar(XMVectorNegate(s)); }
    INLINE Scalar operator+ ( Scalar s1, Scalar s2 ) { return Scalar(XMVectorAdd(s1, s2)); }
    INLINE Scalar operator- ( Scalar s1, Scalar s2 ) { return Scalar(XMVectorSubtract(s1, s2)); }
    INLINE Scalar operator* ( Scalar s1, Scalar s2 ) { return Scalar(XMVectorMultiply(s1, s2)); }
    INLINE Scalar operator/ ( Scalar s1, Scalar s2 ) { return Scalar(XMVectorDivide(s1, s2)); }
    INLINE Scalar operator+ ( Scalar s1, float s2 ) { return s1 + Scalar(s2); }
    INLINE Scalar operator- ( Scalar s1, float s2 ) { return s1 - Scalar(s2); }
    INLINE Scalar operator* ( Scalar s1, float s2 ) { return s1 * Scalar(s2); }
    INLINE Scalar operator/ ( Scalar s1, float s2 ) { return s1 / Scalar(s2); }
    INLINE Scalar operator+ ( float s1, Scalar s2 ) { return Scalar(s1) + s2; }
    INLINE Scalar operator- ( float s1, Scalar s2 ) { return Scalar(s1) - s2; }
    INLINE Scalar operator* ( float s1, Scalar s2 ) { return Scalar(s1) * s2; }
    INLINE Scalar operator/ ( float s1, Scalar s2 ) { return Scalar(s1) / s2; }

} // namespace Math
