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

#define GPU_BVH2_COPY_THREAD_GROUP_WIDTH 1024

// UAVs
#define DestBvhRegister 0
#define SourceBvhRegister 1

// CBVs
#define DispatchWidthConstantsRegister 0

struct DispatchWidthConstant
{
    uint DispatchWidth;
};

#ifdef HLSL
RWByteAddressBuffer DestBVH : UAV_REGISTER(DestBvhRegister);
RWByteAddressBuffer SourceBVH : UAV_REGISTER(SourceBvhRegister);

cbuffer DispatchWidthConstant : CONSTANT_REGISTER(DispatchWidthConstantsRegister)
{
    DispatchWidthConstant Constants;
}
#endif
