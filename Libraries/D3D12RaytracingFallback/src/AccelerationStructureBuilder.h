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

struct AABB;

namespace FallbackLayer
{
    enum AccelerationStructureLayoutType
    {
        BVH2 = 0,
        NumAccelerationStructureLayoutTypes
    };

    class IAccelerationStructureBuilder
    {
    public:
        virtual ~IAccelerationStructureBuilder() {};

        virtual void BuildRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap) = 0;

        virtual void CopyRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode) = 0;

        virtual void GetRaytracingAccelerationStructurePrebuildInfo(
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo) = 0;

        virtual void EmitRaytracingAccelerationStructurePostbuildInfo(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
            _In_  UINT NumSourceAccelerationStructures,
            _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData) = 0;

        virtual AccelerationStructureLayoutType GetAccelerationStructureType() = 0;
    };

    IAccelerationStructureValidator &GetAccelerationStructureValidator(AccelerationStructureLayoutType type);
}
