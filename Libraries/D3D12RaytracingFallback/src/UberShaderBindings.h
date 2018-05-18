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
#include "FallbackDebug.h"
#include "FallbackDxil.h"

#ifndef HLSL
#include "HlslCompat.h"
#else
#include "ShaderUtil.hlsli"
#endif

#define UAVLogRegister 1
#define DebugConstantRegister 10

struct DebugVariables
{
    uint LevelToVisualize;
};

#define FallbackLayerDescriptorHeapRegisterSpace 214743648
#ifdef HLSL
cbuffer Constants : CONSTANT_REGISTER_SPACE(FallbackLayerDispatchConstantsRegister, FallbackLayerRegisterSpace)
{
    uint RayDispatchDimensionsWidth;
    uint RayDispatchDimensionsHeight;
    uint HitGroupShaderRecordStride;
    uint MissShaderRecordStride;

    uint SamplerDescriptorHeapStartLo;
    uint SamplerDescriptorHeapStartHi;
    uint SrvCbvUavDescriptorHeapStartLo;
    uint SrvCbvUavDescriptorHeapStartHi;
};

cbuffer AccelerationStructureList : CONSTANT_REGISTER_SPACE(FallbackLayerAccelerationStructureList, FallbackLayerRegisterSpace)
{
    uint2 TopLevelAccelerationStructureGpuVA;
};

ByteAddressBuffer HitGroupShaderTable : SRV_REGISTER_SPACE(FallbackLayerHitGroupRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
ByteAddressBuffer MissShaderTable : SRV_REGISTER_SPACE(FallbackLayerMissShaderRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
ByteAddressBuffer RayGenShaderTable : SRV_REGISTER_SPACE(FallbackLayerRayGenShaderRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);
ByteAddressBuffer CallableShaderTable : SRV_REGISTER_SPACE(FallbackLayerCallableShaderRecordByteAddressBufferRegister, FallbackLayerRegisterSpace);

RWByteAddressBuffer DescriptorHeapBufferTable[] : UAV_REGISTER_SPACE(FallbackLayerDescriptorHeapTable, FallbackLayerDescriptorHeapRegisterSpace);

#if ENABLE_UAV_LOG
RWStructuredBuffer<uint4> DebugLog : UAV_REGISTER_SPACE(UAVLogRegister, FallbackLayerRegisterSpace);
#endif

#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
cbuffer DebugVariables : CONSTANT_REGISTER_SPACE(DebugConstantRegister, FallbackLayerRegisterSpace)
{
    DebugVariables Debug;
};
#endif
#else
static_assert(FallbackLayerDescriptorHeapRegisterSpace == FallbackLayerRegisterSpace + FallbackLayerDescriptorHeapSpaceOffset, L"#define for FallbackLayerDescriptorHeapRegisterSpace is incorrect");
#endif
