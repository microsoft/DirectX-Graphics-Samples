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

AccelerationStructure::AccelerationStructure() :
	m_buildFlags(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE),
	m_prebuildInfo{}
{
}

void AccelerationStructure::ReleaseD3DResources()
{
	m_accelerationStructure.Reset();
}

void AccelerationStructure::AllocateResource(ID3D12Device* device)
{
	// Allocate resource for acceleration structures.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn’t need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
	{
		D3D12_RESOURCE_STATES initialResourceState;
		if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
		{
			initialResourceState = g_pSample->GetFallbackDevice()->GetAccelerationStructureResourceState();
		}
		else // DirectX Raytracing
		{
			initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		}
		AllocateUAVBuffer(device, m_prebuildInfo.ResultDataMaxSizeInBytes, &m_accelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
	}
}


BottomLevelAccelerationStructure::BottomLevelAccelerationStructure() :
	m_isDirty(true),
	m_transform(XMMatrixIdentity())
{
}

void BottomLevelAccelerationStructure::BuildInstanceDesc(void* destInstanceDesc, UINT* descriptorHeapIndex)
{
	auto InitializeInstanceDesc = [&](auto* instanceDesc, auto bottomLevelAddress)
	{
		*instanceDesc = {};
		instanceDesc->InstanceMask = 1;
		instanceDesc->InstanceContributionToHitGroupIndex = 0;
		instanceDesc->AccelerationStructure = bottomLevelAddress;
		StoreXMMatrixAsTransform3x4(instanceDesc->Transform, m_transform);
	};

	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		WRAPPED_GPU_POINTER bottomLevelASaddress =
			g_pSample->CreateFallbackWrappedPointer(m_accelerationStructure.Get(), static_cast<UINT>(m_prebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32), descriptorHeapIndex);
		InitializeInstanceDesc(static_cast<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC*>(destInstanceDesc), bottomLevelASaddress);
	}
	else // DirectX Raytracing
	{
		InitializeInstanceDesc(static_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(destInstanceDesc), m_accelerationStructure->GetGPUVirtualAddress());
	}
};

// Build geometry descs for bottom-level AS.
void BottomLevelAccelerationStructure::BuildGeometryDescs(const TriangleGeometryBuffer& geometry, UINT numInstances, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
{
	// ToDo pass geometry flag from the sample cpp
	// Mark the geometry as opaque. 
	// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
	// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
	D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescTemplate = {};
	geometryDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDescTemplate.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
	geometryDescTemplate.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDescTemplate.Triangles.VertexBuffer.StrideInBytes = sizeof(DirectX::GeometricPrimitive::VertexType);
	geometryDescTemplate.Flags = geometryFlags;

	m_geometryDescs.resize(numInstances, geometryDescTemplate);

	// ToDo support multiple different geometries
	for (UINT i = 0; i < numInstances; i++)
	{
		auto& geometryDesc = m_geometryDescs[i];
		geometryDesc.Triangles.IndexBuffer = geometry.ib.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.IndexCount = static_cast<UINT>(geometry.ib.resource->GetDesc().Width) / sizeof(Index);
		geometryDesc.Triangles.VertexBuffer.StartAddress = geometry.vb.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexCount = static_cast<UINT>(geometry.vb.resource->GetDesc().Width) / sizeof(DirectX::GeometricPrimitive::VertexType);
		geometryDesc.Triangles.Transform = baseGeometryTransformGPUAddress + i * sizeof(AlignedGeometryTransform3x4);
	}
}

void BottomLevelAccelerationStructure::ComputePrebuildInfo()
{
	// Get the size requirements for the scratch and AS buffers.
	D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
	prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	prebuildInfoDesc.Flags = m_buildFlags;
	prebuildInfoDesc.NumDescs = static_cast<UINT>(m_geometryDescs.size());
	prebuildInfoDesc.pGeometryDescs = m_geometryDescs.data();
	
	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		g_pSample->GetFallbackDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
	}
	else // DirectX Raytracing
	{
		g_pSample->GetDxrDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
	}
	ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
}

void BottomLevelAccelerationStructure::Initialize(ID3D12Device* device, const TriangleGeometryBuffer& geometry, UINT numInstances, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
	m_buildFlags = buildFlags;
	BuildGeometryDescs(geometry, numInstances, baseGeometryTransformGPUAddress);
	ComputePrebuildInfo();
	AllocateResource(device);
	m_isDirty = true;
}

void BottomLevelAccelerationStructure::Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate)
{
	ThrowIfFalse(m_prebuildInfo.ScratchDataSizeInBytes <= scratch->GetDesc().Width, L"Insufficient scratch buffer size provided!");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	{
		bottomLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottomLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		bottomLevelBuildDesc.Flags = m_buildFlags;
		if (bUpdate)
		{
			bottomLevelBuildDesc.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		}
		bottomLevelBuildDesc.ScratchAccelerationStructureData = { scratch->GetGPUVirtualAddress(), scratch->GetDesc().Width };
		bottomLevelBuildDesc.DestAccelerationStructureData = { m_accelerationStructure->GetGPUVirtualAddress(), m_prebuildInfo.ResultDataMaxSizeInBytes };
		bottomLevelBuildDesc.NumDescs = static_cast<UINT>(m_geometryDescs.size());
		bottomLevelBuildDesc.pGeometryDescs = m_geometryDescs.data();
	}

	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
		ID3D12DescriptorHeap *pDescriptorHeaps[] = { descriptorHeap };
		g_pSample->GetFallbackCommandList()->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
		g_pSample->GetFallbackCommandList()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
	}
	else // DirectX Raytracing
	{
		g_pSample->GetDxrCommandList()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
	}
	//ToDo compare perf against BLAS without shared scratch
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure.Get()));

	m_isDirty = false;
}

TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		m_fallbackLayerInstanceDescs.Release();
	}
	else // DirectX Raytracing
	{
		m_dxrInstanceDescs.Release();
	}
}

void TopLevelAccelerationStructure::ComputePrebuildInfo()
{
	// Get the size requirements for the scratch and AS buffers.
	D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
	prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	prebuildInfoDesc.Flags = m_buildFlags;
	prebuildInfoDesc.NumDescs = NumberOfBLAS();

	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		g_pSample->GetFallbackDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
	}
	else // DirectX Raytracing
	{
		g_pSample->GetDxrDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
	}
	ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
}

void TopLevelAccelerationStructure::BuildInstanceDescs(ID3D12Device* device, vector<BottomLevelAccelerationStructure>& vBottomLevelAS, vector<UINT>* bottomLevelASinstanceDescsDescritorHeapIndices)
{
	auto CreateInstanceDescs = [&](auto* structuredBufferInstanceDescs)
	{
		if (structuredBufferInstanceDescs->Size() == 0)
		{
			structuredBufferInstanceDescs->Create(device, static_cast<UINT>(vBottomLevelAS.size()), 1, L"Instance descs.");
		}
		for (UINT i = 0; i < vBottomLevelAS.size(); i++)
		{
			vBottomLevelAS[i].BuildInstanceDesc(&((*structuredBufferInstanceDescs)[i]), &(*bottomLevelASinstanceDescsDescritorHeapIndices)[i]);
		}
		structuredBufferInstanceDescs->CopyStagingToGpu();
	};

	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		CreateInstanceDescs(&m_fallbackLayerInstanceDescs);
	}
	else // DirectX Raytracing
	{
		CreateInstanceDescs(&m_dxrInstanceDescs);
	}
}

void TopLevelAccelerationStructure::UpdateInstanceDescTransforms(vector<BottomLevelAccelerationStructure>& vBottomLevelAS)
{
	auto UpdateTransform = [&](auto* structuredBufferInstanceDescs)
	{
		for (UINT i = 0; i < vBottomLevelAS.size(); i++)
		{
			if (vBottomLevelAS[i].IsDirty())
			{
				StoreXMMatrixAsTransform3x4(
					(*structuredBufferInstanceDescs)[i].Transform,
					vBottomLevelAS[i].GetTransform());
			}
		}
		structuredBufferInstanceDescs->CopyStagingToGpu();
	};

	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		UpdateTransform(&m_fallbackLayerInstanceDescs);
	}
	else // DirectX Raytracing
	{
		UpdateTransform(&m_dxrInstanceDescs);
	}
}

UINT TopLevelAccelerationStructure::NumberOfBLAS()
{
	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		return static_cast<UINT>(m_fallbackLayerInstanceDescs.NumElementsPerInstance());
	}
	else // DirectX Raytracing
	{
		return static_cast<UINT>(m_dxrInstanceDescs.NumElementsPerInstance());
	}
}
void TopLevelAccelerationStructure::Initialize(ID3D12Device* device, vector<BottomLevelAccelerationStructure>& vBottomLevelAS, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, vector<UINT>* bottomLevelASinstanceDescsDescritorHeapIndices)
{
	m_buildFlags = buildFlags;
	BuildInstanceDescs(device, vBottomLevelAS, bottomLevelASinstanceDescsDescritorHeapIndices);
	ComputePrebuildInfo();
	AllocateResource(device);

	// Create a wrapped pointer to the acceleration structure.
	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		UINT numBufferElements = static_cast<UINT>(m_prebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
		m_fallbackAccelerationStructurePointer = g_pSample->CreateFallbackWrappedPointer(m_accelerationStructure.Get(), numBufferElements, &m_fallbackAccelerationStructureDescritorHeapIndex);
	}
}

void TopLevelAccelerationStructure::Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate)
{
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	{
		topLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		topLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		topLevelBuildDesc.Flags = m_buildFlags;
		if (bUpdate)
		{
			topLevelBuildDesc.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		}
		topLevelBuildDesc.ScratchAccelerationStructureData = { scratch->GetGPUVirtualAddress(), scratch->GetDesc().Width };
		topLevelBuildDesc.DestAccelerationStructureData = { m_accelerationStructure->GetGPUVirtualAddress(), m_prebuildInfo.ResultDataMaxSizeInBytes };
		topLevelBuildDesc.NumDescs = NumberOfBLAS();
	}

	if (g_pSample->GetRaytracingAPI() == RaytracingAPI::FallbackLayer)
	{
		topLevelBuildDesc.InstanceDescs = m_fallbackLayerInstanceDescs.GpuVirtualAddress();
		// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
		ID3D12DescriptorHeap *pDescriptorHeaps[] = { descriptorHeap };
		g_pSample->GetFallbackCommandList()->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
		g_pSample->GetFallbackCommandList()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
	}
	else // DirectX Raytracing
	{
		topLevelBuildDesc.InstanceDescs = m_dxrInstanceDescs.GpuVirtualAddress();
		g_pSample->GetDxrCommandList()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
	}
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure.Get()));
}
