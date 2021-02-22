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

#include <cstdint>
#include <string>

enum TexConversionFlags
{
    kSRGB = 1,          // Texture contains sRGB colors
    kPreserveAlpha = 2, // Keep four channels
    kNormalMap = 4,     // Texture contains normals
    kBumpToNormal = 8,  // Generate a normal map from a bump map
    kDefaultBC = 16,    // Apply standard block compression (BC1-5)
    kQualityBC = 32,    // Apply quality block compression (BC6H/7)
    kFlipVertical = 64,
};

inline uint8_t TextureOptions(bool sRGB, bool hasAlpha=false, bool invertY=false)
{
    return (sRGB ? kSRGB : 0) | (hasAlpha ? kPreserveAlpha : 0) | (invertY ? kFlipVertical : 0);
}

// If the DDS version of the texture specified does not exist or is older than the source texture, reconvert it.
void CompileTextureOnDemand(const std::wstring& originalFile, uint32_t flags);

// Loads a non-DDS texture such as TGA, PNG, or JPG, then converts it to a more optimal
// DDS format with a full mip chain.  Resultant file has the same path with the file extension
// changed to "DDS".
bool ConvertToDDS(
    const std::wstring& filePath,	// UTF8-encoded path to source file
    uint32_t Flags                  // flags ORed together
);
