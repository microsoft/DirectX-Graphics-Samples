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

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer RootConstants : register(b0)
{
    float standardNits;        // The reference brightness level of the display.
    uint displayCurve;        // The expected format of the output signal.
};

Texture2D g_scene : register(t0);
Texture2D g_ui : register(t1);
SamplerState g_sampler : register(s0);
