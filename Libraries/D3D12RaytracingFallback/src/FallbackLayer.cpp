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
#include "pch.h"

namespace FallbackLayer
{
    GUID FallbackLayerBlobPrivateDataGUID = { 0xf0545791, 0x860b, 0x472e, 0x9c, 0xc5, 0x84, 0x2c, 0xf1, 0x4e, 0x37, 0x60 };
    GUID FallbackLayerPatchedParameterStartGUID = { 0xea063348, 0x974e, 0x4227, 0x82, 0x55, 0x34, 0x5e, 0x29, 0x14, 0xeb, 0x7f };

    RaytracingDevice::RaytracingDevice(ID3D12Device *pDevice, UINT NodeMask, DWORD createRaytracingFallbackDeviceFlags) :
        m_pDevice(pDevice), m_RaytracingProgramFactory(pDevice), m_AccelerationStructureBuilderFactory(pDevice, NodeMask),
        m_flags(createRaytracingFallbackDeviceFlags)
    {
        // Earlier builds of windows may not support checking shader model yet so this cannot 
        // catch non-Dxil drivers on older builds.
        D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_0 };
        if (FAILED (m_pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel))) || 
            (supportedShaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
        {
            ThrowFailure(E_INVALIDARG, 
                L"The graphics driver does not have shader model 6.0 capabilities (DXIL support) minimally required by the Fallback Layer. For more info, check \"Known working drivers\" in the readme.md");
        }

        D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12Options;
        ThrowInternalFailure(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12Options, sizeof(d3d12Options)));
        if (d3d12Options.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3)
        {
            ThrowFailure(E_INVALIDARG,
                L"The graphics driver does not support resource binding tier 3 (bindless). For more info, check \"Known working drivers\" in the readme.md");
        }

#if ENABLE_UAV_LOG
        UINT uavLogSize = sizeof(LogEntry) * MaxLogEntries;
        {
            auto uavLogDesc = CD3DX12_RESOURCE_DESC::Buffer(uavLogSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            ThrowInternalFailure(m_pDevice->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavLogDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pUAVLog)));
        }

        {
            auto uavLogReadbackDesc = CD3DX12_RESOURCE_DESC::Buffer(uavLogSize);
            auto readbackHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
            ThrowInternalFailure(m_pDevice->CreateCommittedResource(&readbackHeapProperties, D3D12_HEAP_FLAG_NONE, &uavLogReadbackDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pUAVLogReadback)));
        }

        {
            auto zeroBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(LogEntry));
            auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            ThrowInternalFailure(m_pDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &zeroBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pZeroBuffer)));

            LogEntry *entry;
            m_pZeroBuffer->Map(0, nullptr, (void**)&entry);
            ZeroMemory(entry, sizeof(*entry));
        }
#endif

    }


    bool RaytracingDevice::UsingRaytracingDriver()
    {
        return false;
    }

    D3D12_RESOURCE_STATES RaytracingDevice::GetAccelerationStructureResourceState()
    {
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    WRAPPED_GPU_POINTER RaytracingDevice::GetWrappedPointerSimple(UINT32 DescriptorHeapIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuVA)
    {
        UNREFERENCED_PARAMETER(gpuVA);
        return GetWrappedPointerFromDescriptorHeapIndex(DescriptorHeapIndex);
    }

    WRAPPED_GPU_POINTER CreateWrappedPointer(UINT32 DescriptorHeapIndex, UINT32 OffsetInBytes)
    {
        WRAPPED_GPU_POINTER wrappedPointer;
        wrappedPointer.EmulatedGpuPtr.DescriptorHeapIndex = DescriptorHeapIndex;
        wrappedPointer.EmulatedGpuPtr.OffsetInBytes = OffsetInBytes;
        return wrappedPointer;
    }

    WRAPPED_GPU_POINTER RaytracingDevice::GetWrappedPointerFromDescriptorHeapIndex(UINT32 DescriptorHeapIndex, UINT32 OffsetInBytes)
    {
        assert(!UsingRaytracingDriver());
        return CreateWrappedPointer(DescriptorHeapIndex, OffsetInBytes);
    }

    WRAPPED_GPU_POINTER RaytracingDevice::GetWrappedPointerFromGpuVA(D3D12_GPU_VIRTUAL_ADDRESS)
    {
        // Should never be called when using the Fallback Layer
        ThrowFailure(E_INVALIDARG, 
            L"GetWrappedPointerFromGpuVA is invalid when using the compute-based Fallback. Apps should either use GetWrappedPointerSimple or "
            L"GetWrappedPointerFromDescriptorHeapIndex() if it's been verified that ID3D12RaytracingFallbackDevice::UsingRaytracingDriver() == false");
        return {};
    }


    HRESULT STDMETHODCALLTYPE RaytracingDevice::CreateRootSignature(
        _In_  UINT nodeMask,
        _In_reads_(blobLengthInBytes)  const void *pBlobWithRootSignature,
        _In_  SIZE_T blobLengthInBytes,
        REFIID riid,
        _COM_Outptr_  void **ppvRootSignature)
    {
        HRESULT hr;
        hr = m_pDevice->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);

        ID3D12Object *pRootSignatureObject = (ID3D12Object*)*ppvRootSignature;
        CComPtr<ID3D12VersionedRootSignatureDeserializer> pDeserializer;
        bool IsGlobalRootSignature = false;
        {
            if (SUCCEEDED(hr))
            {
                hr = D3D12CreateVersionedRootSignatureDeserializer(pBlobWithRootSignature, blobLengthInBytes, IID_PPV_ARGS(&pDeserializer));
            }

            if (SUCCEEDED(hr))
            {
                const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc = pDeserializer->GetUnconvertedRootSignatureDesc();
                UINT numParameters = GetNumParameters(*pRootSignatureDesc);
                if (numParameters >= RootSignatureParameterOffset::NumParameters)
                {
                    UINT hitRecordParameterSlot = numParameters - RootSignatureParameterOffset::NumParameters + RootSignatureParameterOffset::HitGroupRecord;
                    auto &hitRecordParameter = pRootSignatureDesc->Desc_1_1.pParameters[hitRecordParameterSlot];
                    // Only global root signatures get patched with extra bindings, so we determine if it's a global root signature
                    // by verifying that one patched of the bindings is where we would expect it to be. If it's not, we assume this root signature is
                    // a local root signature.
                    IsGlobalRootSignature = hitRecordParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV &&
                        hitRecordParameter.Descriptor.ShaderRegister == FallbackLayerHitGroupRecordByteAddressBufferRegister &&
                        hitRecordParameter.Descriptor.RegisterSpace == FallbackLayerRegisterSpace;
                }

                if (IsGlobalRootSignature)
                {
                    UINT patchedParameterStartOffset = numParameters - RootSignatureParameterOffset::NumParameters;
                    hr = pRootSignatureObject->SetPrivateData(FallbackLayerPatchedParameterStartGUID, sizeof(patchedParameterStartOffset), &patchedParameterStartOffset);
                }
            }
        }

        bool IsLocalRootSignature = !IsGlobalRootSignature;
        if(IsLocalRootSignature)
        {
            // Save off blob so that it can be deserialized later
            hr = pRootSignatureObject->SetPrivateData(FallbackLayerBlobPrivateDataGUID, static_cast<UINT>(blobLengthInBytes), pBlobWithRootSignature);
        }

        return hr;
    }

    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *GetDescFromRootSignature(ID3D12RootSignature *pRootSignature, ID3D12VersionedRootSignatureDeserializer *pDeserializer)
    {
        UINT blobSize;
        pRootSignature->GetPrivateData(FallbackLayerBlobPrivateDataGUID, &blobSize, nullptr);
        std::unique_ptr<BYTE[]> pBlobData = std::unique_ptr<BYTE[]>(new BYTE[blobSize]);
        pRootSignature->GetPrivateData(FallbackLayerBlobPrivateDataGUID, &blobSize, pBlobData.get());

        D3D12CreateVersionedRootSignatureDeserializer(pBlobData.get(), blobSize, IID_PPV_ARGS(&pDeserializer));
        return pDeserializer->GetUnconvertedRootSignatureDesc();
    }

    template <typename TD3D12_ROOT_SIGNATURE_DESC, typename TD3DX12_ROOT_PARAMETER, typename TD3DX12_DESCRIPTOR_RANGE>
    TD3D12_ROOT_SIGNATURE_DESC PatchRootSignature(
        _In_ const typename TD3D12_ROOT_SIGNATURE_DESC*pRootSignature,
        _In_ bool localRootDescriptorsEnabled,
        _Out_ std::vector<TD3DX12_ROOT_PARAMETER> &patchedRootParameters,
        _Out_ std::vector<TD3DX12_DESCRIPTOR_RANGE> &patchedRanges,
        _Out_ TD3D12_ROOT_SIGNATURE_DESC &patchedRootSignatureDesc)
    {
        bool bLocalRootSignature = pRootSignature->Flags & D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        bool bGlobalRootSignature = !bLocalRootSignature;
        if (bLocalRootSignature)
        {
            for (UINT i = 0; i < pRootSignature->NumParameters; i++)
            {
                if (!localRootDescriptorsEnabled &&
                      (pRootSignature->pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
                       pRootSignature->pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV ||
                       pRootSignature->pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV))
                {
                    ThrowFailure(E_INVALIDARG,
                        L"Root Views, i.e. D3D12_ROOT_PARAMETER_TYPE_(CBV|UAV|SRV), are not supported by the Fallback Layer by default. "
                        "Support can be forced on by using CreateRaytracingFallbackDeviceFlags::EnableRootDescriptorsInShaderRecords, "
                        "however there are 2 existing limitations to root descriptors: The first is that root descriptors must be bound "
                        "using WRAPPED_GPU_POINTER rather than a GPU_VIRTUAL_ADDRESS. The second is that offsets added onto a WRAPPED_GPU_POINTER "
                        "are ignored, so reads from the buffer will always start at the start of the buffer. "
                        "Please see the developer guide for more details");
                }
            }
        }

        patchedRootSignatureDesc = *pRootSignature;
        patchedRootSignatureDesc.Flags = pRootSignature->Flags & ~(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
        if (bGlobalRootSignature)
        {
            patchedRootParameters.resize(pRootSignature->NumParameters + RootSignatureParameterOffset::NumParameters);

            TD3DX12_ROOT_PARAMETER *pOriginalParameters = (TD3DX12_ROOT_PARAMETER*)pRootSignature->pParameters;
            std::copy(pOriginalParameters, pOriginalParameters + pRootSignature->NumParameters, patchedRootParameters.begin());

            UINT CbvSrvUavParamterCount = 0;
            D3D12_DESCRIPTOR_RANGE_TYPE descriptorTypes[] = {
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                D3D12_DESCRIPTOR_RANGE_TYPE_UAV,

                D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER };

            for (auto descriptorType : descriptorTypes)
            {
                UINT numSpacesNeeded = 1;
                if (descriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV || descriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
                {
                    numSpacesNeeded = FallbackLayerNumDescriptorHeapSpacesPerView;
                }

                auto range = TD3DX12_DESCRIPTOR_RANGE(
                    descriptorType, UINT_MAX, FallbackLayerDescriptorHeapTable, FallbackLayerRegisterSpace + FallbackLayerDescriptorHeapSpaceOffset);
                range.OffsetInDescriptorsFromTableStart = 0;
                __if_exists(TD3DX12_DESCRIPTOR_RANGE::Flags)
                {
                    if (range.RangeType != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                    {
                        range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
                    }
                }

                for (UINT i = 0; i < numSpacesNeeded; i++)
                {
                    patchedRanges.push_back(range);
                    range.RegisterSpace++;
                }

                if (descriptorType != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                {
                    CbvSrvUavParamterCount += numSpacesNeeded;
                }
            }

            UINT patchedParameterOffset = pRootSignature->NumParameters;
            patchedRootParameters[patchedParameterOffset + HitGroupRecord].InitAsShaderResourceView(FallbackLayerHitGroupRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
            patchedRootParameters[patchedParameterOffset + MissShaderRecord].InitAsShaderResourceView(FallbackLayerMissShaderRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
            patchedRootParameters[patchedParameterOffset + RayGenShaderRecord].InitAsShaderResourceView(FallbackLayerRayGenShaderRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
            patchedRootParameters[patchedParameterOffset + CallableShaderRecord].InitAsShaderResourceView(FallbackLayerCallableShaderRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
            patchedRootParameters[patchedParameterOffset + DispatchConstants].InitAsConstants(SizeOfInUint32(DispatchRaysConstants), FallbackLayerDispatchConstantsRegister, FallbackLayerRegisterSpace);
            patchedRootParameters[patchedParameterOffset + CbvSrvUavDescriptorHeapAliasedTables].InitAsDescriptorTable(CbvSrvUavParamterCount, patchedRanges.data());
            patchedRootParameters[patchedParameterOffset + SamplerDescriptorHeapAliasedTables].InitAsDescriptorTable(1, patchedRanges.data() + CbvSrvUavParamterCount);
            patchedRootParameters[patchedParameterOffset + AccelerationStructuresList].InitAsConstants(SizeOfInUint32(WRAPPED_GPU_POINTER), FallbackLayerAccelerationStructureList, FallbackLayerRegisterSpace);
#if ENABLE_UAV_LOG
            patchedRootParameters[patchedParameterOffset + DebugUAVLog].InitAsUnorderedAccessView(UAVLogRegister, FallbackLayerRegisterSpace);
#endif
#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
            patchedRootParameters[patchedParameterOffset + DebugConstants].InitAsConstants(SizeOfInUint32(DebugVariables), DebugConstantRegister, FallbackLayerRegisterSpace);
#endif
            patchedRootSignatureDesc.pParameters = (decltype(patchedRootSignatureDesc.pParameters))patchedRootParameters.data();
            patchedRootSignatureDesc.NumParameters = static_cast<UINT>(patchedRootParameters.size());
        }

        return patchedRootSignatureDesc;
    }

    HRESULT RaytracingDevice::D3D12SerializeVersionedRootSignature(
        _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
        _Out_ ID3DBlob** ppBlob,
        _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
    {
        std::vector<CD3DX12_ROOT_PARAMETER> patchedRootParameters;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> patchedRanges;

        std::vector<CD3DX12_ROOT_PARAMETER1> patchedRootParameters1;
        std::vector<CD3DX12_DESCRIPTOR_RANGE1> patchedRanges1;

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC patchedDesc = *pRootSignature;
        switch (pRootSignature->Version)
        {
        case D3D_ROOT_SIGNATURE_VERSION_1_0:
            PatchRootSignature(&pRootSignature->Desc_1_0, AreShaderRecordRootDescriptorsEnabled(), patchedRootParameters, patchedRanges, patchedDesc.Desc_1_0);
            break;
        case D3D_ROOT_SIGNATURE_VERSION_1_1:
            PatchRootSignature(&pRootSignature->Desc_1_1, AreShaderRecordRootDescriptorsEnabled(), patchedRootParameters1, patchedRanges1, patchedDesc.Desc_1_1);
            break;
        }

        return ::D3D12SerializeVersionedRootSignature(&patchedDesc, ppBlob, ppErrorBlob);
    }
    HRESULT WINAPI RaytracingDevice::D3D12SerializeRootSignature(
        _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
        _In_ D3D_ROOT_SIGNATURE_VERSION Version,
        _Out_ ID3DBlob** ppBlob,
        _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
    {
        D3D12_ROOT_SIGNATURE_DESC patchedDesc;
        std::vector<CD3DX12_ROOT_PARAMETER> patchedRootParameters;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> patchedRanges;
        PatchRootSignature(pRootSignature, AreShaderRecordRootDescriptorsEnabled(), patchedRootParameters, patchedRanges, patchedDesc);

        if (patchedDesc.Flags & D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE)
        {
            patchedDesc.Flags &= ~(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
        }

        pRootSignature = &patchedDesc;

        return ::D3D12SerializeRootSignature(pRootSignature, Version, ppBlob, ppErrorBlob);
    }

    void STDMETHODCALLTYPE D3D12RaytracingCommandList::BuildRaytracingAccelerationStructure(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_  UINT NumPostbuildInfoDescs,
        _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs)
    {
#if USE_PIX_MARKERS
        PIXScopedEvent(m_pCommandList.p, FallbackPixColor, L"BuildRaytracingAccelerationStructure");
#endif
        auto &accelerationStructureBuilder = m_device.m_AccelerationStructureBuilderFactory.GetAccelerationStructureBuilder();
        accelerationStructureBuilder.BuildRaytracingAccelerationStructure(
            m_pCommandList,
            pDesc,
            m_pBoundDescriptorHeaps[SrvUavCbvType]);

        if (NumPostbuildInfoDescs)
        {
            accelerationStructureBuilder.EmitRaytracingAccelerationStructurePostbuildInfo(
                m_pCommandList,
                pPostbuildInfoDescs,
                NumPostbuildInfoDescs,
                &pDesc->DestAccelerationStructureData);
        }
    }

    ShaderAssociations RaytracingDevice::ProcessAssociations(_In_ LPCWSTR exportName, _Inout_ RaytracingStateObject &rayTracingStateObject)
    {
        auto &stateObjectCollection = rayTracingStateObject.m_collection;
        CStateObjectInfo::CAssociatedSubobjectIterator associationIterator(&stateObjectCollection.m_stateObjectInfo);
        associationIterator.ResetAndSelectExport(exportName, (D3D12_STATE_SUBOBJECT_TYPE)-1); // TODO: Should make this an explicit type
        UINT numAssociations = (UINT)associationIterator.GetCount();
        ShaderAssociations shaderAssociation = {};
        for (UINT associationIndex = 0; associationIndex < numAssociations; associationIndex++)
        {
            auto pStateObject = associationIterator.Next();
            switch (pStateObject->Type)
            {
            case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
                shaderAssociation.m_shaderConfig = *((D3D12_RAYTRACING_SHADER_CONFIG*)pStateObject->pDesc);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
                shaderAssociation.m_pRootSignature = ((D3D12_LOCAL_ROOT_SIGNATURE*)pStateObject->pDesc)->pLocalRootSignature;
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
            {
                auto *pGlobalRootSignature = ((D3D12_GLOBAL_ROOT_SIGNATURE*)pStateObject->pDesc)->pGlobalRootSignature;
                if (stateObjectCollection.m_pGlobalRootSignature && stateObjectCollection.m_pGlobalRootSignature != pGlobalRootSignature)
                {
                    ThrowFailure(E_NOTIMPL, L"The Fallback Layer currently only supports a single global root signature in the state object");
                }
                else
                {
                    stateObjectCollection.m_pGlobalRootSignature = pGlobalRootSignature;
                }
                break;
            }
            case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
            {
                stateObjectCollection.m_config = *((D3D12_RAYTRACING_PIPELINE_CONFIG*)pStateObject->pDesc);
                break;
            }
            }
        }
        stateObjectCollection.m_maxAttributeSizeInBytes = (UINT)std::max(
            (UINT)stateObjectCollection.m_maxAttributeSizeInBytes,
            (UINT)shaderAssociation.m_shaderConfig.MaxAttributeSizeInBytes);

        return shaderAssociation;
    }


    void RaytracingDevice::ProcessStateObject(_In_ const D3D12_STATE_OBJECT_DESC &stateObject, _Out_ RaytracingStateObject &rayTracingStateObject)
    {
        PFN_CALLBACK_GET_STATE_OBJECT_INFO_FOR_EXISTING_COLLECTION pfnGetStateObjectInfo = [](ID3D12StateObject *pStateObject)->CStateObjectInfo*
        {
            RaytracingStateObject *pRaytracingPipelineState = reinterpret_cast<RaytracingStateObject *>(pStateObject);
            return &pRaytracingPipelineState->GetCachedStateInfo();
        };

        auto &stateObjectCollection = rayTracingStateObject.m_collection;
        auto &stateObjectInfo = stateObjectCollection.m_stateObjectInfo;

        stateObjectInfo.ParseStateObject(&stateObject, pfnGetStateObjectInfo, GetRuntimeData, &stateObjectCollection.m_libraryCache);
        if (stateObjectInfo.GetLog().size())
        {
            std::wstring fullErrorString = L"State object creation failures due to: \n";
            auto &logList = stateObjectInfo.GetLog();
            for (auto &errorStr : logList)
            {
                fullErrorString += errorStr;
                fullErrorString += L"\n";
            }
            ThrowFailure(E_INVALIDARG, fullErrorString.c_str());
        }

        if (stateObject.Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE)
        {
            CStateObjectInfo::CExportedFunctionIterator exportIterator(&stateObjectInfo);
            UINT exportCount = (UINT)exportIterator.GetCount();
            for (UINT i = 0; i < exportCount; i++)
            {
                EXPORTED_FUNCTION exportedFunction;
                exportIterator.Next(&exportedFunction);

                if ((ShaderKind)exportedFunction.pDXILFunction->ShaderKind != ShaderKind::Library)
                {
                    auto shaderAssociation = ProcessAssociations(exportedFunction.MangledName, rayTracingStateObject);
                    shaderAssociation.m_exportToRename = exportedFunction.pDXILFunction->UnmangledName;
                    shaderAssociation.m_mangledExport = exportedFunction.MangledName;
                    DxcExportDesc desc = {};
                    desc.ExportName = exportedFunction.MangledName;
                    bool bRename = std::wstring(exportedFunction.UnmangledName).compare(exportedFunction.pDXILFunction->UnmangledName) != 0;
                    if (bRename)
                    {
                        desc.ExportToRename = exportedFunction.pDXILFunction->Name;
                        shaderAssociation.m_mangledExportToRename = exportedFunction.pDXILFunction->Name;
                    }

                    stateObjectCollection.m_exportDescs.push_back(desc);
                    stateObjectCollection.m_exportNames.push_back(exportedFunction.UnmangledName);
                    stateObjectCollection.m_shaderAssociations[exportedFunction.UnmangledName] = shaderAssociation;
                }
            }

            {
                auto pStateSubobject = stateObjectInfo.GetGloballyAssociatedSubobject(D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK);
                if (pStateSubobject && pStateSubobject->pDesc)
                {
                    stateObjectCollection.m_nodeMask = ((D3D12_NODE_MASK*)pStateSubobject->pDesc)->NodeMask;
                }
            }

            CStateObjectInfo::CExportedHitGroupIterator hitGroupIterator(&stateObjectInfo);
            UINT hitGroupCount = (UINT)hitGroupIterator.GetCount();
            for (UINT i = 0; i < hitGroupCount; i++)
            {
                EXPORTED_HIT_GROUP hitGroupExport;
                hitGroupIterator.Next(&hitGroupExport);

                if (hitGroupExport.pHitGroup && hitGroupExport.pHitGroup->pDesc)
                {
                    auto &hitGroup = *(D3D12_HIT_GROUP_DESC*)hitGroupExport.pHitGroup->pDesc;

                    stateObjectCollection.m_hitGroups[hitGroup.HitGroupExport] = hitGroup;
                    if (hitGroup.AnyHitShaderImport)
                    {
                        stateObjectCollection.IsUsingAnyHit = true;
                    }
                    if (hitGroup.IntersectionShaderImport)
                    {
                        stateObjectCollection.IsUsingIntersection = true;
                    }
                }
            }

            CStateObjectInfo::CDxilLibraryIterator dxilLibIterator(&stateObjectInfo);
            UINT dxilLibCount = (UINT)dxilLibIterator.GetCount();
            stateObjectCollection.m_dxilLibraries.resize(dxilLibCount);
            for (UINT i = 0; i < dxilLibCount; i++)
            {
                dxilLibIterator.Next(&stateObjectCollection.m_dxilLibraries[i]);
            }
        }
    }


    HRESULT STDMETHODCALLTYPE RaytracingDevice::CreateStateObject(
        const D3D12_STATE_OBJECT_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_  void **ppStateObject)
    {
        if (!ppStateObject || riid != __uuidof(ID3D12RaytracingFallbackStateObject))
        {
            ThrowFailure(E_INVALIDARG, L"Null ppStateObject passed in or invalid riid");
        }

        try
        {
            RaytracingStateObject *pRaytracingStateObject = new RaytracingStateObject();
            *ppStateObject = pRaytracingStateObject;
            if (!*ppStateObject)
            {
                ThrowFailure(E_OUTOFMEMORY, L"Out of memory");
            }

            ProcessStateObject(*pDesc, *pRaytracingStateObject);

            if (pDesc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE)
            {
                pRaytracingStateObject->m_spProgram.reset(
                    m_RaytracingProgramFactory.GetRaytracingProgram(pRaytracingStateObject->m_collection));

                pRaytracingStateObject->m_spProgram->SetPredispatchCallback([=](ID3D12GraphicsCommandList *pCommandList, UINT patchRootSignatureParameterStart)
                {
                    UNREFERENCED_PARAMETER(pCommandList);
                    UNREFERENCED_PARAMETER(patchRootSignatureParameterStart);
#if ENABLE_UAV_LOG
                    pCommandList->SetComputeRootUnorderedAccessView(
                        patchRootSignatureParameterStart + DebugUAVLog, m_pUAVLog->GetGPUVirtualAddress());
#endif

#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
                    DebugVariables variables;
                    variables.LevelToVisualize = m_levelToVisualize;
                    pCommandList->SetComputeRoot32BitConstants(patchRootSignatureParameterStart + DebugConstants,
                        SizeOfInUint32(variables),
                        &variables,
                        0);
#endif
                });
            }
        }
        catch (_com_error &e)
        {
            return e.Error();
        }

        return S_OK;
    }

    void STDMETHODCALLTYPE D3D12RaytracingCommandList::EmitRaytracingAccelerationStructurePostbuildInfo(
        _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData)
    {
        if (pDesc->InfoType != D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE ||
            pDesc->InfoType != D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_CURRENT_SIZE)
        {
            ThrowFailure(E_INVALIDARG,
                L"Unsupported InfoType passed in, only supported POSTBUILD_INFO flags are "
                L"D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE and D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_CURRENT_SIZE, "
            );
        }
#if USE_PIX_MARKERS
        PIXScopedEvent(m_pCommandList.p, FallbackPixColor, L"EmitRaytracingAccelerationStructurePostBuildInfo");
#endif

        m_device.GetAccelerationStructureBuilderFactory().GetAccelerationStructureBuilder().EmitRaytracingAccelerationStructurePostbuildInfo(
            m_pCommandList,
            pDesc,
            NumSourceAccelerationStructures,
            pSourceAccelerationStructureData);
    }

    void STDMETHODCALLTYPE D3D12RaytracingCommandList::CopyRaytracingAccelerationStructure(
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode)
    {
#if USE_PIX_MARKERS
        PIXScopedEvent(m_pCommandList.p, FallbackPixColor, L"CopyRaytracingAccelerationStructure");
#endif
        m_device.GetAccelerationStructureBuilderFactory().GetAccelerationStructureBuilder().CopyRaytracingAccelerationStructure(
            m_pCommandList,
            DestAccelerationStructureData,
            SourceAccelerationStructureData,
            Mode
        );
    }


    void STDMETHODCALLTYPE D3D12RaytracingCommandList::SetDescriptorHeaps(
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps)
    {
        m_pCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);

        assert(NumDescriptorHeaps <= DescriptorHeapType::NumTypes);
        ZeroMemory(m_pBoundDescriptorHeaps, sizeof(m_pBoundDescriptorHeaps));
        for (UINT heapIndex = 0; heapIndex < NumDescriptorHeaps; heapIndex++)
        {
            assert(ppDescriptorHeaps[heapIndex]);

            auto heapDesc = ppDescriptorHeaps[heapIndex]->GetDesc();
            assert((heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0 && 
                m_pBoundDescriptorHeaps[D3D12EnumToFallbackEnum(heapDesc.Type)] == nullptr);
                
            m_pBoundDescriptorHeaps[D3D12EnumToFallbackEnum(heapDesc.Type)] = ppDescriptorHeaps[heapIndex];
        }
    }

    void STDMETHODCALLTYPE D3D12RaytracingCommandList::SetTopLevelAccelerationStructure(
        _In_  UINT RootParameterIndex,
        _In_  WRAPPED_GPU_POINTER  BufferLocation)
    {
        m_BoundAccelerationStructures[RootParameterIndex] = BufferLocation;
    }

    void STDMETHODCALLTYPE D3D12RaytracingCommandList::DispatchRays(
        _In_  const D3D12_DISPATCH_RAYS_DESC *pDesc)
    {
        // A view-type descriptor heap is required to be bound since the Top-Level 
        // Acceleration Structure will have emulated-SRV-pointers to Bottom-Level 
        // structures
        RaytracingStateObject *pRaytracingPipelineState = static_cast<RaytracingStateObject *>(m_pStateObject.p);
        if (!pDesc || (!m_pBoundDescriptorHeaps[SrvUavCbvType]))
        {
            ThrowFailure(E_INVALIDARG, 
                L"DispatchRays called without a Descriptor Heap for SRV/UAV/CBV's bound, "
                L"the compute-based implementation requires D3D12RaytracingCommandList::SetDescriptorHeaps"
                L"be called with a valid heap. This is required for emulated pointers to properly work"
            );
        }

        if (!pRaytracingPipelineState)
        {
            ThrowFailure(E_INVALIDARG, L"Null Raytracing Pipeline State provided");
        }
        if (!pRaytracingPipelineState->m_spProgram)
        {
            ThrowFailure(E_INVALIDARG, L"Incomplete state object provided, make sure the state object was created with D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE");
        }

#if ENABLE_UAV_LOG
        m_device.ResetLog(m_pCommandList);
#endif
#if USE_PIX_MARKERS
        PIXScopedEvent(m_pCommandList.p, FallbackPixColor, L"DispatchRays(%d, %d)", pDesc->Width, pDesc->Height);
#endif
        pRaytracingPipelineState->m_spProgram->DispatchRays(
            m_pCommandList,
            m_pBoundDescriptorHeaps[SrvUavCbvType],
            m_pBoundDescriptorHeaps[SamplerType],
            m_BoundAccelerationStructures,
            *pDesc);

#if ENABLE_UAV_LOG
        m_pCommandList->CopyResource(&m_device.GetUAVDebugLogReadbackHeap(), &m_device.GetUAVDebugLog());
#endif
    }

#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
    void RaytracingDevice::VisualizeAccelerationStructureLevel(UINT levelToVisualize)
    {
        m_levelToVisualize = levelToVisualize;
    }
#endif

    void *STDMETHODCALLTYPE RaytracingStateObject::GetShaderIdentifier(LPCWSTR pExportName)
    {
        return m_spProgram->GetShaderIdentifier(pExportName);
    }

    UINT64 STDMETHODCALLTYPE RaytracingStateObject::GetShaderStackSize(LPCWSTR pExportName)
    {
        return m_spProgram->GetShaderStackSize(pExportName);
    }


    UINT STDMETHODCALLTYPE RaytracingDevice::GetShaderIdentifierSize(void)
    {
        return sizeof(ShaderIdentifier);
    }

    void STDMETHODCALLTYPE RaytracingDevice::GetRaytracingAccelerationStructurePrebuildInfo(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
        _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
    {
        m_AccelerationStructureBuilderFactory.GetAccelerationStructureBuilder().GetRaytracingAccelerationStructurePrebuildInfo(
            pDesc,
            pInfo);
    }
} // namespace FallbackLayer
