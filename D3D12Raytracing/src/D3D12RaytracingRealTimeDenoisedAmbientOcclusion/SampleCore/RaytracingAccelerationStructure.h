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

#include "RayTracingHlslCompat.h"
#include "RaytracingSceneDefines.h"

struct AccelerationStructureBuffers
{
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> accelerationStructure;
    ComPtr<ID3D12Resource> instanceDesc;    // Used only for top-level AS
    UINT64                 ResultDataMaxSizeInBytes;
};

// AccelerationStructure
// A base class for bottom-level and top-level AS.
class AccelerationStructure
{
public:
    AccelerationStructure() {}
	virtual ~AccelerationStructure() {}
	void ReleaseD3DResources();
	UINT64 RequiredScratchSize() { return std::max(m_prebuildInfo.ScratchDataSizeInBytes, m_prebuildInfo.UpdateScratchDataSizeInBytes); }
	UINT64 RequiredResultDataSizeInBytes() { return m_prebuildInfo.ResultDataMaxSizeInBytes; }
    ID3D12Resource* GetResource();
	const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo() { return m_prebuildInfo; }
    const std::wstring& GetName() { return m_name; }

    void SetDirty(bool isDirty) { m_isDirty = isDirty; }
    bool IsDirty() { return m_isDirty; }
    UINT64 ResourceSize() { return GetResource()->GetDesc().Width; }

protected:
    ComPtr<ID3D12Resource> m_accelerationStructure;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_prebuildInfo = {};
    std::wstring m_name;

    bool m_isBuilt = false; // whether the AS has been built at least once.
    bool m_isDirty = true; // whether the AS has been modified and needs to be rebuilt.
    bool m_updateOnBuild = false;
    bool m_allowUpdate = false;

	void AllocateResource(ID3D12Device5* device);
};

class BottomLevelAccelerationStructureGeometry
{
public:
    std::wstring                    m_name;
    std::vector<GeometryInstance>	m_geometryInstances;
    std::vector<D3DGeometry>        m_geometries;
    std::vector<D3DTexture>         m_textures;
    UINT                            m_numTriangles = 0;
    DXGI_FORMAT                     m_indexFormat = DXGI_FORMAT_UNKNOWN;
    UINT                            m_ibStrideInBytes = 0;
    DXGI_FORMAT                     m_vertexFormat = DXGI_FORMAT_UNKNOWN;
    UINT                            m_vbStrideInBytes = 0;

    BottomLevelAccelerationStructureGeometry() {}
    BottomLevelAccelerationStructureGeometry(const wchar_t* name) : m_name(name) {}
    BottomLevelAccelerationStructureGeometry(const std::wstring& name) : m_name(name) {}

    void SetName(const std::wstring& name) { m_name = name; }
    const std::wstring& GetName() const { return m_name; }
};

class BottomLevelAccelerationStructure : public AccelerationStructure
{
public:
    BottomLevelAccelerationStructure() {};
	~BottomLevelAccelerationStructure() {}

    void Initialize(ID3D12Device5* device, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry, bool allowUpdate = false, bool bUpdateOnBuild = false);
    void Build(ID3D12GraphicsCommandList4* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress = 0);

    void UpdateGeometryDescsTransform(D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress);
    
    UINT GetInstanceContributionToHitGroupIndex() { return m_instanceContributionToHitGroupIndex; }
    void SetInstanceContributionToHitGroupIndex(UINT index) { m_instanceContributionToHitGroupIndex = index; }

	const XMMATRIX& GetTransform() { return m_transform; }
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& GetGeometryDescs() { return m_geometryDescs; }

private:
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_geometryDescs;
    UINT currentID = 0; 
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_cacheGeometryDescs[3];
    DirectX::XMMATRIX m_transform;
    UINT m_instanceContributionToHitGroupIndex = 0;

	void BuildGeometryDescs(BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry);
	void ComputePrebuildInfo(ID3D12Device5* device);
};

class TopLevelAccelerationStructure : public AccelerationStructure
{
public:
	TopLevelAccelerationStructure() {}
    ~TopLevelAccelerationStructure() {}

	void Initialize(ID3D12Device5* device, UINT numBottomLevelASInstanceDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate = false, bool bUpdateOnBuild = false, const wchar_t* resourceName = nullptr);
	void Build(ID3D12GraphicsCommandList4* commandList, UINT numInstanceDescs, D3D12_GPU_VIRTUAL_ADDRESS InstanceDescs, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false);

private:
	void ComputePrebuildInfo(ID3D12Device5* device, UINT numBottomLevelASInstanceDescs);
};


struct BottomLevelAccelerationStructureInstanceDesc : public D3D12_RAYTRACING_INSTANCE_DESC
{
    void SetTransform(const DirectX::XMMATRIX& transform);
    void GetTransform(DirectX::XMMATRIX* transform);
};
static_assert(sizeof(BottomLevelAccelerationStructureInstanceDesc) == sizeof(D3D12_RAYTRACING_INSTANCE_DESC), L"This is a wrapper used in place of the desc. It has to have the same size");


class RaytracingAccelerationStructureManager
{
public:
    RaytracingAccelerationStructureManager(ID3D12Device5* device, UINT numBottomLevelInstances, UINT frameCount);
    ~RaytracingAccelerationStructureManager() {}

    void AddBottomLevelAS(ID3D12Device5* device, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry, bool allowUpdate = false, bool performUpdateOnBuild = false);
    UINT AddBottomLevelASInstance(const std::wstring& bottomLevelASname, UINT instanceContributionToHitGroupIndex = UINT_MAX, XMMATRIX transform = XMMatrixIdentity(), BYTE InstanceMask = 1);
    void InitializeTopLevelAS(ID3D12Device5* device, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate = false, bool performUpdateOnBuild = false, const wchar_t* resourceName = nullptr);
    void Build(ID3D12GraphicsCommandList4* commandList, ID3D12DescriptorHeap* descriptorHeap, UINT frameIndex, bool bForceBuild = false);
    BottomLevelAccelerationStructureInstanceDesc& GetBottomLevelASInstance(UINT bottomLevelASinstanceIndex) { return m_bottomLevelASInstanceDescs[bottomLevelASinstanceIndex]; }
    const StructuredBuffer<BottomLevelAccelerationStructureInstanceDesc>& GetBottomLevelASInstancesBuffer() { return m_bottomLevelASInstanceDescs; }

    BottomLevelAccelerationStructure& GetBottomLevelAS(const std::wstring& name) { return m_vBottomLevelAS[name]; }
    ID3D12Resource* GetTopLevelASResource() { return m_topLevelAS.GetResource(); }
    UINT64 GetASMemoryFootprint() { return m_ASmemoryFootprint; }
    UINT GetNumberOfBottomLevelASInstances() { return static_cast<UINT>(m_bottomLevelASInstanceDescs.NumElements()); }
    UINT GetMaxInstanceContributionToHitGroupIndex();

private:
    TopLevelAccelerationStructure m_topLevelAS;
    std::map<std::wstring, BottomLevelAccelerationStructure> m_vBottomLevelAS;
    StructuredBuffer<BottomLevelAccelerationStructureInstanceDesc> m_bottomLevelASInstanceDescs;
    UINT m_numBottomLevelASInstances = 0;
    ComPtr<ID3D12Resource>	m_accelerationStructureScratch;
    UINT64 m_scratchResourceSize = 0;
    UINT64 m_ASmemoryFootprint = 0;
};