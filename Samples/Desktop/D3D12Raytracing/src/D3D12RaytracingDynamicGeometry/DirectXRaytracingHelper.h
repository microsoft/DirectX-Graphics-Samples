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
// ToDo move code definitions to cpp
// ToDo Remove
#include "RayTracingHlslCompat.h"

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
	D3D12_GPU_VIRTUAL_ADDRESS transform;
};

enum class RaytracingAPI {
	FallbackLayer,
	DirectXRaytracing,
};

class D3D12RaytracingDynamicGeometry;
extern D3D12RaytracingDynamicGeometry* g_pSample;

struct alignas(16) AlignedGeometryTransform3x4
{
	float transform3x4[12];
};

// AccelerationStructure
// A base class for bottom-level and top-level AS.
class AccelerationStructure
{
protected:
	ComPtr<ID3D12Resource> m_accelerationStructure;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_buildFlags;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_prebuildInfo;
	
public:
	AccelerationStructure();
	virtual ~AccelerationStructure() {}
	void ReleaseD3DResources();
    UINT64 RequiredScratchSize() { return max(m_prebuildInfo.ScratchDataSizeInBytes, m_prebuildInfo.UpdateScratchDataSizeInBytes); }
    UINT64 RequiredResultDataSizeInBytes() { return m_prebuildInfo.ResultDataMaxSizeInBytes; }
	ID3D12Resource* GetResource() { return m_accelerationStructure.Get(); }
	const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo() { return m_prebuildInfo; }
	virtual void Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false) = 0;

protected:
	void AllocateResource(ID3D12Device* device);
};

class BottomLevelAccelerationStructure : public AccelerationStructure
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_geometryDescs;
	DirectX::XMMATRIX m_transform;

	// Runtime state
	bool m_isDirty;		// if true, AS requires an update/build.

public:
	BottomLevelAccelerationStructure();
	~BottomLevelAccelerationStructure() {}
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>* GetGeometryDescs() { return &m_geometryDescs; }
	
	// ToDo:
	// UpdateGeometry()

	void Initialize(ID3D12Device* device, const TriangleGeometryBuffer& geometry, UINT numInstances, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags);
	void Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false);
	void BuildInstanceDesc(void* destInstanceDesc, UINT* descriptorHeapIndex);
	void SetTransform(const DirectX::XMMATRIX& transform)
	{
		m_transform = transform;
		SetDirty(true);
	}
	void SetDirty(bool isDirty) { m_isDirty = isDirty; }
	bool IsDirty() { return m_isDirty; }

	const XMMATRIX& GetTransform() { return m_transform; }

private:
	void BuildGeometryDescs(const TriangleGeometryBuffer& geometry, UINT numInstances, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress);
	void ComputePrebuildInfo();
};

class TopLevelAccelerationStructure : public AccelerationStructure
{
	union {
		// ToDo handle release when API is being changed
		// Descruction is  not safe if API changes before release
		StructuredBuffer<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC> m_fallbackLayerInstanceDescs;
		StructuredBuffer<D3D12_RAYTRACING_INSTANCE_DESC> m_dxrInstanceDescs;
	};

public:
	TopLevelAccelerationStructure() : m_fallbackAccelerationStructureDescritorHeapIndex(UINT_MAX) {}
	~TopLevelAccelerationStructure();

	UINT NumberOfBLAS();

	void Initialize(ID3D12Device* device, std::vector<BottomLevelAccelerationStructure>& vBottomLevelAS, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, std::vector<UINT>* bottomLevelASinstanceDescsDescritorHeapIndices);
	void Build(ID3D12GraphicsCommandList* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool bUpdate = false);
	void UpdateInstanceDescTransforms(std::vector<BottomLevelAccelerationStructure>& vBottomLevelAS);

	const WRAPPED_GPU_POINTER& GetFallbackAccelerationStructurePointer() { return m_fallbackAccelerationStructurePointer; }

private:
	void ComputePrebuildInfo();
	void BuildInstanceDescs(ID3D12Device* device, std::vector<BottomLevelAccelerationStructure>& vBottomLevelAS, std::vector<UINT>* bottomLevelASinstanceDescsDescritorHeapIndices);

	UINT m_fallbackAccelerationStructureDescritorHeapIndex;
	WRAPPED_GPU_POINTER m_fallbackAccelerationStructurePointer;
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