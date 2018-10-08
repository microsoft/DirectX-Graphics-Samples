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
        switch (pDesc->Inputs.Type)
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

    void GpuBvh2Builder::LoadGpuBVHBuffers(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        Level bvhLevel,
        UINT numElements,
        GpuBVHBuffers &buffers)
    {
        D3D12_GPU_VIRTUAL_ADDRESS bvhGpuVA = pDesc->DestAccelerationStructureData;
        ScratchMemoryPartitions scratchMemoryPartition = CalculateScratchMemoryUsage(bvhLevel, numElements);
        D3D12_GPU_VIRTUAL_ADDRESS scratchGpuVA = pDesc->ScratchAccelerationStructureData;
        
        buffers.scratchElementBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToElements;
        buffers.mortonCodeBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToMortonCodes;
        buffers.sceneAABB = scratchGpuVA + scratchMemoryPartition.OffsetToSceneAABB;
        buffers.sceneAABBScratchMemory = scratchGpuVA + scratchMemoryPartition.OffsetToSceneAABBScratchMemory;
        buffers.indexBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToIndexBuffer;
        buffers.hierarchyBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToHierarchy;
        buffers.calculateAABBScratchBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToCalculateAABBDispatchArgs;
        buffers.nodeCountBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToPerNodeCounter;

        if (SupportsTreeletReordering(bvhLevel))
        {
            buffers.baseTreeletsCountBuffer = scratchGpuVA + scratchMemoryPartition.OffsetToBaseTreeletsCount;
            buffers.baseTreeletsIndexBuffer = buffers.baseTreeletsCountBuffer + sizeof(UINT);
        }

        switch(bvhLevel) 
        {
            case Level::Top:
            {
                UINT offsetFromElementsToMetadata = GetOffsetFromLeafNodesToBottomLevelMetadata(numElements);
                buffers.scratchMetadataBuffer = buffers.scratchElementBuffer + offsetFromElementsToMetadata;
                buffers.outputElementBuffer = bvhGpuVA + GetOffsetToLeafNodeAABBs(numElements);
                buffers.outputMetadataBuffer = buffers.outputElementBuffer + offsetFromElementsToMetadata;
                buffers.outputSortCacheBuffer = bvhGpuVA + GetOffsetToBVHSortedIndices(numElements);
                buffers.outputAABBParentBuffer = buffers.outputSortCacheBuffer + GetOffsetFromSortedIndicesToAABBParents(numElements);
            }
            break;
            case Level::Bottom:
            {
                buffers.scratchMetadataBuffer = buffers.scratchElementBuffer + GetOffsetFromPrimitivesToPrimitiveMetaData(numElements);
                buffers.outputElementBuffer = bvhGpuVA + GetOffsetToPrimitives(numElements);
                buffers.outputMetadataBuffer = buffers.outputElementBuffer + GetOffsetFromPrimitivesToPrimitiveMetaData(numElements);
                buffers.outputSortCacheBuffer = buffers.outputMetadataBuffer + GetOffsetFromPrimitiveMetaDataToSortedIndices(numElements);
                buffers.outputAABBParentBuffer = buffers.outputSortCacheBuffer + GetOffsetFromSortedIndicesToAABBParents(numElements);
            }
            break;
        }
    }

    void GpuBvh2Builder::BuildTopLevelBVH(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap)
    {
        const SceneType sceneType = SceneType::BottomLevelBVHs;
        UINT numElements = pDesc->Inputs.NumDescs;
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap = pCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

        BuildBVH(
            pCommandList,
            pDesc,
            Level::Top,
            sceneType,
            numElements,
            globalDescriptorHeap
        );
    }

    void GpuBvh2Builder::BuildBottomLevelBVH(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc)
    {
        const SceneType sceneType = SceneType::Triangles;
        UINT numElements = GetTotalPrimitiveCount(pDesc->Inputs);
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap = D3D12_GPU_DESCRIPTOR_HANDLE();

        BuildBVH(
            pCommandList,
            pDesc,
            Level::Bottom,
            sceneType,
            numElements,
            globalDescriptorHeap
        );
    }

#define updatesAllowed(flags) ((flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) != 0)
#define shouldPerformUpdate(flags) ((flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE) != 0)
    void GpuBvh2Builder::BuildBVH(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        Level bvhLevel,
        SceneType sceneType,
        UINT numElements,
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap)
    {
        if (pDesc->DestAccelerationStructureData == 0)
        {
            ThrowFailure(E_INVALIDARG, L"DestAccelerationStructureData.StartAddress must be non-zero");
        }

        GpuBVHBuffers buffers = {}; LoadGpuBVHBuffers(pDesc, bvhLevel, numElements, buffers);

        const bool updatesAllowed = updatesAllowed(pDesc->Inputs.Flags);
        const bool performUpdate = shouldPerformUpdate(pDesc->Inputs.Flags);

        // Load in the leaf-node elements of the BVH and calculate the entire scene's AABB.
        LoadBVHElements(
            pCommandList,
            pDesc,
            sceneType,
            numElements,
            performUpdate ? buffers.outputElementBuffer   : buffers.scratchElementBuffer, // If we're updating, write straight to output.
            performUpdate ? buffers.outputMetadataBuffer  : buffers.scratchMetadataBuffer, 
            performUpdate ? buffers.outputSortCacheBuffer : 0,
            buffers.sceneAABBScratchMemory,
            buffers.sceneAABB,
            globalDescriptorHeap);

        // If we don't have PERFORM_UPDATE set, rebuild the entire hierarchy.
        // (i.e. calc morton codes, sort, rearrange, build hierarchy, treelet reorder)
        if (!performUpdate) {
            BuildBVHHierarchy(
                pCommandList,
                pDesc,
                sceneType,
                numElements,
                buffers.scratchElementBuffer,
                buffers.outputElementBuffer,
                buffers.scratchMetadataBuffer,
                buffers.outputMetadataBuffer,
                buffers.sceneAABBScratchMemory,
                buffers.sceneAABB,
                buffers.mortonCodeBuffer,
                buffers.indexBuffer,
                updatesAllowed ? buffers.outputSortCacheBuffer : 0,
                buffers.hierarchyBuffer,
                buffers.nodeCountBuffer,
                buffers.baseTreeletsCountBuffer,
                buffers.baseTreeletsIndexBuffer,
                globalDescriptorHeap);
        }

        // Fit AABBs around each node in the hierarchy.
        m_constructAABBPass.ConstructAABB(
            pCommandList,
            sceneType,
            pDesc->DestAccelerationStructureData,
            buffers.calculateAABBScratchBuffer,
            buffers.nodeCountBuffer,
            buffers.hierarchyBuffer,
            buffers.outputAABBParentBuffer,
            globalDescriptorHeap,
            updatesAllowed && !performUpdate,
            performUpdate,
            numElements);
    }

    void GpuBvh2Builder::LoadBVHElements(
        _In_ ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        const SceneType sceneType,
        const UINT numElements,
        D3D12_GPU_VIRTUAL_ADDRESS elementBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS metadataBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABBScratchMemory,
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABB,
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap)
    {
        switch(sceneType) 
        {
            case SceneType::BottomLevelBVHs:
            // Note that the load instances pass does load metadata even though it doesn't take a metadata
            // buffer address. Users don't specify BVH instance metadata, so the shader takes care of
            // putting the metadata where it needs to go on its own.
            m_loadInstancesPass.LoadInstances(
                pCommandList, 
                elementBuffer, 
                pDesc->Inputs.InstanceDescs, 
                pDesc->Inputs.DescsLayout,
                numElements, 
                globalDescriptorHeap,
                indexBuffer);
            break;
            case SceneType::Triangles:
            // Load all the triangles into the bottom-level acceleration structure. This loading is done 
            // one VB/IB pair at a time since each VB will have unique characteristics (topology type/IB format)
            // and will generally have enough verticies to go completely wide
            m_loadPrimitivesPass.LoadPrimitives(
                pCommandList, 
                pDesc->Inputs, 
                numElements, 
                elementBuffer,
                metadataBuffer,
                indexBuffer);
            break;
        }

        m_sceneAABBCalculator.CalculateSceneAABB(
            pCommandList, 
            sceneType, 
            elementBuffer, 
            numElements, 
            sceneAABBScratchMemory, 
            sceneAABB);
    }

    void GpuBvh2Builder::BuildBVHHierarchy(
        _In_ ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        const SceneType sceneType,
        const uint numElements,
        D3D12_GPU_VIRTUAL_ADDRESS scratchElementBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputElementBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS scratchMetadataBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABBScratchMemory,
        D3D12_GPU_VIRTUAL_ADDRESS sceneAABB,
        D3D12_GPU_VIRTUAL_ADDRESS mortonCodeBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputSortCacheBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS nodeCountBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS baseTreeletsCountBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS baseTreeletsIndexBuffer,
        D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap) 
    {
        m_mortonCodeCalculator.CalculateMortonCodes(
            pCommandList, 
            sceneType, 
            scratchElementBuffer, 
            numElements, 
            sceneAABB, 
            indexBuffer, 
            mortonCodeBuffer);

        m_sorterPass.Sort(
            pCommandList, 
            mortonCodeBuffer, 
            indexBuffer, 
            numElements, 
            false, 
            true);

        m_rearrangePass.Rearrange(
            pCommandList,
            sceneType,
            numElements,
            scratchElementBuffer,
            scratchMetadataBuffer,
            indexBuffer,
            outputElementBuffer,
            outputMetadataBuffer,
            outputSortCacheBuffer);

        m_constructHierarchyPass.ConstructHierarchy(
            pCommandList,
            sceneType,
            mortonCodeBuffer,
            hierarchyBuffer,
            globalDescriptorHeap,
            numElements);

        if (sceneType == SceneType::Triangles) 
        {
#if ENABLE_TREELET_REORDERING
            m_treeletReorder.Optimize(
                pCommandList,
                numElements,
                hierarchyBuffer,                
                nodeCountBuffer,
                sceneAABBScratchMemory,
                outputElementBuffer,
                baseTreeletsCountBuffer,
                baseTreeletsIndexBuffer,
                pDesc->Inputs.Flags);
#endif
        }
    }

    void GpuBvh2Builder::CopyRaytracingAccelerationStructure(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode)
    {
        if (Mode == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE ||
            Mode == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT)
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
        UINT64 &totalSize = scratchMemoryPartitions.TotalSize;
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
            INT64 sizeNeededToCalculateAABB = m_sceneAABBCalculator.ScratchBufferSizeNeeded(numPrimitives);
            INT64 sizeNeededForTreeletAABBs = TreeletReorder::RequiredSizeForAABBBuffer(numPrimitives);
            INT64 sizeNeededByMortonCodeAndIndexBuffer = mortonCodeBufferSize + indexBufferSize;
            UINT64 extraBufferSize = std::max(sizeNeededToCalculateAABB, std::max(sizeNeededForTreeletAABBs, sizeNeededByMortonCodeAndIndexBuffer));

            totalSize += extraBufferSize;
        }

        {
            UINT64 sizeNeededForAABBCalculation = 0;
            scratchMemoryPartitions.OffsetToCalculateAABBDispatchArgs = sizeNeededForAABBCalculation;
            sizeNeededForAABBCalculation += ALIGN_GPU_VA_OFFSET(sizeof(UINT) * numPrimitives);

            scratchMemoryPartitions.OffsetToPerNodeCounter = sizeNeededForAABBCalculation;
            sizeNeededForAABBCalculation += ALIGN_GPU_VA_OFFSET(sizeof(UINT) * (numInternalNodes));

            totalSize = std::max(sizeNeededForAABBCalculation, totalSize);
        }

        const UINT64 hierarchySize = ALIGN_GPU_VA_OFFSET(sizeof(HierarchyNode) * totalNumNodes);
        scratchMemoryPartitions.OffsetToHierarchy = totalSize;
        totalSize += hierarchySize;
        
        if (SupportsTreeletReordering(level))
        {
            const UINT baseTreeletsScratchSize = TreeletReorder::RequiredSizeForBaseTreeletBuffers(numPrimitives);
            scratchMemoryPartitions.OffsetToBaseTreeletsCount = totalSize;
            totalSize += baseTreeletsScratchSize;
        }

        return scratchMemoryPartitions;
    }

    void GpuBvh2Builder::GetRaytracingAccelerationStructurePrebuildInfo(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
        _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
    {
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE Type = pDesc->Type;
        UINT NumElements = pDesc->NumDescs;

        UINT numLeaves = 0;
        UINT totalNumNodes = 0;
        Level level = Level::Bottom;

        switch (Type)
        {
        case D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL:
        {
            level = Level::Bottom;
            numLeaves = GetTotalPrimitiveCount(*pDesc);
            totalNumNodes = numLeaves + GetNumberOfInternalNodes(numLeaves);

            pInfo->ResultDataMaxSizeInBytes = sizeof(BVHOffsets) + totalNumNodes * sizeof(AABBNode) + numLeaves * (sizeof(Primitive) + sizeof(PrimitiveMetaData));
        }
        break;
        case D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL:
        {
            level = Level::Top;
            numLeaves = NumElements;
            totalNumNodes = numLeaves + GetNumberOfInternalNodes(numLeaves);

            pInfo->ResultDataMaxSizeInBytes = sizeof(BVHOffsets) + sizeof(AABBNode) * totalNumNodes + sizeof(BVHMetadata) * numLeaves;
        }
        break;
        default:
            ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE provided");
        }

        if (updatesAllowed(pDesc->Flags)) 
        {
            pInfo->ResultDataMaxSizeInBytes += numLeaves * sizeof(UINT); // Saved sorted index buffer
            pInfo->ResultDataMaxSizeInBytes += totalNumNodes * sizeof(UINT); // Parent indices for nodes in hierarchy
        }

        pInfo->ScratchDataSizeInBytes = CalculateScratchMemoryUsage(level, numLeaves).TotalSize;
        pInfo->UpdateScratchDataSizeInBytes = 0;
    }

    void GpuBvh2Builder::EmitRaytracingAccelerationStructurePostbuildInfo(
        _In_  ID3D12GraphicsCommandList *pCommandList,
        _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData)
    {
        m_postBuildInfoQuery.GetCompactedBVHSizes(
            pCommandList,
            pDesc->DestBuffer,
            NumSourceAccelerationStructures,
            pSourceAccelerationStructureData);
    }

    bool GpuBvh2Builder::SupportsTreeletReordering(Level level) 
    {
        return level == Level::Bottom;
    }
}
