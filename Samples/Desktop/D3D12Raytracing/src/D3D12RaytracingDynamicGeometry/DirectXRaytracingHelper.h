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

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)
template <class T>
inline T Clamp(T value, T minValue, T maxValue)
{
	return max(minValue, min(maxValue, value));
}

struct AccelerationStructureBuffers
{
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> accelerationStructure;
    ComPtr<ID3D12Resource> instanceDesc;    // Used only for top-level AS
    UINT64                 ResultDataMaxSizeInBytes;
};


inline void AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource **ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
}
struct TriangleGeometryBuffer
{
	D3DBuffer ib;
	D3DBuffer vb;
};

enum class RaytracingAPI {
	FallbackLayer,
	DirectXRaytracing,
};

// ToDo
struct RaytracingRuntime
{
	RaytracingAPI API;

	// Raytracing Fallback Layer (FL) attributes
	ComPtr<ID3D12RaytracingFallbackDevice> fallbackDevice;
	ComPtr<ID3D12RaytracingFallbackCommandList> fallbackCommandList;

	// DirectX Raytracing (DXR) attributes
	ComPtr<ID3D12DeviceRaytracingPrototype> dxrDevice;
	ComPtr<ID3D12CommandListRaytracingPrototype> dxrCommandList;

};

static RaytracingRuntime g_raytracingRuntime;


class AccelerationStructure
{
protected:
	bool m_isDirty;		// requires an update/rebuild
	static ComPtr<ID3D12Resource> scratch;
	ComPtr<ID3D12Resource> m_accelerationStructure;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_buildFlags;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_prebuildInfo;
	XMMATRIX m_transform;

public:
	AccelerationStructure() {};
	void Reset() {};
	UINT64 RequiredScratchSize() { return m_prebuildInfo.ScratchDataSizeInBytes; }
	ID3D12Resource* GetResource() { return m_accelerationStructure.Get(); }
	virtual void Build(ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false) = 0;
	void SetTransform(const XMMATRIX& transform)
	{
		// ToDo
		m_isDirty = true;
	}
	bool IsDirty() { return m_isDirty; }


	void AllocateResource(ID3D12Device* device)
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
			if (g_raytracingRuntime.API == RaytracingAPI::FallbackLayer)
			{
				initialResourceState = g_raytracingRuntime.fallbackDevice->GetAccelerationStructureResourceState();
			}
			else // DirectX Raytracing
			{
				initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
			}
			AllocateUAVBuffer(device, m_prebuildInfo.ResultDataMaxSizeInBytes, &m_accelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
		}
	}
};
class D3D12RaytracingDynamicGeometry;
D3D12RaytracingDynamicGeometry* pSample;


class BottomLevelAccelerationStructure : public AccelerationStructure
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_geometryDescs;
public:
	BottomLevelAccelerationStructure() {}
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>* GetGeometryDescs() { return &m_geometryDescs; }
private:
	void AddGeometry(const TriangleGeometryBuffer& geometry)
	{
		m_geometryDescs.push_back(D3D12_RAYTRACING_GEOMETRY_DESC{});
		auto& geometryDesc = m_geometryDescs.back();
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(GeometricPrimitive::VertexType);
		// Mark the geometry as opaque. 
		// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
		// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometryDesc.Triangles.IndexBuffer = geometry.ib.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.IndexCount = static_cast<UINT>(geometry.ib.resource->GetDesc().Width) / sizeof(Index);
		geometryDesc.Triangles.VertexBuffer.StartAddress = geometry.vb.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexCount = static_cast<UINT>(geometry.vb.resource->GetDesc().Width) / sizeof(GeometricPrimitive::VertexType);
	}

	// Build geometry descs for bottom-level AS.
	void SetGeometries(const std::vector<TriangleGeometryBuffer>& geometries)
	{
		// Mark the geometry as opaque. 
		// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
		// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
		D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		D3D12_RAYTRACING_GEOMETRY_DESC geometryDescTemplate = {};
		geometryDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDescTemplate.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
		geometryDescTemplate.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDescTemplate.Triangles.VertexBuffer.StrideInBytes = sizeof(GeometricPrimitive::VertexType);
		geometryDescTemplate.Flags = geometryFlags;
		m_geometryDescs.resize(geometries.size(), geometryDescTemplate);

		for (UINT i = 0; i < geometries.size(); i++)
		{
			auto& geometry = geometries[i];
			auto& geometryDesc = m_geometryDescs[i];
			geometryDesc.Triangles.IndexBuffer = geometry.ib.resource->GetGPUVirtualAddress();
			geometryDesc.Triangles.IndexCount = static_cast<UINT>(geometry.ib.resource->GetDesc().Width) / sizeof(Index);
			geometryDesc.Triangles.VertexBuffer.StartAddress = geometry.vb.resource->GetGPUVirtualAddress();
			geometryDesc.Triangles.VertexCount = static_cast<UINT>(geometry.vb.resource->GetDesc().Width) / sizeof(GeometricPrimitive::VertexType);
		}
	}

	void ComputePrebuildInfo()
	{
		// Get the size requirements for the scratch and AS buffers.
		D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
		prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		prebuildInfoDesc.Flags = m_buildFlags;

		prebuildInfoDesc.NumDescs = static_cast<UINT>(m_geometryDescs.size());
		prebuildInfoDesc.pGeometryDescs = m_geometryDescs.data();


		if (g_raytracingRuntime.API == RaytracingAPI::FallbackLayer)
		{
			g_raytracingRuntime.fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
		}
		else // DirectX Raytracing
		{
			g_raytracingRuntime.dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
		}
		ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
	}

public:

	void Initialize(ID3D12Device* device, const std::vector<TriangleGeometryBuffer>& geometries, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
	{
		m_buildFlags = buildFlags;
		SetGeometries(geometries);
		ComputePrebuildInfo();
		AllocateResource(device);
		m_isDirty = true;
	}

	void Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false)
	{
		ThrowIfFalse(m_prebuildInfo.ScratchDataSizeInBytes <= scratch->GetDesc().Width, L"Insufficient scratch buffer size provided!" );

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

		if (g_raytracingRuntime.API == RaytracingAPI::FallbackLayer)
		{
			// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
			ID3D12DescriptorHeap *pDescriptorHeaps[] = { descriptorHeap };
			g_raytracingRuntime.fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
			g_raytracingRuntime.fallbackCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
		}
		else // DirectX Raytracing
		{
			g_raytracingRuntime.dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
		}
		//ToDo compare perf against BLAS without shared scrathc
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure.Get()));
		
		m_isDirty = false;
	}

	void CopyInstanceDescTo(ID3D12Device* device, void* destInstanceDesc);

};

class TopLevelAccelerationStructure : public AccelerationStructure
{
	//StructuredBuffer<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC> m_instanceDescs;
	//StructuredBuffer<D3D12_RAYTRACING_INSTANCE_DESC> m_instanceDescs;
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> m_instanceDescs;

public:
	TopLevelAccelerationStructure() {}
private:

	void ComputePrebuildInfo()
	{
		// Get the size requirements for the scratch and AS buffers.
		D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
		prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		prebuildInfoDesc.Flags = m_buildFlags;
		prebuildInfoDesc.NumDescs = static_cast<UINT>(m_instanceDescs.size());

		if (g_raytracingRuntime.API == RaytracingAPI::FallbackLayer)
		{
			g_raytracingRuntime.fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
		}
		else // DirectX Raytracing
		{
			g_raytracingRuntime.dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &m_prebuildInfo);
		}
		ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0);
	}

	void BuildInstanceDescs(std::vector<BottomLevelAccelerationStructure>& vBottomLevelAS)
	{
	//ToDo
	}

public:
	void Initialize(ID3D12Device* device, std::vector<BottomLevelAccelerationStructure>& vBottomLevelAS, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
	{
		m_buildFlags = buildFlags;
		ComputePrebuildInfo();
		BuildInstanceDescs(vBottomLevelAS);
		AllocateResource(device);
		m_isDirty = true;
	}

	void Build(ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false)
	{
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

		if (g_raytracingRuntime.API == RaytracingAPI::FallbackLayer)
		{
			// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
			ID3D12DescriptorHeap *pDescriptorHeaps[] = { descriptorHeap };
			g_raytracingRuntime.fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
			g_raytracingRuntime.fallbackCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
		}
		else // DirectX Raytracing
		{
			g_raytracingRuntime.dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
		}

		m_isDirty = false;
	}
};

// Shader record = {{Shader ID}, {RootArguments}}
class ShaderRecord
{
public:
    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
    {
    }

    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
        localRootArguments(pLocalRootArguments, localRootArgumentsSize)
    {
    }

    void CopyTo(void* dest) const
    {
        uint8_t* byteDest = static_cast<uint8_t*>(dest);
        memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
        if (localRootArguments.ptr)
        {
            memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
        }
    }

    struct PointerWithSize {
        void *ptr;
        UINT size;

        PointerWithSize() : ptr(nullptr), size(0) {}
        PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
    };
    PointerWithSize shaderIdentifier;
    PointerWithSize localRootArguments;
};

// Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}
class ShaderTable : public GpuUploadBuffer
{
    uint8_t* m_mappedShaderRecords;
    UINT m_shaderRecordSize;

    // Debug support
    std::wstring m_name;
    std::vector<ShaderRecord> m_shaderRecords;

    ShaderTable() {}
public:
    ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr) 
        : m_name(resourceName)
    {
        m_shaderRecordSize = Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
        m_shaderRecords.reserve(numShaderRecords);
        UINT bufferSize = numShaderRecords * m_shaderRecordSize;
        Allocate(device, bufferSize, resourceName);
        m_mappedShaderRecords = MapCpuWriteOnly();
    }
    
    void push_back(const ShaderRecord& shaderRecord)
    {
        ThrowIfFalse(m_shaderRecords.size() < m_shaderRecords.capacity());
        m_shaderRecords.push_back(shaderRecord);
        shaderRecord.CopyTo(m_mappedShaderRecords);
        m_mappedShaderRecords += m_shaderRecordSize;
    }

    UINT GetShaderRecordSize() { return m_shaderRecordSize; }

    // Pretty-print the shader records.
    void DebugPrint(std::unordered_map<void*, std::wstring> shaderIdToStringMap)
    {
        std::wstringstream wstr;
        wstr << L"|--------------------------------------------------------------------\n";
        wstr << L"|Shader table - " << m_name.c_str() << L": " 
             << m_shaderRecordSize << L" | "
             << m_shaderRecords.size() * m_shaderRecordSize << L" bytes\n";

        for (UINT i = 0; i < m_shaderRecords.size(); i++)
        {
            wstr << L"| [" << i << L"]: ";
            wstr << shaderIdToStringMap[m_shaderRecords[i].shaderIdentifier.ptr] << L", ";
            wstr << m_shaderRecords[i].shaderIdentifier.size << L" + " << m_shaderRecords[i].localRootArguments.size << L" bytes \n";
        }
        wstr << L"|--------------------------------------------------------------------\n";
        wstr << L"\n";
        OutputDebugStringW(wstr.str().c_str());
    }
};


template<class T, size_t N>
void DefineExports(T* obj, LPCWSTR(&Exports)[N])
{
    for (UINT i = 0; i < N; i++)
    {
        obj->DefineExport(Exports[i]);
    }
}

template<class T, size_t N, size_t M>
void DefineExports(T* obj, LPCWSTR(&Exports)[N][M])
{
    for (UINT i = 0; i < N; i++)
        for (UINT j = 0; j < M; j++)
        {
            obj->DefineExport(Exports[i][j]);
        }
}


inline void AllocateUploadBuffer(ID3D12Device* pDevice, void *pData, UINT64 datasize, ID3D12Resource **ppResource, const wchar_t* resourceName = nullptr)
{
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
    ThrowIfFailed(pDevice->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(ppResource)));
    if (resourceName)
    {
        (*ppResource)->SetName(resourceName);
    }
    void *pMappedData;
    (*ppResource)->Map(0, nullptr, &pMappedData);
    memcpy(pMappedData, pData, datasize);
    (*ppResource)->Unmap(0, nullptr);
}

// Pretty-print a state object tree.
inline void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
    std::wstringstream wstr;
    wstr << L"\n";
    wstr << L"--------------------------------------------------------------------\n";
    wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

    auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
    {
        std::wostringstream woss;
        for (UINT i = 0; i < numExports; i++)
        {
            woss << L"|";
            if (depth > 0)
            {
                for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
            }
            woss << L" [" << i << L"]: ";
            if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
			woss << exports[i].Name << L"\n";
        }
        return woss.str();
    };

    for (UINT i = 0; i < desc->NumSubobjects; i++)
    {
        wstr << L"| [" << i << L"]: ";
        switch (desc->pSubobjects[i].Type)
        {
        case D3D12_STATE_SUBOBJECT_TYPE_FLAGS:
            wstr << L"Flags (not yet defined)\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE:
            wstr << L"Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
            wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
            wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_CACHED_STATE_OBJECT:
            wstr << L"Cached State Object (not yet defined)\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
        {
            wstr << L"DXIL Library 0x";
            auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
            wstr << ExportTree(1, lib->NumExports, lib->pExports);
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
        {
            wstr << L"Existing Library 0x";
            auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << collection->pExistingCollection << L"\n";
            wstr << ExportTree(1, collection->NumExports, collection->pExports);
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            wstr << L"Subobject to Exports Association (Subobject [";
            auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
            wstr << index << L"])\n";
            for (UINT j = 0; j < association->NumExports; j++)
            {
                wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
            }
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            wstr << L"DXIL Subobjects to Exports Association (";
            auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            wstr << association->SubobjectToAssociate << L")\n";
            for (UINT j = 0; j < association->NumExports; j++)
            {
                wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
            }
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
        {
            wstr << L"Raytracing Shader Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
            wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
        {
            wstr << L"Raytracing Pipeline Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
        {
            wstr << L"Hit Group (";
            auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
            wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
            wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
            wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
            break;
        }
        }
        wstr << L"|--------------------------------------------------------------------\n";
    }
    wstr << L"\n";
    OutputDebugStringW(wstr.str().c_str());
}

// Enable experimental features required for compute-based raytracing fallback.
// This will set active D3D12 devices to DEVICE_REMOVED state.
// Returns bool whether the call succeeded and the device supports the feature.
inline bool EnableComputeRaytracingFallback(IDXGIAdapter1* adapter)
{
	ComPtr<ID3D12Device> testDevice;
	UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels };

	return SUCCEEDED(D3D12EnableExperimentalFeatures(1, experimentalFeatures, nullptr, nullptr))
		&& SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)));
}

// Enable experimental features required for driver and compute-based fallback raytracing.
// This will set active D3D12 devices to DEVICE_REMOVED state.
// Returns bool whether the call succeeded and the device supports the feature.
inline bool EnableRaytracing(IDXGIAdapter1* adapter)
{
	ComPtr<ID3D12Device> testDevice;
	ComPtr<ID3D12DeviceRaytracingPrototype> testRaytracingDevice;
	UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels, D3D12RaytracingPrototype };

	return SUCCEEDED(D3D12EnableExperimentalFeatures(2, experimentalFeatures, nullptr, nullptr))
		&& SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)))
		&& SUCCEEDED(testDevice->QueryInterface(IID_PPV_ARGS(&testRaytracingDevice)));
}

inline void StoreXMMatrixAsTransform3x4
(
    float transform3x4[12],
    const XMMATRIX& m
)
{
    XMMATRIX mT = XMMatrixTranspose(m); // convert row-major to column-major
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&transform3x4[0]), mT.r[0]);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&transform3x4[4]), mT.r[1]);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&transform3x4[8]), mT.r[2]);
}