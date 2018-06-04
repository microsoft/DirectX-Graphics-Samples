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
#ifndef HLSL
#include "HlslCompat.h"
#else
#include "RaytracingHlslCompat.h"
#include "ShaderUtil.hlsli"
#endif

struct LoadPrimitivesInputConstants
{
    uint ElementBufferStride;
    uint IndexBufferOffset;
    uint NumPrimitivesBound;

    uint PrimitiveOffset;
    uint TotalPrimitiveCount;

    uint GeometryContributionToHitGroupIndex;

    // TODO: Consider inlining into separate shaders
    uint HasValidTransform; 
    uint GeometryFlags;
    uint PerformUpdate;
};

// UAVs
#define OutputPrimitiveBufferRegister 0
#define OutputMetadataBufferRegister 1
#define CachedSortBufferRegister 2

//SRVs 
#define ElementBufferRegister 0
#define IndexBufferRegister 1
#define TransformRegister 2

// CBVs
#define LoadInstancesConstantsRegister 0

#ifdef HLSL

RWStructuredBuffer<Primitive> PrimitiveBuffer : UAV_REGISTER(OutputPrimitiveBufferRegister);
RWStructuredBuffer<PrimitiveMetaData> MetadataBuffer : UAV_REGISTER(OutputMetadataBufferRegister);
RWStructuredBuffer<uint> CachedSortBuffer : UAV_REGISTER(CachedSortBufferRegister);

ByteAddressBuffer elementBuffer : SRV_REGISTER(ElementBufferRegister);
ByteAddressBuffer indexBuffer : SRV_REGISTER(IndexBufferRegister);
StructuredBuffer<float4> TransformBuffer : SRV_REGISTER(TransformRegister);
cbuffer LoadPrimitivesConstants : CONSTANT_REGISTER(LoadInstancesConstantsRegister)
{
    LoadPrimitivesInputConstants Constants;
}

uint GetOutputIndex(uint inputIndex) 
{
    if (Constants.PerformUpdate)
    {
        return CachedSortBuffer[inputIndex];
    }
    return inputIndex;
}

void StorePrimitiveMetadata(uint globalPrimitiveIndex, uint localPrimitiveIndex)
{
    PrimitiveMetaData metaData;
    metaData.GeometryContributionToHitGroupIndex = Constants.GeometryContributionToHitGroupIndex;
    metaData.PrimitiveIndex = localPrimitiveIndex;
    metaData.GeometryFlags = Constants.GeometryFlags;
    MetadataBuffer[globalPrimitiveIndex] = metaData;
}
#endif
