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

#include "QuadVertexShader.hlsl"

float4 Blit(float2 uv)
{
    return g_tex.Sample(g_samp, uv);
}

float4 mainBlit(PSInput input) : SV_TARGET
{
    return Blit(input.uv);
}
