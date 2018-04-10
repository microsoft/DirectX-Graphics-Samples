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
    UINT Size() const { return Align(shaderIdentifier.size + localRootArguments.size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT); }

    void CopyTo(void* dest) 
    {
        uint8_t* byteDest = static_cast<uint8_t*>(dest);
        memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
        if (localRootArguments.ptr)
        {
            memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, shaderIdentifier.size);
        }
    }

    void AllocateAsUploadBuffer(ID3D12Device* pDevice, ID3D12Resource **ppResource, const wchar_t* resourceName = nullptr)
    {
        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        UINT size = Align(Size(), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
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
        uint8_t *pMappedData;
        (*ppResource)->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
        memcpy(pMappedData, shaderIdentifier.ptr, shaderIdentifier.size);
        if (localRootArguments.ptr)
        {
            memcpy(pMappedData + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
        }
        (*ppResource)->Unmap(0, nullptr);
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

class ShaderTable
{
    UINT m_maxShaderRecordSize;
    std::vector<ShaderRecord> m_shaderRecords;

public:
    ShaderTable() : m_maxShaderRecordSize(0) {}
    UINT GetMaxShaderRecordSize() const { return m_maxShaderRecordSize; }
    UINT Size() const { return static_cast<UINT>(m_shaderRecords.size()) * GetMaxShaderRecordSize(); }

    void push_back(const ShaderRecord& shaderRecord) 
    { 
        m_maxShaderRecordSize = max(shaderRecord.Size(), m_maxShaderRecordSize);
        m_shaderRecords.push_back(shaderRecord); 
    }

    void AllocateAsUploadBuffer(ID3D12Device* pDevice, ID3D12Resource **ppResource, const wchar_t* resourceName = nullptr)
    {
        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        UINT bufferSize = Size();
        UINT shaderRecordStride = GetMaxShaderRecordSize();

        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
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
        uint8_t *pMappedData;
        (*ppResource)->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
        for (auto& shaderRecord : m_shaderRecords)
        {
            shaderRecord.CopyTo(pMappedData);
            pMappedData += shaderRecordStride;
        }
        (*ppResource)->Unmap(0, nullptr);
    }
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
    wstr << L"D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

    auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
    {
        std::wostringstream woss;
        for (UINT i = 0; i < numExports; i++)
        {
            if (depth > 0)
            {
                for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
                woss << (i == numExports - 1 ? L"\xC0" : L"\xC3");
            }
            woss << L"[" << i << L"]: ";
            if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
            woss << exports[i].Name << L"\n";
        }
        return woss.str();
    };

    for (UINT i = 0; i < desc->NumSubobjects; i++)
    {
        wstr << L"[" << i << L"]: ";
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_FLAGS)
        {
            wstr << L"Flags (not yet defined)\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE)
        {
            wstr << L"Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE)
        {
            wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK)
        {
            wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_CACHED_STATE_OBJECT)
        {
            wstr << L"Cached State Object (not yet defined)\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY)
        {
            wstr << L"DXIL Library 0x";
            auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
            wstr << ExportTree(1, lib->NumExports, lib->pExports);
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION)
        {
            wstr << L"Existing Library 0x";
            auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << collection->pExistingCollection << L"\n";
            wstr << ExportTree(1, collection->NumExports, collection->pExports);
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION)
        {
            wstr << L"Subobject to Exports Association (Subobject [";
            auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
            wstr << index << L"])\n";
            for (UINT j = 0; j < association->NumExports; j++) wstr << (j == association->NumExports - 1 ? L" \xC0" : L" \xC3") << L"[" << j << L"]: " << association->pExports[j] << L"\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION)
        {
            wstr << L"DXIL Subobjects to Exports Association (";
            auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            wstr << association->SubobjectToAssociate << L")\n";
            for (UINT j = 0; j < association->NumExports; j++) wstr << (j == association->NumExports - 1 ? L" \xC0" : L" \xC3") << L"[" << j << L"]: " << association->pExports[j] << L"\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG)
        {
            wstr << L"Raytracing Shader Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L" \xC3" << L"[0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
            wstr << L" \xC0" << L"[1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG)
        {
            wstr << L"Raytracing Pipeline Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L" \xC0" << L"[0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
        }
        if (desc->pSubobjects[i].Type == D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP)
        {
            wstr << L"Hit Group (";
            auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
            wstr << L" \xC3" << L"[0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
            wstr << L" \xC3" << L"[1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
            wstr << L" \xC0" << L"[2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
        }
    }
    OutputDebugStringW(wstr.str().c_str());
}

// Enable experimental features and return if they are supported.
// To test them being supported we need to check both their enablement as well as device creation afterwards.
template <std::size_t N>
inline bool EnableD3D12ExperimentalFeatures(UUID(&experimentalFeatures)[N])
{
    ComPtr<ID3D12Device> testDevice;
    return SUCCEEDED(D3D12EnableExperimentalFeatures(N, experimentalFeatures, nullptr, nullptr))
        && SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)));
}

// Enable experimental features required for compute-based raytracing fallback.
// This will set active D3D12 devices to DEVICE_REMOVED state.
// Returns bool whether the call succeeded and the device supports the feature.
inline bool EnableComputeRaytracingFallback()
{
    UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels };
    return EnableD3D12ExperimentalFeatures(experimentalFeatures);
}

// Enable experimental features required for driver and compute-based fallback raytracing.
// This will set active D3D12 devices to DEVICE_REMOVED state.
// Returns bool whether the call succeeded and the device supports the feature.
inline bool EnableRaytracing()
{
    UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels, D3D12RaytracingPrototype };
    return EnableD3D12ExperimentalFeatures(experimentalFeatures);
}