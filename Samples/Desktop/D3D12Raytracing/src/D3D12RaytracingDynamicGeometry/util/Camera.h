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
		float ZMin = 0.01f;
		float ZMax = 1000.f;

		Camera();
		~Camera();

		const DirectX::XMVECTOR& Eye() const { return m_eye; }
		const DirectX::XMVECTOR& At() const { return m_at; }
		const DirectX::XMVECTOR& Up() const { return m_up; }
		DirectX::XMVECTOR Forward() { return DirectX::XMVectorSubtract(m_at, m_eye); }


		void GetViewProj(DirectX::XMMATRIX *view, DirectX::XMMATRIX *proj, float fovInDegrees, UINT screenWidth, UINT screenHeight, bool rhCoords = false);

		void Reset();
		void Set(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& at, const DirectX::XMVECTOR& up);
		static Camera *get();
		void RotateAroundYAxis(float angleRad);
		void RotateYaw(float angleRad);
		void RotatePitch(float angleRad);
		void SetViewMatrix(const DirectX::XMMATRIX& transform);
	private:
		static Camera* mCamera;
		DirectX::XMVECTOR m_eye; // Where the camera is in world space. Z increases into of the screen when using LH coord system (which we are and DX uses)
		DirectX::XMVECTOR m_at; // What the camera is looking at (world origin)
		DirectX::XMVECTOR m_up; // Which way is up
	};
}
