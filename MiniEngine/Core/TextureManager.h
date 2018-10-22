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

#pragma once

#include "pch.h"
#include "GpuResource.h"
#include "Utility.h"

class Texture : public GpuResource
{
    friend class CommandContext;

public:

    Texture() { m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
    Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) : m_hCpuDescriptorHandle(Handle) {}

    // Create a 1-level 2D texture
    void Create(size_t Pitch, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData );
    void Create(size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData )
    {
        Create(Width, Width, Height, Format, InitData);
    }

    void CreateTGAFromMemory( const void* memBuffer, size_t fileSize, bool sRGB );
    bool CreateDDSFromMemory( const void* memBuffer, size_t fileSize, bool sRGB );
    void CreatePIXImageFromMemory( const void* memBuffer, size_t fileSize );

    virtual void Destroy() override
    {
        GpuResource::Destroy();
        // This leaks descriptor handles.  We should really give it back to be reused.
        m_hCpuDescriptorHandle.ptr = 0;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_hCpuDescriptorHandle; }

    bool operator!() { return m_hCpuDescriptorHandle.ptr == 0; }

protected:

    D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuDescriptorHandle;
};

class ManagedTexture : public Texture
{
public:
    ManagedTexture( const std::wstring& FileName ) : m_MapKey(FileName), m_IsValid(true) {}

    void operator= ( const Texture& Texture );

    void WaitForLoad(void) const;
    void Unload(void);

    void SetToInvalidTexture(void);
    bool IsValid(void) const { return m_IsValid; }

private:
    std::wstring m_MapKey;        // For deleting from the map later
    bool m_IsValid;
};

namespace TextureManager
{
    void Initialize( const std::wstring& TextureLibRoot );
    void Shutdown(void);

    const ManagedTexture* LoadFromFile( const std::wstring& fileName, bool sRGB = false );
    const ManagedTexture* LoadDDSFromFile( const std::wstring& fileName, bool sRGB = false );
    const ManagedTexture* LoadTGAFromFile( const std::wstring& fileName, bool sRGB = false );
    const ManagedTexture* LoadPIXImageFromFile( const std::wstring& fileName );

    inline const ManagedTexture* LoadFromFile( const std::string& fileName, bool sRGB = false )
    {
        return LoadFromFile(MakeWStr(fileName), sRGB);
    }

    inline const ManagedTexture* LoadDDSFromFile( const std::string& fileName, bool sRGB = false )
    {
        return LoadDDSFromFile(MakeWStr(fileName), sRGB);
    }

    inline const ManagedTexture* LoadTGAFromFile( const std::string& fileName, bool sRGB = false )
    {
        return LoadTGAFromFile(MakeWStr(fileName), sRGB);
    }

    inline const ManagedTexture* LoadPIXImageFromFile( const std::string& fileName )
    {
        return LoadPIXImageFromFile(MakeWStr(fileName));
    }

    const Texture& GetBlackTex2D(void);
    const Texture& GetWhiteTex2D(void);
}
