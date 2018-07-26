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

#include "gradient.hlsli"
#include "color.hlsli"

float4 PSMain(PSInput input) : SV_TARGET
{
    // This draws a perceptual gradient rather than a linear gradient.
    return float4(pow(abs(input.color.rgb), 2.2), input.color.a);
}
