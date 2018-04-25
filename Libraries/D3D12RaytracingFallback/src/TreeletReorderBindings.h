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
#include "RaytracingHlslCompat.h"
#ifdef HLSL
#include "ShaderUtil.hlsli"
#endif

struct InputConstants
{
    uint NumberOfElements;
    uint MinTrianglesPerTreelet;
};

// CBVs
#define ConstantsRegister 0

// UAVs
#define HierarchyBufferRegister 0
#define NumTrianglesBufferRegister 1
#define AABBBufferRegister 2
#define ElementBufferRegister 3

#define GlobalDescriptorHeapRegister 0
#define GlobalDescriptorHeapRegisterSpace 1

#ifdef HLSL
// These need to be UAVs despite being read-only because the fallback layer only gets a 
// GPU VA and the API doesn't allow any way to transition that GPU VA from UAV->SRV

globallycoherent RWStructuredBuffer<HierarchyNode> hierarchyBuffer : UAV_REGISTER(HierarchyBufferRegister);
RWByteAddressBuffer NumTrianglesBuffer : UAV_REGISTER(NumTrianglesBufferRegister);
globallycoherent RWStructuredBuffer<AABB> AABBBuffer : UAV_REGISTER(AABBBufferRegister);
RWStructuredBuffer<Primitive> InputBuffer : UAV_REGISTER(ElementBufferRegister);
RWByteAddressBuffer DescriptorHeapBufferTable[] : UAV_REGISTER_SPACE(GlobalDescriptorHeapRegister, GlobalDescriptorHeapRegisterSpace);

cbuffer TreeletConstants : CONSTANT_REGISTER(ConstantsRegister)
{
    InputConstants Constants;
};
#endif
