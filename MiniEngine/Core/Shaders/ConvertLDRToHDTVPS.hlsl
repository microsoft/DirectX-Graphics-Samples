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

#include "ShaderUtility.hlsli"

Texture2D<float3> ColorTex : register(t0);

float3 main( float4 position : SV_Position ) : SV_Target0
{
	float3 LinearRGB = LinearizeColor(ColorTex[(int2)position.xy], LDR_COLOR_FORMAT);

#ifdef _XBOX_ONE
	LinearRGB = SRGBToLinear_Exact(LinearToREC709_Exact(LinearRGB));
#endif

	return ApplyColorProfile(LinearRGB, DISPLAY_PLANE_FORMAT);
}