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
#include "Validate.hlsli"

RWByteAddressBuffer outputBuffer : register(u0);
cbuffer Constants : register(b0)
{
    float4 color0;
}

cbuffer Constants : register(b1)
{
    float4 color1;
}


cbuffer Constants : register(b2)
{
    float4 unusedPadding;
    float4 color2;
}

[shader("miss")]
void miss(inout EmptyPayload payload)
{
    outputBuffer.Store4(0, asuint(color0));
    outputBuffer.Store4(16, asuint(color1));
    outputBuffer.Store4(32, asuint(color2));
}
