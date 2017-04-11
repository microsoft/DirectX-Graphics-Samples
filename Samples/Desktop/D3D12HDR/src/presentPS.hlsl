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
	// The scene is rendered in linear space.
	// For ST.2084 and linear output, the upper half of the scene is understood to
	// be coming from Rec 709 primaries and the lower half of the scene is understood
	// to be coming from Rec 2020 primaries. (sRGB is Rec 709 by definition)
	float3 scene = g_scene.Sample(g_sampler, input.uv).rgb;

	// Direct2D renders the UI layer in gamma-corrected sRGB space.
	float4 uiLayer = g_ui.Sample(g_sampler, input.uv);

	// Compose the scene and UI layers.
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
		result = ApplyST2084(result * hdrScalar);
	}
	else // displayCurve == DISPLAY_CURVE_LINEAR
	{
		// Just pass through
	}

	return result;
}
