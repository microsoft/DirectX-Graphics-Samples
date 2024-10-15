//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

namespace GameCore
{
	class Camera
	{
	public:
		float ZMin = NEAR_PLANE;  
		float ZMax = FAR_PLANE;
		float fov = FOVY;

		Camera();
		~Camera();

		// Accessors.
		Camera* Get() { return s_camera; }
		const DirectX::XMVECTOR& Eye() const { return m_eye; }
		const DirectX::XMVECTOR& At() const { return m_at; }
		const DirectX::XMVECTOR& Up() const { return m_up; }
		DirectX::XMVECTOR Forward() { return DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(m_at, m_eye)); }

		// Member functions.
		void GetProj(DirectX::XMMATRIX *proj, UINT screenWidth, UINT screenHeight, bool rhCoords = false) const;
		void GetViewProj(DirectX::XMMATRIX *view, DirectX::XMMATRIX *proj, UINT screenWidth, UINT screenHeight, bool rhCoords = false) const;
		void Set(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& at, const DirectX::XMVECTOR& up);
		void RotateAroundYAxis(float angleRad);
		void RotateYaw(float angleRad);
		void RotatePitch(float angleRad);
		void TranslateForward(float delta);
		void TranslateRight(float delta);
		void TranslateUp(float delta);
		void TranslateRightUpForward(float deltaRight, float deltaUp, float deltaForward);

	private:
		static Camera* s_camera;
		// Homogenous vectors: points(x,y,z,1), vectors(x,y,z,0).
		DirectX::XMVECTOR m_eye; // Where the camera is in world space. Z increases into of the screen when using LH coord system (which we are and DX uses).
		DirectX::XMVECTOR m_at; // What the camera is looking at (world origin).
		DirectX::XMVECTOR m_up; // Normalized up vector.
	};
}
