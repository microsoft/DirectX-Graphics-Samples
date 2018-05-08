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
            _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Flags);

        virtual void GetRaytracingAccelerationStructurePrebuildInfo(
            _In_  D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo);

        virtual void EmitRaytracingAccelerationStructurePostBuildInfo(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestBuffer,
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
            UINT OffsetToSceneAABB;
            UINT OffsetToElements;
            UINT OffsetToMortonCodes;
            UINT OffsetToIndexBuffer;
            UINT OffsetToHierarchy;

            UINT OffsetToSceneAABBScratchMemory;

            UINT OffsetToCalculateAABBDispatchArgs;
            UINT OffsetToPerNodeCounter;
            UINT TotalSize;
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

        void BuildTopLevelBVH(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap
        );

        void BuildBottomLevelBVH(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc
        );
    };
}
