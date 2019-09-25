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

namespace SampleScene
{
	const WCHAR* Type::Names[Type::Count] = { L"Main Sample Scene" };
	Params args[Type::Count];

	// Initialize scene parameters
	Initialize initializeObject;
	Initialize::Initialize()
	{
		// Camera Position
		{
			auto& camera = args[Type::Main].camera;
            camera.position.eye = { -38.5863f, 13.9563f, -24.2481f, 1 };
            camera.position.at = { -37.9042f, 13.5773f, -23.6219f, 1 };
            camera.position.up = { 0.351221f, 0.877166f, 0.32744f, 0 };
			camera.boundaries.min = XMVectorSetY(-XMVectorSplatInfinity(), 0);
			camera.boundaries.max = XMVectorSplatInfinity();
		}
	}
}
