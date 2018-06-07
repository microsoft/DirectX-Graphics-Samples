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

#include "stdafx.h"
#include "DirectXRaytracingHelper.h"
#include "D3D12RaytracingDynamicGeometry.h"

using namespace std;
using namespace DX;
using namespace DirectX;


void BottomLevelAccelerationStructure::CopyInstanceDescTo(ID3D12Device* device, void* destInstanceDesc)
{
	auto BuildInstanceDesc = [&](auto* instanceDesc, auto& bottomLevelAddress)
	{
		instanceDesc->InstanceMask = 1;
		instanceDesc->InstanceContributionToHitGroupIndex = 0;
		instanceDesc->AccelerationStructure = bottomLevelAddress;
		StoreXMMatrixAsTransform3x4(instanceDesc->Transform, m_transform);
	};

	if (g_raytracingRuntime.API == RaytracingAPI::FallbackLayer)
	{
		D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDesc = {};
		WRAPPED_GPU_POINTER bottomLevelASaddress =
			pSample->CreateFallbackWrappedPointer(m_accelerationStructure.Get(), static_cast<UINT>(m_prebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32));
		BuildInstanceDesc(&instanceDesc, bottomLevelASaddress);
		memcpy(destInstanceDesc, &instanceDesc, sizeof(instanceDesc));
	}
	else // DirectX Raytracing
	{
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		BuildInstanceDesc(&instanceDesc, m_accelerationStructure->GetGPUVirtualAddress());
		memcpy(destInstanceDesc, &instanceDesc, sizeof(instanceDesc));
	}
};