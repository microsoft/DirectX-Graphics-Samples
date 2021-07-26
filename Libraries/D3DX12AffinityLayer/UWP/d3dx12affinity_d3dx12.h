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

/**
 * This header provides "affinity" versions of some structs provided by <d3dx12.h>
 * Any struct that includes "ID3D12____" pointers needs to have an affinity version
 * that has a "CD3DX12____" pointer instead. AFAIK I haven't modified any of these
 * structs so if there are any differences vs. <d3dx12.h> it is probably due to
 * API changes.
 *
 * Very few if any of these have been used extensively, so they are relatively
 * untested.
 */

#pragma once

#include "d3dx12.h"
#include "d3dx12affinity.h"

//------------------------------------------------------------------------------------------------
struct CD3DX12_AFFINITY_RESOURCE_BARRIER : public D3DX12_AFFINITY_RESOURCE_BARRIER
{
    CD3DX12_AFFINITY_RESOURCE_BARRIER()
    {}
    explicit CD3DX12_AFFINITY_RESOURCE_BARRIER(const D3DX12_AFFINITY_RESOURCE_BARRIER &o) :
        D3DX12_AFFINITY_RESOURCE_BARRIER(o)
    {}
    static inline CD3DX12_AFFINITY_RESOURCE_BARRIER Transition(
        _In_ CD3DX12AffinityResource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
    {
        CD3DX12_AFFINITY_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3DX12_AFFINITY_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        result.Flags = flags;
        barrier.Transition.pResource = pResource;
        barrier.Transition.StateBefore = stateBefore;
        barrier.Transition.StateAfter = stateAfter;
        barrier.Transition.Subresource = subresource;
        return result;
    }
    static inline CD3DX12_AFFINITY_RESOURCE_BARRIER Aliasing(
        _In_ CD3DX12AffinityResource* pResourceBefore,
        _In_ CD3DX12AffinityResource* pResourceAfter)
    {
        CD3DX12_AFFINITY_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3DX12_AFFINITY_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barrier.Aliasing.pResourceBefore = pResourceBefore;
        barrier.Aliasing.pResourceAfter = pResourceAfter;
        return result;
    }
    static inline CD3DX12_AFFINITY_RESOURCE_BARRIER UAV(
        _In_ CD3DX12AffinityResource* pResource)
    {
        CD3DX12_AFFINITY_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3DX12_AFFINITY_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = pResource;
        return result;
    }
    operator const D3DX12_AFFINITY_RESOURCE_BARRIER&() const { return *this; }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION : public D3DX12_AFFINITY_TEXTURE_COPY_LOCATION
{
    CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION()
    {}
    explicit CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION(const D3DX12_AFFINITY_TEXTURE_COPY_LOCATION &o) :
        D3DX12_AFFINITY_TEXTURE_COPY_LOCATION(o)
    {}
    CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION(CD3DX12AffinityResource* pRes) { pResource = pRes; }
    CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION(CD3DX12AffinityResource* pRes, D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& Footprint)
    {
        pResource = pRes;
        Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        PlacedFootprint = Footprint;
    }
    CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION(CD3DX12AffinityResource* pRes, UINT Sub)
    {
        pResource = pRes;
        Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        SubresourceIndex = Sub;
    }
};

//------------------------------------------------------------------------------------------------
inline UINT8 D3D12GetFormatPlaneCount(
    _In_ CD3DX12AffinityDevice* pDevice,
    DXGI_FORMAT Format
    )
{
    D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = { Format };
    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo), 0)))
    {
        return 0;
    }
    return formatInfo.PlaneCount;
}

//------------------------------------------------------------------------------------------------
struct CD3DX12_AFFINITY_RESOURCE_DESC : public D3D12_RESOURCE_DESC
{
    CD3DX12_AFFINITY_RESOURCE_DESC()
    {}
    explicit CD3DX12_AFFINITY_RESOURCE_DESC(const D3D12_RESOURCE_DESC& o) :
        D3D12_RESOURCE_DESC(o)
    {}
    CD3DX12_AFFINITY_RESOURCE_DESC(
        D3D12_RESOURCE_DIMENSION dimension,
        UINT64 alignment,
        UINT64 width,
        UINT height,
        UINT16 depthOrArraySize,
        UINT16 mipLevels,
        DXGI_FORMAT format,
        UINT sampleCount,
        UINT sampleQuality,
        D3D12_TEXTURE_LAYOUT layout,
        D3D12_RESOURCE_FLAGS flags)
    {
        Dimension = dimension;
        Alignment = alignment;
        Width = width;
        Height = height;
        DepthOrArraySize = depthOrArraySize;
        MipLevels = mipLevels;
        Format = format;
        SampleDesc.Count = sampleCount;
        SampleDesc.Quality = sampleQuality;
        Layout = layout;
        Flags = flags;
    }
    static inline CD3DX12_AFFINITY_RESOURCE_DESC Buffer(
        const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
    {
        return CD3DX12_AFFINITY_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, resAllocInfo.Alignment, resAllocInfo.SizeInBytes,
            1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
    }
    static inline CD3DX12_AFFINITY_RESOURCE_DESC Buffer(
        UINT64 width,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        UINT64 alignment = 0)
    {
        return CD3DX12_AFFINITY_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1,
            DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
    }
    static inline CD3DX12_AFFINITY_RESOURCE_DESC Tex1D(
        DXGI_FORMAT format,
        UINT64 width,
        UINT16 arraySize = 1,
        UINT16 mipLevels = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0)
    {
        return CD3DX12_AFFINITY_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE1D, alignment, width, 1, arraySize,
            mipLevels, format, 1, 0, layout, flags);
    }
    static inline CD3DX12_AFFINITY_RESOURCE_DESC Tex2D(
        DXGI_FORMAT format,
        UINT64 width,
        UINT height,
        UINT16 arraySize = 1,
        UINT16 mipLevels = 0,
        UINT sampleCount = 1,
        UINT sampleQuality = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0)
    {
        return CD3DX12_AFFINITY_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height, arraySize,
            mipLevels, format, sampleCount, sampleQuality, layout, flags);
    }
    static inline CD3DX12_AFFINITY_RESOURCE_DESC Tex3D(
        DXGI_FORMAT format,
        UINT64 width,
        UINT height,
        UINT16 depth,
        UINT16 mipLevels = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0)
    {
        return CD3DX12_AFFINITY_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE3D, alignment, width, height, depth,
            mipLevels, format, 1, 0, layout, flags);
    }
    inline UINT16 Depth() const
    {
        return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
    }
    inline UINT16 ArraySize() const
    {
        return (Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
    }
    inline UINT8 PlaneCount(_In_ CD3DX12AffinityDevice* pDevice) const
    {
        return D3D12GetFormatPlaneCount(pDevice, Format);
    }
    inline UINT Subresources(_In_ CD3DX12AffinityDevice* pDevice) const
    {
        return MipLevels * ArraySize() * PlaneCount(pDevice);
    }
    inline UINT CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice)
    {
        return D3D12CalcSubresource(MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize());
    }
    operator const D3D12_RESOURCE_DESC&() const { return *this; }
};
inline bool operator==(const CD3DX12_AFFINITY_RESOURCE_DESC& l, const CD3DX12_AFFINITY_RESOURCE_DESC& r)
{
    return l.Dimension == r.Dimension &&
        l.Alignment == r.Alignment &&
        l.Width == r.Width &&
        l.Height == r.Height &&
        l.DepthOrArraySize == r.DepthOrArraySize &&
        l.MipLevels == r.MipLevels &&
        l.Format == r.Format &&
        l.SampleDesc.Count == r.SampleDesc.Count &&
        l.SampleDesc.Quality == r.SampleDesc.Quality &&
        l.Layout == r.Layout &&
        l.Flags == r.Flags;
}
inline bool operator!=(const CD3DX12_AFFINITY_RESOURCE_DESC& l, const CD3DX12_AFFINITY_RESOURCE_DESC& r)
{
    return !(l == r);
}

//------------------------------------------------------------------------------------------------
// Returns required size of a buffer to be used for data upload
inline UINT64 GetRequiredIntermediateSize(
    _In_ CD3DX12AffinityResource* pDestinationResource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources)
{
    D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc(/* DeviceIndex */ 0);
    UINT64 RequiredSize = 0;

    CD3DX12AffinityDevice* pDevice;
    pDestinationResource->GetDevice(&pDevice);
    pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &RequiredSize);

    return RequiredSize;
}

//------------------------------------------------------------------------------------------------
// All arrays must be populated (e.g. by calling GetCopyableFootprints)
inline UINT64 UpdateSubresources(
    _In_ CD3DX12AffinityGraphicsCommandList* pCmdList,
    _In_ CD3DX12AffinityResource* pDestinationResource,
    _In_ CD3DX12AffinityResource* pIntermediate,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
    UINT64 RequiredSize,
    _In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
    _In_reads_(NumSubresources) const UINT* pNumRows,
    _In_reads_(NumSubresources) const UINT64* pRowSizesInBytes,
    _In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData)
{
    // Minor validation
    D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc(/* DeviceIndex */ 0);
    D3D12_RESOURCE_DESC DestinationDesc = pDestinationResource->GetDesc(/* DeviceIndex */ 0);
    if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
        IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
        RequiredSize >(SIZE_T) - 1 ||
        (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
            (FirstSubresource != 0 || NumSubresources != 1)))
    {
        return 0;
    }

    BYTE* pData;
    HRESULT hr = pIntermediate->Map(0, NULL, reinterpret_cast<void**>(&pData));
    if (FAILED(hr))
    {
        return 0;
    }

    for (UINT i = 0; i < NumSubresources; ++i)
    {
        if (pRowSizesInBytes[i] > (SIZE_T)-1) return 0;
        D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
        MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
    }
    pIntermediate->Unmap(0, NULL);

    if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        CD3DX12_BOX SrcBox(UINT(pLayouts[0].Offset), UINT(pLayouts[0].Offset + pLayouts[0].Footprint.Width));
        pCmdList->CopyBufferRegion(
            pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
    }
    else
    {
        for (UINT i = 0; i < NumSubresources; ++i)
        {
            CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION Dst(pDestinationResource, i + FirstSubresource);
            CD3DX12_AFFINITY_TEXTURE_COPY_LOCATION Src(pIntermediate, pLayouts[i]);
            pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        }
    }
    return RequiredSize;
}

//------------------------------------------------------------------------------------------------
// Heap-allocating UpdateSubresources implementation
inline UINT64 UpdateSubresources(
    _In_ CD3DX12AffinityGraphicsCommandList* pCmdList,
    _In_ CD3DX12AffinityResource* pDestinationResource,
    _In_ CD3DX12AffinityResource* pIntermediate,
    UINT64 IntermediateOffset,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
    _In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData)
{
    UINT64 RequiredSize = 0;
    UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * NumSubresources;
    if (MemToAlloc > SIZE_MAX)
    {
        return 0;
    }
    void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
    if (pMem == NULL)
    {
        return 0;
    }
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
    UINT64* pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + NumSubresources);
    UINT* pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + NumSubresources);

    D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc(/* DeviceIndex */ 0);
    CD3DX12AffinityDevice* pDevice;
    pDestinationResource->GetDevice(&pDevice);
    pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);

    UINT64 Result = UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, pSrcData);
    HeapFree(GetProcessHeap(), 0, pMem);
    return Result;
}

//------------------------------------------------------------------------------------------------
// Stack-allocating UpdateSubresources implementation
template <UINT MaxSubresources>
inline UINT64 UpdateSubresources(
    _In_ CD3DX12AffinityGraphicsCommandList* pCmdList,
    _In_ CD3DX12AffinityResource* pDestinationResource,
    _In_ CD3DX12AffinityResource* pIntermediate,
    UINT64 IntermediateOffset,
    _In_range_(0, MaxSubresources) UINT FirstSubresource,
    _In_range_(1, MaxSubresources - FirstSubresource) UINT NumSubresources,
    _In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData)
{
    UINT64 RequiredSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[MaxSubresources];
    UINT NumRows[MaxSubresources];
    UINT64 RowSizesInBytes[MaxSubresources];

    D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc(/* DeviceIndex */ 0);
    CD3DX12AffinityDevice* pDevice;
    pDestinationResource->GetDevice(&pDevice);
    pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, Layouts, NumRows, RowSizesInBytes, &RequiredSize);
    pDevice->Release();

    return UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, Layouts, NumRows, RowSizesInBytes, pSrcData);
}

//------------------------------------------------------------------------------------------------
inline CD3DX12AffinityCommandList* const* CommandListCast(CD3DX12AffinityGraphicsCommandList* const* pp)
{
    // This cast is useful for passing strongly typed command list pointers into
    // ExecuteCommandLists.
    // This cast is valid as long as the const-ness is respected. D3D12 APIs do
    // respect the const-ness of their arguments.
    return reinterpret_cast<CD3DX12AffinityCommandList* const*>(pp);
}
