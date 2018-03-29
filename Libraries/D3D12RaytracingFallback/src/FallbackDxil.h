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

#define FallbackLayerRegisterSpace 214743647


// SRVs
#define FallbackLayerHitGroupRecordByteAddressBufferRegister 0
#define FallbackLayerMissShaderRecordByteAddressBufferRegister 1
#define FallbackLayerRayGenShaderRecordByteAddressBufferRegister 2
#define FallbackLayerCallableShaderRecordByteAddressBufferRegister 3

// SRV & UAV
#define FallbackLayerDescriptorHeapBufferTable 0

// There's a driver issue on some hardware that has issues
// starting a bindless table on any register but 0, so
// make sure each bindless table has it's own register space
#define FallbackLayerDescriptorHeapStartingSpaceOffset 1

// CBVs
#define FallbackLayerDispatchConstantsRegister 0
#define FallbackLayerAccelerationStructureList 1

#ifndef HLSL
struct ShaderInfo
{
    const wchar_t *ExportName;
    unsigned int SamplerDescriptorSizeInBytes;
    unsigned int SrvCbvUavDescriptorSizeInBytes;
    unsigned int ShaderRecordIdentifierSizeInBytes;
    bool IsLib;
    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *pRootSignatureDesc;
};

struct DispatchRaysConstants
{
    unsigned __int32 RayDispatchDimensionsWidth;
    unsigned __int32 RayDispatchDimensionsHeight;
    unsigned __int32 HitGroupShaderRecordStride;
    unsigned __int32 MissShaderRecordStride;

    // 64-bit values
    unsigned __int64 SamplerDescriptorHeapStart;
    unsigned __int64 SrvCbvUavDescriptorHeapStart;
};

enum DescriptorRangeTypes
{
    SRV = 0,
    CBV,
    UAV,
    Sampler,
    NumRangeTypes
};

enum RegisterSpaceOffsets
{
    SRVBufferOffset = FallbackLayerDescriptorHeapStartingSpaceOffset,
    SRVStructuredBufferOffset,
    SRVRawBufferOffset,
    SRVTexture1DOffset,
    SRVTexture1DArrayOffset,
    SRVTexture2DOffset,
    SRVTexture2DMSOffset,
    SRVTexture2DArrayOffset,
    SRVTexture2DMSArrayOffset,
    SRVTexture3DOffset,
    SRVTextureCubeOffset,
    SRVTextureCubeArrayOffset,

    SRVStartOffset = SRVBufferOffset,
    NumSRVSpaces = SRVTextureCubeArrayOffset - SRVStartOffset + 1,

    UAVRawBufferOffset = FallbackLayerDescriptorHeapStartingSpaceOffset,
    UAVBufferOffset,
    UAVStructuredBufferOffset,
    UAVTexture1DOffset,
    UAVTexture1DArrayOffset,
    UAVTexture2DOffset,
    UAVTexture2DArrayOffset,
    UAVTexture3DOffset,

    UAVStartOffset = UAVRawBufferOffset,
    NumUAVSpaces = UAVTexture3DOffset - UAVStartOffset + 1,

    SamplerOffset = FallbackLayerDescriptorHeapStartingSpaceOffset,
    NumSamplerSpaces = 1,

    ConstantBufferOffset = FallbackLayerDescriptorHeapStartingSpaceOffset,
    NumConstantBufferSpaces = 1,

    NumSpaces = NumSRVSpaces + NumUAVSpaces + NumConstantBufferSpaces + NumSamplerSpaces,
};

enum RootSignatureParameterOffset
{
    HitGroupRecord = 0,
    MissShaderRecord,
    RayGenShaderRecord,
    CallableShaderRecord,
    DispatchConstants,
    CbvSrvUavDescriptorHeapAliasedTables,
    SamplerDescriptorHeapAliasedTables,
    AccelerationStructuresList,
#if ENABLE_UAV_LOG
    DebugUAVLog,
#endif
#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
    DebugConstants,
#endif
    NumParameters
};
#endif
