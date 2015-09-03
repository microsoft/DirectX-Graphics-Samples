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

#pragma once

#include "pch.h"

class GraphicsContext;
class ComputeContext;

void InitializeSamplerHeap( UINT HeapSize );

class Sampler
{
	friend class CommandContext;

public:
	Sampler() {}
	Sampler( D3D12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor )
		: m_hCpuDescriptorHandle(hCpuDescriptor) {}

	void Create( const D3D12_SAMPLER_DESC& Desc );

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle() const { return m_hCpuDescriptorHandle; }

protected:

	D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuDescriptorHandle;
};
