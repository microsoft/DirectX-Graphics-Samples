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

#include "present.hlsli"
#include "color.hlsli"

float3 PSMain(PSInput input) : SV_TARGET
{
    // The scene, including brightness bars and color palettes, is rendered with linear gamma and Rec.709 primaries. (DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709) 
    float3 scene = g_scene.Sample(g_sampler, input.uv).rgb;

    // Direct2D renders the UI layer with gamma 2.2 in Rec.709 primaries. (DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709, known as sRGB)
    float4 uiLayer = g_ui.Sample(g_sampler, input.uv);

    // Compose the scene and UI layers. Note that we convert UI layer from gamma 2.2 to to linear gamma before composing so both UI layers and the scene are with linear gamma and Rec.709 primaries. 
    float3 result = lerp(scene, SRGBToLinear(uiLayer.rgb), uiLayer.a);

    if (displayCurve == DISPLAY_CURVE_SRGB)
    {
        result = LinearToSRGB(result);
    }
    else if (displayCurve == DISPLAY_CURVE_ST2084)
    {
        const float st2084max = 10000.0;
        const float hdrScalar = standardNits / st2084max;

        // The HDR scene is in Rec.709, but the display is Rec.2020
        result = Rec709ToRec2020(result);

        // Apply the ST.2084 curve to the scene.
        result = LinearToST2084(result * hdrScalar);
    }
    else // displayCurve == DISPLAY_CURVE_LINEAR
    {
        // Just pass through
    }

    return result;
}