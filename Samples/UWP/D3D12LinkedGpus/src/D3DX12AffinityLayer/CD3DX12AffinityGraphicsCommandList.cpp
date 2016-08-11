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

#include "d3dx12affinity.h"
#include "Utils.h"

void STDMETHODCALLTYPE CD3DX12AffinityGraphicsCommandList::SetAffinity(UINT AffinityMask)
{
    CD3DX12AffinityObject::SetAffinity(AffinityMask);
    mAccumulatedAffinityMask |= AffinityMask;
}

D3D12_COMMAND_LIST_TYPE CD3DX12AffinityGraphicsCommandList::GetType()
{
    return mGraphicsCommandLists[0]->GetType();
}

HRESULT CD3DX12AffinityGraphicsCommandList::Close()
{
#if ALWAYS_RESET_ALL_COMMAND_LISTS
    for (UINT i = 0; i < GetNodeCount(); ++i)
    {
        ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
#else
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
#endif
            HRESULT const hr = List->Close();

            if (S_OK != hr)
            {
                return hr;
            }
        }
#ifdef ALWAYS_RESET_ALL_COMMAND_LISTS
#else
    }
#endif

    return S_OK;
}

HRESULT CD3DX12AffinityGraphicsCommandList::Reset(
    CD3DX12AffinityCommandAllocator* pAllocator,
    CD3DX12AffinityPipelineState* pInitialState)
{
    if (mUseDeviceActiveMaskOnReset)
    {
        mAccumulatedAffinityMask = 0;
        SetAffinity(1 << GetActiveNodeIndex());
    }

#if ALWAYS_RESET_ALL_COMMAND_LISTS
    for (UINT i = 0; i < GetNodeCount(); ++i)
    {
        ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
#else
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
#endif

            HRESULT const hr = List->Reset(pAllocator->GetChildObject(i),
                pInitialState ? pInitialState->mPipelineStates[i] : nullptr);

            if (S_OK != hr)
            {
                return hr;
            }
        }
    }

    return S_OK;
}

void CD3DX12AffinityGraphicsCommandList::ClearState(
    CD3DX12AffinityPipelineState* pPipelineState)
{
    CD3DX12AffinityPipelineState* PipelineState = static_cast<CD3DX12AffinityPipelineState*>(pPipelineState);

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->ClearState(PipelineState->mPipelineStates[i]);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::DrawInstanced(
    UINT VertexCountPerInstance,
    UINT InstanceCount,
    UINT StartVertexLocation,
    UINT StartInstanceLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::Dispatch(
    UINT ThreadGroupCountX,
    UINT ThreadGroupCountY,
    UINT ThreadGroupCountZ)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::CopyBufferRegion(
    CD3DX12AffinityResource* pDstBuffer,
    UINT64 DstOffset,
    CD3DX12AffinityResource* pSrcBuffer,
    UINT64 SrcOffset,
    UINT64 NumBytes)
{
    CD3DX12AffinityResource* DstBuffer = static_cast<CD3DX12AffinityResource*>(pDstBuffer);
    CD3DX12AffinityResource* SrcBuffer = static_cast<CD3DX12AffinityResource*>(pSrcBuffer);

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->CopyBufferRegion(DstBuffer->mResources[i], DstOffset, SrcBuffer->mResources[i], SrcOffset, NumBytes);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::CopyTextureRegion(
    const D3DX12_AFFINITY_TEXTURE_COPY_LOCATION* pDst,
    UINT DstX,
    UINT DstY,
    UINT DstZ,
    const D3DX12_AFFINITY_TEXTURE_COPY_LOCATION* pSrc,
    const D3D12_BOX* pSrcBox)
{
    CD3DX12AffinityResource* DstTexture = static_cast<CD3DX12AffinityResource*>(pDst->pResource);
    CD3DX12AffinityResource* SrcTexture = static_cast<CD3DX12AffinityResource*>(pSrc->pResource);

    D3D12_TEXTURE_COPY_LOCATION Dst = pDst->ToD3D12();
    D3D12_TEXTURE_COPY_LOCATION Src = pSrc->ToD3D12();

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

            Dst.pResource = DstTexture->mResources[i];
            Src.pResource = SrcTexture->mResources[i];
            List->CopyTextureRegion(&Dst, DstX, DstY, DstZ, &Src, pSrcBox);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::CopyResource(
    CD3DX12AffinityResource* pDstResource,
    CD3DX12AffinityResource* pSrcResource)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->CopyResource(pDstResource->mResources[i], pSrcResource->mResources[i]);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::CopyTiles(
    CD3DX12AffinityResource* pTiledResource,
    const D3D12_TILED_RESOURCE_COORDINATE* pTileRegionStartCoordinate,
    const D3D12_TILE_REGION_SIZE* pTileRegionSize,
    CD3DX12AffinityResource* pBuffer,
    UINT64 BufferStartOffsetInBytes,
    D3D12_TILE_COPY_FLAGS Flags)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->CopyTiles(
                pTiledResource->mResources[i],
                pTileRegionStartCoordinate,
                pTileRegionSize,
                pBuffer->mResources[i],
                BufferStartOffsetInBytes,
                Flags);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ResolveSubresource(
    CD3DX12AffinityResource* pDstResource,
    UINT DstSubresource,
    CD3DX12AffinityResource* pSrcResource,
    UINT SrcSubresource,
    DXGI_FORMAT Format)
{

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->ResolveSubresource(pDstResource->mResources[i], DstSubresource, pSrcResource->mResources[i], SrcSubresource, Format);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::IASetPrimitiveTopology(
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->IASetPrimitiveTopology(PrimitiveTopology);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::RSSetViewports(
    UINT NumViewports,
    const D3D12_VIEWPORT* pViewports)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->RSSetViewports(NumViewports, pViewports);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::RSSetScissorRects(
    UINT NumRects,
    const D3D12_RECT* pRects)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->RSSetScissorRects(NumRects, pRects);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::OMSetBlendFactor(
    const FLOAT BlendFactor[4])
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->OMSetBlendFactor(BlendFactor);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::OMSetStencilRef(
    UINT StencilRef)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            List->OMSetStencilRef(StencilRef);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ResourceBarrier(
    UINT NumBarriers,
    const D3DX12_AFFINITY_RESOURCE_BARRIER* pBarriers)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

            mCachedResourceBarriers.resize(NumBarriers);
            for (UINT b = 0; b < NumBarriers; ++b)
            {
                D3D12_RESOURCE_BARRIER Use = pBarriers[b].ToD3D12();

                switch (pBarriers[b].Type)
                {
                case D3D12_RESOURCE_BARRIER_TYPE_TRANSITION:
                {
                    if (pBarriers[b].Transition.pResource)
                    {
                        Use.Transition.pResource = pBarriers[b].Transition.pResource->mResources[i];
                    }
                    break;
                }
                case D3D12_RESOURCE_BARRIER_TYPE_ALIASING:
                {
                    if (pBarriers[b].Aliasing.pResourceAfter)
                    {
                        Use.Aliasing.pResourceAfter = pBarriers[b].Aliasing.pResourceAfter->mResources[i];
                    }
                    if (pBarriers[b].Aliasing.pResourceBefore)
                    {
                        Use.Aliasing.pResourceBefore = pBarriers[b].Aliasing.pResourceBefore->mResources[i];
                    }
                    break;
                }
                case D3D12_RESOURCE_BARRIER_TYPE_UAV:
                {
                    if (pBarriers[b].UAV.pResource)
                    {
                        Use.UAV.pResource = pBarriers[b].UAV.pResource->mResources[i];
                    }
                    break;
                }
                }

                mCachedResourceBarriers[b] = Use;
            }

            List->ResourceBarrier(NumBarriers, mCachedResourceBarriers.data());
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ExecuteBundle(
    CD3DX12AffinityGraphicsCommandList* pCommandList)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

            List->ExecuteBundle(pCommandList->mGraphicsCommandLists[i]);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetDescriptorHeaps(
    UINT NumDescriptorHeaps,
    CD3DX12AffinityDescriptorHeap** ppDescriptorHeaps)
{
    mCachedDescriptorHeaps.resize(NumDescriptorHeaps);
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            for (UINT h = 0; h < NumDescriptorHeaps; ++h)
            {
                mCachedDescriptorHeaps[h] = static_cast<CD3DX12AffinityDescriptorHeap*>(ppDescriptorHeaps[h])->GetChildObject(i);
            }

            List->SetDescriptorHeaps(NumDescriptorHeaps, mCachedDescriptorHeaps.data());
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRootSignature(
    CD3DX12AffinityRootSignature* pRootSignature)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetComputeRootSignature(pRootSignature->mRootSignatures[i]);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRootSignature(
    CD3DX12AffinityRootSignature* pRootSignature)
{
    CD3DX12AffinityRootSignature* AffinityRootSignature = static_cast<CD3DX12AffinityRootSignature*>(pRootSignature);

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetGraphicsRootSignature(AffinityRootSignature->mRootSignatures[i]);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRoot32BitConstant(
    UINT RootParameterIndex,
    UINT SrcData,
    UINT DestOffsetIn32BitValues)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRoot32BitConstant(
    UINT RootParameterIndex,
    UINT SrcData,
    UINT DestOffsetIn32BitValues)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRoot32BitConstants(
    UINT RootParameterIndex,
    UINT Num32BitValuesToSet,
    const void* pSrcData,
    UINT DestOffsetIn32BitValues)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
           
            List->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRoot32BitConstants(
    UINT RootParameterIndex,
    UINT Num32BitValuesToSet,
    const void* pSrcData,
    UINT DestOffsetIn32BitValues)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRootConstantBufferView(
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetComputeRootConstantBufferView(RootParameterIndex, GetParentDevice()->GetGPUVirtualAddress(BufferLocation, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRootConstantBufferView(
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetGraphicsRootConstantBufferView(RootParameterIndex, GetParentDevice()->GetGPUVirtualAddress(BufferLocation, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRootShaderResourceView(
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetComputeRootShaderResourceView(RootParameterIndex, GetParentDevice()->GetGPUVirtualAddress(BufferLocation, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRootShaderResourceView(
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetGraphicsRootShaderResourceView(RootParameterIndex, GetParentDevice()->GetGPUVirtualAddress(BufferLocation, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRootUnorderedAccessView(
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetComputeRootUnorderedAccessView(RootParameterIndex, GetParentDevice()->GetGPUVirtualAddress(BufferLocation, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRootUnorderedAccessView(
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetGraphicsRootUnorderedAccessView(RootParameterIndex, GetParentDevice()->GetGPUVirtualAddress(BufferLocation, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::IASetVertexBuffers(
    UINT StartSlot,
    UINT NumViews,
    const D3D12_VERTEX_BUFFER_VIEW* pViews)
{
    mCachedBufferViews.resize(NumViews);

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            for (UINT v = 0; v < NumViews; ++v)
            {
                mCachedBufferViews[v] = pViews[v];
                mCachedBufferViews[v].BufferLocation = GetParentDevice()->GetGPUVirtualAddress(pViews[v].BufferLocation, i);
            }

            List->IASetVertexBuffers(StartSlot, NumViews, mCachedBufferViews.data());
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SOSetTargets(
    UINT StartSlot,
    UINT NumViews,
    const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews)
{
    mCachedStreamOutBufferViews.resize(NumViews);

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            for (UINT v = 0; v < NumViews; ++v)
            {
                mCachedStreamOutBufferViews[v] = pViews[v];
                mCachedStreamOutBufferViews[v].BufferLocation = GetParentDevice()->GetGPUVirtualAddress(pViews[v].BufferLocation, i);
                mCachedStreamOutBufferViews[v].BufferFilledSizeLocation = GetParentDevice()->GetGPUVirtualAddress(pViews[v].BufferFilledSizeLocation, i);
            }

            List->SOSetTargets(StartSlot, NumViews, mCachedStreamOutBufferViews.data());
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::OMSetRenderTargets(
    UINT NumRenderTargetDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
    BOOL RTsSingleHandleToDescriptorRange,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            mCachedRenderTargetViews.resize(NumRenderTargetDescriptors);
            for (UINT r = 0; r < NumRenderTargetDescriptors; ++r)
            {
                mCachedRenderTargetViews[r] = GetParentDevice()->GetCPUHeapPointer(pRenderTargetDescriptors[r], i);
            }

            if (pDepthStencilDescriptor)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE ActualDepthStencilDescriptor = GetParentDevice()->GetCPUHeapPointer(*pDepthStencilDescriptor, i);
                List->OMSetRenderTargets(NumRenderTargetDescriptors, mCachedRenderTargetViews.data(), RTsSingleHandleToDescriptorRange, &ActualDepthStencilDescriptor);
            }
            else
            {
                List->OMSetRenderTargets(NumRenderTargetDescriptors, mCachedRenderTargetViews.data(), RTsSingleHandleToDescriptorRange, nullptr);
            }
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ClearDepthStencilView(
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
    D3D12_CLEAR_FLAGS ClearFlags,
    FLOAT Depth,
    UINT8 Stencil,
    UINT NumRects,
    const D3D12_RECT* pRects)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {

            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->ClearDepthStencilView(GetParentDevice()->GetCPUHeapPointer(DepthStencilView, i), ClearFlags, Depth, Stencil, NumRects, pRects);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ClearRenderTargetView(
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
    const FLOAT ColorRGBA[4],
    UINT NumRects,
    const D3D12_RECT* pRects)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

#ifdef D3DX12_DEBUG_CLEAR_WHITE
            FLOAT White[4] = { 1, 1, 1, 1 };
            List->ClearRenderTargetView(GetParentDevice()->GetCPUHeapPointer(RenderTargetView, i), White, NumRects, pRects);
#else
            List->ClearRenderTargetView(GetParentDevice()->GetCPUHeapPointer(RenderTargetView, i), ColorRGBA, NumRects, pRects);
#endif
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ClearUnorderedAccessViewUint(
    D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
    D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
    CD3DX12AffinityResource* pResource,
    const UINT Values[4],
    UINT NumRects,
    const D3D12_RECT* pRects)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->ClearUnorderedAccessViewUint(
                GetParentDevice()->GetGPUHeapPointer(ViewGPUHandleInCurrentHeap, i),
                GetParentDevice()->GetCPUHeapPointer(ViewCPUHandle, i),
                pResource->mResources[i], Values, NumRects, pRects);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ClearUnorderedAccessViewFloat(
    D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
    D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
    CD3DX12AffinityResource* pResource,
    const FLOAT Values[4],
    UINT NumRects,
    const D3D12_RECT* pRects)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->ClearUnorderedAccessViewFloat(
                GetParentDevice()->GetGPUHeapPointer(ViewGPUHandleInCurrentHeap, i),
                GetParentDevice()->GetCPUHeapPointer(ViewCPUHandle, i),
                pResource->mResources[i], Values, NumRects, pRects);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::DiscardResource(
    CD3DX12AffinityResource* pResource,
    const D3D12_DISCARD_REGION* pRegion)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->DiscardResource(
                pResource->mResources[i],
                pRegion);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::BeginQuery(
    CD3DX12AffinityQueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT Index)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            ID3D12QueryHeap* QueryHeap = pQueryHeap->mQueryHeaps[i];

            List->BeginQuery(
                QueryHeap,
                Type,
                Index);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::EndQuery(
    CD3DX12AffinityQueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT Index)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            ID3D12QueryHeap* QueryHeap = pQueryHeap->mQueryHeaps[i];

            List->EndQuery(
                QueryHeap,
                Type,
                Index);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ResolveQueryData(
    CD3DX12AffinityQueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT StartIndex,
    UINT NumQueries,
    CD3DX12AffinityResource* pDestinationBuffer,
    UINT64 AlignedDestinationBufferOffset)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            ID3D12QueryHeap* QueryHeap = pQueryHeap->mQueryHeaps[i];
            ID3D12Resource* DestinationBuffer = pDestinationBuffer->mResources[i];

            List->ResolveQueryData(
                QueryHeap,
                Type,
                StartIndex,
                NumQueries,
                DestinationBuffer,
                AlignedDestinationBufferOffset);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetPredication(
    CD3DX12AffinityResource* pBuffer,
    UINT64 AlignedBufferOffset,
    D3D12_PREDICATION_OP Operation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetPredication(
                pBuffer->mResources[i],
                AlignedBufferOffset,
                Operation);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetMarker(
    UINT Metadata,
    const void* pData,
    UINT Size)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

            List->SetMarker(
                Metadata,
                pData,
                Size);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::BeginEvent(
    UINT Metadata,
    const void* pData,
    UINT Size)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

            List->BeginEvent(
                Metadata,
                pData,
                Size);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::EndEvent(void)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];

            List->EndEvent();
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::ExecuteIndirect(
    CD3DX12AffinityCommandSignature* pCommandSignature,
    UINT MaxCommandCount,
    CD3DX12AffinityResource* pArgumentBuffer,
    UINT64 ArgumentBufferOffset,
    CD3DX12AffinityResource* pCountBuffer,
    UINT64 CountBufferOffset)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->ExecuteIndirect(
                pCommandSignature->GetChildObject(i),
                MaxCommandCount,
                pArgumentBuffer->mResources[i], ArgumentBufferOffset,
                pCountBuffer ? pCountBuffer->mResources[i] : nullptr, CountBufferOffset);
        }
    }
}

CD3DX12AffinityGraphicsCommandList::CD3DX12AffinityGraphicsCommandList(CD3DX12AffinityDevice* device, ID3D12GraphicsCommandList** graphicsCommandLists, UINT Count, bool UseDeviceActiveMaskOnReset)
    : CD3DX12AffinityCommandList(device, reinterpret_cast<ID3D12CommandList**>(graphicsCommandLists), Count)
    , mUseDeviceActiveMaskOnReset(UseDeviceActiveMaskOnReset)
    , mAccumulatedAffinityMask(0)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mGraphicsCommandLists[i] = graphicsCommandLists[i];
        }
        else
        {
            mGraphicsCommandLists[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"GraphicsCommandList";
#endif

    if (UseDeviceActiveMaskOnReset)
    {
        SetAffinity(1 << GetActiveNodeIndex());
    }
    else
    {
        mAccumulatedAffinityMask = GetNodeMask();
    }
}

void CD3DX12AffinityGraphicsCommandList::SetPipelineState(
    CD3DX12AffinityPipelineState* pPipelineState)
{
    CD3DX12AffinityPipelineState* PipelineState = static_cast<CD3DX12AffinityPipelineState*>(pPipelineState);

    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetPipelineState(PipelineState->mPipelineStates[i]);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetComputeRootDescriptorTable(
    UINT RootParameterIndex,
    D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
            
            List->SetComputeRootDescriptorTable(RootParameterIndex, GetParentDevice()->GetGPUHeapPointer(BaseDescriptor, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::SetGraphicsRootDescriptorTable(
    UINT RootParameterIndex,
    D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            mGraphicsCommandLists[i]->SetGraphicsRootDescriptorTable(
                RootParameterIndex,
                GetParentDevice()->GetGPUHeapPointer(BaseDescriptor, i));
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::IASetIndexBuffer(
    const D3D12_INDEX_BUFFER_VIEW* pView)
{
    if (pView)
    {
        D3D12_INDEX_BUFFER_VIEW View = *pView;

        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & mAffinityMask) != 0)
            {
                ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
                
                View.BufferLocation = GetParentDevice()->GetGPUVirtualAddress(pView->BufferLocation, i);
                List->IASetIndexBuffer(&View);
            }
        }
    }
    else
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & mAffinityMask) != 0)
            {
                ID3D12GraphicsCommandList* List = mGraphicsCommandLists[i];
                List->IASetIndexBuffer(nullptr);
            }
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::DrawIndexedInstanced(
    UINT IndexCountPerInstance,
    UINT InstanceCount,
    UINT StartIndexLocation,
    INT BaseVertexLocation,
    UINT StartInstanceLocation)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & mAffinityMask) != 0)
        {
            mGraphicsCommandLists[i]->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }
    }
}

void CD3DX12AffinityGraphicsCommandList::BroadcastResource(CD3DX12AffinityResource* pResource, UINT NodeIndex, UINT TargetNodeMask)
{
    // The command list affinity must match the supplied source node
    DEBUG_ASSERT(mAffinityMask == (1 << NodeIndex));

    // Copy is a push operation on the Source node commandlist to a target resource
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
    {
        if (((1 << i) & TargetNodeMask) != 0)
        {
            if (NodeIndex != i)
            {
                mGraphicsCommandLists[NodeIndex]->CopyResource(
                    pResource->GetChildObject(i),
                    pResource->GetChildObject(NodeIndex)
                    );
            }
        }
    }
}

ID3D12GraphicsCommandList* CD3DX12AffinityGraphicsCommandList::GetChildObject(UINT AffinityIndex)
{
    return mGraphicsCommandLists[AffinityIndex];
}

UINT CD3DX12AffinityGraphicsCommandList::GetActiveAffinityMask()
{
    return mAccumulatedAffinityMask;
}
