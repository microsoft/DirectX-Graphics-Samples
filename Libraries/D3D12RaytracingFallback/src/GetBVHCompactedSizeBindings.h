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

#define NumberOfReadableBVHsPerDispatch 30
struct GetBVHCompactedSizeConstants
{
    uint NumberOfBoundBVHs;
};

// UAVs
#define OutputCountRegister 0
#define BVHStartRegister 1

// CBVs
#define GetBVHCompactedSizeConstantsRegister 0

#ifdef HLSL
cbuffer GetBVHCompactedSizeConstants : CONSTANT_REGISTER(GetBVHCompactedSizeConstantsRegister)
{
    GetBVHCompactedSizeConstants Constants;
}

RWByteAddressBuffer OutputCount : UAV_REGISTER(OutputCountRegister);

RWByteAddressBuffer BVH1  : register(u1);
RWByteAddressBuffer BVH2  : register(u2);
RWByteAddressBuffer BVH3  : register(u3);
RWByteAddressBuffer BVH4  : register(u4);
RWByteAddressBuffer BVH5  : register(u5);
RWByteAddressBuffer BVH6  : register(u6);
RWByteAddressBuffer BVH7  : register(u7);
RWByteAddressBuffer BVH8  : register(u8);
RWByteAddressBuffer BVH9  : register(u9);
RWByteAddressBuffer BVH10 : register(u10);
RWByteAddressBuffer BVH11 : register(u11);
RWByteAddressBuffer BVH12 : register(u12);
RWByteAddressBuffer BVH13 : register(u13);
RWByteAddressBuffer BVH14 : register(u14);
RWByteAddressBuffer BVH15 : register(u15);
RWByteAddressBuffer BVH16 : register(u16);
RWByteAddressBuffer BVH17 : register(u17);
RWByteAddressBuffer BVH18 : register(u18);
RWByteAddressBuffer BVH19 : register(u19);
RWByteAddressBuffer BVH20 : register(u20);
RWByteAddressBuffer BVH21 : register(u21);
RWByteAddressBuffer BVH22 : register(u22);
RWByteAddressBuffer BVH23 : register(u23);
RWByteAddressBuffer BVH24 : register(u24);
RWByteAddressBuffer BVH25 : register(u25);
RWByteAddressBuffer BVH26 : register(u26);
RWByteAddressBuffer BVH27 : register(u27);
RWByteAddressBuffer BVH28 : register(u28);
RWByteAddressBuffer BVH29 : register(u29);
RWByteAddressBuffer BVH30 : register(u30);

#endif
