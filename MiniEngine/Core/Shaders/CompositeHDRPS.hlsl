//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "ToneMappingUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float3> MainBuffer : register(t0);
Texture2D<float4> OverlayBuffer : register(t1);

cbuffer CB0 : register(b0)
{
    float PaperWhiteRatio; // PaperWhite / 10000.0
    float MaxBrightness;
}

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
    int2 ST = (int2)position.xy;

    float3 MainColor = ApplyREC2084Curve(MainBuffer[ST] / 10000.0);

    float4 OverlayColor = OverlayBuffer[ST];

    OverlayColor.rgb = RemoveSRGBCurve(OverlayColor.rgb);
    OverlayColor.rgb = REC709toREC2020(OverlayColor.rgb / (OverlayColor.a == 0.0 ? 1.0 : OverlayColor.a));
    OverlayColor.rgb = ApplyREC2084Curve(OverlayColor.rgb * PaperWhiteRatio);

    return lerp(MainColor, OverlayColor.rgb, OverlayColor.a);
}
