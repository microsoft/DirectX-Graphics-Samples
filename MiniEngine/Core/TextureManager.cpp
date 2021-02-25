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
//

#include "pch.h"
#include "TextureManager.h"
#include "DDSTextureLoader.h"
#include "Texture.h"
#include "Utility.h"
#include "FileUtility.h"
#include "GraphicsCommon.h"
#include "CommandContext.h"
#include <map>
#include <thread>

using namespace std;
using namespace Graphics;
using Utility::ByteArray;

//
// A ManagedTexture allows for multiple threads to request a Texture load of the same
// file.  It also contains a reference count of the Texture so that it can be freed
// when it is no longer referenced.
//
// Raw ManagedTexture pointers are not exposed to clients.  
//
class ManagedTexture : public Texture
{
    friend class TextureRef;

public:
    ManagedTexture( const wstring& FileName );

    void WaitForLoad(void) const;
    void CreateFromMemory(ByteArray memory, eDefaultTexture fallback, bool sRGB);

private:

    bool IsValid(void) const { return m_IsValid; }
    void Unload();

    std::wstring m_MapKey;		// For deleting from the map later
    bool m_IsValid;
    bool m_IsLoading;
    size_t m_ReferenceCount;
};

namespace TextureManager
{
    wstring s_RootPath = L"";
    map<wstring, std::unique_ptr<ManagedTexture>> s_TextureCache;

    void Initialize( const wstring& TextureLibRoot )
    {
        s_RootPath = TextureLibRoot;
    }

    void Shutdown( void )
    {
        s_TextureCache.clear();
    }

    mutex s_Mutex;

    ManagedTexture* FindOrLoadTexture( const wstring& fileName, eDefaultTexture fallback, bool forceSRGB )
    {
        ManagedTexture* tex = nullptr;

        {
            lock_guard<mutex> Guard(s_Mutex);

            wstring key = fileName;
            if (forceSRGB)
                key += L"_sRGB";

            // Search for an existing managed texture
            auto iter = s_TextureCache.find(key);
            if (iter != s_TextureCache.end())
            {
                // If a texture was already created make sure it has finished loading before
                // returning a point to it.
                tex = iter->second.get();
                tex->WaitForLoad();
                return tex;
            }
            else
            {
                // If it's not found, create a new managed texture and start loading it
                tex = new ManagedTexture(key);
                s_TextureCache[key].reset(tex);
            }
        }

        Utility::ByteArray ba = Utility::ReadFileSync( s_RootPath + fileName );
        tex->CreateFromMemory(ba, fallback, forceSRGB);

        // This was the first time it was requested, so indicate that the caller must read the file
        return tex;
    }

    void DestroyTexture(const wstring& key)
    {
        lock_guard<mutex> Guard(s_Mutex);

        auto iter = s_TextureCache.find(key);
        if (iter != s_TextureCache.end())
            s_TextureCache.erase(iter);
    }

} // namespace TextureManager

ManagedTexture::ManagedTexture( const wstring& FileName )
    : m_MapKey(FileName), m_IsValid(false), m_IsLoading(true), m_ReferenceCount(0)
{
    m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}

void ManagedTexture::CreateFromMemory(ByteArray ba, eDefaultTexture fallback, bool forceSRGB)
{
    if (ba->size() == 0)
    {
        m_hCpuDescriptorHandle = GetDefaultTexture(fallback);
    }
    else
    {
        // We probably have a texture to load, so let's allocate a new descriptor
        m_hCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        if ( SUCCEEDED( CreateDDSTextureFromMemory( g_Device, (const uint8_t*)ba->data(), ba->size(),
            0, forceSRGB, m_pResource.GetAddressOf(), m_hCpuDescriptorHandle) ) )
        {
            m_IsValid = true;
            D3D12_RESOURCE_DESC desc = GetResource()->GetDesc();
            m_Width = (uint32_t)desc.Width;
            m_Height = desc.Height;
            m_Depth = desc.DepthOrArraySize;
        }
        else
        {
            g_Device->CopyDescriptorsSimple(1, m_hCpuDescriptorHandle, GetDefaultTexture(fallback),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
    }

    m_IsLoading = false;
}

void ManagedTexture::WaitForLoad( void ) const
{
    while ((volatile bool&)m_IsLoading)
        this_thread::yield();
}

void ManagedTexture::Unload()
{
    TextureManager::DestroyTexture(m_MapKey);
}

TextureRef::TextureRef( const TextureRef& ref ) : m_ref(ref.m_ref)
{
    if (m_ref != nullptr)
        ++m_ref->m_ReferenceCount;
}

TextureRef::TextureRef( ManagedTexture* tex ) : m_ref(tex)
{
    if (m_ref != nullptr)
        ++m_ref->m_ReferenceCount;
}

TextureRef::~TextureRef()
{
    if (m_ref != nullptr && --m_ref->m_ReferenceCount == 0)
        m_ref->Unload();
}

void TextureRef::operator= (std::nullptr_t)
{
    if (m_ref != nullptr)
        --m_ref->m_ReferenceCount;

    m_ref = nullptr;
}

void TextureRef::operator= (TextureRef& rhs)
{
    if (m_ref != nullptr)
        --m_ref->m_ReferenceCount;

    m_ref = rhs.m_ref;

    if (m_ref != nullptr)
        ++m_ref->m_ReferenceCount;
}

bool TextureRef::IsValid() const
{
    return m_ref && m_ref->IsValid();
}

const Texture* TextureRef::Get( void ) const
{
    return m_ref;
}

const Texture* TextureRef::operator->( void ) const
{
    ASSERT(m_ref != nullptr);
    return m_ref;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureRef::GetSRV() const
{
    if (m_ref != nullptr)
        return m_ref->GetSRV();
    else
        return GetDefaultTexture(kMagenta2D);
}


TextureRef TextureManager::LoadDDSFromFile( const wstring& filePath, eDefaultTexture fallback, bool forceSRGB )
{
    return FindOrLoadTexture(filePath, fallback, forceSRGB);
}

TextureRef TextureManager::LoadDDSFromFile( const string& filePath, eDefaultTexture fallback, bool forceSRGB )
{
    return LoadDDSFromFile(Utility::UTF8ToWideString(filePath), fallback, forceSRGB);
}
