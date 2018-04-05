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
float4 ReadData();

RWByteAddressBuffer outputBuffer : register(u0);

[shader("miss")]
void miss(inout EmptyPayload payload : SV_RayPayload)
{
    float4 color0 = ReadData();
    outputBuffer.Store4(0, asuint(color0));
}
