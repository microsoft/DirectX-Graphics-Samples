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

#include "pch.h"
#include "DepthBuffer.h"
#include "GraphicsCore.h"
#include "EsramAllocator.h"
#include "DescriptorHeap.h"

using namespace Graphics;

void DepthBuffer::Create( const std::wstring& name, size_t width, size_t height, DXGI_FORMAT format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr )
{
	D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(width, height, 1, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = format;
	CreateTextureResource(Graphics::g_Device, name, ResourceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(Graphics::g_Device, format);
}

void DepthBuffer::Create( const std::wstring& name, size_t width, size_t height, DXGI_FORMAT format, EsramAllocator& esramAllocator )
{
	Create(name, width, height, format);
}

void DepthBuffer::CreateDerivedViews( ID3D12Device* Device, DXGI_FORMAT Format )
{
	ID3D12Resource* Resource = m_pResource.Get();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = GetDSVFormat(Format);
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

	if (m_hDSV[0].ptr == ~0ull)
	{
		m_hDSV[0] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_hDSV[1] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[0]);

	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[1]);

	DXGI_FORMAT stencilReadFormat = GetStencilFormat(Format);
	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_hDSV[2].ptr == ~0ull)
		{
			m_hDSV[2] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_hDSV[3] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[2]);

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[3]);
	}
	else
	{
		m_hDSV[2] = m_hDSV[0];
		m_hDSV[3] = m_hDSV[1];
	}

	if (m_hDepthSRV.ptr == ~0ull)
		m_hDepthSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Create the shader resource view
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = GetDepthFormat(Format);
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Texture2D.MipLevels = 1;
    Device->CreateShaderResourceView( Resource, &SRVDesc, m_hDepthSRV );

	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_hStencilSRV.ptr == ~0ull)
			m_hStencilSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		SRVDesc.Format = stencilReadFormat;
		Device->CreateShaderResourceView( Resource, &SRVDesc, m_hStencilSRV );
	}
}
