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

namespace FallbackLayer
{
    extern GUID FallbackLayerBlobPrivateDataGUID;
    extern GUID FallbackLayerPatchedParameterStartGUID;

    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *GetDescFromRootSignature(
        ID3D12RootSignature *pRootSignature,
        ID3D12VersionedRootSignatureDeserializer *pDeserializer);

    class RaytracingDevice;

    class D3D12RaytracingCommandList : public ID3D12RaytracingFallbackCommandList
    {
    public:
        D3D12RaytracingCommandList(RaytracingDevice &device, ID3D12GraphicsCommandList *pCommandList) : m_device(device), m_pCommandList(pCommandList) {}
        virtual ~D3D12RaytracingCommandList() {}

        virtual void STDMETHODCALLTYPE BuildRaytracingAccelerationStructure(
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_  UINT NumPostbuildInfoDescs,
            _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs);

        virtual void STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostbuildInfo(
            _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
            _In_  UINT NumSourceAccelerationStructures,
            _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData);

        virtual void STDMETHODCALLTYPE CopyRaytracingAccelerationStructure(
            _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode);

        virtual void STDMETHODCALLTYPE SetDescriptorHeaps(
            _In_  UINT NumDescriptorHeaps,
            _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps);

        virtual void STDMETHODCALLTYPE D3D12RaytracingCommandList::SetTopLevelAccelerationStructure(
            _In_  UINT RootParameterIndex,
            _In_  WRAPPED_GPU_POINTER  BufferLocation);

        virtual void STDMETHODCALLTYPE SetPipelineState1(
            _In_  ID3D12RaytracingFallbackStateObject *pStateObject)
        {
            m_pStateObject = pStateObject;
        }

        virtual void STDMETHODCALLTYPE DispatchRays(
            _In_  const D3D12_DISPATCH_RAYS_DESC *pDesc);
    private:
        enum DescriptorHeapType
        {
            SrvUavCbvType = 0,
            SamplerType,
            NumTypes
        };

        static DescriptorHeapType D3D12EnumToFallbackEnum(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
        {
            switch (heapType)
            {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
                return SrvUavCbvType;
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                return SamplerType;
            default:
                ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_DESCRIPTOR_HEAP_TYPE type");
                return (DescriptorHeapType )-1;
            }
        }

        CComPtr<ID3D12RaytracingFallbackStateObject> m_pStateObject;
        std::unordered_map<UINT, WRAPPED_GPU_POINTER> m_BoundAccelerationStructures;
        ID3D12DescriptorHeap *m_pBoundDescriptorHeaps[DescriptorHeapType::NumTypes] = {};
        ATL::CComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        RaytracingDevice &m_device;
        COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pCommandList.p);
    };

    class RaytracingStateObject : public ID3D12RaytracingFallbackStateObject
    {
    public:
        virtual ~RaytracingStateObject() {}

        virtual void *STDMETHODCALLTYPE GetShaderIdentifier(LPCWSTR pExportName);
        virtual UINT64 STDMETHODCALLTYPE GetShaderStackSize(_In_  LPCWSTR pExportName);

        virtual UINT64 STDMETHODCALLTYPE GetPipelineStackSize(void)
        {
            return m_collection.m_pipelineStackSize;
        }
        virtual void STDMETHODCALLTYPE SetPipelineStackSize(UINT64 PipelineStackSizeInBytes)
        {
            m_collection.m_pipelineStackSize = PipelineStackSizeInBytes;
        }

        virtual ID3D12StateObject *GetStateObject()
        {
            return (ID3D12StateObject *)this;
        }

        CStateObjectInfo &GetCachedStateInfo()
        {
            return m_collection.m_stateObjectInfo;
        }
    private:

        StateObjectCollection m_collection;
        std::unique_ptr<IRaytracingProgram> m_spProgram;
        friend RaytracingDevice;
        friend D3D12RaytracingCommandList;
        COM_IMPLEMENTATION();
    };

    WRAPPED_GPU_POINTER CreateWrappedPointer(UINT32 DescriptorHeapIndex, UINT32 OffsetInBytes = 0);

    class RaytracingDevice : public ID3D12RaytracingFallbackDevice
    {
    public:
        RaytracingDevice(ID3D12Device *pDevice, UINT NodeMask, DWORD createRaytracingFallbackDeviceFlags);
        virtual ~RaytracingDevice() {}

        virtual bool UsingRaytracingDriver();

        virtual WRAPPED_GPU_POINTER GetWrappedPointerSimple(UINT32 DescriptorHeapIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuVA);
        virtual WRAPPED_GPU_POINTER GetWrappedPointerFromDescriptorHeapIndex(UINT32 DescriptorHeapIndex, UINT32 OffsetInBytes = 0);
        virtual WRAPPED_GPU_POINTER GetWrappedPointerFromGpuVA(D3D12_GPU_VIRTUAL_ADDRESS gpuVA);
        virtual D3D12_RESOURCE_STATES GetAccelerationStructureResourceState();


        virtual HRESULT STDMETHODCALLTYPE CreateStateObject(
            const D3D12_STATE_OBJECT_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppStateObject);

        virtual UINT STDMETHODCALLTYPE GetShaderIdentifierSize(void);

        virtual void STDMETHODCALLTYPE GetRaytracingAccelerationStructurePrebuildInfo(
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo);

        virtual void QueryRaytracingCommandList(ID3D12GraphicsCommandList *pCommandList, 
            REFIID riid,
            _COM_Outptr_  void **ppRaytracingCommandList)
        {
            if (riid == __uuidof(ID3D12RaytracingFallbackCommandList))
            {
                *ppRaytracingCommandList = new D3D12RaytracingCommandList(*this, pCommandList);
            }
        }

        AccelerationStructureBuilderFactory &GetAccelerationStructureBuilderFactory()
        {
            return m_AccelerationStructureBuilderFactory;
        }

        virtual HRESULT STDMETHODCALLTYPE CreateRootSignature(
            _In_  UINT nodeMask,
            _In_reads_(blobLengthInBytes)  const void *pBlobWithRootSignature,
            _In_  SIZE_T blobLengthInBytes,
            REFIID riid,
            _COM_Outptr_  void **ppvRootSignature);

        virtual HRESULT D3D12SerializeVersionedRootSignature(
            _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
            _Out_ ID3DBlob** ppBlob,
            _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob);

        virtual HRESULT WINAPI D3D12SerializeRootSignature(
            _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
            _In_ D3D_ROOT_SIGNATURE_VERSION Version,
            _Out_ ID3DBlob** ppBlob,
            _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob);

        bool AreShaderRecordRootDescriptorsEnabled()
        {
            return m_flags & CreateRaytracingFallbackDeviceFlags::EnableRootDescriptorsInShaderRecords;
        }

    private:
        void ProcessStateObject(_In_ const D3D12_STATE_OBJECT_DESC &stateObject, _Out_ RaytracingStateObject &rayTracingStateObject);
        ShaderAssociations ProcessAssociations(_In_ LPCWSTR exportName, _Inout_ RaytracingStateObject &rayTracingStateObject);

        friend D3D12RaytracingCommandList;
        CComPtr<ID3D12Device> m_pDevice;
        AccelerationStructureBuilderFactory m_AccelerationStructureBuilderFactory;
        RaytracingProgramFactory m_RaytracingProgramFactory;
        DWORD m_flags;

        COM_IMPLEMENTATION_WITH_QUERYINTERFACE(m_pDevice.p)

#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
    public:
        void VisualizeAccelerationStructureLevel(UINT levelToVisualize);
    private:
        // Initialize to obscenely high so that all leaf geometry
        // is rendered
        UINT m_levelToVisualize = 100; 
#endif

#if ENABLE_UAV_LOG
        CComPtr<ID3D12Resource> m_pZeroBuffer;
        CComPtr<ID3D12Resource> m_pUAVLog;
        CComPtr<ID3D12Resource> m_pUAVLogReadback;

    public:
        static const UINT MaxLogEntries = 1000;

        void ResetLog(ID3D12GraphicsCommandList *pCommandList)
        {
            pCommandList->CopyBufferRegion(m_pUAVLog, 0, m_pZeroBuffer, 0, m_pZeroBuffer->GetDesc().Width);
        }

        ID3D12Resource &GetUAVDebugLog()
        {
            return *m_pUAVLog;
        }

        ID3D12Resource &GetUAVDebugLogReadbackHeap()
        {
            return *m_pUAVLogReadback;
        }
#endif
    };

#if ENABLE_UAV_LOG
struct LogEntry
{
    UINT32 EntryType;
    UINT32 ValueA;
    UINT32 ValueB;
    UINT32 ValueC;
};
#endif

}// namespace FallbackLayer
