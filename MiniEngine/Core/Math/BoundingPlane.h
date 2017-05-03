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
    class BoundingPlane
    {
    public:

        BoundingPlane() {}
        BoundingPlane( Vector3 normalToPlane, float distanceFromOrigin ) : m_repr(normalToPlane, distanceFromOrigin) {}
        BoundingPlane( Vector3 pointOnPlane, Vector3 normalToPlane );
        BoundingPlane( float A, float B, float C, float D ) : m_repr(A, B, C, D) {}
        BoundingPlane( const BoundingPlane& plane ) : m_repr(plane.m_repr) {}
        explicit BoundingPlane( Vector4 plane ) : m_repr(plane) {}

        INLINE operator Vector4() const { return m_repr; }

        Vector3 GetNormal( void ) const { return Vector3(m_repr); }

        Scalar DistanceFromPoint( Vector3 point ) const
        {
            //return Dot(point, GetNormal()) + m_repr.GetW();
            return Dot( Vector4(point, 1.0f), m_repr );
        }

        friend BoundingPlane operator* ( const OrthogonalTransform& xform, BoundingPlane plane )
        {
            return BoundingPlane( xform.GetTranslation() - plane.GetNormal() * plane.m_repr.GetW(), xform.GetRotation() * plane.GetNormal() );
        }

        friend BoundingPlane operator* ( const Matrix4& mat, BoundingPlane plane )
        {
            return BoundingPlane( Transpose(Invert(mat)) * plane.m_repr );
        }

    private:

        Vector4 m_repr;
    };

    //=======================================================================================================
    // Inline implementations
    //
    inline BoundingPlane::BoundingPlane( Vector3 pointOnPlane, Vector3 normalToPlane )
    {
        // Guarantee a normal.  This constructor isn't meant to be called frequently, but if it is, we can change this.
        normalToPlane = Normalize(normalToPlane);	
        m_repr = Vector4(normalToPlane, -Dot(pointOnPlane, normalToPlane));
    }

    //=======================================================================================================
    // Functions operating on planes
    //
    inline BoundingPlane PlaneFromPointsCCW( Vector3 A, Vector3 B, Vector3 C )
    {
        return BoundingPlane( A, Cross(B - A, C - A) );
    }


} // namespace Math
