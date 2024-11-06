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

#include "VectorMath.h"

namespace Math
{
    class BoundingSphere
    {
    public:
        BoundingSphere() {}
        BoundingSphere( float x, float y, float z, float r ) : m_repr(x, y, z, r) {}
        BoundingSphere( const XMFLOAT4* unaligned_array ) : m_repr(*unaligned_array) {}
        BoundingSphere( Vector3 center, Scalar radius );
        BoundingSphere( EZeroTag ) : m_repr(kZero) {}
        explicit BoundingSphere (const XMVECTOR& v) : m_repr(v) {}
        explicit BoundingSphere( const XMFLOAT4& f4 ) : m_repr(f4) {}
        explicit BoundingSphere( Vector4 sphere ) : m_repr(sphere) {}
        explicit operator Vector4() const { return Vector4(m_repr); }

        Vector3 GetCenter( void ) const { return Vector3(m_repr); }
        Scalar GetRadius( void ) const { return m_repr.GetW(); }

        BoundingSphere Union( const BoundingSphere& rhs );

    private:

        Vector4 m_repr;
    };

    //=======================================================================================================
    // Inline implementations
    //

    inline BoundingSphere::BoundingSphere( Vector3 center, Scalar radius )
    {
        m_repr = Vector4(center);
        m_repr.SetW(radius);
    }

} // namespace Math
