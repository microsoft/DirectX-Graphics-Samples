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
};

// UAVs
#define MortonCodesBufferRegister 0
#define HierarchyBufferRegister 1

#define GlobalDescriptorHeapRegister 0
#define GlobalDescriptorHeapRegisterSpace 1

// CBVs
#define InputConstantsRegister 0

#ifdef HLSL
RWStructuredBuffer<uint> mortonCodes : UAV_REGISTER(MortonCodesBufferRegister);
RWStructuredBuffer<HierarchyNode> hierarchyBuffer : UAV_REGISTER(HierarchyBufferRegister);
RWByteAddressBuffer DescriptorHeapBufferTable[] : UAV_REGISTER_SPACE(GlobalDescriptorHeapRegister, GlobalDescriptorHeapRegisterSpace);

cbuffer ConstructHierarchyConstants : CONSTANT_REGISTER(InputConstantsRegister)
{
    InputConstants Constants;
};
#endif
