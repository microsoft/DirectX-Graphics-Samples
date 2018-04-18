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

#pragma once

namespace FallbackLayer
{
    GpuBvh2Builder::GpuBvh2Builder(ID3D12Device *pDevice, UINT totalLaneCount, UINT nodeMask) :
        m_sceneAABBCalculator(pDevice, nodeMask),
        m_mortonCodeCalculator(pDevice, nodeMask),
        m_sorterPass(pDevice, nodeMask),
        m_rearrangePass(pDevice, nodeMask),
        m_loadInstancesPass(pDevice, nodeMask),
        m_loadPrimitivesPass(pDevice, nodeMask),
        m_constructHierarchyPass(pDevice, nodeMask),
        m_constructAABBPass(pDevice, nodeMask),
        m_postBuildInfoQuery(pDevice, nodeMask),
        m_copyPass(pDevice, totalLaneCount, nodeMask),
        m_treeletReorder(pDevice, nodeMask)
    {}

    void GpuBvh2Builder::BuildRaytracingAccelerationStructure(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap)
    {
#ifdef DEBUG
        D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
        prebuildInfoDesc.DescsLayout = pDesc->DescsLayout;
        prebuildInfoDesc.Flags = pDesc->Flags;
        prebuildInfoDesc.NumDescs = pDesc->NumDescs;
        prebuildInfoDesc.pGeometryDescs = pDesc->pGeometryDescs;
        prebuildInfoDesc.ppGeometryDescs = pDesc->ppGeometryDescs;
        prebuildInfoDesc.Type = pDesc->Type;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildOutput;

        CComPtr<ID3D12Device> pDevice;
        pCommandList->GetDevice(IID_PPV_ARGS(&pDevice));

        GetRaytracingAccelerationStructurePrebuildInfo(pDevice, &prebuildInfoDesc, &prebuildOutput);
        if (pDesc->DestAccelerationStructureData.SizeInBytes < prebuildOutput.ResultDataMaxSizeInBytes)
        {
            ThrowFailure(E_INVALIDARG, L"DestAccelerationStructureData.SizeInBytes too small, "
                L"ensure the size matches up with a size returned from "
                L"EmitRaytracingAccelerationStructurePostBuildInfo/GetRaytracingAccelerationStructurePrebuildInfo");
        }

        if (pDesc->ScratchAccelerationStructureData.SizeInBytes < prebuildOutput.ScratchDataSizeInBytes)
        {
            ThrowFailure(E_INVALIDARG, L"pDesc->ScratchAccelerationStructureData.SizeInBytes too small, "
                L"ensure the size matches up with a size returned from "
                L"EmitRaytracingAccelerationStructurePostBuildInfo/GetRaytracingAccelerationStructurePrebuildInfo");
        }
#endif

        switch (pDesc->Type)
        {
        case D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL:
            BuildBottomLevelBVH(pCommandList, pDesc);
            break;

        case D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL:
            BuildTopLevelBVH(pCommandList, pDesc, pCbvSrvUavDescriptorHeap);
            break;
        default:
            ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE provided");
        }
    }

    void GpuBvh2Builder::BuildTopLevelBVH(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap)
    {
        CComPtr<ID3D12Device> pDevice;
        pCommandList->GetDevice(IID_PPV_ARGS(&pDevice));

        ScratchMemoryPartitions scratchMemoryPartition = CalculateScratchMemoryUsage(Level::Top, pDesc->NumDescs);
        D3D12_GPU_VIRTUAL_ADDRESS scratchGpuVA = pDesc->ScratchAccelerationStructureData.StartAddress;
        D3D12_GPU_VIRTUAL_ADDRESS scratchBVHBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToElements;
        D3D12_GPU_VIRTUAL_ADDRESS mortonCodeBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToMortonCodes;
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABB = scratchGpuVA + scratchMemoryPartition.OffsetToSceneAABB;
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABBScratchMemory = scratchGpuVA + scratchMemoryPartition.OffsetToSceneAABBScratchMemory;
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToIndexBuffer;
        D3D12_GPU_VIRTUAL_ADDRESS calculateAABBScratchBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToCalculateAABBDispatchArgs;
        D3D12_GPU_VIRTUAL_ADDRESS nodeCountBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToPerNodeCounter;
        D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToHierarchy;

        const UINT64 totalSize = sizeof(D3D12_GPU_VIRTUAL_ADDRESS) * pDesc->NumDescs;
        D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

        UINT numElements = pDesc->NumDescs;
        const SceneType sceneType = SceneType::BottomLevelBVHs;
        m_loadInstancesPass.LoadInstances(pCommandList, scratchBVHBuffer, pDesc->InstanceDescs, pDesc->DescsLayout, pDesc->NumDescs, pCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        m_sceneAABBCalculator.CalculateSceneAABB(pCommandList, sceneType, scratchBVHBuffer, numElements, sceneAABBScratchMemory, sceneAABB);
        m_mortonCodeCalculator.CalculateMortonCodes(pCommandList, sceneType, scratchBVHBuffer, numElements, sceneAABB, indexBuffer, mortonCodeBuffer);
        m_sorterPass.Sort(pCommandList, mortonCodeBuffer, indexBuffer, numElements, false, true);

        UINT offsetFromBufferToMetadata = GetOffsetFromLeafNodesToBottomLevelMetadata(numElements);
        auto outputBVHLocation = pDesc->DestAccelerationStructureData.StartAddress + GetOffsetToLeafNodeAABBs(numElements);
        m_rearrangePass.Rearrange(
            pCommandList,
            sceneType,
            scratchBVHBuffer,
            numElements,
            indexBuffer,
            outputBVHLocation,
            scratchBVHBuffer + offsetFromBufferToMetadata,
            outputBVHLocation + offsetFromBufferToMetadata);

        m_constructHierarchyPass.ConstructHierarchy(
            pCommandList,
            sceneType,
            mortonCodeBuffer,
            hierarchyBuffer,
            pCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
            numElements);

        m_constructAABBPass.ConstructAABB(
            pCommandList,
            sceneType,
            pDesc->DestAccelerationStructureData.StartAddress,
            calculateAABBScratchBuffer,
            nodeCountBuffer,
            mortonCodeBuffer,
            hierarchyBuffer,
            pCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
            numElements);
    }

    void GpuBvh2Builder::BuildBottomLevelBVH(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc)
    {
        if (pDesc->DestAccelerationStructureData.StartAddress == 0)
        {
            ThrowFailure(E_INVALIDARG, L"DestAccelerationStructureData.StartAddress must be non-zero");
        }

        // Load all the triangles into the bottom-level acceleration structure. This loading is done 
        // one VB/IB pair at a time since each VB will have unique characteristics (topology type/IB format)
        // and will generally have enough verticies to go completely wide
        UINT totalTriangles = GetTotalPrimitiveCount(*pDesc);
        ScratchMemoryPartitions scratchMemoryPartition = CalculateScratchMemoryUsage(Level::Bottom, totalTriangles);
        D3D12_GPU_VIRTUAL_ADDRESS scratchGpuVA = pDesc->ScratchAccelerationStructureData.StartAddress;
        D3D12_GPU_VIRTUAL_ADDRESS scratchTriangleBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToElements;
        D3D12_GPU_VIRTUAL_ADDRESS scratchMetadataBuffer = scratchTriangleBuffer + GetOffsetFromPrimitivesToPrimitiveMetaData(totalTriangles);
        D3D12_GPU_VIRTUAL_ADDRESS mortonCodeBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToMortonCodes;
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABB = scratchGpuVA + scratchMemoryPartition.OffsetToSceneAABB;
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABBScratchMemory = scratchGpuVA + scratchMemoryPartition.OffsetToSceneAABBScratchMemory;
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToIndexBuffer;
        D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToHierarchy;

        D3D12_GPU_VIRTUAL_ADDRESS calculateAABBScratchBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToCalculateAABBDispatchArgs;
        D3D12_GPU_VIRTUAL_ADDRESS nodeCountBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToPerNodeCounter;
        
        const SceneType sceneType = SceneType::Triangles;
        m_loadPrimitivesPass.LoadPrimitives(pCommandList, *pDesc, totalTriangles, scratchTriangleBuffer, scratchMetadataBuffer);
        m_sceneAABBCalculator.CalculateSceneAABB(pCommandList, sceneType, scratchTriangleBuffer, totalTriangles, sceneAABBScratchMemory, sceneAABB);
        m_mortonCodeCalculator.CalculateMortonCodes(pCommandList, sceneType, scratchTriangleBuffer, totalTriangles, sceneAABB, indexBuffer, mortonCodeBuffer);
        m_sorterPass.Sort(pCommandList, mortonCodeBuffer, indexBuffer, totalTriangles, false, true);

        D3D12_GPU_VIRTUAL_ADDRESS outputTriangleBuffer = pDesc->DestAccelerationStructureData.StartAddress + GetOffsetToPrimitives(totalTriangles);
        D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer = outputTriangleBuffer + GetOffsetFromPrimitivesToPrimitiveMetaData(totalTriangles);

        m_rearrangePass.Rearrange(
            pCommandList,
            sceneType,
            scratchTriangleBuffer,
            totalTriangles,
            indexBuffer,
            outputTriangleBuffer,
            scratchMetadataBuffer,
            outputMetadataBuffer);

        m_constructHierarchyPass.ConstructHierarchy(
            pCommandList,
            sceneType,
            mortonCodeBuffer,
            hierarchyBuffer,
            D3D12_GPU_DESCRIPTOR_HANDLE(),
            totalTriangles);

        m_treeletReorder.Optimize(
            pCommandList,
            totalTriangles,
            hierarchyBuffer,
            nodeCountBuffer,
            sceneAABBScratchMemory,
            outputTriangleBuffer,
            D3D12_GPU_DESCRIPTOR_HANDLE(),
            pDesc->Flags);

        m_constructAABBPass.ConstructAABB(
            pCommandList,
            sceneType,
            pDesc->DestAccelerationStructureData.StartAddress,
            calculateAABBScratchBuffer,
            nodeCountBuffer,
            mortonCodeBuffer,
            hierarchyBuffer,
            D3D12_GPU_DESCRIPTOR_HANDLE(),
            totalTriangles);
    }

    void GpuBvh2Builder::CopyRaytracingAccelerationStructure(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Flags)
    {
        if (Flags == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE ||
            Flags == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT)
        {
            m_copyPass.CopyRaytracingAccelerationStructure(pCommandList, DestAccelerationStructureData, SourceAccelerationStructureData);
        }
        else
        {
            ThrowFailure(E_INVALIDARG, 
                L"The only flags supported for CopyRaytracingAccelerationStructure are: "
                L"D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE/D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT");
        }
    }

    GpuBvh2Builder::ScratchMemoryPartitions GpuBvh2Builder::CalculateScratchMemoryUsage(Level level, UINT numPrimitives)
    {
#define ALIGN(alignment, num) (((num + alignment - 1) / alignment) * alignment)
#define ALIGN_GPU_VA_OFFSET(num) ALIGN(4, num)

        ScratchMemoryPartitions scratchMemoryPartitions = {};
        UINT &totalSize = scratchMemoryPartitions.TotalSize;
        UINT numInternalNodes = GetNumberOfInternalNodes(numPrimitives);
        UINT totalNumNodes = numPrimitives + numInternalNodes;

        scratchMemoryPartitions.OffsetToSceneAABB = totalSize;
        totalSize += ALIGN_GPU_VA_OFFSET(sizeof(AABB));

        const UINT sizePerElement = level == Level::Bottom ?
            sizeof(Primitive) + sizeof(PrimitiveMetaData) :
            (sizeof(AABBNode) + sizeof(BVHMetadata));
        scratchMemoryPartitions.OffsetToElements = totalSize;
        totalSize += ALIGN_GPU_VA_OFFSET(sizePerElement * numPrimitives);

        const UINT mortonCodeBufferSize = ALIGN_GPU_VA_OFFSET(sizeof(UINT) * numPrimitives);
        scratchMemoryPartitions.OffsetToMortonCodes = totalSize;

        const UINT indexBufferSize = ALIGN_GPU_VA_OFFSET(sizeof(UINT) * numPrimitives);
        scratchMemoryPartitions.OffsetToIndexBuffer = scratchMemoryPartitions.OffsetToMortonCodes + indexBufferSize;

        {
            // The scratch buffer used for calculating AABBs can alias over the MortonCode/IndexBuffer
            // because it's calculated before the MortonCode/IndexBuffer are needed. Additionally,
            // the AABB buffer used for treelet reordering is done after both stages so it can also alias
            scratchMemoryPartitions.OffsetToSceneAABBScratchMemory = scratchMemoryPartitions.OffsetToMortonCodes;
            INT sizeNeededToCalculateAABB = m_sceneAABBCalculator.ScratchBufferSizeNeeded(numPrimitives);
            INT sizeNeededForTreeletAABBs = TreeletReorder::RequiredSizeForAABBBuffer(numPrimitives);
            INT sizeNeededByMortonCodeAndIndexBuffer = mortonCodeBufferSize + indexBufferSize;
            UINT extraBufferSize = std::max(sizeNeededToCalculateAABB, std::max(sizeNeededForTreeletAABBs, sizeNeededByMortonCodeAndIndexBuffer));
            
            totalSize += extraBufferSize;
        }


        {
            UINT sizeNeededForAABBCalculation = 0;
            scratchMemoryPartitions.OffsetToCalculateAABBDispatchArgs = sizeNeededForAABBCalculation;
            sizeNeededForAABBCalculation += ALIGN_GPU_VA_OFFSET(sizeof(UINT) * numPrimitives);

            scratchMemoryPartitions.OffsetToPerNodeCounter = sizeNeededForAABBCalculation;
            sizeNeededForAABBCalculation += ALIGN_GPU_VA_OFFSET(sizeof(UINT) * (numInternalNodes));

            totalSize = std::max(sizeNeededForAABBCalculation, totalSize);
        }

        const UINT hierarchySize = ALIGN_GPU_VA_OFFSET(sizeof(HierarchyNode) * totalNumNodes);
        scratchMemoryPartitions.OffsetToHierarchy = totalSize;
        totalSize += hierarchySize;

        return scratchMemoryPartitions;
    }

    void GpuBvh2Builder::GetRaytracingAccelerationStructurePrebuildInfo(
        _In_  D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC *pDesc,
        _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
    {
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE Type = pDesc->Type;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags = pDesc->Flags;
        UINT NumElements = pDesc->NumDescs;

        switch (Type)
        {
        case D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL:
        {
            UINT totalNumberOfTriangles = GetTotalPrimitiveCount(*pDesc);
            const UINT numLeaves = totalNumberOfTriangles;
            // A full binary tree with N leaves will always have N - 1 internal nodes
            const UINT numInternalNodes = GetNumberOfInternalNodes(numLeaves);
            const UINT totalNumNodes = numLeaves + numInternalNodes;

            pInfo->ResultDataMaxSizeInBytes = sizeof(BVHOffsets) + totalNumberOfTriangles * (sizeof(Primitive) + sizeof(PrimitiveMetaData)) +
                totalNumNodes * sizeof(AABBNode);

            pInfo->ScratchDataSizeInBytes = CalculateScratchMemoryUsage(Level::Bottom, totalNumberOfTriangles).TotalSize;
            pInfo->UpdateScratchDataSizeInBytes = 0;
        }
        break;
        case D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL:
        {
            const UINT numLeaves = NumElements;

            const UINT numInternalNodes = GetNumberOfInternalNodes(numLeaves);
            const UINT totalNumNodes = numLeaves + numInternalNodes;

            pInfo->ResultDataMaxSizeInBytes = sizeof(BVHOffsets) + sizeof(AABBNode) * totalNumNodes + sizeof(BVHMetadata) * numLeaves;
            pInfo->ScratchDataSizeInBytes = CalculateScratchMemoryUsage(Level::Top, numLeaves).TotalSize;
            pInfo->UpdateScratchDataSizeInBytes = 0;
        }
        break;
        default:
            ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE provided");
        }
    }

    void GpuBvh2Builder::EmitRaytracingAccelerationStructurePostBuildInfo(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestBuffer,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData)
    {
        m_postBuildInfoQuery.GetCompactedBVHSizes(
            pCommandList, 
            DestBuffer, 
            NumSourceAccelerationStructures, 
            pSourceAccelerationStructureData);
    }
}
