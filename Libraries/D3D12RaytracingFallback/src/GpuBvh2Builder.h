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
    class GpuBvh2Builder : public IAccelerationStructureBuilder
    {
    public:
        GpuBvh2Builder(ID3D12Device *pDevice, UINT totalLaneCount, UINT nodeMask);
        virtual ~GpuBvh2Builder() {}

        virtual void BuildRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap
        );

        virtual void CopyRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode);

        virtual void GetRaytracingAccelerationStructurePrebuildInfo(
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo);

        virtual void EmitRaytracingAccelerationStructurePostbuildInfo(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
            _In_  UINT NumSourceAccelerationStructures,
            _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData);

        virtual AccelerationStructureLayoutType GetAccelerationStructureType() { return BVH2; }
        
    private: 
        enum class Level
        {
            Bottom,
            Top
        };

        struct ScratchMemoryPartitions
        {
            UINT64 OffsetToSceneAABB;
            UINT64 OffsetToElements;
            UINT64 OffsetToMortonCodes;
            UINT64 OffsetToIndexBuffer;
            UINT64 OffsetToHierarchy;
            UINT64 OffsetToBaseTreeletsCount;

            UINT64 OffsetToSceneAABBScratchMemory;

            UINT64 OffsetToCalculateAABBDispatchArgs;
            UINT64 OffsetToPerNodeCounter;
            UINT64 TotalSize;
        };

        ScratchMemoryPartitions CalculateScratchMemoryUsage(Level level, UINT numTriangles);

        SceneAABBCalculator m_sceneAABBCalculator;
        MortonCodesCalculator m_mortonCodeCalculator;
        BitonicSort m_sorterPass;
        RearrangeElementsPass m_rearrangePass;
        LoadInstancesPass m_loadInstancesPass;
        LoadPrimitivesPass m_loadPrimitivesPass;
        ConstructAABBPass m_constructAABBPass;
        ConstructHierarchyPass m_constructHierarchyPass;
        TreeletReorder m_treeletReorder;

        PostBuildInfoQuery m_postBuildInfoQuery;
        GpuBvh2Copy m_copyPass;

        struct GpuBVHBuffers {
            D3D12_GPU_VIRTUAL_ADDRESS scratchElementBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS outputElementBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS scratchMetadataBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS sceneAABBScratchMemory;
            D3D12_GPU_VIRTUAL_ADDRESS sceneAABB;
            D3D12_GPU_VIRTUAL_ADDRESS mortonCodeBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS indexBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS outputSortCacheBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS hierarchyBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS nodeCountBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS baseTreeletsCountBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS baseTreeletsIndexBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS calculateAABBScratchBuffer;
            D3D12_GPU_VIRTUAL_ADDRESS outputAABBParentBuffer;
        };

        void LoadGpuBVHBuffers(
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            Level bvhLevel,
            UINT numElements,
            GpuBVHBuffers &buffers
        );
        
        void BuildTopLevelBVH(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap
        );

        void BuildBottomLevelBVH(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc
        );

        void BuildBVH(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            Level bvhLevel,
            SceneType sceneType,
            UINT numElements,
            D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap
        );

        void LoadBVHElements(
            _In_ ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            const SceneType sceneType,
            const uint totalElements,
            D3D12_GPU_VIRTUAL_ADDRESS elementBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS metadataBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS indexBuffer,
            D3D12_GPU_VIRTUAL_ADDRESS sceneAABBScratchMemory,
            D3D12_GPU_VIRTUAL_ADDRESS sceneAABB,
            D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap
        );

        void BuildBVHHierarchy(
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
            D3D12_GPU_DESCRIPTOR_HANDLE globalDescriptorHeap
        );

        bool SupportsTreeletReordering(Level level);
    };
}
