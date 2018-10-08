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
#include "ShaderUtil.hlsli"
#endif
#include "RaytracingHlslCompat.h"

#define ElementsSummedPerThread 8

struct InputConstants
{
    uint NumberOfTriangles;
    uint UpdatesAllowed;
};

// UAVs
#define InputElementBufferRegister   0
#define InputMetadataBufferRegister  1
#define IndexBufferRegister          2
#define OutputElementBufferRegister  3
#define OutputMetadataBufferRegister 4
#define OutputIndexBufferRegister    5

// CBVs
#define InputConstantsRegister 0

#ifdef HLSL
// These need to be UAVs despite being read-only because the fallback layer only gets a 
// GPU VA and the API doesn't allow any way to transition that GPU VA from UAV->SRV
RWStructuredBuffer<uint> IndexBuffer : UAV_REGISTER(IndexBufferRegister);
RWStructuredBuffer<uint> OutputIndexBuffer : UAV_REGISTER(OutputIndexBufferRegister);

cbuffer RearrangeElementsConstants : CONSTANT_REGISTER(InputConstantsRegister)
{
    InputConstants Constants;
}
#endif