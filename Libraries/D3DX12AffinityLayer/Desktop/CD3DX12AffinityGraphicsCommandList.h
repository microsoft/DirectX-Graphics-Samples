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

#include "Utils.h"
#include "CD3DX12AffinityCommandList.h"
#include "CD3DX12AffinityQueryHeap.h"
#include "CD3DX12AffinityDevice.h"

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityGraphicsCommandList : public CD3DX12AffinityCommandList
{
public:
    virtual void STDMETHODCALLTYPE SetAffinity(
        _In_  UINT AffinityMask);

    D3D12_COMMAND_LIST_TYPE GetType();

    HRESULT Close();

    HRESULT Reset(
        _In_  CD3DX12AffinityCommandAllocator* pAllocator,
        _In_opt_  CD3DX12AffinityPipelineState* pInitialState);

    void ClearState(
        _In_opt_  CD3DX12AffinityPipelineState* pPipelineState);

    void DrawInstanced(
        _In_  UINT VertexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartVertexLocation,
        _In_  UINT StartInstanceLocation);

    void DrawIndexedInstanced(
        _In_  UINT IndexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartIndexLocation,
        _In_  INT BaseVertexLocation,
        _In_  UINT StartInstanceLocation);

    void Dispatch(
        _In_  UINT ThreadGroupCountX,
        _In_  UINT ThreadGroupCountY,
        _In_  UINT ThreadGroupCountZ);

    void CopyBufferRegion(
        _In_  CD3DX12AffinityResource* pDstBuffer,
        UINT64 DstOffset,
        _In_  CD3DX12AffinityResource* pSrcBuffer,
        UINT64 SrcOffset,
        UINT64 NumBytes);

    void CopyTextureRegion(
        _In_  const D3DX12_AFFINITY_TEXTURE_COPY_LOCATION* pDst,
        UINT DstX,
        UINT DstY,
        UINT DstZ,
        _In_  const D3DX12_AFFINITY_TEXTURE_COPY_LOCATION* pSrc,
        _In_opt_  const D3D12_BOX* pSrcBox);

    void CopyResource(
        _In_  CD3DX12AffinityResource* pDstResource,
        _In_  CD3DX12AffinityResource* pSrcResource);

    void CopyTiles(
        _In_  CD3DX12AffinityResource* pTiledResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE* pTileRegionStartCoordinate,
        _In_  const D3D12_TILE_REGION_SIZE* pTileRegionSize,
        _In_  CD3DX12AffinityResource* pBuffer,
        UINT64 BufferStartOffsetInBytes,
        D3D12_TILE_COPY_FLAGS Flags);

    void ResolveSubresource(
        _In_  CD3DX12AffinityResource* pDstResource,
        _In_  UINT DstSubresource,
        _In_  CD3DX12AffinityResource* pSrcResource,
        _In_  UINT SrcSubresource,
        _In_  DXGI_FORMAT Format);

    void IASetPrimitiveTopology(
        _In_  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology);

    void RSSetViewports(
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
        _In_reads_(NumViewports)  const D3D12_VIEWPORT* pViewports);

    void RSSetScissorRects(
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects);

    void OMSetBlendFactor(
        _In_opt_  const FLOAT BlendFactor[4]);

    void OMSetStencilRef(
        _In_  UINT StencilRef);

    void SetPipelineState(
        _In_  CD3DX12AffinityPipelineState* pPipelineState);

    void ResourceBarrier(
        _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3DX12_AFFINITY_RESOURCE_BARRIER* pBarriers);

    void ExecuteBundle(
        _In_  CD3DX12AffinityGraphicsCommandList* pCommandList);

    void SetDescriptorHeaps(
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  CD3DX12AffinityDescriptorHeap** ppDescriptorHeaps);

    void SetComputeRootSignature(
        _In_  CD3DX12AffinityRootSignature* pRootSignature);

    void SetGraphicsRootSignature(
        _In_  CD3DX12AffinityRootSignature* pRootSignature);

    void SetComputeRootDescriptorTable(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);

    void SetGraphicsRootDescriptorTable(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);

    void SetComputeRoot32BitConstant(
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues);

    void SetGraphicsRoot32BitConstant(
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues);

    void SetComputeRoot32BitConstants(
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet* sizeof(UINT))  const void* pSrcData,
        _In_  UINT DestOffsetIn32BitValues);

    void SetGraphicsRoot32BitConstants(
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet* sizeof(UINT))  const void* pSrcData,
        _In_  UINT DestOffsetIn32BitValues);

    void SetComputeRootConstantBufferView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void SetGraphicsRootConstantBufferView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void SetComputeRootShaderResourceView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void SetGraphicsRootShaderResourceView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void SetComputeRootUnorderedAccessView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void SetGraphicsRootUnorderedAccessView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void IASetIndexBuffer(
        _In_opt_  const D3D12_INDEX_BUFFER_VIEW* pView);

    void IASetVertexBuffers(
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_VERTEX_BUFFER_VIEW* pViews);

    void SOSetTargets(
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews);

    void OMSetRenderTargets(
        _In_  UINT NumRenderTargetDescriptors,
        _In_  const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        _In_  BOOL RTsSingleHandleToDescriptorRange,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);

    void ClearDepthStencilView(
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
        _In_  D3D12_CLEAR_FLAGS ClearFlags,
        _In_  FLOAT Depth,
        _In_  UINT8 Stencil,
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects);

    void ClearRenderTargetView(
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
        _In_  const FLOAT ColorRGBA[4],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects);

    void ClearUnorderedAccessViewUint(
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  CD3DX12AffinityResource* pResource,
        _In_  const UINT Values[4],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects);

    void ClearUnorderedAccessViewFloat(
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  CD3DX12AffinityResource* pResource,
        _In_  const FLOAT Values[4],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects);

    void DiscardResource(
        _In_  CD3DX12AffinityResource* pResource,
        _In_opt_  const D3D12_DISCARD_REGION* pRegion);

    void BeginQuery(
        _In_  CD3DX12AffinityQueryHeap* pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index);

    void EndQuery(
        _In_  CD3DX12AffinityQueryHeap* pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index);

    void ResolveQueryData(
        _In_  CD3DX12AffinityQueryHeap* pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  CD3DX12AffinityResource* pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset);

    void SetPredication(
        _In_opt_  CD3DX12AffinityResource* pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation);

    void SetMarker(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size);

    void BeginEvent(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size);

    void EndEvent(void);

    void ExecuteIndirect(
        _In_  CD3DX12AffinityCommandSignature* pCommandSignature,
        _In_  UINT MaxCommandCount,
        _In_  CD3DX12AffinityResource* pArgumentBuffer,
        _In_  UINT64 ArgumentBufferOffset,
        _In_opt_  CD3DX12AffinityResource* pCountBuffer,
        _In_  UINT64 CountBufferOffset);

    void BroadcastResource(CD3DX12AffinityResource* pResource, UINT NodeIndex, UINT TargetNodeMask);

    CD3DX12AffinityGraphicsCommandList(CD3DX12AffinityDevice* device, ID3D12GraphicsCommandList** graphicsCommandLists, UINT Count, bool UseDeviceActiveMaskOnReset);

    ID3D12GraphicsCommandList* GetChildObject(UINT AffinityIndex);
    UINT GetActiveAffinityMask();

private:
    ID3D12GraphicsCommandList* mGraphicsCommandLists[D3DX12_MAX_ACTIVE_NODES];
    UINT mAccumulatedAffinityMask;
    bool mUseDeviceActiveMaskOnReset;
    std::vector<D3D12_RESOURCE_BARRIER> mCachedResourceBarriers;
    std::vector<ID3D12DescriptorHeap*> mCachedDescriptorHeaps;
    std::vector<D3D12_VERTEX_BUFFER_VIEW> mCachedBufferViews;
    std::vector<D3D12_STREAM_OUTPUT_BUFFER_VIEW> mCachedStreamOutBufferViews;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> mCachedRenderTargetViews;
};
