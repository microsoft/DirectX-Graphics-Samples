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

#include "GpuResource.h"

class EsramAllocator;

class PixelBuffer : public GpuResource
{
public:
	PixelBuffer() : m_Width(0), m_Height(0) {}

	size_t GetWidth(void) const						{ return m_Width; }
	size_t GetHeight(void) const					{ return m_Height; }
	size_t GetDepth(void) const						{ return m_ArraySize; }
	const DXGI_FORMAT& GetFormat(void) const		{ return m_Format; }

protected:

	D3D12_RESOURCE_DESC DescribeTex2D( size_t width, size_t height, size_t depthOrArraySize, DXGI_FORMAT format, UINT flags);

	void AssociateWithResource( ID3D12Device* device, const std::wstring& name, ID3D12Resource* Resource, D3D12_RESOURCE_STATES CurrentState );

	void CreateTextureResource( ID3D12Device* device, const std::wstring& name, const D3D12_RESOURCE_DESC& ResourceDesc,
		D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN );

	void CreateTextureResource( ID3D12Device* device, const std::wstring& name, const D3D12_RESOURCE_DESC& ResourceDesc,
		D3D12_CLEAR_VALUE ClearValue, EsramAllocator& esramAllocator );

	static DXGI_FORMAT GetBaseFormat( DXGI_FORMAT Format );
	static DXGI_FORMAT GetUAVFormat( DXGI_FORMAT Format );
	static DXGI_FORMAT GetDSVFormat( DXGI_FORMAT Format );
	static DXGI_FORMAT GetDepthFormat( DXGI_FORMAT Format );
	static DXGI_FORMAT GetStencilFormat( DXGI_FORMAT Format );

	size_t			m_Width;
	size_t			m_Height;
	size_t			m_ArraySize;
	DXGI_FORMAT		m_Format;
};
