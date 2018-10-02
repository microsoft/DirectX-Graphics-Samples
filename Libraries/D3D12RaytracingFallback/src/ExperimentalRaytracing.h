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

class ExperimentalRaytracingStateObject : public ID3D12RaytracingFallbackStateObject
{
public:
    ExperimentalRaytracingStateObject(ID3D12StateObjectPrototype *pStateObject) : m_pStateObject(pStateObject)
    {
        ThrowFailure(m_pStateObject->QueryInterface(&m_pStateObjectInfo));
    }

    virtual ~ExperimentalRaytracingStateObject() {}

    virtual void *STDMETHODCALLTYPE GetShaderIdentifier(LPCWSTR pExportName)
    {
        return m_pStateObjectInfo->GetShaderIdentifier(pExportName);
    }

    virtual UINT64 STDMETHODCALLTYPE GetShaderStackSize(_In_  LPCWSTR pExportName)
    {
        return m_pStateObjectInfo->GetShaderStackSize(pExportName);
    }

    virtual UINT64 STDMETHODCALLTYPE GetPipelineStackSize(void)
    {
        return m_pStateObjectInfo->GetPipelineStackSize();
    }

    virtual void STDMETHODCALLTYPE SetPipelineStackSize(
        UINT64 PipelineStackSizeInBytes)
    {
        m_pStateObjectInfo->SetPipelineStackSize(PipelineStackSizeInBytes);
    }

    virtual ID3D12StateObject *GetStateObject()
    {
        return m_pStateObject;
    }
private:
    CComPtr<ID3D12StateObjectPropertiesPrototype> m_pStateObjectInfo;
    CComPtr<ID3D12StateObjectPrototype> m_pStateObject;
    COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pStateObject.p);
};

static void ConvertGeometryDescs(
    _In_ UINT numDescs,
    _In_ D3D12_ELEMENTS_LAYOUT layout,
    const D3D12_RAYTRACING_GEOMETRY_DESC *pGeometryDescs,
    const D3D12_RAYTRACING_GEOMETRY_DESC *const *ppGeometryDescs,
    _Out_ std::vector<D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC> &geometryDescs)
{
    geometryDescs.resize(numDescs);

    for (UINT i = 0; i < numDescs; i++)
    {
        const D3D12_RAYTRACING_GEOMETRY_DESC &geomDesc = (layout == D3D12_ELEMENTS_LAYOUT_ARRAY) ?
            pGeometryDescs[i] : *ppGeometryDescs[i];

        D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC &convertedDesc = geometryDescs[i];
        convertedDesc.Type = geomDesc.Type;
        convertedDesc.Flags = geomDesc.Flags;
        if (convertedDesc.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
        {
            const auto &triangleDesc = geomDesc.Triangles;
            auto &convertedTriangleDesc = convertedDesc.Triangles;
            convertedTriangleDesc.IndexBuffer = triangleDesc.IndexBuffer;
            convertedTriangleDesc.IndexCount = triangleDesc.IndexCount;
            convertedTriangleDesc.IndexFormat = triangleDesc.IndexFormat;
            convertedTriangleDesc.Transform = triangleDesc.Transform3x4;
            convertedTriangleDesc.VertexBuffer = triangleDesc.VertexBuffer;
            convertedTriangleDesc.VertexCount = triangleDesc.VertexCount;
            convertedTriangleDesc.VertexFormat = triangleDesc.VertexFormat;
        }
        else
        {
            convertedDesc.AABBs = geomDesc.AABBs;
        }
    }
}

class ExperimentalRaytracingCommandList : public ID3D12RaytracingFallbackCommandList
{
public:
    ExperimentalRaytracingCommandList(ID3D12GraphicsCommandList *pCommandList) :
        m_pCommandList(pCommandList)
    {
        ThrowFailure(m_pCommandList->QueryInterface(&m_pRaytracingCommandList));
    }

    virtual ~ExperimentalRaytracingCommandList() {}


    virtual void STDMETHODCALLTYPE BuildRaytracingAccelerationStructure(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_  UINT NumPostbuildInfoDescs,
        _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs)
    {
        D3D12_EXPERIMENTAL_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
        buildDesc.DescsLayout = pDesc->Inputs.DescsLayout;
        buildDesc.DestAccelerationStructureData = { pDesc->DestAccelerationStructureData, 0 };
        buildDesc.Flags = pDesc->Inputs.Flags;
        buildDesc.NumDescs = pDesc->Inputs.NumDescs;
        buildDesc.ScratchAccelerationStructureData = { pDesc->ScratchAccelerationStructureData, 0 };
        buildDesc.SourceAccelerationStructureData = pDesc->SourceAccelerationStructureData;
        buildDesc.Type = pDesc->Inputs.Type;

        std::vector<D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC> geometryDescs;
        if (pDesc->Inputs.Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
        {
            buildDesc.InstanceDescs = pDesc->Inputs.InstanceDescs;
        }
        else
        {
            ConvertGeometryDescs(
                pDesc->Inputs.NumDescs, 
                pDesc->Inputs.DescsLayout, 
                pDesc->Inputs.pGeometryDescs, 
                pDesc->Inputs.ppGeometryDescs, geometryDescs);
            buildDesc.pGeometryDescs = geometryDescs.data();
            buildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        }
        m_pRaytracingCommandList->BuildRaytracingAccelerationStructure(&buildDesc);
        if (NumPostbuildInfoDescs > 0)
        {
            std::vector<D3D12_GPU_VIRTUAL_ADDRESS> accelerationStructureAddresses(NumPostbuildInfoDescs);
            std::fill(accelerationStructureAddresses.begin(), accelerationStructureAddresses.end(), pDesc->DestAccelerationStructureData);
            EmitRaytracingAccelerationStructurePostbuildInfo(pPostbuildInfoDescs, NumPostbuildInfoDescs, accelerationStructureAddresses.data());
        }
    }

    virtual void STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostbuildInfo(
        _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData)
    {
        m_pRaytracingCommandList->EmitRaytracingAccelerationStructurePostBuildInfo(
            { pDesc->DestBuffer, 0 },
            pDesc->InfoType,
            NumSourceAccelerationStructures,
            pSourceAccelerationStructureData);
    }

    virtual void STDMETHODCALLTYPE CopyRaytracingAccelerationStructure(
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Flags)
    {
        m_pRaytracingCommandList->CopyRaytracingAccelerationStructure(
            { DestAccelerationStructureData, 0 },
            SourceAccelerationStructureData,
            Flags);
    }

    virtual void STDMETHODCALLTYPE SetDescriptorHeaps(
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps)
    {
        m_pCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
    }

    virtual void STDMETHODCALLTYPE SetTopLevelAccelerationStructure(
        _In_  UINT RootParameterIndex,
        _In_  WRAPPED_GPU_POINTER  BufferLocation)
    {
        m_pCommandList->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation.GpuVA);
    }

    virtual void STDMETHODCALLTYPE DispatchRays(
        _In_  const D3D12_DISPATCH_RAYS_DESC *pDesc)
    {
        if(pDesc->Depth > 1) 
        {
            ThrowFailure(E_INVALIDARG);
        }

        D3D12_EXPERIMENTAL_DISPATCH_RAYS_DESC experimentalDesc;
        experimentalDesc.CallableShaderTable = pDesc->CallableShaderTable;
        experimentalDesc.Height = pDesc->Height;
        experimentalDesc.Width = pDesc->Width;
        experimentalDesc.HitGroupTable = pDesc->HitGroupTable;
        experimentalDesc.MissShaderTable = pDesc->MissShaderTable;
        experimentalDesc.RayGenerationShaderRecord = pDesc->RayGenerationShaderRecord;
        m_pRaytracingCommandList->DispatchRays((ID3D12StateObjectPrototype *)m_pStateObject->GetStateObject(), &experimentalDesc);
    }

    virtual void STDMETHODCALLTYPE SetPipelineState1(
        _In_  ID3D12RaytracingFallbackStateObject *pStateObject)
    {
        m_pStateObject = pStateObject;
    }
private:
    
    
    CComPtr<ID3D12RaytracingFallbackStateObject> m_pStateObject;
    CComPtr<ID3D12GraphicsCommandList> m_pCommandList;
    CComPtr<ID3D12CommandListRaytracingPrototype> m_pRaytracingCommandList;
    COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pRaytracingCommandList.p);
};

class ExperimentalRaytracingDevice : public ID3D12RaytracingFallbackDevice
{

public:
    ExperimentalRaytracingDevice(ID3D12Device *pDevice) : m_pDevice(pDevice)
    {
        ThrowFailure(m_pDevice->QueryInterface(&m_pRaytracingDevice));
    }

    virtual ~ExperimentalRaytracingDevice() {}

    virtual bool UsingRaytracingDriver() { return true; };

    // Automatically determine how to create WRAPPED_GPU_POINTER based on UsingRaytracingDriver()
    virtual WRAPPED_GPU_POINTER GetWrappedPointerSimple(UINT32 DescriptorHeapIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuVA)
    {
        UNREFERENCED_PARAMETER(DescriptorHeapIndex);
        return GetWrappedPointerFromGpuVA(gpuVA);
    }

    virtual WRAPPED_GPU_POINTER GetWrappedPointerFromDescriptorHeapIndex(UINT32, UINT32)
    {
        ThrowFailure(E_INVALIDARG,
            L"GetWrappedPointerFromDescriptorHeapIndex is invalid when using the Fallback Layer with a Raytracing Driver. Apps should either use GetWrappedPointerSimple or "
            L"GetWrappedPointerFromGpuVA() if it's been verified that ID3D12RaytracingFallbackDevice::UsingRaytracingDriver() == true");
        return WRAPPED_GPU_POINTER();
    }

    virtual WRAPPED_GPU_POINTER GetWrappedPointerFromGpuVA(D3D12_GPU_VIRTUAL_ADDRESS gpuVA)
    {
        WRAPPED_GPU_POINTER pointer;
        pointer.GpuVA = gpuVA;
        return pointer;
    }

    virtual D3D12_RESOURCE_STATES GetAccelerationStructureResourceState()
    {
        return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    virtual HRESULT STDMETHODCALLTYPE CreateStateObject(
        const D3D12_STATE_OBJECT_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_  void **ppStateObject)
    {
        if (!ppStateObject || riid != __uuidof(ID3D12RaytracingFallbackStateObject))
        {
            ThrowFailure(E_INVALIDARG, L"Null ppStateObject passed in or invalid riid");
        }

        std::vector<D3D12_STATE_SUBOBJECT> patchedSubobjects(pDesc->NumSubobjects);
        std::vector<D3D12_EXPERIMENTAL_HIT_GROUP_DESC> cachedHitGroupDescs;
        cachedHitGroupDescs.reserve(pDesc->NumSubobjects); // Ensure no re-allocations are done
        memcpy(patchedSubobjects.data(), pDesc->pSubobjects, sizeof(D3D12_STATE_SUBOBJECT) * pDesc->NumSubobjects);
        for (UINT i = 0; i < pDesc->NumSubobjects; i++)
        {
            auto &subobject = patchedSubobjects[i] = pDesc->pSubobjects[i];
            switch (subobject.Type)
            {
            case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
                {
                    auto &hitGroupDesc = *(D3D12_HIT_GROUP_DESC*)subobject.pDesc;
                    D3D12_EXPERIMENTAL_HIT_GROUP_DESC experimentalHitGroupDesc;
                    experimentalHitGroupDesc.AnyHitShaderImport = hitGroupDesc.AnyHitShaderImport;
                    experimentalHitGroupDesc.ClosestHitShaderImport = hitGroupDesc.ClosestHitShaderImport;
                    experimentalHitGroupDesc.HitGroupExport = hitGroupDesc.HitGroupExport;
                    experimentalHitGroupDesc.IntersectionShaderImport = hitGroupDesc.IntersectionShaderImport;
                    cachedHitGroupDescs.push_back(experimentalHitGroupDesc);
                    subobject.pDesc = &cachedHitGroupDescs.back();
                    break;
                }
            case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
                {
                    auto &association = *(D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*)subobject.pDesc;
                    UINT shaderAssociationIndex = (UINT)(association.pSubobjectToAssociate - pDesc->pSubobjects);
                    association.pSubobjectToAssociate = &patchedSubobjects[shaderAssociationIndex];
                    break;
                }
            }
        }

        D3D12_STATE_OBJECT_DESC patchedStateObject = *pDesc;
        patchedStateObject.pSubobjects = patchedSubobjects.data();
        CComPtr<ID3D12StateObjectPrototype> pStateObject;
        HRESULT hr = m_pRaytracingDevice->CreateStateObject(&patchedStateObject, IID_PPV_ARGS(&pStateObject));

        if (SUCCEEDED(hr))
        {
            *ppStateObject = new ExperimentalRaytracingStateObject(pStateObject);
            hr = *ppStateObject ? S_OK : E_OUTOFMEMORY;
        }

        return  hr;
    }

    virtual UINT STDMETHODCALLTYPE GetShaderIdentifierSize(void)
    {
        return m_pRaytracingDevice->GetShaderIdentifierSize();
    }

    virtual void STDMETHODCALLTYPE GetRaytracingAccelerationStructurePrebuildInfo(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
        _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
    {
        D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC getPrebuildInfo;
        getPrebuildInfo.DescsLayout = pDesc->DescsLayout;
        getPrebuildInfo.Flags = pDesc->Flags;
        getPrebuildInfo.NumDescs = pDesc->NumDescs;
        getPrebuildInfo.Type = pDesc->Type;

        std::vector<D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC> geomDescs;
        if (getPrebuildInfo.Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
        {
            ConvertGeometryDescs(
                pDesc->NumDescs,
                pDesc->DescsLayout,
                pDesc->pGeometryDescs,
                pDesc->ppGeometryDescs, 
                geomDescs);

            getPrebuildInfo.pGeometryDescs = geomDescs.data();
            getPrebuildInfo.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        }

        m_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(
            &getPrebuildInfo,
            pInfo);
    }

    virtual void QueryRaytracingCommandList(ID3D12GraphicsCommandList *pCommandList, 
        REFIID riid,
        _COM_Outptr_  void **ppRaytracingCommandList)
    {
        if (riid == __uuidof(ID3D12RaytracingFallbackCommandList))
        {
            *ppRaytracingCommandList = new ExperimentalRaytracingCommandList(pCommandList);
        }
    }

    virtual HRESULT STDMETHODCALLTYPE CreateRootSignature(
        _In_  UINT nodeMask,
        _In_reads_(blobLengthInBytes)  const void *pBlobWithRootSignature,
        _In_  SIZE_T blobLengthInBytes,
        REFIID riid,
        _COM_Outptr_  void **ppvRootSignature)
    {
        return m_pDevice->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);
    }

    virtual HRESULT D3D12SerializeVersionedRootSignature(
        _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
        _Out_ ID3DBlob** ppBlob,
        _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
    {
        return ::D3D12SerializeVersionedRootSignature(pRootSignature, ppBlob, ppErrorBlob);
    }

    virtual HRESULT WINAPI D3D12SerializeRootSignature(
        _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
        _In_ D3D_ROOT_SIGNATURE_VERSION Version,
        _Out_ ID3DBlob** ppBlob,
        _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
    {
        return ::D3D12SerializeRootSignature(pRootSignature, Version, ppBlob, ppErrorBlob);
    }
private:
    CComPtr<ID3D12DeviceRaytracingPrototype> m_pRaytracingDevice;
    CComPtr<ID3D12Device> m_pDevice;
    COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pDevice.p);
};
