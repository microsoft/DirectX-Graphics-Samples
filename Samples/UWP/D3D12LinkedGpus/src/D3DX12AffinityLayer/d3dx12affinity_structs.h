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
 * This header provides "affinity" versions of the structs provided by <d3d12.h>
 * Any struct that includes "ID3D12____" pointers needs to have an affinity version
 * that has a "CD3DX12____" pointer instead. The only modification made to these
 * structs is to add ToD3D12() methods that produce copies of the D3D12 structs
 * with NULL substituted for the "ID3D12____" members, so if there are any differences
 * vs. <d3dx12.h> it is probably due to API changes.
 */

#pragma once

#include "d3dx12affinity.h"

typedef struct D3DX12_AFFINITY_GRAPHICS_PIPELINE_STATE_DESC
{
    CD3DX12AffinityRootSignature* pRootSignature;
    D3D12_SHADER_BYTECODE VS;
    D3D12_SHADER_BYTECODE PS;
    D3D12_SHADER_BYTECODE DS;
    D3D12_SHADER_BYTECODE HS;
    D3D12_SHADER_BYTECODE GS;
    D3D12_STREAM_OUTPUT_DESC StreamOutput;
    D3D12_BLEND_DESC BlendState;
    UINT SampleMask;
    D3D12_RASTERIZER_DESC RasterizerState;
    D3D12_DEPTH_STENCIL_DESC DepthStencilState;
    D3D12_INPUT_LAYOUT_DESC InputLayout;
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
    UINT NumRenderTargets;
    DXGI_FORMAT RTVFormats[8];
    DXGI_FORMAT DSVFormat;
    DXGI_SAMPLE_DESC SampleDesc;
    UINT NodeMask;
    D3D12_CACHED_PIPELINE_STATE CachedPSO;
    D3D12_PIPELINE_STATE_FLAGS Flags;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC ToD3D12() const
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
        Desc.pRootSignature = nullptr;
        Desc.VS = VS;
        Desc.PS = PS;
        Desc.DS = DS;
        Desc.HS = HS;
        Desc.GS = GS;
        Desc.StreamOutput = StreamOutput;
        Desc.BlendState = BlendState;
        Desc.SampleMask = SampleMask;
        Desc.RasterizerState = RasterizerState;
        Desc.DepthStencilState = DepthStencilState;
        Desc.InputLayout = InputLayout;
        Desc.IBStripCutValue = IBStripCutValue;
        Desc.PrimitiveTopologyType = PrimitiveTopologyType;
        Desc.NumRenderTargets = NumRenderTargets;
        for (int i = 0; i < 8; ++i)
            Desc.RTVFormats[i] = RTVFormats[i];
        Desc.DSVFormat = DSVFormat;
        Desc.SampleDesc = SampleDesc;
        Desc.NodeMask = NodeMask;
        Desc.CachedPSO = CachedPSO;
        Desc.Flags = Flags;

        return Desc;
    }
} D3DX12_AFFINITY_GRAPHICS_PIPELINE_STATE_DESC;

typedef struct D3DX12_AFFINITY_COMPUTE_PIPELINE_STATE_DESC
{
    CD3DX12AffinityRootSignature* pRootSignature;
    D3D12_SHADER_BYTECODE CS;
    UINT NodeMask;
    D3D12_CACHED_PIPELINE_STATE CachedPSO;
    D3D12_PIPELINE_STATE_FLAGS Flags;

    D3D12_COMPUTE_PIPELINE_STATE_DESC ToD3D12() const
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC Desc;
        Desc.pRootSignature = nullptr;
        Desc.CS = CS;
        Desc.NodeMask = NodeMask;
        Desc.CachedPSO = CachedPSO;
        Desc.Flags = Flags;

        return Desc;
    }
} D3DX12_AFFINITY_COMPUTE_PIPELINE_STATE_DESC;

typedef struct D3DX12_AFFINITY_RESOURCE_TRANSITION_BARRIER
{
    CD3DX12AffinityResource* pResource;
    UINT Subresource;
    D3D12_RESOURCE_STATES StateBefore;
    D3D12_RESOURCE_STATES StateAfter;

    D3D12_RESOURCE_TRANSITION_BARRIER ToD3D12() const
    {
        D3D12_RESOURCE_TRANSITION_BARRIER Desc;
        Desc.pResource = nullptr;
        Desc.Subresource = Subresource;
        Desc.StateBefore = StateBefore;
        Desc.StateAfter = StateAfter;

        return Desc;
    }
} D3DX12_AFFINITY_RESOURCE_TRANSITION_BARRIER;

typedef struct D3DX12_AFFINITY_RESOURCE_ALIASING_BARRIER
{
    CD3DX12AffinityResource* pResourceBefore;
    CD3DX12AffinityResource* pResourceAfter;

    D3D12_RESOURCE_ALIASING_BARRIER ToD3D12() const
    {
        D3D12_RESOURCE_ALIASING_BARRIER Desc;
        Desc.pResourceBefore = nullptr;
        Desc.pResourceAfter = nullptr;

        return Desc;
    }
} D3DX12_AFFINITY_RESOURCE_ALIASING_BARRIER;

typedef struct D3DX12_AFFINITY_RESOURCE_UAV_BARRIER
{
    CD3DX12AffinityResource* pResource;

    D3D12_RESOURCE_UAV_BARRIER ToD3D12() const
    {
        D3D12_RESOURCE_UAV_BARRIER Desc;
        Desc.pResource = nullptr;

        return Desc;
    }
} D3DX12_AFFINITY_RESOURCE_UAV_BARRIER;

typedef struct D3DX12_AFFINITY_RESOURCE_BARRIER
{
    D3D12_RESOURCE_BARRIER_TYPE Type;
    D3D12_RESOURCE_BARRIER_FLAGS Flags;
    union
    {
        D3DX12_AFFINITY_RESOURCE_TRANSITION_BARRIER Transition;
        D3DX12_AFFINITY_RESOURCE_ALIASING_BARRIER Aliasing;
        D3DX12_AFFINITY_RESOURCE_UAV_BARRIER UAV;
    };

    D3D12_RESOURCE_BARRIER ToD3D12() const
    {
        D3D12_RESOURCE_BARRIER Desc;
        Desc.Type = Type;
        Desc.Flags = Flags;
        switch (Type)
        {
        case D3D12_RESOURCE_BARRIER_TYPE_TRANSITION:
            Desc.Transition = Transition.ToD3D12();
            break;
        case D3D12_RESOURCE_BARRIER_TYPE_ALIASING:
            Desc.Aliasing = Aliasing.ToD3D12();
            break;
        case D3D12_RESOURCE_BARRIER_TYPE_UAV:
            Desc.UAV = UAV.ToD3D12();
            break;
        }

        return Desc;
    }
} D3DX12_AFFINITY_RESOURCE_BARRIER;

typedef struct D3DX12_AFFINITY_TEXTURE_COPY_LOCATION
{
    CD3DX12AffinityResource* pResource;
    D3D12_TEXTURE_COPY_TYPE Type;
    union
    {
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
        UINT SubresourceIndex;
    };

    D3D12_TEXTURE_COPY_LOCATION ToD3D12() const
    {
        D3D12_TEXTURE_COPY_LOCATION Desc;
        Desc.pResource = nullptr;
        switch (Type)
        {
        case D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT:
            Desc.PlacedFootprint = PlacedFootprint;
            break;
        case D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX:
            Desc.SubresourceIndex = SubresourceIndex;
            break;
        }
        Desc.Type = Type;

        return Desc;
    }
} D3DX12_AFFINITY_TEXTURE_COPY_LOCATION;
