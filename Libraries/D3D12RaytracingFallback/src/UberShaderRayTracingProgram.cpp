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
#include "CompiledShaders\StateMachineLib.h"
#ifdef DUMP_UBERSHADER
static UINT g_ReproNumber = 0;
#include <fstream>
#endif

namespace FallbackLayer
{
    void CompilePSO(ID3D12Device *pDevice, D3D12_SHADER_BYTECODE shaderByteCode, const StateObjectCollection &stateObjectCollection, ID3D12PipelineState **ppPipelineState)
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.CS = CD3DX12_SHADER_BYTECODE(shaderByteCode);
        psoDesc.NodeMask = stateObjectCollection.m_nodeMask;
        psoDesc.pRootSignature = stateObjectCollection.m_pGlobalRootSignature;

        ThrowInternalFailure(pDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(ppPipelineState)));
    }

    UINT64 UberShaderRaytracingProgram::GetShaderStackSize(LPCWSTR pExportName)
    {
        auto shaderData = GetShaderData(pExportName);
        return shaderData ? shaderData->stackSize : 0;
    }

    UberShaderRaytracingProgram::ShaderData *UberShaderRaytracingProgram::GetShaderData(LPCWSTR pExportName)
    {
      if (pExportName)
      {
        const auto &shaderData = m_ExportNameToShaderData.find(pExportName);
        if (shaderData != m_ExportNameToShaderData.end())
        {
          return &shaderData->second;
        }
      }
      return nullptr;
    }

    StateIdentifier UberShaderRaytracingProgram::GetStateIdentfier(LPCWSTR pExportName)
    {
        auto shaderData = GetShaderData(pExportName);
        return shaderData ? shaderData->stateIdentifier.StateId : (StateIdentifier)0u;
    }


    UberShaderRaytracingProgram::UberShaderRaytracingProgram(ID3D12Device *pDevice, DxilShaderPatcher &dxilShaderPatcher, const StateObjectCollection &stateObjectCollection) :
        m_DxilShaderPatcher(dxilShaderPatcher)
    {
        UINT numLibraries = (UINT)stateObjectCollection.m_dxilLibraries.size();

        UINT numShaders = 0;
        for (auto &lib : stateObjectCollection.m_dxilLibraries)
        {
            numShaders += lib.NumExports;
        }

        std::vector<CComPtr<IDxcBlob>> patchedBlobList;

        UINT cbvSrvUavHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        UINT samplerHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        
        ViewKey SRVViewsList[FallbackLayerNumDescriptorHeapSpacesPerView];
        UINT SRVsUsed = 0;
        ViewKey UAVViewsList[FallbackLayerNumDescriptorHeapSpacesPerView];
        UINT UAVsUsed = 0;
        
        CComPtr<IDxcBlob> pAppLibrariesBlob;
        {
            std::vector<DxilLibraryInfo> libraryInfo;
            for (UINT i = 0; i < numLibraries; i++)
            {
                auto &inputLib = stateObjectCollection.m_dxilLibraries[i];
                libraryInfo.emplace_back((void *)inputLib.DXILLibrary.pShaderBytecode, inputLib.DXILLibrary.BytecodeLength);
            }
            m_DxilShaderPatcher.RenameAndLink(libraryInfo, stateObjectCollection.m_exportDescs, &pAppLibrariesBlob);
        }

        std::vector<DxilLibraryInfo> librariesInfo;
        DxilLibraryInfo outputLibInfo((void *)pAppLibrariesBlob->GetBufferPointer(), (UINT)pAppLibrariesBlob->GetBufferSize());
        CComPtr<IDxcBlob> pOutputBlob;
        std::vector<LPCWSTR> exportNames;
        for (auto &associationPair : stateObjectCollection.m_shaderAssociations)
        {
            auto &exportName = associationPair.first;
            exportNames.push_back(exportName.c_str());
            auto &shaderAssociation = associationPair.second;

            if (shaderAssociation.m_pRootSignature)
            {
                CComPtr<ID3D12VersionedRootSignatureDeserializer> pDeserializer;
                ShaderInfo shaderInfo;
                shaderInfo.pRootSignatureDesc = GetDescFromRootSignature(shaderAssociation.m_pRootSignature, pDeserializer);
                shaderInfo.pSRVRegisterSpaceArray = SRVViewsList;
                shaderInfo.pNumSRVSpaces = &SRVsUsed;
                shaderInfo.pUAVRegisterSpaceArray = UAVViewsList;
                shaderInfo.pNumUAVSpaces = &UAVsUsed;

                if (GetNumParameters(*shaderInfo.pRootSignatureDesc) > 0)
                {
                    shaderInfo.SamplerDescriptorSizeInBytes = samplerHandleSize;
                    shaderInfo.SrvCbvUavDescriptorSizeInBytes = cbvSrvUavHandleSize;
                    shaderInfo.ShaderRecordIdentifierSizeInBytes = sizeof(ShaderIdentifier);
                    shaderInfo.ExportName = exportName.c_str();

                    CComPtr<IDxcBlob> pPatchedBlob;
                    m_DxilShaderPatcher.PatchShaderBindingTables(
                        (const BYTE *)outputLibInfo.pByteCode,
                        (UINT)outputLibInfo.BytecodeLength,
                        &shaderInfo,
                        &pPatchedBlob);

                    pOutputBlob = pPatchedBlob;
                    outputLibInfo = DxilLibraryInfo(pOutputBlob->GetBufferPointer(), pOutputBlob->GetBufferSize());
                }
            }
        }

        librariesInfo.emplace_back(outputLibInfo);
        {
            auto &traversalShader = stateObjectCollection.m_traversalShader.DXILLibrary;
            librariesInfo.emplace_back((void *)traversalShader.pShaderBytecode, traversalShader.BytecodeLength);
            exportNames.push_back(L"Fallback_TraceRay");
        }

        {
            librariesInfo.emplace_back((void *)g_pStateMachineLib, ARRAYSIZE(g_pStateMachineLib));
        }

#ifdef DUMP_UBERSHADER
        DumpReproInformation(stateObjectCollection.m_maxAttributeSizeInBytes, librariesInfo, exportNames);
#endif

        CComPtr<IDxcBlob> pCollectionBlob;
        std::vector<DxcShaderInfo> shaderInfo;

        m_DxilShaderPatcher.LinkCollection(stateObjectCollection.m_maxAttributeSizeInBytes, librariesInfo, exportNames, shaderInfo, &pCollectionBlob);

        UINT traceRayStackSize = shaderInfo[exportNames.size() - 1].StackSize;
        for (size_t i = 0; i < exportNames.size() - 1; ++i)
        {
            auto &shader = shaderInfo[i];
            bool isRaygen = shader.Type == ShaderType::Raygen;
            UINT shaderStackSize = shader.StackSize;
            if (isRaygen)
            {
                m_largestRayGenStackSize = std::max(shaderStackSize, m_largestRayGenStackSize);
            }
            else if (shader.Type == ShaderType::Miss)
            {
                shaderStackSize += traceRayStackSize;
                m_largestNonRayGenStackSize = std::max(shaderStackSize, m_largestNonRayGenStackSize);
            }

            m_ExportNameToShaderData[exportNames[i]] = { {shader.Identifier, 0}, shaderStackSize };
        }

        for (auto &hitGroupMapEntry : stateObjectCollection.m_hitGroups)
        {
            auto closestHitName = hitGroupMapEntry.second.ClosestHitShaderImport;
            auto anyHitName = hitGroupMapEntry.second.AnyHitShaderImport;
            auto intersectionName = hitGroupMapEntry.second.IntersectionShaderImport;

            ShaderIdentifier shaderId = {};
            shaderId.StateId = GetStateIdentfier(closestHitName);
            shaderId.AnyHitId = GetStateIdentfier(anyHitName);
            shaderId.IntersectionShaderId = GetStateIdentfier(intersectionName);
            UINT shaderStackSize = (UINT)std::max(std::max(
                GetShaderStackSize(closestHitName), GetShaderStackSize(anyHitName)), GetShaderStackSize(intersectionName));

            m_largestNonRayGenStackSize = std::max(shaderStackSize + traceRayStackSize, m_largestNonRayGenStackSize);
            auto hitGroupName = hitGroupMapEntry.first;
            m_ExportNameToShaderData[hitGroupName] = { shaderId, shaderStackSize };
        }

        UINT stackSize = stateObjectCollection.m_config.MaxTraceRecursionDepth * m_largestNonRayGenStackSize + m_largestRayGenStackSize;
        CComPtr<IDxcBlob> pLinkedBlob;
        m_DxilShaderPatcher.LinkStateObject(stateObjectCollection.m_maxAttributeSizeInBytes, stackSize, pCollectionBlob, exportNames, shaderInfo, &pLinkedBlob);

        CompilePSO(
            pDevice, 
            CD3DX12_SHADER_BYTECODE(pLinkedBlob->GetBufferPointer(), pLinkedBlob->GetBufferSize()), 
            stateObjectCollection, 
            &m_pRayTracePSO);
        
        UINT sizeOfParamterStart = sizeof(m_patchRootSignatureParameterStart);
        ThrowFailure(stateObjectCollection.m_pGlobalRootSignature->GetPrivateData(
            FallbackLayerPatchedParameterStartGUID,
            &sizeOfParamterStart,
            &m_patchRootSignatureParameterStart),
            L"Root signatures in a state object must be created through "
            L"Fallback Layer-specific interaces. Either use RaytracingDevice::D3D12SerializeRootSignature "
            L"or RaytracingDevice::D3D12SerializeFallbackRootSignature and create with "
            L"RaytracingDevice::CreateRootSignature");
    }

    void UberShaderRaytracingProgram::DumpReproInformation(UINT maxAttributeSizeInBytes, std::vector<DxilLibraryInfo> librariesInfo, std::vector<LPCWSTR> exportNames)
    {
#ifdef DUMP_UBERSHADER
        std::ofstream cmdFile("fallbackcompilerrepro" + std::to_string(g_ReproNumber++) + ".cmd");
        if (cmdFile.is_open())
        {
            cmdFile << "set maxAttributeSizeInBytes=" << maxAttributeSizeInBytes << "\n";

            cmdFile << "set libraries=\"";
            for (int libIndex = 0; libIndex < librariesInfo.size(); libIndex++)
            {
                char libFileName[25];
                sprintf_s(libFileName, "lib@%p.bin", (void *)librariesInfo[libIndex].pByteCode);
                std::ofstream libFile(libFileName, std::ios::binary);
                cmdFile << libFileName << ";";
                if (libFile.is_open())
                {
                    libFile.write((char *)librariesInfo[libIndex].pByteCode, librariesInfo[libIndex].BytecodeLength);
                    libFile.close();
                }
            }
            cmdFile << "\"\n";
            cmdFile << "set exportNames=\"";
            for (int nameIndex = 0; nameIndex < exportNames.size(); nameIndex++)
            {
                char exportName[128]; // Convert wide string to normal string
                sprintf_s(exportName, "%S", exportNames[nameIndex]);
                cmdFile << exportName << ";";
            }
            cmdFile << "\"\n";

            cmdFile << "cls\n";
            cmdFile << "cmd /K \"FallbackLayerCompilerRepro.exe %maxAttributeSizeInBytes% %libraries% %exportNames%\"";
            cmdFile.close();
        }
        else
        {
            ThrowFailure(-1, L"Could not create cmd file for UberShaderRayTracingProgram DXIL dump.\n");
        }
#endif
    }

    ShaderIdentifier *UberShaderRaytracingProgram::GetShaderIdentifier(LPCWSTR pExportName)
    {
        auto pEntry = m_ExportNameToShaderData.find(pExportName);
        if (pEntry == m_ExportNameToShaderData.end())
        {
            return nullptr;
        }
        else
        {
            // Handing out this pointer is safe because the map is read-only at this point
            return &pEntry->second.stateIdentifier;
        }
    }

    void UberShaderRaytracingProgram::DispatchRays(
        ID3D12GraphicsCommandList *pCommandList, 
        ID3D12DescriptorHeap *pSrvCbvUavDescriptorHeap,
        ID3D12DescriptorHeap *pSamplerDescriptorHeap,
        const std::unordered_map<UINT, WRAPPED_GPU_POINTER> &boundAccelerationStructures,
        const D3D12_DISPATCH_RAYS_DESC &desc)
    {
        assert(pSrvCbvUavDescriptorHeap);
        pCommandList->SetComputeRootDescriptorTable(m_patchRootSignatureParameterStart + CbvSrvUavDescriptorHeapAliasedTables, pSrvCbvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

        if (pSamplerDescriptorHeap)
        {
            pCommandList->SetComputeRootDescriptorTable(m_patchRootSignatureParameterStart + SamplerDescriptorHeapAliasedTables, pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        }
        if (desc.HitGroupTable.StartAddress)
        {
            pCommandList->SetComputeRootShaderResourceView(m_patchRootSignatureParameterStart + HitGroupRecord, desc.HitGroupTable.StartAddress);
        }
        if (desc.MissShaderTable.StartAddress)
        {
            pCommandList->SetComputeRootShaderResourceView(m_patchRootSignatureParameterStart + MissShaderRecord, desc.MissShaderTable.StartAddress);
        }
        if (desc.RayGenerationShaderRecord.StartAddress)
        {
            pCommandList->SetComputeRootShaderResourceView(m_patchRootSignatureParameterStart + RayGenShaderRecord, desc.RayGenerationShaderRecord.StartAddress);
        }
        if (desc.CallableShaderTable.StartAddress)
        {
            pCommandList->SetComputeRootShaderResourceView(m_patchRootSignatureParameterStart + CallableShaderRecord, desc.CallableShaderTable.StartAddress);
        }

        DispatchRaysConstants constants;
        constants.RayDispatchDimensionsWidth = desc.Width;
        constants.RayDispatchDimensionsHeight = desc.Height;
        constants.HitGroupShaderRecordStride = static_cast<UINT>(desc.HitGroupTable.StrideInBytes);
        constants.MissShaderRecordStride = static_cast<UINT>(desc.MissShaderTable.StrideInBytes);
        constants.SrvCbvUavDescriptorHeapStart = pSrvCbvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
        constants.SamplerDescriptorHeapStart = pSamplerDescriptorHeap ? pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr : 0;
        
        pCommandList->SetComputeRoot32BitConstants(m_patchRootSignatureParameterStart + DispatchConstants, SizeOfInUint32(DispatchRaysConstants), &constants, 0);

        UINT entriesAdded = 0;
        std::vector<WRAPPED_GPU_POINTER> AccelerationStructuresEntries(boundAccelerationStructures.size());
        for (auto &entry : boundAccelerationStructures)
        {
            AccelerationStructuresEntries[entriesAdded++] = entry.second;
        }

        pCommandList->SetComputeRoot32BitConstants(
            m_patchRootSignatureParameterStart + AccelerationStructuresList, (UINT)(AccelerationStructuresEntries.size() * (SizeOfInUint32(*AccelerationStructuresEntries.data()))), AccelerationStructuresEntries.data(), 0);

#ifdef DEBUG
        m_pPredispatchCallback(pCommandList, m_patchRootSignatureParameterStart);
#endif

        UINT dispatchWidth = DivideAndRoundUp<UINT>(desc.Width, THREAD_GROUP_WIDTH);
        UINT dispatchHeight = DivideAndRoundUp<UINT>(desc.Height, THREAD_GROUP_HEIGHT);

        pCommandList->SetPipelineState(m_pRayTracePSO);
        pCommandList->Dispatch(dispatchWidth, dispatchHeight, 1);
    }
}
