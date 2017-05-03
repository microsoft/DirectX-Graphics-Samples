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

#include "BoundingPlane.h"
#include "BoundingSphere.h"

namespace Math
{
    class Frustum
    {
    public:
        Frustum() {}

        Frustum( const Matrix4& ProjectionMatrix );

        enum CornerID
        {
            kNearLowerLeft, kNearUpperLeft, kNearLowerRight, kNearUpperRight,
            kFarLowerLeft,  kFarUpperLeft,  kFarLowerRight,  kFarUpperRight
        };

        enum PlaneID
        {
            kNearPlane, kFarPlane, kLeftPlane, kRightPlane, kTopPlane, kBottomPlane
        };

        Vector3         GetFrustumCorner( CornerID id ) const   { return m_FrustumCorners[id]; }
        BoundingPlane   GetFrustumPlane( PlaneID id ) const     { return m_FrustumPlanes[id]; }

        // Test whether the bounding sphere intersects the frustum.  Intersection is defined as either being
        // fully contained in the frustum, or by intersecting one or more of the planes.
        bool IntersectSphere( BoundingSphere sphere );

        friend Frustum  operator* ( const OrthogonalTransform& xform, const Frustum& frustum );	// Fast
        friend Frustum  operator* ( const AffineTransform& xform, const Frustum& frustum );		// Slow
        friend Frustum  operator* ( const Matrix4& xform, const Frustum& frustum );				// Slowest (and most general)

    private:

        // Perspective frustum constructor (for pyramid-shaped frusta)
        void ConstructPerspectiveFrustum( float HTan, float VTan, float NearClip, float FarClip );

        // Orthographic frustum constructor (for box-shaped frusta)
        void ConstructOrthographicFrustum( float Left, float Right, float Top, float Bottom, float NearClip, float FarClip );

        Vector3 m_FrustumCorners[8];		// the corners of the frustum
        BoundingPlane m_FrustumPlanes[6];			// the bounding planes
    };

    //=======================================================================================================
    // Inline implementations
    //

    inline bool Frustum::IntersectSphere( BoundingSphere sphere )
    {
        float radius = sphere.GetRadius();
        for (int i = 0; i < 6; ++i)
        {
            if (m_FrustumPlanes[i].DistanceFromPoint(sphere.GetCenter()) + radius < 0.0f)
                return false;
        }
        return true;
    }

    inline Frustum operator* ( const OrthogonalTransform& xform, const Frustum& frustum )
    {
        Frustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

        // Why isn't there an Invert( OrthogonalTransform ) function?
        Matrix4 XForm = Transpose(Matrix4(xform.GetRotation(), -(xform.GetRotation() * xform.GetTranslation())));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

    inline Frustum operator* ( const AffineTransform& xform, const Frustum& frustum )
    {
        Frustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

        Matrix4 XForm = Transpose(Invert(Matrix4(xform)));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

    inline Frustum operator* ( const Matrix4& mtx, const Frustum& frustum )
    {
        Frustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = Vector3( mtx * frustum.m_FrustumCorners[i] );

        Matrix4 XForm = Transpose(Invert(mtx));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

} // namespace Math
