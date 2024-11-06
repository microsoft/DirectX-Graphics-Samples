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

#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// outdated warning about for-loop variable scope
#pragma warning (disable: 3078)
// single-iteration loop
#pragma warning (disable: 3557)

#define Renderer_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(Sampler(s0, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(SRV(t10, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "CBV(b1), " \
    "SRV(t20, visibility = SHADER_VISIBILITY_VERTEX), " \
    "StaticSampler(s10, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s11, visibility = SHADER_VISIBILITY_PIXEL," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "comparisonFunc = COMPARISON_GREATER_EQUAL," \
        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)," \
    "StaticSampler(s12, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)"

// Common (static) samplers
SamplerState defaultSampler : register(s10);
SamplerComparisonState shadowSampler : register(s11);
SamplerState cubeMapSampler : register(s12);

#ifndef ENABLE_TRIANGLE_ID
    #define ENABLE_TRIANGLE_ID 0
#endif

#if ENABLE_TRIANGLE_ID

uint HashTriangleID(uint vertexID)
{
	// TBD SM6.1 stuff
	uint Index0 = EvaluateAttributeAtVertex(vertexID, 0);
	uint Index1 = EvaluateAttributeAtVertex(vertexID, 1);
	uint Index2 = EvaluateAttributeAtVertex(vertexID, 2);

	// When triangles are clipped (to the near plane?) their interpolants can sometimes
	// be reordered.  To stabilize the ID generation, we need to sort the indices before
	// forming the hash.
	uint I0 = __XB_Min3_U32(Index0, Index1, Index2);
	uint I1 = __XB_Med3_U32(Index0, Index1, Index2);
	uint I2 = __XB_Max3_U32(Index0, Index1, Index2);
	return (I2 & 0xFF) << 16 | (I1 & 0xFF) << 8 | (I0 & 0xFF0000FF);
}

#endif // ENABLE_TRIANGLE_ID

#endif // __COMMON_HLSLI__
