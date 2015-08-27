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
#include "ColorBuffer.h"
#include "GraphicsCore.h"
#include "EsramAllocator.h"

void ColorBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, size_t ArraySize)
{
	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

    RTVDesc.Format = Format;
    UAVDesc.Format = GetUAVFormat(Format);
	SRVDesc.Format = Format;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (ArraySize > 1)
	{
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		RTVDesc.Texture2DArray.MipSlice = 0;
		RTVDesc.Texture2DArray.FirstArraySlice = 0;
		RTVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		UAVDesc.Texture2DArray.MipSlice = 0;
		UAVDesc.Texture2DArray.FirstArraySlice = 0;
		UAVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		SRVDesc.Texture2DArray.MipLevels = 1;
		SRVDesc.Texture2DArray.MostDetailedMip = 0;
		SRVDesc.Texture2DArray.FirstArraySlice = 0;
		SRVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
	}
	else
	{
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;

		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Texture2D.MipSlice = 0;

		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Texture2D.MostDetailedMip = 0;
	}

	if (m_SRVHandle.ptr == ~0ull)
	{
		m_RTVHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_UAVHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_SRVHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ID3D12Resource* Resource = m_pResource.Get();

    // Create the render target view
	Device->CreateRenderTargetView(Resource, &RTVDesc, m_RTVHandle);

    // Create the UAV (RWTexture2D)
	Device->CreateUnorderedAccessView(Resource, nullptr, &UAVDesc, m_UAVHandle);

    // Create the shader resource view
	Device->CreateShaderResourceView(Resource, &SRVDesc, m_SRVHandle);
}

void ColorBuffer::CreateFromSwapChain( const std::wstring& name, ID3D12Resource* BaseResource )
{
	AssociateWithResource(Graphics::g_Device, name, BaseResource, D3D12_RESOURCE_STATE_PRESENT);

	// BUG:  Currently, we are prohibited from creating UAVs of the swap chain.  We can create an SRV, but we
	// don't have a need other than generality.
	//CreateDerivedViews(Graphics::g_Device, BaseResource->GetDesc().Format, 1);

	// WORKAROUND:  Just create a typical RTV
	m_RTVHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	Graphics::g_Device->CreateRenderTargetView(m_pResource.Get(), nullptr, m_RTVHandle);
}

void ColorBuffer::Create( const std::wstring& name, size_t width, size_t height, size_t arrayCount, DXGI_FORMAT format,
	D3D12_GPU_VIRTUAL_ADDRESS VidMem )
{
	D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(width, height, arrayCount, format,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = format;
	ClearValue.Color[0] = m_ClearColor.R();
	ClearValue.Color[1] = m_ClearColor.G();
	ClearValue.Color[2] = m_ClearColor.B();
	ClearValue.Color[3] = m_ClearColor.A();

	CreateTextureResource(Graphics::g_Device, name, ResourceDesc, ClearValue, VidMem);
	CreateDerivedViews(Graphics::g_Device, format, arrayCount);
}

void ColorBuffer::Create( const std::wstring& name, size_t width, size_t height, size_t arrayCount, DXGI_FORMAT format,
	EsramAllocator& esramAllocator )
{
	Create(name, width, height, arrayCount, format);
}
