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
// Used to describe each mip index.
// Each resource will have one MipDescription per standard mipmap, and one shared
// MipDescription for all packed mips.
//
struct MipDescription
{
	UINT WidthInTiles;
	UINT HeightInTiles;
	UINT HeapStartIndex;
};

//
// Stores information about each mipmap in a resource. This includes tile information,
// an array of heaps backing the resource, and the last rendering fence which
// referenced this specific mipmap.
// Each mipmap may be backed by multiple heaps, currently defined in 16MB chunks.
// Breaking large allocations into smaller chunks helps reduce pressure in the kernel's
// memory manager when finding space to place the resource in VRAM, and reduces overall
// memory fragmentation.
//
struct ResourceMip
{
	MipDescription Desc;
	ID3D12Heap** ppHeaps;
	UINT64 ReferenceFence;
};

//
// Describes the priority which the worker thread uses to page in a resource. A
// higher priority resource will always be paged in before a lower priority resource.
//
enum ResourcePriority
{
	ERP_VeryHigh,
	ERP_High,
	ERP_Medium,
	ERP_Low,
	_ERP_COUNT
};

//
// This enum is used to restrict the extent of trimming operations when the process
// is over its local memory budget. This helps ensure that lower priority resources not
// cause higher priority resources to be trimmed, and prevents the worker thread from
// recursively paging in new content.
// e.g. a high quality prefetched mip will not trim a mipmap that is currently
// visible on the screen.
//
enum ResourceTrimPass
{
	ERTP_None,
	ERTP_NonPrefetchable,
	ERTP_NonVisible,
	ERTP_Visible,
};

//
// Stores the per-resource device dependent state. This data must be recreated when
// the graphics device is removed due to TDR, driver upgrade, surprise removal, etc.
// Mipmap information and heap count must be recalculated because the newly chosen
// video adapter may tile resources differently than the current one, and so the actual
// size of Mips[] may vary from device to device.
//
struct ResourceDeviceState
{
	ID3D12Resource* pD3DResource;
	UINT32 NumHeaps;

	//
	// Mips must always be the last element, since it is actually a dynamic array.
	// The actual count of the array is equal to the number of unique mip heaps.
	// There is one mip heap for all packed mipmaps (mipmaps which can be packed
	// into one or few tiles), and one mip heap for each standard mipmap (mipmaps
	// which cannot be packed).
	//
	ResourceMip Mips[1];
};

//
// A resource object stores the device independent state information about a resource
// which does not have to be recreated when a device is removed.
//
struct Resource
{
	// List entry for tracking the existence of the Resource object.
	LIST_ENTRY ListEntry;

	// List entry for tracking the commitment of mipmaps for this resource.
	LIST_ENTRY CommittedListEntry;

	// List entry used by the worker thread to prioritize paging operations.
	LIST_ENTRY PrioritizationEntry;

	// List entry used to track paging requests.
	LIST_ENTRY PagingEntry;

	CRITICAL_SECTION ReferenceLock;

#if(_DEBUG)
	// Store the file name of the resource for easy identification during debugging.
	const wchar_t* pFileName;
#endif

	// The image decoder associated with this resource if it is backed by a file.
	IWICBitmapDecoder* pDecoder;

	// The image index associated with this resource if it is generated at runtime.
	UINT GeneratedImageIndex;

	// The number of tiles required to store packed mipmaps.
	UINT32 PackedMipTileCount;
	union
	{
		// The number of standard mipmaps in this resource.
		UINT8 NumStandardMips : MAX_MIP_COUNT_BITS;

		// Index to the first packed mipmap.
		UINT8 PackedMipHeapIndex : MAX_MIP_COUNT_BITS;
	};

	// The number of packed mipmaps in this resource.
	UINT8 NumPackedMips : MAX_MIP_COUNT_BITS;

	// Set by the paging thread to indicate the most detailed mip level that is
	// currently paged in and fully resident.
	UINT8 MostDetailedMipResident : MAX_MIP_COUNT_BITS;

	// Used to restrict the maximum mip quality that can be used during rendering
	// when preparing to trim mipmaps from visible resources.
	UINT8 MipRestriction : MAX_MIP_COUNT_BITS;

	// The current mip level that is visible on screen for this resource. When
	// the resource is not visible, this value will be UNDEFINED_MIPMAP_INDEX.
	UINT8 VisibleMip : MAX_MIP_COUNT_BITS;

	// The minimum mip level, requested by the render thread, that the paging thread
	// should attempt to prefetch to avoid texture popping issues caused by fast
	// camera movement.
	UINT8 PrefetchMip : MAX_MIP_COUNT_BITS;

	// True if the paging operation determined during prioritization should ignore
	// the local memory budget. This is used when paging in minimum quality mipmaps
	// to ensure that every resource has at least some low quality content.
	bool bIgnoreBudget : 1;

	// The maximum trimming pass that should be used to help resolve paging failures
	// when paging in a resource would normally go over the budget. This limitation
	// prevents resources from recursively trimming one another by preventing lower
	// priority resources from trimming higher priority ones.
	ResourceTrimPass TrimLimit;

	//
	// Device dependent state information.
	//
	ResourceDeviceState* pDeviceState;
};
