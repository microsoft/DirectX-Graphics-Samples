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

#include "ParticleUtility.hlsli"

RWByteAddressBuffer g_DispatchIndirectArgs : register(u0);
RWByteAddressBuffer g_DrawIndirectArgs : register(u1);

[RootSignature(Particle_RootSig)]
[numthreads(1, 1, 1)]
void main( uint GI : SV_GroupIndex )
{
	uint InstanceCount = g_DrawIndirectArgs.Load(4);

	uint NextPow2 = (1 << firstbithigh(InstanceCount)) - 1;
	NextPow2 = (InstanceCount + NextPow2) & ~NextPow2;

	uint NumGroups = (NextPow2 + 2047) / 2048;

	g_DispatchIndirectArgs.Store3(0, uint3(NumGroups, 1, 1));

	// Reset instance count so we can cull and determine how many we need to actually draw
	g_DrawIndirectArgs.Store(4, 0);
}
