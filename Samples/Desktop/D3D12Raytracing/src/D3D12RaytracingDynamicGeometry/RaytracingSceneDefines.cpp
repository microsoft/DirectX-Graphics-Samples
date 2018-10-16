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

#include "RayTracingSceneDefines.h"


namespace Scene
{
	const WCHAR* Type::Names[] = { L"Single object", L" Geometric forest", L"Squid room" };
	Params args[Scene::Type::Count];

	// Initialize scene parameters
	Initialize initializeObject;
	Initialize::Initialize()
	{
		// Camera Position
		{
			auto& camera = args[Scene::Type::SingleObject].camera;
#if 1
			camera.position.eye = { 0.509415329, 2.28009248, -4.69469929, 1.00000000 };
			camera.position.at = { 1.17371607, -6.80418682, 2.02239656, 1.00000000 };
			camera.position.up = { 0.109009691, 0.596148014, 0.795441568, 0.000000000 };
#else
			camera.position.eye = { 0, 6.3f, -10.0f, 1 };
			camera.position.at = { 0, 1, 0, 1 };
			XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };
			XMVECTOR forward = XMVector4Normalize(camera.position.at - camera.position.eye);
			camera.position.up = XMVector3Normalize(XMVector3Cross(forward, right));
#endif
			camera.boundaries.min = -XMVectorSplatInfinity();
			camera.boundaries.max = XMVectorSplatInfinity();
		}
		{
			// ToDo
			auto& camera = args[Scene::Type::GeometricForest].camera;
			camera.position.eye = { 0, 80, 268.555980f, 1 };
			camera.position.at = { 0, 80, 0, 1 };
			camera.position.up = { 0, 1, 0, 0 };
			camera.boundaries.min = -XMVectorSplatInfinity();
			camera.boundaries.max = XMVectorSplatInfinity();
		}
		{
			auto& camera = args[Scene::Type::SquidRoom].camera;
			camera.position.eye = { 0, 80, 268.555980f, 1 };
			camera.position.at = { 0, 80, 0, 1 };
			camera.position.up = { 0, 1, 0, 0 };
			camera.boundaries.min = { -430, 2.2f, -428, 1 };
			camera.boundaries.max = { 408, 358, 416, 1 };
		}
	}
}
