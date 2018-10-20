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

#include "../stdafx.h"
#include "DXSampleHelper.h"
#include "..\RaytracingSceneDefines.h"
#include "DXRTextWithPrimitiveScene.h"
#include "DDSTextureLoader.h"
#include "ResourceUploadBatch.h"

using namespace std;
using namespace DirectX;


// Load geometry from a file into buffers.
void DXRTextWithPrimitives::LoadDensityMap(
	ID3D12Device* device,
	ID3D12CommandQueue* commandQueue,
	LPCWSTR assetPath,
)
{
	ComPtr<ID3D12Resource> texDensityMap;
	ResourceUploadBatch resourceUpload(device);
	resourceUpload.Begin();
	ThrowIfFailed(CreateDDSTextureFromFile(device, resourceUpload, assetPath, &resource), L"Failed to load DDS texture from a file.");

	// Upload the resources to the GPU.
	auto uploadResourcesFinished = resourceUpload.End(commandQueue);

	// Wait for the upload thread to terminate
	uploadResourcesFinished.wait();
}

void LoadScene(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	ID3D12CommandQueue* commandQueue
)
{
	
	LoadDensityMap
}