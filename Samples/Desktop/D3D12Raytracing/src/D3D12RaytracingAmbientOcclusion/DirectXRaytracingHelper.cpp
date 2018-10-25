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
#include "D3D12RaytracingAmbientOcclusion.h"

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
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
	AllocateUAVBuffer(device, m_prebuildInfo.ResultDataMaxSizeInBytes, &m_accelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
}


BottomLevelAccelerationStructure::BottomLevelAccelerationStructure() :
	m_isDirty(true),
	m_transform(XMMatrixIdentity()),
    m_instanceContributionToHitGroupIndex(0)
{
}

void BottomLevelAccelerationStructure::BuildInstanceDesc(void* destInstanceDesc, UINT* descriptorHeapIndex)
{
	auto InitializeInstanceDesc = [&](auto* instanceDesc, auto bottomLevelAddress)
	{
		*instanceDesc = {};
		instanceDesc->InstanceMask = 1;
		instanceDesc->InstanceContributionToHitGroupIndex = m_instanceContributionToHitGroupIndex;
		instanceDesc->AccelerationStructure = bottomLevelAddress;
        XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc->Transform), m_transform);
	};

	InitializeInstanceDesc(static_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(destInstanceDesc), m_accelerationStructure->GetGPUVirtualAddress());
};

void BottomLevelAccelerationStructure::UpdateGeometryDescsTransform(D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
{
	for (UINT i = 0; i < m_geometryDescs.size(); i++)
	{
		auto& geometryDesc = m_geometryDescs[i];
		geometryDesc.Triangles.Transform3x4 = baseGeometryTransformGPUAddress + i * sizeof(AlignedGeometryTransform3x4);
	}
}

// Build geometry descs for bottom-level AS.
void BottomLevelAccelerationStructure::BuildGeometryDescs(DXGI_FORMAT indexFormat, UINT ibStrideInBytes, UINT vbStrideInBytes, vector<GeometryInstance>& geometries)
{
	// ToDo pass geometry flag from the sample cpp
	// Mark the geometry as opaque. 
	// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
	// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
	D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescTemplate = {};
	geometryDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDescTemplate.Triangles.IndexFormat = indexFormat;
	geometryDescTemplate.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDescTemplate.Flags = geometryFlags;

	m_geometryDescs.reserve(geometries.size());

	for (auto& geometry: geometries)
	{
		auto& geometryDesc = geometryDescTemplate;
		geometryDesc.Triangles.IndexBuffer = geometry.ib.indexBuffer;
		geometryDesc.Triangles.IndexCount = geometry.ib.count;
		geometryDesc.Triangles.VertexBuffer = geometry.vb.vertexBuffer;
		geometryDesc.Triangles.VertexCount = geometry.vb.count;

		m_geometryDescs.push_back(geometryDesc);
	}
}

void BottomLevelAccelerationStructure::ComputePrebuildInfo()
{
	// Get the size requirements for the scratch and AS buffers.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
    bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    bottomLevelInputs.Flags = m_buildFlags;
    bottomLevelInputs.NumDescs = static_cast<UINT>(m_geometryDescs.size());
    bottomLevelInputs.pGeometryDescs = m_geometryDescs.data();
	
	g_pSample->GetDxrDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &m_prebuildInfo);
	ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
}

void BottomLevelAccelerationStructure::Initialize(
	ID3D12Device* device, 
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, 
	DXGI_FORMAT indexFormat, 
	UINT ibStrideInBytes, 
	UINT vbStrideInBytes, 
	vector<GeometryInstance>& geometries)
{
	m_buildFlags = buildFlags;
	BuildGeometryDescs(indexFormat, ibStrideInBytes, vbStrideInBytes, geometries);
	ComputePrebuildInfo();
	AllocateResource(device);
	m_isDirty = true;
}

void BottomLevelAccelerationStructure::Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress, bool bUpdate)
{
	ThrowIfFalse(m_prebuildInfo.ScratchDataSizeInBytes <= scratch->GetDesc().Width, L"Insufficient scratch buffer size provided!");
	
    if (baseGeometryTransformGPUAddress > 0)
    {
        UpdateGeometryDescsTransform(baseGeometryTransformGPUAddress);
    }

    // ToDo cleanup
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	{
        // ToDo remove repeating BOTTOM_LEVEL flag specification
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        bottomLevelInputs.Flags = m_buildFlags;
		if (bUpdate)
		{
            bottomLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		}
        bottomLevelInputs.NumDescs = static_cast<UINT>(m_geometryDescs.size());
        bottomLevelInputs.pGeometryDescs = m_geometryDescs.data();

		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
	}

	g_pSample->GetDxrCommandList()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure.Get()));

	m_isDirty = false;
}

TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
	m_dxrInstanceDescs.Release();
}

void TopLevelAccelerationStructure::ComputePrebuildInfo()
{
	// Get the size requirements for the scratch and AS buffers.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = m_buildFlags;
    topLevelInputs.NumDescs = NumberOfBLAS();

	g_pSample->GetDxrDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &m_prebuildInfo);
	ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
}

void TopLevelAccelerationStructure::BuildInstanceDescs(ID3D12Device* device, vector<BottomLevelAccelerationStructure>& vBottomLevelAS, vector<UINT>* bottomLevelASinstanceDescsDescritorHeapIndices)
{
	auto CreateInstanceDescs = [&](auto* structuredBufferInstanceDescs)
	{
		if (structuredBufferInstanceDescs->Size() != vBottomLevelAS.size())
		{
			structuredBufferInstanceDescs->Create(device, static_cast<UINT>(vBottomLevelAS.size()), g_pSample->GetDeviceResources().GetBackBufferCount(), L"Instance descs.");
		}
		for (UINT i = 0; i < vBottomLevelAS.size(); i++)
		{
			vBottomLevelAS[i].BuildInstanceDesc(&((*structuredBufferInstanceDescs)[i]), &(*bottomLevelASinstanceDescsDescritorHeapIndices)[i]);
		}
		// Initialize all gpu instances, in case we do only partial runtime updates.
		for (UINT i = 0; i < g_pSample->GetDeviceResources().GetBackBufferCount(); i++)
		{
			structuredBufferInstanceDescs->CopyStagingToGpu(i);
		}
	}; 
	CreateInstanceDescs(&m_dxrInstanceDescs);
}

void TopLevelAccelerationStructure::UpdateInstanceDescTransforms(vector<BottomLevelAccelerationStructure>& vBottomLevelAS)
{

	for (UINT i = 0; i < vBottomLevelAS.size(); i++)
	{
		// ToDo should the transform be stored inside TLAS?
		if (vBottomLevelAS[i].IsDirty())
		{
            XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_dxrInstanceDescs[i].Transform), vBottomLevelAS[i].GetTransform());
		}
	}
	// ToDo do per instance copies instead?
	m_dxrInstanceDescs.CopyStagingToGpu(g_pSample->GetDeviceResources().GetCurrentFrameIndex());	
}

UINT TopLevelAccelerationStructure::NumberOfBLAS()
{
	return static_cast<UINT>(m_dxrInstanceDescs.NumElementsPerInstance());
}

void TopLevelAccelerationStructure::Initialize(ID3D12Device* device, vector<BottomLevelAccelerationStructure>& vBottomLevelAS, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, vector<UINT>* bottomLevelASinstanceDescsDescritorHeapIndices)
{
	m_buildFlags = buildFlags;
	BuildInstanceDescs(device, vBottomLevelAS, bottomLevelASinstanceDescsDescritorHeapIndices);
	ComputePrebuildInfo();
	AllocateResource(device);
}

void TopLevelAccelerationStructure::Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate)
{
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
    {
        // ToDo remove repeating TOP_LEVEL flag specification
        topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelInputs.Flags = m_buildFlags;
        if (bUpdate)
        {
            topLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        }
        topLevelInputs.NumDescs = NumberOfBLAS();

        topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
        topLevelBuildDesc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
    }

    topLevelInputs.InstanceDescs = m_dxrInstanceDescs.GpuVirtualAddress();
	g_pSample->GetDxrCommandList()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure.Get()));
}
