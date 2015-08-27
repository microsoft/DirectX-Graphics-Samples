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

#include "PixelBuffer.h"
#include "Color.h"

class EsramAllocator;

class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer( Color ClearColor = Color(0.0f, 0.0f, 0.0f, 0.0f)  )
		: m_ClearColor(ClearColor)
	{
		m_SRVHandle.ptr = ~0ull;
		m_RTVHandle.ptr = ~0ull;
		m_UAVHandle.ptr = ~0ull;
	}

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain( const std::wstring& name, ID3D12Resource* BaseResource );

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create( const std::wstring& name, size_t width, size_t height, size_t arrayCount, DXGI_FORMAT format,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN );
	
	// Create a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
	// this functions the same as Create() without a video address.
	void Create( const std::wstring& name, size_t width, size_t height, size_t arrayCount, DXGI_FORMAT format,
		EsramAllocator& esramAllocator );

	// Get pre-created CPU-visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE&	GetSRV(void) const	{ return m_SRVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE&	GetRTV(void) const	{ return m_RTVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE&	GetUAV(void) const	{ return m_UAVHandle; }

	Color GetClearColor() const { return m_ClearColor; }

private:

	void CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT Format, size_t ArraySize);

	Color							m_ClearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_SRVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_RTVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_UAVHandle;
};