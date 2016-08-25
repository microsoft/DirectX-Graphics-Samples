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
#include "TextureManager.h"
#include "FileUtility.h"
#include "DDSTextureLoader.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include <map>
#include <thread>

using namespace std;
using namespace Graphics;

static UINT BytesPerPixel( DXGI_FORMAT Format )
{
	return (UINT)BitsPerPixel(Format) / 8;
};

void Texture::Create( size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData )
{
	m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = Width;
	texDesc.Height = (UINT)Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		m_UsageState, nullptr, MY_IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())));

	m_pResource->SetName(L"Texture");

	D3D12_SUBRESOURCE_DATA texResource;
	texResource.pData = InitialData;
	texResource.RowPitch = Width * BytesPerPixel(Format);
	texResource.SlicePitch = texResource.RowPitch * Height;

	CommandContext::InitializeTexture(*this, 1, &texResource);

	if (m_hCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		m_hCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateShaderResourceView(m_pResource.Get(), nullptr, m_hCpuDescriptorHandle);
}

void Texture::CreateTGAFromMemory( const void* _filePtr, size_t, bool sRGB )
{
	const uint8_t* filePtr = (const uint8_t*)_filePtr;

	// Skip first two bytes
	filePtr += 2;

	/*uint8_t imageTypeCode =*/ *filePtr++;

	// Ignore another 9 bytes
	filePtr += 9;

	uint16_t imageWidth = *(uint16_t*)filePtr;
	filePtr += sizeof(uint16_t);
	uint16_t imageHeight = *(uint16_t*)filePtr;
	filePtr += sizeof(uint16_t);
	uint8_t bitCount = *filePtr++;

	// Ignore another byte
	filePtr++;

	uint32_t* formattedData = new uint32_t[imageWidth * imageHeight];
	uint32_t* iter = formattedData;

	uint8_t numChannels = bitCount / 8;
	uint32_t numBytes = imageWidth * imageHeight * numChannels;

	switch (numChannels)
	{
	default:
		break;
	case 3:
		for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 3)
		{
			*iter++ = 0xff000000 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
			filePtr += 3;
		}
		break;
	case 4:
		for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 4)
		{
			*iter++ = filePtr[3] << 24 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
			filePtr += 4;
		}
		break;
	}

	Create( imageWidth, imageHeight, sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, formattedData );

	delete [] formattedData;
}

bool Texture::CreateDDSFromMemory( const void* filePtr, size_t fileSize, bool sRGB )
{
	if (m_hCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		m_hCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	HRESULT hr = CreateDDSTextureFromMemory( Graphics::g_Device,
		(const uint8_t*)filePtr, fileSize, 0, sRGB, &m_pResource, m_hCpuDescriptorHandle );

	return SUCCEEDED(hr);
}

namespace TextureManager
{
	wstring s_RootPath = L"";
	map< wstring, unique_ptr<ManagedTexture> > s_TextureCache;

	void Initialize( const std::wstring& TextureLibRoot )
	{
		s_RootPath = TextureLibRoot;
	}

	void Shutdown( void )
	{
		s_TextureCache.clear();
	}

	pair<ManagedTexture*, bool> FindOrLoadTexture( const wstring& fileName )
	{
		static mutex s_Mutex;
		lock_guard<mutex> Guard(s_Mutex);

		auto iter = s_TextureCache.find(fileName);

		// If it's found, it has already been loaded or the load process has begun
		if (iter != s_TextureCache.end())
			return make_pair(iter->second.get(), false);

		ManagedTexture* NewTexture = new ManagedTexture(fileName);
		s_TextureCache[fileName].reset( NewTexture );

		// This was the first time it was requested, so indicate that the caller must read the file
		return make_pair(NewTexture, true);
	}

	const Texture& GetBlackTex2D(void)
	{
		auto ManagedTex = FindOrLoadTexture(L"DefaultBlackTexture");

		ManagedTexture* ManTex = ManagedTex.first;
		const bool RequestsLoad = ManagedTex.second;

		if (!RequestsLoad)
		{
			ManTex->WaitForLoad();
			return *ManTex;
		}

		uint32_t BlackPixel = 0;
		ManTex->Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackPixel);
		return *ManTex;
	}

	const Texture& GetWhiteTex2D(void)
	{
		auto ManagedTex = FindOrLoadTexture(L"DefaultWhiteTexture");

		ManagedTexture* ManTex = ManagedTex.first;
		const bool RequestsLoad = ManagedTex.second;

		if (!RequestsLoad)
		{
			ManTex->WaitForLoad();
			return *ManTex;
		}

		uint32_t WhitePixel = ~0;
		ManTex->Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhitePixel);
		return *ManTex;
	}

	const Texture& GetMagentaTex2D(void)
	{
		auto ManagedTex = FindOrLoadTexture(L"DefaultMagentaTexture");

		ManagedTexture* ManTex = ManagedTex.first;
		const bool RequestsLoad = ManagedTex.second;

		if (!RequestsLoad)
		{
			ManTex->WaitForLoad();
			return *ManTex;
		}

		uint32_t MagentaPixel = 0x00FF00FF;
		ManTex->Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &MagentaPixel);
		return *ManTex;
	}

} // namespace TextureManager

void ManagedTexture::WaitForLoad( void ) const
{
	volatile D3D12_CPU_DESCRIPTOR_HANDLE& VolHandle = (volatile D3D12_CPU_DESCRIPTOR_HANDLE&)m_hCpuDescriptorHandle;
	volatile bool& VolValid = (volatile bool&)m_IsValid;
	while (VolHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN && VolValid)
		this_thread::yield();
}

void ManagedTexture::SetToInvalidTexture( void )
{
	m_hCpuDescriptorHandle = TextureManager::GetMagentaTex2D().GetSRV();
	m_IsValid = false;
}

const ManagedTexture* TextureManager::LoadFromFile( const std::wstring& fileName, bool sRGB )
{
	std::wstring CatPath = fileName;

	const ManagedTexture* Tex = LoadDDSFromFile( CatPath + L".dds", sRGB );
	if (!Tex->IsValid())
		Tex = LoadTGAFromFile( CatPath + L".tga", sRGB );

	return Tex;
}

const ManagedTexture* TextureManager::LoadDDSFromFile( const std::wstring& fileName, bool sRGB )
{
	auto ManagedTex = FindOrLoadTexture(fileName);

	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad)
	{
		ManTex->WaitForLoad();
		return ManTex;
	}

	Utility::ByteArray ba = Utility::ReadFileSync( s_RootPath + fileName );
	if (ba->size() == 0 || !ManTex->CreateDDSFromMemory( ba->data(), ba->size(), sRGB ))
		ManTex->SetToInvalidTexture();
	else
		ManTex->GetResource()->SetName(fileName.c_str());

	return ManTex;
}

const ManagedTexture* TextureManager::LoadTGAFromFile( const std::wstring& fileName, bool sRGB )
{
	auto ManagedTex = FindOrLoadTexture(fileName);

	ManagedTexture* ManTex = ManagedTex.first;
	const bool RequestsLoad = ManagedTex.second;

	if (!RequestsLoad)
	{
		ManTex->WaitForLoad();
		return ManTex;
	}

	Utility::ByteArray ba = Utility::ReadFileSync( s_RootPath + fileName );
	if (ba->size() > 0)
	{
		ManTex->CreateTGAFromMemory( ba->data(), ba->size(), sRGB );
		ManTex->GetResource()->SetName(fileName.c_str());
	}
	else
		ManTex->SetToInvalidTexture();

	return ManTex;
}
