//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

//
// Converts the provided source WIC pixel format into the closest matching
// target format which we can use with tiled resources.
//
bool GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat);

//
// Converts the WIC pixel format to the corresponding DXGI format. Returns
// DXGI_FORMAT_UNKNOWN if the pixel format is not supported.
//
DXGI_FORMAT GetDXGIFormatFromPixelFormat(const GUID* pPixelFormat);

//
// Gets the index to the least detailed mipmap for the specified resource.
//
inline UINT8 GetLeastDetailedMipIndex(_In_ const Resource* pResource)
{
	return pResource->NumStandardMips + pResource->NumPackedMips - 1;
}

//
// Gets the index to the heap corresponding to the least detailed mipmap
// in the resource. All packed mipmaps share a single heap index, so this value
// is not equal to the least detailed mipmap index.
//
inline UINT8 GetLeastDetailedMipHeapIndex(_In_ const Resource* pResource)
{
	if (pResource->NumPackedMips != 0)
	{
		return pResource->PackedMipHeapIndex;
	}
	return pResource->NumStandardMips - 1;
}

//
// Returns true if 'MipToCheck' is less detailed that 'CurrentMip'
//
inline bool IsLessDetailedMip(UINT8 CurrentMip, UINT8 MipToCheck)
{
	return MipToCheck > CurrentMip;
}

//
// Returns true if 'MipToCheck' is more detailed that 'CurrentMip'
//
inline bool IsMoreDetailedMip(UINT8 CurrentMip, UINT8 MipToCheck)
{
	return MipToCheck < CurrentMip;
}

//
// Calculates the mipmap level that would be used when sampling a resource, given
// the orthographic camera zoom level.
//
inline float CalculateRequiredMipLevel(_In_ const Resource* pResource, float ImageScale)
{
	D3D12_RESOURCE_DESC Desc = pResource->pDeviceState->pD3DResource->GetDesc();

	//
	// Calculate rough derivative, knowing that the image is a screen-space quad.
	// This should be equal to the ratio of texels per pixel (ImageScale is equal
	// to the screen space pixel size of the texture with projection zoom applied).
	//
	float d = Desc.Width / ImageScale;
	float dSqr = d * d;

	//
	// Mip = log2(DerivativeSquared) / 2
	//
	float Log = log(dSqr) / log(2.0f);
	float Mip = 0.5f * Log;
	return max(Mip, 0.0f);
}

//
// Gets the total number of mipmaps in this resource.
//
inline UINT8 GetResourceMipCount(_In_ const Resource* pResource)
{
	return pResource->NumStandardMips + pResource->NumPackedMips;
}

//
// Gets the accent color for the generated image.
//
UINT GetGeneratedImageColor(UINT ImageIndex);

//
// Returns true if the two rectangles intersect.
//
inline bool RectIntersects(const RectF& a, const RectF& b)
{
	return a.Left < b.Right && a.Right > b.Left &&
		a.Top < b.Bottom && a.Bottom > b.Top;
}

//
// Returns true if the two rectangles are with the specified Tolerence distance from one another.
//
inline bool RectNearlyIntersects(const RectF& a, const RectF& b, float Tolerence)
{
	return a.Left - Tolerence < b.Right && a.Right + Tolerence > b.Left &&
		a.Top - Tolerence < b.Bottom && a.Bottom + Tolerence> b.Top;
}

//
// Inflates a rectangle by the specified size on all sides.
//
inline void InflateRectangle(RectF& Rectangle, float Size)
{
	Rectangle.Bottom += Size;
	Rectangle.Left -= Size;
	Rectangle.Right += Size;
	Rectangle.Top -= Size;
}

//
// Returns the more detailed of two mipmap levels.
//
inline UINT8 ChooseMoreDetailedMip(UINT8 MipA, UINT8 MipB)
{
	return min(MipA, MipB);
}

//
// Returns the less detailed of two mipmap levels.
//
inline UINT8 ChooseLessDetailedMip(UINT8 MipA, UINT8 MipB)
{
	return max(MipA, MipB);
}

//
// Returns the mip level after increasing the quality by 'IncreaseBy' levels of quality,
// and clamping the quality to 0.
//
inline UINT8 IncreaseMipQuality(UINT8 Mip, UINT8 IncreaseBy)
{
	if (IncreaseBy > Mip)
	{
		return 0;
	}
	return Mip - IncreaseBy;
}

//
// Returns the mip level after decreasing the quality by 'DecreaseBy' levels of quality.
//
inline UINT8 DecreaseMipQuality(UINT8 Mip, UINT8 DecreaseBy)
{
	return Mip + DecreaseBy;
}

//
// Translates a lowercase character to the equivalent virtual key
//
inline UINT GetVirtualKeyFromCharacter(char c)
{
	return 0x41 + (c - 'a');
}

//
// Calculates the number of heaps required to page in the provided resource mipmap, based
// on the maximimum heap size.
//
inline UINT GetResourceMipHeapCount(const ResourceMip& rMip)
{
	return ((rMip.Desc.WidthInTiles * rMip.Desc.HeightInTiles * TILE_SIZE) + MAX_HEAP_SIZE - 1) / MAX_HEAP_SIZE;
}

//
// Gets the heap index for a resource, given the mip level. All packed mipmaps
// share a single heap index.
//
inline UINT8 GetMipHeapIndexForResource(_In_ const Resource* pResource, UINT8 Mip)
{
	UINT8 Index = Mip;
	if (Index > pResource->PackedMipHeapIndex)
	{
		Index = pResource->PackedMipHeapIndex;
	}

	return Index;
}

//
// Calculates the size of a non-packed mipmap, in bytes.
//
inline UINT64 GetNonPackedMipSize(_In_ const Resource* pResource, UINT MipHeapIndex)
{
	ResourceMip* pMip = &pResource->pDeviceState->Mips[MipHeapIndex];
	return (UINT64)pMip->Desc.WidthInTiles * (UINT64)pMip->Desc.HeightInTiles * TILE_SIZE;
}

//
// Gets a friendly string for the provided D3D feature level.
//
inline LPCSTR GetFeatureLevelName(D3D_FEATURE_LEVEL FeatureLevel)
{
	switch (FeatureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1: return "9.1";
	case D3D_FEATURE_LEVEL_9_2: return "9.2";
	case D3D_FEATURE_LEVEL_9_3: return "9.3";
	case D3D_FEATURE_LEVEL_10_0: return "10.0";
	case D3D_FEATURE_LEVEL_10_1: return "10.1";
	case D3D_FEATURE_LEVEL_11_0: return "11.0";
	case D3D_FEATURE_LEVEL_11_1: return "11.1";
	case D3D_FEATURE_LEVEL_12_0: return "12.0";
	case D3D_FEATURE_LEVEL_12_1: return "12.1";

	default: return "12.1+";
	}
}

//
// Calculates the required size of a constant buffer, based on the D3D12 alignment
// restriction of constant buffers (256 bytes).
//
inline UINT32 CalculateConstantBufferSize(UINT32 Size)
{
	//
	// Constant buffers are required to be 256 byte aligned, and multiples of 256 bytes.
	//
	return (Size + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

//
// Get the absolute path to the running exe.
//
inline void GetWorkingDir(_Out_writes_z_(pathSize) WCHAR* path, UINT pathSize)
{
	if (path == nullptr)
	{
		return;
	}

	DWORD size = GetModuleFileName(nullptr, path, pathSize);

	if (size == 0 || size == pathSize)
	{
		// Method failed or path was truncated.
		*path = L'\0';
		return;
	}

	WCHAR* lastSlash = wcsrchr(path, L'\\');
	if (lastSlash)
	{
		*(lastSlash+1) = L'\0';
	}
}
