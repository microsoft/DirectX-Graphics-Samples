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

struct LoadInstancesConstants
{
    uint NumberOfElements;
};

// UAVs
#define OutputBVHRegister 0
#define DescriptorHeapBufferRegister 0
#define DescriptorHeapBufferRegisterSpace 1

//SRVs 
#define InstanceDescsRegister 0
#define DescriptorHeapSRVBufferRegister 0
#define DescriptorHeapSRVBufferRegisterSpace 2

// CBVs
#define LoadInstancesConstantsRegister 0

#ifdef HLSL

RWByteAddressBuffer outputBVH : UAV_REGISTER(OutputBVHRegister);
RWByteAddressBuffer DescriptorHeapBufferTable[] : UAV_REGISTER_SPACE(DescriptorHeapBufferRegister, DescriptorHeapBufferRegisterSpace);

ByteAddressBuffer instanceDescs : SRV_REGISTER(InstanceDescsRegister);
ByteAddressBuffer DescriptorHeapSRVBufferTable[] : SRV_REGISTER_SPACE(DescriptorHeapSRVBufferRegister, DescriptorHeapSRVBufferRegisterSpace);

cbuffer TopLevelConstants : CONSTANT_REGISTER(LoadInstancesConstantsRegister)
{
    LoadInstancesConstants Constants;
};
#endif
