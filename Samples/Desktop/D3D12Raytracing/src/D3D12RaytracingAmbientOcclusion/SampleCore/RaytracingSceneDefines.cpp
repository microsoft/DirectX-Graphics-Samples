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
            camera.position.eye = { -35.7656f, 14.7652f, -22.5312f, 1 };
            camera.position.at = { -35.0984f, 14.345f, -21.9154f, 1 };
            camera.position.up = { 0.378971f, 0.854677f, 0.354824f, 0 };
			camera.boundaries.min = -XMVectorSplatInfinity();
			camera.boundaries.max = XMVectorSplatInfinity();

            // ToDoF remove
#if 1  
            camera.position.eye = { -38.5863f, 13.9563f, -24.2481f, 1 };
            camera.position.at = { -37.9042f, 13.5773f, -23.6219f, 1 };
            camera.position.up = { 0.351221f, 0.877166f, 0.32744f, 0 };
#endif
		}
	}
}
