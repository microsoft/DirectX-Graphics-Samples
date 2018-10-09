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

#include "Frustum.h"

namespace GameCore
{
    class BaseCamera
    {
    public:

        // Call this function once per frame and after you've changed any state.  This
        // regenerates all matrices.  Calling it more or less than once per frame will break
        // temporal effects and cause unpredictable results.
        void Update();

        // Public functions for controlling where the camera is and its orientation
        void SetEyeAtUp( XMVECTOR eye, XMVECTOR at, XMVECTOR up );
        void SetLookDirection( XMVECTOR forward, XMVECTOR up );
        void SetRotation( Quaternion basisRotation );
        void SetPosition( XMVECTOR worldPos );
        void SetTransform( const AffineTransform& xform );
        void SetTransform( const OrthogonalTransform& xform );

        const Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }
        const XMVECTOR GetRightVec() const { return m_CameraToWorld.GetX(); }
        const XMVECTOR GetUpVec() const { return m_Basis.GetY(); }
        const XMVECTOR GetForwardVec() const { return -m_Basis.GetZ(); }
        const XMVECTOR GetPosition() const { return m_CameraToWorld.GetTranslation(); }

        // Accessors for reading the various matrices and frusta
        const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
        const XMMATRIX& GetProjMatrix() const { return m_ProjMatrix; }
        const XMMATRIX& GetViewProjMatrix() const { return m_ViewProjMatrix; }
        const XMMATRIX& GetReprojectionMatrix() const { return m_ReprojectMatrix; }

    protected:

        BaseCamera() : m_CameraToWorld(XMMatrixIdentity()), m_Basis(XMMatrixIdentity()) {}

        void SetProjMatrix( const XMMATRIX& ProjMat ) { m_ProjMatrix = ProjMat; }

        XMMATRIX m_CameraToWorld;

        // Redundant data cached for faster lookups.
        Matrix3 m_Basis;

        // Transforms homogeneous coordinates from world space to view space.  In this case, view space is defined as +X is
        // to the right, +Y is up, and -Z is forward.  This has to match what the projection matrix expects, but you might
        // also need to know what the convention is if you work in view space in a shader.
        XMMATRIX m_ViewMatrix;        // i.e. "World-to-View" matrix

        // The projection matrix transforms view space to clip space.  Once division by W has occurred, the final coordinates
        // can be transformed by the viewport matrix to screen space.  The projection matrix is determined by the screen aspect 
        // and camera field of view.  A projection matrix can also be orthographic.  In that case, field of view would be defined
        // in linear units, not angles.
        XMMATRIX m_ProjMatrix;        // i.e. "View-to-Projection" matrix

        // A concatenation of the view and projection matrices.
        XMMATRIX m_ViewProjMatrix;    // i.e.  "World-To-Projection" matrix.

        // The view-projection matrix from the previous frame
        XMMATRIX m_PreviousViewProjMatrix;

        // Projects a clip-space coordinate to the previous frame (useful for temporal effects).
        XMMATRIX m_ReprojectMatrix;

        Frustum m_FrustumVS;        // View-space view frustum
        Frustum m_FrustumWS;        // World-space view frustum

    };

    class Camera : public BaseCamera
    {
    public:
        Camera();

        // Controls the view-to-projection matrix
        void SetPerspectiveMatrix( float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip );
        void SetFOV( float verticalFovInRadians ) { m_VerticalFOV = verticalFovInRadians; UpdateProjMatrix(); }
        void SetAspectRatio( float heightOverWidth ) { m_AspectRatio = heightOverWidth; UpdateProjMatrix(); }
        void SetZRange( float nearZ, float farZ) { m_NearClip = nearZ; m_FarClip = farZ; UpdateProjMatrix(); }
        void ReverseZ( bool enable ) { m_ReverseZ = enable; UpdateProjMatrix(); }

        float GetFOV() const { return m_VerticalFOV; }
        float GetNearClip() const { return m_NearClip; }
        float GetFarClip() const { return m_FarClip; }
        float GetClearDepth() const { return m_ReverseZ ? 0.0f : 1.0f; }

    private:

        void UpdateProjMatrix( void );

        float m_VerticalFOV;            // Field of view angle in radians
        float m_AspectRatio;
        float m_NearClip;
        float m_FarClip;
        bool m_ReverseZ;                // Invert near and far clip distances so that Z=0 is the far plane
    };

    inline void BaseCamera::SetEyeAtUp( XMVECTOR eye, XMVECTOR at, XMVECTOR up )
    {
        SetLookDirection(at - eye, up);
        SetPosition(eye);
    }

    inline void BaseCamera::SetPosition( XMVECTOR worldPos )
    {
        m_CameraToWorld.SetTranslation( worldPos );
    }

    inline void BaseCamera::SetTransform( const AffineTransform& xform )
    {
        // By using these functions, we rederive an orthogonal transform.
        SetLookDirection(-xform.GetZ(), xform.GetY());
        SetPosition(xform.GetTranslation());
    }

    inline void BaseCamera::SetRotation( Quaternion basisRotation )
    {
        m_CameraToWorld.SetRotation(Normalize(basisRotation));
        m_Basis = Matrix3(m_CameraToWorld.GetRotation());
    }

    inline Camera::Camera() : m_ReverseZ(true)
    {
        SetPerspectiveMatrix( XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f );
    }

    inline void Camera::SetPerspectiveMatrix( float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip )
    {
        m_VerticalFOV = verticalFovRadians;
        m_AspectRatio = aspectHeightOverWidth;
        m_NearClip = nearZClip;
        m_FarClip = farZClip;

        UpdateProjMatrix();

        m_PreviousViewProjMatrix = m_ViewProjMatrix;
    }

} // namespace Math
