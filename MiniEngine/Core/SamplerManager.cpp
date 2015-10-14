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
// Author(s):  James Stanard
//             Alex Nankervis
//

#include "pch.h"
#include "SamplerManager.h"
#include "GraphicsCore.h"
#include "Hash.h"
#include <map>

using namespace std;
using Graphics::g_Device;

namespace
{
	map< size_t, D3D12_CPU_DESCRIPTOR_HANDLE > s_SamplerCache;
}

void SamplerDescriptor::Create( const D3D12_SAMPLER_DESC& Desc )
{
	size_t hashValue = Utility::HashState(&Desc);
	auto iter = s_SamplerCache.find(hashValue);
	if (iter != s_SamplerCache.end())
	{
		*this = SamplerDescriptor(iter->second);
		return;
	}

	m_hCpuDescriptorHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	g_Device->CreateSampler(&Desc, m_hCpuDescriptorHandle);
}
