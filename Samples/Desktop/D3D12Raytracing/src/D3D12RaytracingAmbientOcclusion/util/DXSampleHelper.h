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

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
using Microsoft::WRL::ComPtr;

class HrException : public std::runtime_error
{
    inline std::string HrToString(HRESULT hr)
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
        return std::string(s_str);
    }
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

inline void ThrowIfFailed(HRESULT hr, const wchar_t* msg)
{
    if (FAILED(hr))
    {
        OutputDebugString(msg);
        throw HrException(hr);
    }
}

template<typename... Args>
inline void ThrowIfFailed(HRESULT hr, const wchar_t* format, Args... args)
{
	if (FAILED(hr))
	{
		WCHAR msg[128];
		swprintf_s(msg, format, args...);
		OutputDebugString(msg);
		throw HrException(hr);
	}
}

inline void ThrowIfFalse(bool value)
{
    ThrowIfFailed(value ? S_OK : E_FAIL);
}

inline void ThrowIfFalse(bool value, const wchar_t* msg)
{
    ThrowIfFailed(value ? S_OK : E_FAIL, msg);
}


inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
    if (path == nullptr)
    {
        throw std::exception();
    }

    DWORD size = GetModuleFileName(nullptr, path, pathSize);
    if (size == 0 || size == pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }
}

inline HRESULT ReadDataFromFile(LPCWSTR filename, byte** data, UINT* size, bool rhCoords = true)
{
    using namespace Microsoft::WRL;

    CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {};
    extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = nullptr;
    extendedParams.hTemplateFile = nullptr;

    Wrappers::FileHandle file(CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams));
    if (file.Get() == INVALID_HANDLE_VALUE)
    {
        throw std::exception();
    }

    FILE_STANDARD_INFO fileInfo = {};
    if (!GetFileInformationByHandleEx(file.Get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
    {
        throw std::exception();
    }

    if (fileInfo.EndOfFile.HighPart != 0)
    {
        throw std::exception();
    }

	// ToDo replace malloc
    *data = reinterpret_cast<byte*>(malloc(fileInfo.EndOfFile.LowPart));
    *size = fileInfo.EndOfFile.LowPart;

    if (!ReadFile(file.Get(), *data, fileInfo.EndOfFile.LowPart, nullptr, nullptr))
    {
        throw std::exception();
    }

    return S_OK;
}

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
    pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
    WCHAR fullName[50];
    if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
    {
        pObject->SetName(fullName);
    }
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

inline UINT Align(UINT size, UINT alignment)
{
    return (size + (alignment - 1)) & ~(alignment - 1);
}

inline UINT CalculateConstantBufferByteSize(UINT byteSize)
{
    // Constant buffer size is required to be aligned.
    return Align(byteSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}

#ifdef D3D_COMPILE_STANDARD_FILE_INCLUDE
inline Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
    const std::wstring& filename,
    const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint,
    const std::string& target)
{
    UINT compileFlags = 0;
#if defined(_DEBUG) || defined(DBG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr;

    Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
    {
        OutputDebugStringA((char*)errors->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    return byteCode;
}
#endif

// Resets all elements in a ComPtr array.
template<class T>
void ResetComPtrArray(T* comPtrArray)
{
    for (auto &i : *comPtrArray)
    {
        i.Reset();
    }
}


// Resets all elements in a unique_ptr array.
template<class T>
void ResetUniquePtrArray(T* uniquePtrArray)
{
    for (auto &i : *uniquePtrArray)
    {
        i.reset();
    }
}

class GpuUploadBuffer
{
public:
	// ToDo return resource instead of ComPtr?
    ComPtr<ID3D12Resource> GetResource() { return m_resource; }
    virtual void Release() { m_resource.Reset(); }
	UINT64 Size() { return m_resource.Get() ? m_resource->GetDesc().Width : 0; }
protected:
    ComPtr<ID3D12Resource> m_resource;

    GpuUploadBuffer() {}
    ~GpuUploadBuffer()
    {
        if (m_resource.Get())
        {
            m_resource->Unmap(0, nullptr);
        }
    }

    void Allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
    {
        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_resource)));
		if (resourceName)
		{
			m_resource->SetName(resourceName);
		}
    }

    uint8_t* MapCpuWriteOnly()
    {
        uint8_t* mappedData;
        // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
        return mappedData;
    }
};

// Helper class to create and update a constant buffer with proper constant buffer alignments.
// Usage: 
//    ConstantBuffer<...> cb;
//    cb.Create(...);
//    cb.staging.var = ... ; | cb->var = ... ; 
//    cb.CopyStagingToGPU(...);
//    Set...View(..., cb.GputVirtualAddress());
template <class T>
class ConstantBuffer : public GpuUploadBuffer
{
    uint8_t* m_mappedConstantData;
    UINT m_alignedInstanceSize;
    UINT m_numInstances;

public:
    ConstantBuffer() : m_alignedInstanceSize(0), m_numInstances(0), m_mappedConstantData(nullptr) {}

    void Create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
        m_numInstances = numInstances;
        m_alignedInstanceSize = Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        UINT bufferSize = numInstances * m_alignedInstanceSize;
        Allocate(device, bufferSize, resourceName);
        m_mappedConstantData = MapCpuWriteOnly();
    }

    void CopyStagingToGpu(UINT instanceIndex = 0)
    {
        memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
    }

    // Accessors
    T staging;
    T* operator->() { return &staging; }
    UINT NumInstances() { return m_numInstances; }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }
};


// Helper class to create and update a structured buffer.
// Usage: 
//    StructuredBuffer<...> sb;
//    sb.Create(...);
//    sb[index].var = ... ; 
//    sb.CopyStagingToGPU(...);
//    Set...View(..., sb.GputVirtualAddress());
template <class T>
class StructuredBuffer : public GpuUploadBuffer
{
    T* m_mappedBuffers;
    std::vector<T> m_staging;
    UINT m_numInstances;

public:
    // Performance tip: Align structures on sizeof(float4) boundary.
    // Ref: https://developer.nvidia.com/content/understanding-structured-buffer-performance
    static_assert(sizeof(T) % 16 == 0, L"Align structure buffers on 16 byte boundary for performance reasons.");

    StructuredBuffer() : m_mappedBuffers(nullptr), m_numInstances(0) {}

    void Create(ID3D12Device* device, UINT numElements, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
		m_numInstances = numInstances;
        m_staging.resize(numElements);
        UINT bufferSize = numInstances * numElements * sizeof(T);
        Allocate(device, bufferSize, resourceName);
        m_mappedBuffers = reinterpret_cast<T*>(MapCpuWriteOnly());
    }

    void CopyStagingToGpu(UINT instanceIndex = 0)
    {
        memcpy(m_mappedBuffers + instanceIndex * NumElementsPerInstance(), &m_staging[0], InstanceSize());
    }

    auto begin() { return m_staging.begin(); }
    auto end() { return m_staging.end(); }

    // Accessors
    T& operator[](UINT elementIndex) { return m_staging[elementIndex]; }
    size_t NumElementsPerInstance() { return m_staging.size(); }
	UINT ElementSize() { return sizeof(T); }
    UINT NumInstances() { return m_staging.size(); }
    size_t InstanceSize() { return NumElementsPerInstance() * ElementSize(); }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize();
    }
};

class DescriptorHeap
{
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	UINT m_descriptorsAllocated;
	UINT m_descriptorSize;

public:
	DescriptorHeap(ID3D12Device* device, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		// Allocate a heap for descriptors:
		// 2 per geometry - vertex and index  buffer SRVs
		// 1 - raytracing output texture SRV
		// 2 per BLAS - one for the acceleration structure and one for its instance desc 
		// 1 - top level acceleration structure
		//ToDo
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Type = type;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;
		device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
		NAME_D3D12_OBJECT(m_descriptorHeap);

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ID3D12DescriptorHeap* GetHeap() { return m_descriptorHeap.Get(); }
	ID3D12DescriptorHeap** GetAddressOf() { return m_descriptorHeap.GetAddressOf(); }
	UINT DescriptorSize() { return m_descriptorSize; }

	// Allocate a descriptor and return its index. 
	// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
	UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX)
	{
		auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
		{
			ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
			descriptorIndexToUse = m_descriptorsAllocated++;
		}
		*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
		return descriptorIndexToUse;
	}
};


inline float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

// Returns a cycling <0 -> 1 -> 0> animation interpolant 
inline float CalculateAnimationInterpolant(float elapsedTime, float cycleDuration)
{
	float curLinearCycleTime = fmod(elapsedTime, cycleDuration) / cycleDuration;
	curLinearCycleTime = (curLinearCycleTime <= 0.5f) ? 2 * curLinearCycleTime : 1 - 2 * (curLinearCycleTime - 0.5f);
	return lerp(0.0f, 1.0f, curLinearCycleTime);
}



inline float NumMPixelsPerSecond(float timeMs, UINT width, UINT height)
{
	float resolution = static_cast<float>(width * height);
	float raytracingTime = 0.001f * timeMs;
	return resolution / (raytracingTime * static_cast<float>(1e6));
}


// ToDo Gpu*flags?
namespace ResourceRWFlags {
	enum Enum
	{
		None = 0x0,
		AllowRead = 0x1,
		AllowWrite = 0x2,
	};
}

// ToDo turn into class and check rwFlags being properly set on access.
struct RWGpuResource
{
	UINT rwFlags = ResourceRWFlags::None;
	ComPtr<ID3D12Resource> resource;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorReadAccess = { UINT64_MAX };
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorWriteAccess = { UINT64_MAX };
	UINT descriptorHeapIndex = UINT_MAX;
};


// Combine with CreateRT?
inline void CreateTextureSRV(
	ID3D12Device* device,
	ID3D12Resource* resource,
	DescriptorHeap* descriptorHeap,
	UINT* descriptorHeapIndex,
	D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle)
{
	// Describe and create an SRV.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	*descriptorHeapIndex = descriptorHeap->AllocateDescriptor(cpuHandle, *descriptorHeapIndex);
	device->CreateShaderResourceView(resource, &srvDesc, *cpuHandle);
	*gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(),
		*descriptorHeapIndex, descriptorHeap->DescriptorSize());
};

inline void CreateRenderTargetResource(
	ID3D12Device* device,
	DXGI_FORMAT format,
	UINT width,
	UINT height,
	DescriptorHeap* descriptorHeap,
	RWGpuResource* dest,
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET,
	const wchar_t* resourceName = nullptr)
{
	auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, initialResourceState, nullptr, IID_PPV_ARGS(&dest->resource)));
	if (resourceName)
	{
		dest->resource->SetName(resourceName);
	}


	if (dest->rwFlags & ResourceRWFlags::AllowWrite)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
		dest->descriptorHeapIndex = descriptorHeap->AllocateDescriptor(&uavDescriptorHandle, dest->descriptorHeapIndex);
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		device->CreateUnorderedAccessView(dest->resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
		dest->gpuDescriptorWriteAccess = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(), dest->descriptorHeapIndex, descriptorHeap->DescriptorSize());
	}

	if (dest->rwFlags & ResourceRWFlags::AllowRead)
	{
		// ToDo cleanup and combine
		D3D12_CPU_DESCRIPTOR_HANDLE dummyHandle;
		CreateTextureSRV(device, dest->resource.Get(), descriptorHeap, &dest->descriptorHeapIndex, &dummyHandle, &dest->gpuDescriptorReadAccess);
	}
}

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

// ToDo: standardize create/alloc resource calls
inline void AllocateUAVBuffer(
	ID3D12Device* device, 
	UINT numElements,	// ToDo use template?
	UINT elementSize,
	RWGpuResource* dest,
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
	DescriptorHeap* descriptorHeap = nullptr,
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, 
	const wchar_t* resourceName = nullptr)
{
	AllocateUAVBuffer(device, numElements * elementSize, &dest->resource, initialResourceState, resourceName);

	if (dest->rwFlags & ResourceRWFlags::AllowWrite)
	{
		assert(descriptorHeap);
		D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
		dest->descriptorHeapIndex = descriptorHeap->AllocateDescriptor(&uavDescriptorHandle, dest->descriptorHeapIndex);
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		UAVDesc.Buffer.NumElements = numElements;
		UAVDesc.Buffer.FirstElement = 0;
		UAVDesc.Format = format;
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		device->CreateUnorderedAccessView(dest->resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
		dest->gpuDescriptorWriteAccess = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(), dest->descriptorHeapIndex, descriptorHeap->DescriptorSize());
	}

	if (dest->rwFlags & ResourceRWFlags::AllowRead)
	{
		assert(descriptorHeap);
		assert(0 && L"ToDo");
	}
}

inline void AllocateReadBackBuffer(
	ID3D12Device* device,
	UINT64 bufferSize,
	ID3D12Resource **dest, 
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, 
	LPCWSTR resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	ThrowIfFailed(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(dest)));
	if (resourceName)
	{
		(*dest)->SetName(resourceName);
	}
}

inline UINT CeilDivide(UINT value, UINT divisor)
{
	return (value + divisor - 1) / divisor;
}

inline UINT CeilLogWithBase(UINT value, UINT base)
{
	return static_cast<UINT>(ceil(log(value)/ log(base)));
}

inline void SerializeAndCreateRootSignature(
	ID3D12Device* device,
	D3D12_ROOT_SIGNATURE_DESC& desc, 
	ComPtr<ID3D12RootSignature>* rootSignature, 
	LPCWSTR resourceName = nullptr)
{
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
	ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSignature))));

	// ToDo is this check needed?
	if (resourceName)
	{
		(*rootSignature)->SetName(resourceName);
	}
}


struct D3DBuffer
{
	ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
	UINT heapIndex = UINT_MAX;
};

// ToDo rename 
struct D3DGeometry
{
	struct Buffer
	{
		D3DBuffer buffer;
		ComPtr<ID3D12Resource> upload;	// ToDo delete these after initialization? Should there be a common upload?
	};

	Buffer vb;
	Buffer ib;
};



struct GeometryDescriptor
{
	struct Buffer {
		UINT startIndex;
		UINT count;
		union {
			Index* indices;
			VertexPositionNormalTextureTangent* vertices;
		};
	};

	Buffer vb;
	Buffer ib;
};

class GeometryInstance
{
public:

	GeometryInstance() {}

	GeometryInstance(D3DGeometry& geometry)
	{
		ib.startIndex = 0;
		ib.count = static_cast<UINT>(geometry.ib.buffer.resource->GetDesc().Width / sizeof(Index));
		ib.indexBuffer = geometry.ib.buffer.resource->GetGPUVirtualAddress();
		ib.gpuDescriptorHandle = geometry.ib.buffer.gpuDescriptorHandle;
		vb.startIndex = 0;
#if ONLY_SQUID_SCENE_BLAS
		vb.count = static_cast<UINT>(geometry.vb.buffer.resource->GetDesc().Width / sizeof(VertexPositionNormalTextureTangent));
		vb.vertexBuffer.StrideInBytes = sizeof(VertexPositionNormalTextureTangent);
#else
		vb.count = static_cast<UINT>(geometry.vb.buffer.resource->GetDesc().Width / sizeof(DirectX::VertexPositionNormalTexture));
		vb.vertexBuffer.StrideInBytes = sizeof(DirectX::VertexPositionNormalTexture);
#endif
		vb.vertexBuffer.StartAddress = geometry.vb.buffer.resource->GetGPUVirtualAddress();
		vb.gpuDescriptorHandle = geometry.vb.buffer.gpuDescriptorHandle;
	}

	struct Buffer {
		UINT startIndex;
		UINT count;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
		union {
			D3D12_GPU_VIRTUAL_ADDRESS indexBuffer;
			D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE vertexBuffer;
		};
	};

	Buffer vb;
	Buffer ib;
	D3D12_GPU_VIRTUAL_ADDRESS transform;
};



// Create a SRV for a buffer.
inline void CreateBufferSRV(
	ID3D12Resource* resource,
	ID3D12Device* device,
	UINT numElements,
	UINT elementSize,
	DescriptorHeap* descriptorHeap,
	D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptorHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle,
	UINT* heapIndex,
	UINT firstElement = 0)
{
	// SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.FirstElement = firstElement;
	if (elementSize == 0)
	{
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else
	{
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}
	*heapIndex = descriptorHeap->AllocateDescriptor(cpuDescriptorHandle, *heapIndex);
	device->CreateShaderResourceView(resource, &srvDesc, *cpuDescriptorHandle);
	*gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(),
			*heapIndex, descriptorHeap->DescriptorSize());
};

// Create a SRV for a buffer.
inline void CreateBufferSRV(
	ID3D12Device* device,
	UINT numElements,
	UINT elementSize,
	DescriptorHeap* descriptorHeap,
	D3DBuffer* dest,
	UINT firstElement = 0)
{
	return CreateBufferSRV(
		dest->resource.Get(), 
		device, numElements, 
		elementSize, 
		descriptorHeap, 
		&dest->cpuDescriptorHandle, 
		&dest->gpuDescriptorHandle, 
		&dest->heapIndex, 
		firstElement);
};

// ToDo move?
// Load geometry from a file into buffers.
inline void CreateGeometry(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	DescriptorHeap* descriptorHeap,
	const GeometryDescriptor& desc,
	D3DGeometry* geometry
)
{
	// Create the index buffer.
	{
		ThrowIfFalse(desc.ib.indices);
		UINT indexDataSize = desc.ib.count * sizeof(desc.ib.indices[0]);
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&geometry->ib.buffer.resource)));
		geometry->ib.buffer.resource->SetName(L"Index buffer: Squid room");

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&geometry->ib.upload)));
		geometry->ib.upload->SetName(L"Index buffer upload: Squid room");

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = desc.ib.indices;
		indexData.RowPitch = indexDataSize;
		indexData.SlicePitch = indexData.RowPitch;

		PIXBeginEvent(commandList, 0, L"Copy index buffer data to default resource...");
		{
			UpdateSubresources<1>(commandList, geometry->ib.buffer.resource.Get(), geometry->ib.upload.Get(), 0, 0, 1, &indexData);
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geometry->ib.buffer.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
		}
		PIXEndEvent(commandList);

		// Using raw typeless buffer. 
		// - numElements - number of 32bit dwords.
		// - elementSize - 0.
		UINT numElements = indexDataSize / sizeof(UINT);
		UINT elementStride = 0;	
		CreateBufferSRV(device, numElements, elementStride, descriptorHeap, &geometry->ib.buffer);
	}

	// Create the vertex buffer.
	{
		UINT vertexDataSize = desc.vb.count * sizeof(desc.vb.vertices[0]);
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&geometry->vb.buffer.resource)));
		geometry->vb.buffer.resource->SetName(L"VertexPositionNormalTextureTangent buffer: Squid room");

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&geometry->vb.upload)));
		geometry->vb.upload->SetName(L"VertexPositionNormalTextureTangent buffer upload: Squid room");

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = desc.vb.vertices;
		vertexData.RowPitch = vertexDataSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		PIXBeginEvent(commandList, 0, L"Copy vertex buffer data to default resource...");
		{
			UpdateSubresources<1>(commandList, geometry->vb.buffer.resource.Get(), geometry->vb.upload.Get(), 0, 0, 1, &vertexData);
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geometry->vb.buffer.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		}
		PIXEndEvent(commandList);

		CreateBufferSRV(device, desc.vb.count, sizeof(desc.vb.vertices[0]), descriptorHeap, &geometry->vb.buffer);
	}
}
