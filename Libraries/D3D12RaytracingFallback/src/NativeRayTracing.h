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

class NativeRaytracingStateObject : public ID3D12RaytracingFallbackStateObject
{
public:
    NativeRaytracingStateObject(ID3D12StateObject *pStateObject) : m_pStateObject(pStateObject)
    {
        ThrowFailure(m_pStateObject->QueryInterface(&m_pStateObjectProperties));
    }

    virtual ~NativeRaytracingStateObject() {}

    virtual void *STDMETHODCALLTYPE GetShaderIdentifier(LPCWSTR pExportName)
    {
        return m_pStateObjectProperties->GetShaderIdentifier(pExportName);
    }

    virtual UINT64 STDMETHODCALLTYPE GetShaderStackSize(_In_  LPCWSTR pExportName)
    {
        return m_pStateObjectProperties->GetShaderStackSize(pExportName);
    }

    virtual UINT64 STDMETHODCALLTYPE GetPipelineStackSize(void)
    {
        return m_pStateObjectProperties->GetPipelineStackSize();
    }

    virtual void STDMETHODCALLTYPE SetPipelineStackSize(
        UINT64 PipelineStackSizeInBytes)
    {
        m_pStateObjectProperties->SetPipelineStackSize(PipelineStackSizeInBytes);
    }

    virtual ID3D12StateObject *GetStateObject()
    {
        return m_pStateObject;
    }
private:
    CComPtr<ID3D12StateObjectProperties> m_pStateObjectProperties;
    CComPtr<ID3D12StateObject> m_pStateObject;
    COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pStateObject.p);
};

class NativeRaytracingCommandList : public ID3D12RaytracingFallbackCommandList
{
public:
    NativeRaytracingCommandList(ID3D12GraphicsCommandList *pCommandList) 
    {
        ThrowFailure(pCommandList->QueryInterface(&m_pCommandList), 
            L"QueryInterface for ID3D12GraphicsCommandList4 failed");
    }

    virtual ~NativeRaytracingCommandList() {}


    virtual void STDMETHODCALLTYPE BuildRaytracingAccelerationStructure(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_  UINT NumPostbuildInfoDescs,
        _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs)
    {
        m_pCommandList->BuildRaytracingAccelerationStructure(
            pDesc, 
            NumPostbuildInfoDescs, 
            pPostbuildInfoDescs);
    }

    virtual void STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostbuildInfo(
        _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData)
    {
        m_pCommandList->EmitRaytracingAccelerationStructurePostbuildInfo(
            pDesc,
            NumSourceAccelerationStructures,
            pSourceAccelerationStructureData);
    }

    virtual void STDMETHODCALLTYPE CopyRaytracingAccelerationStructure(
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Flags)
    {
        m_pCommandList->CopyRaytracingAccelerationStructure(
            DestAccelerationStructureData,
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
        m_pCommandList->DispatchRays(pDesc);
    }

    virtual void STDMETHODCALLTYPE SetPipelineState1(
        _In_  ID3D12RaytracingFallbackStateObject *pStateObject)
    {
        NativeRaytracingStateObject *pNativeStateObject = reinterpret_cast<NativeRaytracingStateObject *>(pStateObject);
        m_pCommandList->SetPipelineState1(pNativeStateObject->GetStateObject());
    }
private:
    CComPtr<ID3D12GraphicsCommandList4> m_pCommandList;
    COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pCommandList.p);
};

class NativeRaytracingDevice : public ID3D12RaytracingFallbackDevice
{

public:
    NativeRaytracingDevice(ID3D12Device *pDevice)
    {
        ThrowFailure(pDevice->QueryInterface(&m_pDevice));
    }

    virtual ~NativeRaytracingDevice() {}

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

        CComPtr<ID3D12StateObjectPrototype> pStateObject;
        HRESULT hr = m_pDevice->CreateStateObject(pDesc, IID_PPV_ARGS(&pStateObject));

        if (SUCCEEDED(hr))
        {
            *ppStateObject = new NativeRaytracingStateObject(pStateObject);
            hr = *ppStateObject ? S_OK : E_OUTOFMEMORY;
        }

        return  hr;
    }

    virtual UINT STDMETHODCALLTYPE GetShaderIdentifierSize(void)
    {
        return D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    virtual void STDMETHODCALLTYPE GetRaytracingAccelerationStructurePrebuildInfo(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
        _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
    {
        m_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(
            pDesc,
            pInfo);
    }

    virtual void QueryRaytracingCommandList(ID3D12GraphicsCommandList *pCommandList, 
        REFIID riid,
        _COM_Outptr_  void **ppRaytracingCommandList)
    {
        if (riid == __uuidof(ID3D12RaytracingFallbackCommandList))
        {
            *ppRaytracingCommandList = new NativeRaytracingCommandList(pCommandList);
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
    CComPtr<ID3D12Device5> m_pDevice;
    COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pDevice.p);
};
