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

#include "GpuResource.h"
#include "Utility.h"

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
using Microsoft::WRL::ComPtr;


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

template <typename T>
inline bool IsInRange(const T& a, const T& _min, const T& _max)
{
    return a >= _min && a <= _max;
}

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

// Align to a certain value of power of 2.
inline constexpr UINT Align(UINT size, UINT alignment)
{
    return (size + (alignment - 1)) & ~(alignment - 1);
}

inline constexpr UINT CalculateConstantBufferByteSize(UINT byteSize)
{
    // Constant buffer size is required to be aligned.
    return Align(byteSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}

#ifdef D3D_COMPILE_STANDARD_FILE_INCLUDE
inline ComPtr<ID3DBlob> CompileShader(
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

    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;
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

struct D3DBuffer
{
    ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = { UINT64_MAX };
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = { UINT64_MAX };
    UINT heapIndex = UINT_MAX;
};

struct D3DGeometry
{
    struct Buffer
    {
        D3DBuffer buffer;
        ComPtr<ID3D12Resource> upload;	// TODO: release after initialization
    };

    Buffer vb;
    Buffer ib;
};

struct D3DTexture
{
    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12Resource> upload;      // TODO: release after initialization
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
    UINT heapIndex = UINT_MAX;
};


class GpuUploadBuffer
{
public:
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

    void Allocate(ID3D12Device5* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
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

    void Create(ID3D12Device5* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
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
    // Align staging object on 16B boundary for faster mempcy to the memory returned by Map()
    alignas(16) T staging;
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
    static_assert(sizeof(T) % 16 == 0, "Align structure buffers on 16 byte boundary for performance reasons.");

    StructuredBuffer() : m_mappedBuffers(nullptr), m_numInstances(0) {}

    void Create(ID3D12Device5* device, UINT numElements, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
		m_numInstances = numInstances;
        m_staging.resize(numElements);
        UINT bufferSize = numInstances * numElements * sizeof(T);
        Allocate(device, bufferSize, resourceName);
        m_mappedBuffers = reinterpret_cast<T*>(MapCpuWriteOnly());
    }

    void CopyStagingToGpu(UINT instanceIndex = 0)
    {
        memcpy(m_mappedBuffers + instanceIndex * NumElements(), &m_staging[0], InstanceSize());
    }

    auto begin() { return m_staging.begin(); }
    auto end() { return m_staging.end(); }
    auto begin() const { return m_staging.begin(); }
    auto end() const { return m_staging.end(); }

    // Accessors
    T& operator[](UINT elementIndex) { return m_staging[elementIndex]; }
    const T& operator[](UINT elementIndex) const { return m_staging[elementIndex]; }
    size_t NumElements() const { return m_staging.size(); }
	UINT ElementSize() const { return sizeof(T); }
    UINT NumInstances() const { return m_numInstances; }
    size_t InstanceSize() const { return NumElements() * ElementSize(); }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0, UINT elementIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize() + elementIndex * ElementSize();
    }
};

namespace DX
{
    class DescriptorHeap
    {
        ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
        UINT m_descriptorsAllocated;
        UINT m_descriptorSize;

    public:
        DescriptorHeap(ID3D12Device5* device, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
        {
            m_descriptorsAllocated = 0;

            D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
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
        // Passing descriptorIndexToUse as UINT_MAX will allocate next available descriptor.
        // Otherwise the descriptorIndexToUse will be used instead of allocating a new one.
        UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX)
        {
            if (descriptorIndexToUse == UINT_MAX)
            {
                ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
                descriptorIndexToUse = m_descriptorsAllocated++;
            }
            else
            {
                ThrowIfFalse(descriptorIndexToUse < m_descriptorHeap->GetDesc().NumDescriptors, L"Requested descriptor index is out of bounds!");
                m_descriptorsAllocated = std::max(descriptorIndexToUse + 1, m_descriptorsAllocated);
            }

            auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
            *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
            return descriptorIndexToUse;
        }

        // Allocate multiple descriptor indices and return an index of a first one. 
        // Passing firstDescriptorIndexToUse as UINT_MAX will allocate next available descriptors.
        // Otherwise the firstDescriptorIndexToUse will be used instead of allocating a new one.
        UINT AllocateDescriptorIndices(UINT numDescriptors, UINT firstDescriptorIndexToUse = UINT_MAX)
        {
            firstDescriptorIndexToUse = AllocateDescriptor(&D3D12_CPU_DESCRIPTOR_HANDLE(), firstDescriptorIndexToUse);

            for (UINT i = 1; i < numDescriptors; i++)
            {
                AllocateDescriptor(&D3D12_CPU_DESCRIPTOR_HANDLE(), firstDescriptorIndexToUse + i);
            }
            return firstDescriptorIndexToUse;
        }
    };
}

inline float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

inline float clamp(float a, float _min, float _max)
{
    return std::max(_min, std::min(_max, a));
}

inline float saturate(float a)
{
    return clamp(a, 0, 1);
}

inline float relativeCoef(float a, float _min, float _max)
{
    float _a = clamp(a, _min, _max);
    return (_a - _min) / (_max - _min);
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

inline void CreateTextureSRV(
	ID3D12Device5* device,
	ID3D12Resource* resource,
	DX::DescriptorHeap* descriptorHeap,
	UINT* descriptorHeapIndex,
	D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle,
    D3D12_SRV_DIMENSION srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D)
{
	// Describe and create an SRV.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = srvDimension;
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

// Loads a DDS texture and issues upload on the commandlist. 
// The caller is expected to execute the commandList.
inline void LoadDDSTexture(
    ID3D12Device5* device,
    ID3D12GraphicsCommandList4* commandList,
    const wchar_t* filename,
    DX::DescriptorHeap* descriptorHeap,
    ID3D12Resource** ppResource,
    ID3D12Resource** ppUpload,
    UINT* descriptorHeapIndex,
    D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle,
    D3D12_SRV_DIMENSION srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D)
{
    std::unique_ptr<uint8_t[]> ddsData;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    ThrowIfFailed(LoadDDSTextureFromFile(device, filename, ppResource, ddsData, subresources));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*ppResource, 0, static_cast<UINT>(subresources.size()));

    // Create the GPU upload buffer.
    ThrowIfFailed(
        device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(ppUpload)));

    UpdateSubresources(commandList, *ppResource, *ppUpload, 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*ppResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    CreateTextureSRV(device, *ppResource, descriptorHeap, descriptorHeapIndex, cpuHandle, gpuHandle, srvDimension);
}

inline void LoadDDSTexture(
    ID3D12Device5* device,
    ID3D12GraphicsCommandList4* commandList,
    const wchar_t* filename,
    DX::DescriptorHeap* descriptorHeap,
    D3DTexture* tex,
    D3D12_SRV_DIMENSION srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D)
{
    LoadDDSTexture(device, commandList, filename, descriptorHeap, &tex->resource, &tex->upload, &tex->heapIndex, &tex->cpuDescriptorHandle, &tex->gpuDescriptorHandle, srvDimension);
}

// Loads a WIC texture and issues upload on the commandlist. 
// The caller is expected to execute the commandList.
inline void LoadWICTexture(
    ID3D12Device5* device,
    ID3D12GraphicsCommandList4* commandList,
    const wchar_t* filename,
    DX::DescriptorHeap* descriptorHeap,
    ID3D12Resource** ppResource,
    ID3D12Resource** ppUpload,
    UINT* descriptorHeapIndex,
    D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle)
{
    std::unique_ptr<uint8_t[]> decodedData;
    D3D12_SUBRESOURCE_DATA subresource;
    ThrowIfFailed(LoadWICTextureFromFile(device, filename, ppResource, decodedData, subresource));


    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*ppResource, 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(
        device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(ppUpload)));

    UpdateSubresources(commandList, *ppResource, *ppUpload, 0, 0, 1, &subresource);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*ppResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    CreateTextureSRV(device, *ppResource, descriptorHeap, descriptorHeapIndex, cpuHandle, gpuHandle);
}

// Loads a WIC texture
inline void LoadWICTexture(
    ID3D12Device5* device,
    ResourceUploadBatch* resourceUpload,
    const wchar_t* filename,
    DX::DescriptorHeap* descriptorHeap,
    ID3D12Resource** ppResource,
    UINT* descriptorHeapIndex,
    D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle,
    bool loadMipmaps = true)
{
    ThrowIfFailed(CreateWICTextureFromFile(device, *resourceUpload, filename, ppResource, loadMipmaps));
    CreateTextureSRV(device, *ppResource, descriptorHeap, descriptorHeapIndex, cpuHandle, gpuHandle);
}


// Loads a texture and issues upload on the commandlist. 
// The caller is expected to execute the commandList.
inline void LoadTexture(
    ID3D12Device5* device,
    ID3D12GraphicsCommandList4* commandList,
    const wchar_t* filename,
    DX::DescriptorHeap* descriptorHeap,
    ID3D12Resource** ppResource,
    ID3D12Resource** ppUpload,
    UINT* descriptorHeapIndex,
    D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle)
{
    size_t len = wcsnlen_s(filename, 2048);
    if (len >= 4 && wcscmp(filename + len - 4, L".dds") == 0)
    {
        LoadDDSTexture(device, commandList, filename, descriptorHeap, ppResource, ppUpload, descriptorHeapIndex, cpuHandle, gpuHandle);
    }
    else
    {
        LoadWICTexture(device, commandList, filename, descriptorHeap, ppResource, ppUpload, descriptorHeapIndex, cpuHandle, gpuHandle);
    }
}

inline void CreateRenderTargetResource(
	ID3D12Device5* device,
	DXGI_FORMAT format,
	UINT width,
	UINT height,
	DX::DescriptorHeap* descriptorHeap,
	GpuResource* dest,
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET,
	const wchar_t* resourceName = nullptr)
{
	auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, initialResourceState, nullptr, IID_PPV_ARGS(&dest->resource)));
    dest->m_UsageState = initialResourceState;
	if (resourceName)
	{
		dest->resource->SetName(resourceName);
	}


	if (dest->rwFlags & GpuResource::RWFlags::AllowWrite)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
		dest->uavDescriptorHeapIndex = descriptorHeap->AllocateDescriptor(&uavDescriptorHandle, dest->uavDescriptorHeapIndex);
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		device->CreateUnorderedAccessView(dest->resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
		dest->gpuDescriptorWriteAccess = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(), dest->uavDescriptorHeapIndex, descriptorHeap->DescriptorSize());
	}

	if (dest->rwFlags & GpuResource::RWFlags::AllowRead)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dummyHandle;
		CreateTextureSRV(device, dest->resource.Get(), descriptorHeap, &dest->srvDescriptorHeapIndex, &dummyHandle, &dest->gpuDescriptorReadAccess);
	}
}

inline void AllocateUAVBuffer(ID3D12Device5* pDevice, UINT64 bufferSize, ID3D12Resource **ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
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

// Create a SRV for a buffer.
inline void CreateBufferSRV(
    ID3D12Resource* resource,
    ID3D12Device5* device,
    UINT numElements,
    UINT elementSize,
    DX::DescriptorHeap* descriptorHeap,
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

inline void AllocateUAVBuffer(
	ID3D12Device5* device, 
	UINT numElements,
	UINT elementSize,
	GpuResource* dest,
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
	DX::DescriptorHeap* descriptorHeap = nullptr,
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, 
	const wchar_t* resourceName = nullptr)
{
	AllocateUAVBuffer(device, numElements * elementSize, &dest->resource, initialResourceState, resourceName); 
    dest->m_UsageState = initialResourceState;

	if (dest->rwFlags & GpuResource::RWFlags::AllowWrite)
	{
		ThrowIfFalse(descriptorHeap);
		D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
		dest->uavDescriptorHeapIndex = descriptorHeap->AllocateDescriptor(&uavDescriptorHandle, dest->uavDescriptorHeapIndex);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.Buffer.NumElements = numElements;
		UAVDesc.Buffer.FirstElement = 0;
        UAVDesc.Buffer.StructureByteStride = elementSize;
		UAVDesc.Format = format;
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		device->CreateUnorderedAccessView(dest->resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
		dest->gpuDescriptorWriteAccess = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(), dest->uavDescriptorHeapIndex, descriptorHeap->DescriptorSize());
	}

	if (dest->rwFlags & GpuResource::RWFlags::AllowRead)
	{
        D3D12_CPU_DESCRIPTOR_HANDLE dummyHandle;
        CreateBufferSRV(
            dest->resource.Get(),
            device, numElements,
            elementSize,
            descriptorHeap,
            &dummyHandle,
            &dest->gpuDescriptorReadAccess,
            &dest->srvDescriptorHeapIndex,
            0);
	}
}

inline void AllocateReadBackBuffer(
	ID3D12Device5* device,
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
	return static_cast<UINT>(ceil(log(value)/log(base)));
}

inline void SerializeAndCreateRootSignature(
	ID3D12Device5* device,
	D3D12_ROOT_SIGNATURE_DESC& desc, 
	ComPtr<ID3D12RootSignature>* rootSignature, 
	LPCWSTR resourceName = nullptr)
{
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
	ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSignature))));

	if (resourceName)
	{
		(*rootSignature)->SetName(resourceName);
	}
}

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

	GeometryInstance() : transform(0) {}

	GeometryInstance(
        const D3DGeometry& geometry, 
        UINT _materialID, 
        D3D12_GPU_DESCRIPTOR_HANDLE _diffuseTexture, 
        D3D12_GPU_DESCRIPTOR_HANDLE _normalTexture,
        // Mark the geometry as opaque by default. 
        // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
        // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
        D3D12_RAYTRACING_GEOMETRY_FLAGS _geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
        bool _isVertexAnimated = false) : 
        materialID(_materialID), diffuseTexture(_diffuseTexture), normalTexture(_normalTexture), transform(0), isVertexAnimated(_isVertexAnimated)
	{
        geometryFlags = _geometryFlags;
		ib.startIndex = 0;
		ib.count = static_cast<UINT>(geometry.ib.buffer.resource->GetDesc().Width / sizeof(Index));
		ib.indexBuffer = geometry.ib.buffer.resource->GetGPUVirtualAddress();
		ib.gpuDescriptorHandle = geometry.ib.buffer.gpuDescriptorHandle;
		vb.startIndex = 0;
		vb.count = static_cast<UINT>(geometry.vb.buffer.resource->GetDesc().Width / sizeof(VertexPositionNormalTextureTangent));
		vb.vertexBuffer.StrideInBytes = sizeof(VertexPositionNormalTextureTangent);
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
	UINT materialID;
    bool isVertexAnimated;
    D3D12_GPU_DESCRIPTOR_HANDLE diffuseTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE normalTexture;
    D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags;
};


// Create a SRV for a buffer.
inline void CreateBufferSRV(
	ID3D12Device5* device,
	UINT numElements,
	UINT elementSize,
	DX::DescriptorHeap* descriptorHeap,
	D3DBuffer* dest,
	UINT firstElement = 0)
{
	CreateBufferSRV(
		dest->resource.Get(), 
		device, numElements, 
		elementSize, 
		descriptorHeap, 
		&dest->cpuDescriptorHandle, 
		&dest->gpuDescriptorHandle, 
		&dest->heapIndex, 
		firstElement);
};

// Allocates raw typeless buffer.
inline void AllocateRawTypelessBuffer(
    ID3D12Device5* device,
    UINT numElements,
    UINT elementByteSize,
    DX::DescriptorHeap* descriptorHeap,
    D3DBuffer* buffer,
    D3D12_RESOURCE_STATES initialResourceState,
    const wchar_t* resourceName = nullptr)
{
    UINT size = numElements * elementByteSize;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(size),
        initialResourceState,
        nullptr,
        IID_PPV_ARGS(&buffer->resource)));
    if (resourceName)
    {
        buffer->resource->SetName(resourceName);
    }

    // Using raw typeless buffer. 
    // - numElements - number of 32bit dwords.
    // - elementSize - 0.
    UINT numDWORDs = size / sizeof(UINT);
    UINT elementStride = 0;
    CreateBufferSRV(device, numDWORDs, elementStride, descriptorHeap, buffer);
}

// Allocates index buffer.
inline void AllocateIndexBuffer(
    ID3D12Device5* device,
    UINT numElements,
    UINT elementByteSize,
    DX::DescriptorHeap* descriptorHeap,
    D3DBuffer* buffer,
    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_INDEX_BUFFER
)
{
    AllocateRawTypelessBuffer(device, numElements, elementByteSize, descriptorHeap, buffer, initialResourceState, L"Index buffer");
}


inline void AllocateBuffer(
    ID3D12Device5* device,
    UINT numElements,
    UINT elementByteSize,
    DX::DescriptorHeap* descriptorHeap,
    D3DBuffer* buffer,
    D3D12_RESOURCE_STATES initialResourceState,
    const wchar_t* resourceName = nullptr)
{
    UINT vertexDataSize = numElements * elementByteSize;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
        initialResourceState,
        nullptr,
        IID_PPV_ARGS(&buffer->resource)));
    if (resourceName)
    {
        buffer->resource->SetName(resourceName);
    }

    CreateBufferSRV(device, numElements, elementByteSize, descriptorHeap, buffer);
}

// Allocates vertex buffer.
inline void AllocateVertexBuffer(
    ID3D12Device5* device,
    UINT numElements,
    UINT elementByteSize,
    DX::DescriptorHeap* descriptorHeap,
    D3DBuffer* buffer,
    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
)
{
    AllocateBuffer(device, numElements, elementByteSize, descriptorHeap, buffer, initialResourceState, L"Vertex buffer");
}


// Allocates buffers for the geometry without uploading anything.
inline void UploadDataToBuffer(
    ID3D12Device5* device,
    ID3D12GraphicsCommandList4* commandList,
    const void* pSrcData,
    UINT numElements,
    UINT elementByteSize,
    ID3D12Resource* destBuffer,
    ID3D12Resource** ppUploadResource,
    D3D12_RESOURCE_STATES outResourceState
)
{
    UINT indexDataSize = numElements * elementByteSize;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexDataSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(ppUploadResource)));
    (*ppUploadResource)->SetName(L"Buffer upload");

    // Copy data to the upload heap and then schedule a copy 
    // from the upload heap to the index buffer.
    D3D12_SUBRESOURCE_DATA data = {};
    data.pData = pSrcData;
    data.RowPitch = indexDataSize;
    data.SlicePitch = data.RowPitch;

    PIXBeginEvent(commandList, 0, L"Copy buffer data to default resource...");
    {
        UpdateSubresources<1>(commandList, destBuffer, *ppUploadResource, 0, 0, 1, &data);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(destBuffer, D3D12_RESOURCE_STATE_COPY_DEST, outResourceState));
    }
    PIXEndEvent(commandList);

}

inline void CreateGeometry(
	ID3D12Device5* device,
	ID3D12GraphicsCommandList4* commandList,
	DX::DescriptorHeap* descriptorHeap,
	const GeometryDescriptor& desc,
	D3DGeometry* geometry
)
{
    // Create index buffer and upload data to it.
    AllocateIndexBuffer(device, desc.ib.count, sizeof(Index), descriptorHeap, &geometry->ib.buffer, D3D12_RESOURCE_STATE_COPY_DEST);
    UploadDataToBuffer(device, commandList, desc.ib.indices, desc.ib.count, sizeof(Index), geometry->ib.buffer.resource.Get(), &geometry->ib.upload, D3D12_RESOURCE_STATE_INDEX_BUFFER);

    // Create vertex buffer and upload data to it.
    AllocateVertexBuffer(device, desc.vb.count, sizeof(VertexPositionNormalTextureTangent), descriptorHeap, &geometry->vb.buffer, D3D12_RESOURCE_STATE_COPY_DEST);
    UploadDataToBuffer(device, commandList, desc.vb.vertices, desc.vb.count, sizeof(VertexPositionNormalTextureTangent), geometry->vb.buffer.resource.Get(), &geometry->vb.upload, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

inline void CopyResource(
    ID3D12GraphicsCommandList4* commandList,
    ID3D12Resource* srcResource,
    ID3D12Resource* destResource,
    D3D12_RESOURCE_STATES inSrcResourceState,
    D3D12_RESOURCE_STATES inDestResourceState,
    D3D12_RESOURCE_STATES outSrcResourceState,
    D3D12_RESOURCE_STATES outDestResourceState)
{
    D3D12_RESOURCE_BARRIER preCopyBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(srcResource, inSrcResourceState, D3D12_RESOURCE_STATE_COPY_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(destResource, inDestResourceState, D3D12_RESOURCE_STATE_COPY_DEST)
    };
    commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    commandList->CopyResource(destResource, srcResource);

    D3D12_RESOURCE_BARRIER postCopyBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(srcResource, D3D12_RESOURCE_STATE_COPY_SOURCE, outSrcResourceState),
        CD3DX12_RESOURCE_BARRIER::Transition(destResource, D3D12_RESOURCE_STATE_COPY_DEST, outDestResourceState)
    };

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

inline void CopyResource(
    ID3D12GraphicsCommandList4* commandList,
    ID3D12Resource* srcResource,
    ID3D12Resource* destResource,
    D3D12_RESOURCE_STATES inSrcResourceState,
    D3D12_RESOURCE_STATES inDestResourceState)
{
    CopyResource(
        commandList,
        srcResource,
        destResource,
        inSrcResourceState,
        inDestResourceState,
        inSrcResourceState,
        inDestResourceState);
}

inline void CopyTextureRegion(
    ID3D12GraphicsCommandList4* commandList,
    ID3D12Resource* srcResource,
    ID3D12Resource* destResource,
    const D3D12_BOX *srcBox,
    D3D12_RESOURCE_STATES inSrcResourceState,
    D3D12_RESOURCE_STATES inDestResourceState,
    D3D12_RESOURCE_STATES outSrcResourceState,
    D3D12_RESOURCE_STATES outDestResourceState)
{
    D3D12_RESOURCE_BARRIER preCopyBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(srcResource, inSrcResourceState, D3D12_RESOURCE_STATE_COPY_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(destResource, inDestResourceState, D3D12_RESOURCE_STATE_COPY_DEST)
    };
    commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    CD3DX12_TEXTURE_COPY_LOCATION copySrc(srcResource);
    CD3DX12_TEXTURE_COPY_LOCATION copyDest(destResource);

    commandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, srcBox);

    D3D12_RESOURCE_BARRIER postCopyBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(srcResource, D3D12_RESOURCE_STATE_COPY_SOURCE, outSrcResourceState),
        CD3DX12_RESOURCE_BARRIER::Transition(destResource, D3D12_RESOURCE_STATE_COPY_DEST, outDestResourceState)
    };

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

inline void CopyTextureRegion(
    ID3D12GraphicsCommandList4* commandList,
    ID3D12Resource* src,
    ID3D12Resource* dest,
    const D3D12_BOX *srcBox,
    D3D12_RESOURCE_STATES inSrcResourceState,
    D3D12_RESOURCE_STATES inDestResourceState)
{
    CopyTextureRegion(
        commandList,
        src,
        dest,
        srcBox,
        inSrcResourceState,
        inDestResourceState,
        inSrcResourceState,
        inDestResourceState);
}

// Calculates a normalized tangent vector for a triangle given vertices' positions p* and their uv* coordinates.
inline XMFLOAT3 CalculateTangent(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT2& uv0, const XMFLOAT2& uv1, const XMFLOAT2& uv2)
{
    // A tangent can be computed by solving the following equations
    // E1 = (P1 - P0) = T * (u1 - u0) + B * (v1 - v0)
    // E2 = (P2 - P0) = T * (u2 - u0) + B * (v2 - v0)
    //   expressed in Matrix form as
    //   (  E1  ) =  (  u10  v10  ) (  T  ) 
    //   (  E2  ) =  (  u20  v20  ) (  B  )  
    //
    //  That is by multiplying with an inverse UV matrix and solving for T
    //
    //   (  T  ) =   1 /     (  v20  -v10  )  (  E1  )
    //   (  B  ) =   det(UV) (  -u20  u10  )  (  E2  )
    //  where det(UV) = u10 * v20 - v10 * u20

    XMFLOAT3 e10 = XMFLOAT3(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
    XMFLOAT3 e20 = XMFLOAT3(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);

    XMFLOAT2 d_uv10 = XMFLOAT2(uv1.x - uv0.x, uv1.y - uv0.y);
    XMFLOAT2 d_uv20 = XMFLOAT2(uv2.x - uv0.x, uv2.y - uv0.y);

    float invDetUV = 1 / (d_uv10.x * d_uv20.y - d_uv10.y * d_uv20.x);

    XMFLOAT3 tangent;
    tangent.x = invDetUV * (e10.x * d_uv20.y - e20.x * d_uv10.y);
    tangent.y = invDetUV * (e10.y * d_uv20.y - e20.y * d_uv10.y);
    tangent.z = invDetUV * (e10.z * d_uv20.y - e20.z * d_uv10.y);

    return tangent;
}


inline UINT NumMantissaBitsInFloatFormat(UINT FloatFormatBitLength)
{
    switch (FloatFormatBitLength)
    {
    case 32: return 23;
    case 16: return 10;
    case 11: return 6;
    case 10: return 5;
    }

    return 0;
}