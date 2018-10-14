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
		{
			auto& camera = args[Scene::Type::SingleObject].cameraPosition;
			camera.eye = { 0, 6.3f, -17.0f, 1 };
			camera.at = { 0, 1, 0, 1 };
			XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };
			XMVECTOR forward = XMVector4Normalize(camera.at - camera.eye);
			camera.up = XMVector3Normalize(XMVector3Cross(forward, right));
		}
		{
			// ToDo
			auto& camera = args[Scene::Type::GeometricForest].cameraPosition;
			camera.eye = { 0, 80, 268.555980f, 1 };
			camera.at = { 0, 80, 0, 1 };
			camera.up = { 0, 1, 0, 0 };
		}
		{
			auto& camera = args[Scene::Type::SquidRoom].cameraPosition;
			camera.eye = { 0, 80, 268.555980f, 1 };
			camera.at = { 0, 80, 0, 1 };
			camera.up = { 0, 1, 0, 0 };
		}
	}
}
