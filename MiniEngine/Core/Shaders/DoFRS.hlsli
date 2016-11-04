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

#define DoF_RootSig \
	"RootFlags(0), " \
	"CBV(b0), " \
	"DescriptorTable(SRV(t0, numDescriptors = 6))," \
	"DescriptorTable(UAV(u0, numDescriptors = 3))," \
	"RootConstants(b1, num32BitConstants = 1), " \
	"StaticSampler(s0," \
		"addressU = TEXTURE_ADDRESS_BORDER," \
		"addressV = TEXTURE_ADDRESS_BORDER," \
		"addressW = TEXTURE_ADDRESS_BORDER," \
		"borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK," \
		"filter = FILTER_MIN_MAG_MIP_POINT)," \
	"StaticSampler(s1," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"filter = FILTER_MIN_MAG_MIP_POINT)," \
	"StaticSampler(s2," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"filter = FILTER_MIN_MAG_MIP_LINEAR)"
