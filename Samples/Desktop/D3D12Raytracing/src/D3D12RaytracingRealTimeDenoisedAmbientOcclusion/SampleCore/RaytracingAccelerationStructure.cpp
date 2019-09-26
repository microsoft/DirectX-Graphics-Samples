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
#include "RaytracingAccelerationStructure.h"
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "EngineProfiling.h"

using namespace std;


void AccelerationStructure::ReleaseD3DResources()
{
	m_accelerationStructure.Reset();
}

ID3D12Resource* AccelerationStructure::GetResource()
{
    return m_accelerationStructure.Get();
}

void AccelerationStructure::AllocateResource(ID3D12Device5* device)
{
	// Allocate resource for acceleration structures.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn’t need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    // Buffer resources must have 64KB alignment which satisfies the AS resource requirement to have alignment of 256 (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT).
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
	AllocateUAVBuffer(device, m_prebuildInfo.ResultDataMaxSizeInBytes, &m_accelerationStructure, initialResourceState, m_name.c_str());
}


void BottomLevelAccelerationStructure::UpdateGeometryDescsTransform(D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
{
    struct alignas(16) AlignedGeometryTransform3x4
    {
        float transform3x4[12];
    };

	for (UINT i = 0; i < m_geometryDescs.size(); i++)
	{
		auto& geometryDesc = m_geometryDescs[i];
		geometryDesc.Triangles.Transform3x4 = baseGeometryTransformGPUAddress + i * sizeof(AlignedGeometryTransform3x4);
	}
}

// Build geometry descs for bottom-level AS.
void BottomLevelAccelerationStructure::BuildGeometryDescs(BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry)
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescTemplate = {};
	geometryDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDescTemplate.Triangles.IndexFormat = bottomLevelASGeometry.m_indexFormat;
	geometryDescTemplate.Triangles.VertexFormat = bottomLevelASGeometry.m_vertexFormat;
	m_geometryDescs.reserve(bottomLevelASGeometry.m_geometryInstances.size());

	for (auto& geometry: bottomLevelASGeometry.m_geometryInstances)
	{
		auto& geometryDesc = geometryDescTemplate;
        geometryDescTemplate.Flags = geometry.geometryFlags;
		geometryDesc.Triangles.IndexBuffer = geometry.ib.indexBuffer;
		geometryDesc.Triangles.IndexCount = geometry.ib.count;
		geometryDesc.Triangles.VertexBuffer = geometry.vb.vertexBuffer;
		geometryDesc.Triangles.VertexCount = geometry.vb.count;
		geometryDesc.Triangles.Transform3x4 = geometry.transform;

		m_geometryDescs.push_back(geometryDesc);
	}
}

void BottomLevelAccelerationStructure::ComputePrebuildInfo(ID3D12Device5* device)
{
	// Get the size requirements for the scratch and AS buffers.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
    bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    bottomLevelInputs.Flags = m_buildFlags;
    bottomLevelInputs.NumDescs = static_cast<UINT>(m_geometryDescs.size());
    bottomLevelInputs.pGeometryDescs = m_geometryDescs.data();
	
	device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &m_prebuildInfo);
	ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
}

void BottomLevelAccelerationStructure::Initialize(
	ID3D12Device5* device, 
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, 
    BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry, 
    bool allowUpdate,
    bool bUpdateOnBuild)
{
    m_allowUpdate = allowUpdate;
    m_updateOnBuild = bUpdateOnBuild;

    m_buildFlags = buildFlags;
    m_name = bottomLevelASGeometry.GetName();
    
    if (allowUpdate)
    {
        m_buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    }

	BuildGeometryDescs(bottomLevelASGeometry);
	ComputePrebuildInfo(device);
	AllocateResource(device);

	m_isDirty = true;
    m_isBuilt = false;
}

// The caller must add a UAV barrier before using the resource.
void BottomLevelAccelerationStructure::Build(
    ID3D12GraphicsCommandList4* commandList, 
    ID3D12Resource* scratch, 
    ID3D12DescriptorHeap* descriptorHeap, 
    D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
{
	ThrowIfFalse(m_prebuildInfo.ScratchDataSizeInBytes <= scratch->GetDesc().Width, L"Insufficient scratch buffer size provided!");
	
    if (baseGeometryTransformGPUAddress > 0)
    {
        UpdateGeometryDescsTransform(baseGeometryTransformGPUAddress);
    }

    currentID = (currentID + 1) % Sample::FrameCount;
    m_cacheGeometryDescs[currentID].clear();
    m_cacheGeometryDescs[currentID].resize(m_geometryDescs.size());
    copy(m_geometryDescs.begin(), m_geometryDescs.end(), m_cacheGeometryDescs[currentID].begin());

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	{
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        bottomLevelInputs.Flags = m_buildFlags;
		if (m_isBuilt && m_allowUpdate && m_updateOnBuild)
		{
            bottomLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
            bottomLevelBuildDesc.SourceAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
		}
        bottomLevelInputs.NumDescs = static_cast<UINT>(m_cacheGeometryDescs[currentID].size());
        bottomLevelInputs.pGeometryDescs = m_cacheGeometryDescs[currentID].data();

		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
	}

	commandList->SetDescriptorHeaps(1, &descriptorHeap);
    commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

	m_isDirty = false;
    m_isBuilt = true;
}

void TopLevelAccelerationStructure::ComputePrebuildInfo(ID3D12Device5* device, UINT numBottomLevelASInstanceDescs)
{
	// Get the size requirements for the scratch and AS buffers.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = m_buildFlags;
    topLevelInputs.NumDescs = numBottomLevelASInstanceDescs;

	device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &m_prebuildInfo);
	ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
}

void TopLevelAccelerationStructure::Initialize(
    ID3D12Device5* device, 
    UINT numBottomLevelASInstanceDescs, 
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, 
    bool allowUpdate,
    bool bUpdateOnBuild,
    const wchar_t* resourceName)
{
    m_allowUpdate = allowUpdate;
    m_updateOnBuild = bUpdateOnBuild; 
	m_buildFlags = buildFlags;
    m_name = resourceName;

    if (allowUpdate)
    {
        m_buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    }

	ComputePrebuildInfo(device, numBottomLevelASInstanceDescs);
	AllocateResource(device);

    m_isDirty = true;
    m_isBuilt = false;
}

void TopLevelAccelerationStructure::Build(ID3D12GraphicsCommandList4* commandList, UINT numBottomLevelASInstanceDescs, D3D12_GPU_VIRTUAL_ADDRESS bottomLevelASnstanceDescs, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate)
{
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
    {
        topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelInputs.Flags = m_buildFlags;
        if (m_isBuilt && m_allowUpdate && m_updateOnBuild)
        {
            topLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        }
        topLevelInputs.NumDescs = numBottomLevelASInstanceDescs;

        topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
        topLevelBuildDesc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
    }
    topLevelInputs.InstanceDescs = bottomLevelASnstanceDescs;

    commandList->SetDescriptorHeaps(1, &descriptorHeap);
    commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    m_isDirty = false;
    m_isBuilt = true;
}

RaytracingAccelerationStructureManager::RaytracingAccelerationStructureManager(ID3D12Device5* device, UINT numBottomLevelInstances, UINT frameCount)
{
    m_bottomLevelASInstanceDescs.Create(device, numBottomLevelInstances, frameCount, L"Bottom-Level Acceleration Structure Instance descs.");
}

// Adds a bottom-level Acceleration Structure.
// The passed in bottom-level AS geometry must have a unique name.
// Requires a corresponding 1 or more AddBottomLevelASInstance() calls to be added to the top-level AS for the bottom-level AS to be included.
void RaytracingAccelerationStructureManager::AddBottomLevelAS(
    ID3D12Device5* device,
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
    BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry,
    bool allowUpdate,
    bool performUpdateOnBuild)
{
    ThrowIfFalse(m_vBottomLevelAS.find(bottomLevelASGeometry.GetName()) == m_vBottomLevelAS.end(),
        L"A bottom level acceleration structure with that name already exists.");

    auto& bottomLevelAS = m_vBottomLevelAS[bottomLevelASGeometry.GetName()];

    bottomLevelAS.Initialize(device, buildFlags, bottomLevelASGeometry, allowUpdate);

    m_ASmemoryFootprint += bottomLevelAS.RequiredResultDataSizeInBytes();
    m_scratchResourceSize = max(bottomLevelAS.RequiredScratchSize(), m_scratchResourceSize);

    m_vBottomLevelAS[bottomLevelAS.GetName()] = bottomLevelAS;
}

// Adds an instance of a bottom-level Acceleration Structure.
// Requires a call InitializeTopLevelAS() call to be added to top-level AS.
UINT RaytracingAccelerationStructureManager::AddBottomLevelASInstance(
    const wstring& bottomLevelASname,
    UINT instanceContributionToHitGroupIndex,
    XMMATRIX transform,
    BYTE instanceMask)
{
    ThrowIfFalse(m_numBottomLevelASInstances < m_bottomLevelASInstanceDescs.NumElements(), L"Not enough instance desc buffer size.");

    UINT instanceIndex = m_numBottomLevelASInstances++;
    auto& bottomLevelAS = m_vBottomLevelAS[bottomLevelASname];
    
    auto& instanceDesc = m_bottomLevelASInstanceDescs[instanceIndex];
    instanceDesc.InstanceMask = instanceMask;
    instanceDesc.InstanceContributionToHitGroupIndex = instanceContributionToHitGroupIndex != UINT_MAX ? instanceContributionToHitGroupIndex : bottomLevelAS.GetInstanceContributionToHitGroupIndex();
    instanceDesc.AccelerationStructure = bottomLevelAS.GetResource()->GetGPUVirtualAddress();
    XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), transform);    

    return instanceIndex;
};

UINT RaytracingAccelerationStructureManager::GetMaxInstanceContributionToHitGroupIndex()
{
    UINT maxInstanceContributionToHitGroupIndex = 0;
    for (UINT i = 0; i < m_numBottomLevelASInstances; i++)
    {
        auto& instanceDesc = m_bottomLevelASInstanceDescs[i];
        maxInstanceContributionToHitGroupIndex = max(maxInstanceContributionToHitGroupIndex, instanceDesc.InstanceContributionToHitGroupIndex);
    }
    return maxInstanceContributionToHitGroupIndex;
};

// Initializes the top-level Acceleration Structure.
void RaytracingAccelerationStructureManager::InitializeTopLevelAS(
    ID3D12Device5* device,
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, 
    bool allowUpdate, 
    bool performUpdateOnBuild,
    const wchar_t* resourceName)
{
    m_topLevelAS.Initialize(device, GetNumberOfBottomLevelASInstances(), buildFlags, allowUpdate, performUpdateOnBuild, resourceName);

    m_ASmemoryFootprint += m_topLevelAS.RequiredResultDataSizeInBytes();
    m_scratchResourceSize = max(m_topLevelAS.RequiredScratchSize(), m_scratchResourceSize);

    AllocateUAVBuffer(device, m_scratchResourceSize, &m_accelerationStructureScratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Acceleration structure scratch resource");
}

// Builds all bottom-level and top-level Acceleration Structures.
void RaytracingAccelerationStructureManager::Build(
    ID3D12GraphicsCommandList4* commandList, 
    ID3D12DescriptorHeap* descriptorHeap,
    UINT frameIndex,
    bool bForceBuild)
{
    ScopedTimer _prof(L"Acceleration Structure build", commandList);

    m_bottomLevelASInstanceDescs.CopyStagingToGpu(frameIndex);

    // Build all bottom-level AS.
    {
        ScopedTimer _prof(L"Bottom Level AS", commandList);
        for (auto& bottomLevelASpair : m_vBottomLevelAS)
        {
            auto& bottomLevelAS = bottomLevelASpair.second;
            if (bForceBuild || bottomLevelAS.IsDirty())
            {
                ScopedTimer _prof(bottomLevelAS.GetName(), commandList);

                D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
                bottomLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), descriptorHeap, baseGeometryTransformGpuAddress);

                // Since a single scratch resource is reused, put a barrier in-between each call.
                // PEFORMANCE tip: use separate scratch memory per BLAS build to allow a GPU driver to overlap build calls.
                commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS.GetResource()));
            }
        }
    }
    
    // Build the top-level AS.
    {
        ScopedTimer _prof(L"Top Level AS", commandList);

        bool performUpdate = false; // Always rebuild top-level Acceleration Structure.
        D3D12_GPU_VIRTUAL_ADDRESS instanceDescs = m_bottomLevelASInstanceDescs.GpuVirtualAddress(frameIndex);
        m_topLevelAS.Build(commandList, GetNumberOfBottomLevelASInstances(), instanceDescs, m_accelerationStructureScratch.Get(), descriptorHeap, performUpdate);

        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_topLevelAS.GetResource()));
    }
}

void BottomLevelAccelerationStructureInstanceDesc::SetTransform(const XMMATRIX& transform)
{
    XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(Transform), transform);
}

void BottomLevelAccelerationStructureInstanceDesc::GetTransform(XMMATRIX* transform)
{
    *transform = XMLoadFloat3x4(reinterpret_cast<XMFLOAT3X4*>(Transform));
}