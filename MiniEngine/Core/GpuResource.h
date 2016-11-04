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

class GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	GpuResource() : 
		m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		m_UsageState(D3D12_RESOURCE_STATE_COMMON),
		m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{}

	GpuResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState) :
		m_pResource(pResource),
		m_UsageState(CurrentState),
		m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	void Destroy()
	{
		m_pResource = nullptr;
	}

	ID3D12Resource* operator->() { return m_pResource.Get(); } 
	const ID3D12Resource* operator->() const { return m_pResource.Get(); }

	ID3D12Resource* GetResource() { return m_pResource.Get(); } 
	const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

protected:

	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
	D3D12_RESOURCE_STATES m_UsageState;
	D3D12_RESOURCE_STATES m_TransitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};
