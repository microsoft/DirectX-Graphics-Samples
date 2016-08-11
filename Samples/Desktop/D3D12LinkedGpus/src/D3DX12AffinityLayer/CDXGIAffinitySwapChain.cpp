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
#include "d3dx12.h"
#include "Utils.h"

#include <string>

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetParent(
    REFIID riid,
    void** ppParent,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetParent(riid, ppParent);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::Present(
    UINT SyncInterval,
    UINT Flags,
    UINT AffinityMask)
{
    UINT IndicesCount = 0;
    HRESULT hr = S_OK;
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();

    switch (mMode)
    {
    case EAffinitySwapChainMode::MultiWindow:
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain* SwapChain = mSwapChains[i];
                hr = SwapChain->Present(SyncInterval, Flags);
                RETURN_IF_FAILED(hr);
            }
        }
        break;
    }
    case EAffinitySwapChainMode::LDA:
    {
        hr = mSwapChains[0]->Present(SyncInterval, Flags);
        break;
    }
    case EAffinitySwapChainMode::SingleWindow:
    {
        UINT const i = 0;
        DebugLog(L"Presenting contents of backbuffer %d on device %d", mCurrentBackBufferIndex, i);

        DebugLog(L" [marshalled]\n");
        SDeviceContext& Context = mDeviceContexts[i];
        ID3D12Resource* RenderTarget = Context.mRenderTargets[mCurrentBackBufferIndex];
        ID3D12Resource* CopyBuffer = Context.mCrossAdapterCopyBuffers[mCurrentBackBufferIndex];

        Context.mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE));
        Context.mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CopyBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
        Context.mDisplayCommandList->CopyResource(CopyBuffer, RenderTarget);
        Context.mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CopyBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
        Context.mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT));
        Context.mDisplayCommandList->Close();

        if (Context.mFenceValue)
            Context.mDisplayCommandQueue->Wait(Context.mCrossAdapterCopyFence, Context.mFenceValue);
        ID3D12CommandList* ppCommandLists1[] = { Context.mDisplayCommandList };
        Context.mDisplayCommandQueue->ExecuteCommandLists(_countof(ppCommandLists1), ppCommandLists1);
        Context.mDisplayCommandQueue->Signal(Context.mCrossAdapterCopyFence, ++Context.mFenceValue);

        Context.mDisplayCommandList->Reset(Context.mDisplayCommandAllocator, nullptr);

        ID3D12Resource* CopyBufferOnHost = Context.mCrossAdapterCopyBuffersOnHost[mCurrentBackBufferIndex];
        ID3D12Resource* TargetBackBufferResource = nullptr;
        mSwapChains[0]->GetBuffer(mCurrentBackBufferIndex, IID_PPV_ARGS(&TargetBackBufferResource));

        mDeviceContexts[0].mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TargetBackBufferResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST));
        mDeviceContexts[0].mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CopyBufferOnHost, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
        mDeviceContexts[0].mDisplayCommandList->CopyResource(TargetBackBufferResource, CopyBufferOnHost);
        mDeviceContexts[0].mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CopyBufferOnHost, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
        mDeviceContexts[0].mDisplayCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TargetBackBufferResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT));
        mDeviceContexts[0].mDisplayCommandList->Close();

        mDeviceContexts[0].mDisplayCommandQueue->Wait(Context.mCrossAdapterCopyFenceOnHost, Context.mFenceValue);
        ID3D12CommandList* ppCommandLists2[] = { mDeviceContexts[0].mDisplayCommandList };
        mDeviceContexts[0].mDisplayCommandQueue->ExecuteCommandLists(_countof(ppCommandLists2), ppCommandLists2);
        mDeviceContexts[0].mDisplayCommandQueue->Signal(Context.mCrossAdapterCopyFenceOnHost, ++Context.mFenceValue);

        mDeviceContexts[0].mDisplayCommandList->Reset(mDeviceContexts[0].mDisplayCommandAllocator, nullptr);

        hr = mSwapChains[0]->Present(SyncInterval, Flags);

        TargetBackBufferResource->Release();

        break;
    }
    }

    mCurrentBackBufferIndex = mSwapChains[0]->GetCurrentBackBufferIndex();
    return hr;
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetBuffer(
    UINT Buffer,
    REFIID riid,
    void** ppSurface,
    UINT AffinityMask)
{
    UINT IndicesCount = 0;
    
    std::vector<ID3D12Resource*> Resources;
    Resources.resize(GetNodeCount(), nullptr);
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();

    switch (mMode)
    {
    case EAffinitySwapChainMode::MultiWindow:
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain* SwapChain = mSwapChains[i];
                ID3D12Resource* Surface = nullptr;
                HRESULT const hr = SwapChain->GetBuffer(Buffer, IID_PPV_ARGS(&Surface));

                if (S_OK == hr)
                {
                    Resources[i] = Surface;
                }
                else
                {
                    return hr;
                }
            }
        }
        break;
    }
    case EAffinitySwapChainMode::LDA:
    {
        for (UINT b = 0; b < mNumBackBuffers; ++b)
        {
            UINT bufferIndex = b % mNumRequestedBackBuffers;
            if (bufferIndex == Buffer)
            {
                UINT nodeIndex = b % GetNodeCount();
                HRESULT hr = mSwapChains[0]->GetBuffer(b, IID_PPV_ARGS(&Resources[nodeIndex]));
                if (FAILED(hr))
                {
                    return hr;
                }
            }
        }
        break;
    }
    case EAffinitySwapChainMode::SingleWindow:
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                ID3D12Resource* Surface = nullptr;
                HRESULT hr = S_OK;

                Surface = mDeviceContexts[i].mRenderTargets[Buffer];
                Surface->AddRef();

                if (S_OK == hr)
                {
                    Resources[i] = Surface;
                }
                else
                {
                    return hr;
                }
            }
        }
        break;
    }
    }

    CD3DX12AffinityResource* Resource = new CD3DX12AffinityResource(GetParentDevice(), &(Resources[0]), (UINT)Resources.size());
#ifdef DEBUG_OBJECT_NAME
    Resource->mObjectDebugName = L"SwapChainBackBuffer";
#endif

    *ppSurface = Resource;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::SetFullscreenState(
    BOOL Fullscreen,
    IDXGIOutput* pTarget,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetFullscreenState(Fullscreen, pTarget);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }

        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetFullscreenState(Fullscreen, pTarget);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetFullscreenState(
    BOOL* pFullscreen,
    IDXGIOutput** ppTarget,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetFullscreenState(pFullscreen, ppTarget);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetDesc(
    DXGI_SWAP_CHAIN_DESC* pDesc,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetDesc(pDesc);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::ResizeBuffers(
    UINT BufferCount,
    UINT Width,
    UINT Height,
    DXGI_FORMAT NewFormat,
    UINT SwapChainFlags,
    UINT AffinityMask)
{
    DebugLog(L"Doing swap chain buffer resize...\n");
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();

    // First free up any existing resources.

    for (UINT i = 1; i < mDeviceContexts.size(); ++i)
    {
        CDXGIAffinitySwapChain::SDeviceContext& DeviceContext = mDeviceContexts[i];

        for (size_t i = 0; i < DeviceContext.mRenderTargets.size(); ++i)
        {
            DeviceContext.mRenderTargets[i]->Release();
        }
        DeviceContext.mRenderTargets.clear();
        for (size_t i = 0; i < DeviceContext.mCrossAdapterCopyBufferHeaps.size(); ++i)
        {
            DeviceContext.mCrossAdapterCopyBufferHeaps[i]->Release();
        }
        DeviceContext.mCrossAdapterCopyBufferHeaps.clear();
        for (size_t i = 0; i < DeviceContext.mCrossAdapterCopyBuffers.size(); ++i)
        {
            DeviceContext.mCrossAdapterCopyBuffers[i]->Release();
        }
        DeviceContext.mCrossAdapterCopyBuffers.clear();
        for (size_t i = 0; i < DeviceContext.mCrossAdapterCopyBufferHeapsOnHost.size(); ++i)
        {
            DeviceContext.mCrossAdapterCopyBufferHeapsOnHost[i]->Release();
        }
        DeviceContext.mCrossAdapterCopyBufferHeapsOnHost.clear();
        for (size_t i = 0; i < DeviceContext.mCrossAdapterCopyBuffersOnHost.size(); ++i)
        {
            DeviceContext.mCrossAdapterCopyBuffersOnHost[i]->Release();
        }
        DeviceContext.mCrossAdapterCopyBuffersOnHost.clear();
    }

    // Resize the actual swap buffers.

    switch (mMode)
    {
    case EAffinitySwapChainMode::MultiWindow:
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain* SwapChain = mSwapChains[i];
                RETURN_IF_FAILED(
                    SwapChain->ResizeBuffers(
                        BufferCount,
                        Width,
                        Height,
                        NewFormat,
                        SwapChainFlags));
            }
        }
        break;
    }
    case EAffinitySwapChainMode::LDA:
    {
        std::vector<UINT> creationNodeMasks;
        std::vector<IUnknown*> pQueues;
        UINT nodeCount = GetNodeCount();
        if (nodeCount > 1)
        {
            mNumBackBuffers = lcm(BufferCount, nodeCount);
            mNumRequestedBackBuffers = BufferCount;
            creationNodeMasks.resize(mNumBackBuffers);
            pQueues.resize(mNumBackBuffers);
            for (UINT i = 0; i < mNumBackBuffers; i++)
            {
                creationNodeMasks[i] = GetParentDevice()->AffinityIndexToNodeMask(i % nodeCount);
                pQueues[i] = mParentQueue->GetChildObject(i % nodeCount);
            }
            RETURN_IF_FAILED(
                mSwapChains[0]->ResizeBuffers1(
                    mNumBackBuffers,
                    Width,
                    Height,
                    NewFormat,
                    SwapChainFlags,
                    creationNodeMasks.data(),
                    pQueues.data()));

            GetParentDevice()->g_ActiveNodeIndex = 0;
        }
        else
        {
            RETURN_IF_FAILED(
                mSwapChains[0]->ResizeBuffers(
                    BufferCount,
                    Width,
                    Height,
                    NewFormat,
                    SwapChainFlags));
        }
        break;
    }
    case EAffinitySwapChainMode::SingleWindow:
        RETURN_IF_FAILED(
            mSwapChains[0]->ResizeBuffers(
                BufferCount,
                Width,
                Height,
                NewFormat,
                SwapChainFlags));
        break;
    }

    mCurrentBackBufferIndex = 0;

    // Recreate render targets.

    if (mMode != EAffinitySwapChainMode::LDA)
    {
        for (UINT i = 0; i < mDeviceContexts.size(); ++i)
        {
            RETURN_IF_FAILED(
                CreateDummyRenderTargetsAndCrossAdapterCopyBuffers(
                    i,
                    GetParentDevice(),
                    mHostDevice,
                    mDeviceContexts[i],
                    Width, Height, NewFormat,
                    mNumBackBuffers));
        }
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::ResizeTarget(
    const DXGI_MODE_DESC* pNewTargetParameters,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->ResizeTarget(pNewTargetParameters);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }

        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->ResizeTarget(pNewTargetParameters);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetContainingOutput(
    IDXGIOutput** ppOutput,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetContainingOutput(ppOutput);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetFrameStatistics(
    DXGI_FRAME_STATISTICS* pStats,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetFrameStatistics(pStats);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetLastPresentCount(
    UINT* pLastPresentCount,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetLastPresentCount(pLastPresentCount);
}

HRESULT CDXGIAffinitySwapChain::GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetDesc1(pDesc);
}

HRESULT CDXGIAffinitySwapChain::GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetFullscreenDesc(pDesc);
}

HRESULT CDXGIAffinitySwapChain::GetHwnd(HWND* pHwnd, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetHwnd(pHwnd);
}

HRESULT CDXGIAffinitySwapChain::GetCoreWindow(REFIID refiid, void** ppUnk, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetCoreWindow(refiid, ppUnk);
}

HRESULT CDXGIAffinitySwapChain::Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters, UINT AffinityMask)
{
    HRESULT_METHOD_NOT_YET_IMPLEMENTED(CDXGIAffinitySwapChain::Present1);
}

BOOL CDXGIAffinitySwapChain::IsTemporaryMonoSupported(UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->IsTemporaryMonoSupported();
}

HRESULT CDXGIAffinitySwapChain::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetRestrictToOutput(ppRestrictToOutput);
}

HRESULT CDXGIAffinitySwapChain::SetBackgroundColor(const DXGI_RGBA* pColor, UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain1* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetBackgroundColor(pColor);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }

        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetBackgroundColor(pColor);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT CDXGIAffinitySwapChain::GetBackgroundColor(DXGI_RGBA* pColor, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetBackgroundColor(pColor);
}

HRESULT CDXGIAffinitySwapChain::SetRotation(DXGI_MODE_ROTATION Rotation, UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain1* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetRotation(Rotation);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }
        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetRotation(Rotation);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT CDXGIAffinitySwapChain::GetRotation(DXGI_MODE_ROTATION* pRotation, UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetRotation(pRotation);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::SetSourceSize(
    UINT Width,
    UINT Height,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain2* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetSourceSize(Width, Height);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }

        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetSourceSize(Width, Height);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetSourceSize(
    UINT* pWidth,
    UINT* pHeight,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetSourceSize(pWidth, pHeight);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::SetMaximumFrameLatency(
    UINT MaxLatency,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain2* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetMaximumFrameLatency(MaxLatency);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }
        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetMaximumFrameLatency(MaxLatency);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetMaximumFrameLatency(
    UINT* pMaxLatency,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetMaximumFrameLatency(pMaxLatency);
}

HANDLE STDMETHODCALLTYPE CDXGIAffinitySwapChain::GetFrameLatencyWaitableObject(UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetFrameLatencyWaitableObject();
}

HRESULT CDXGIAffinitySwapChain::SetMatrixTransform(
    const DXGI_MATRIX_3X2_F* pMatrix,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain2* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetMatrixTransform(pMatrix);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }

        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetMatrixTransform(pMatrix);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT CDXGIAffinitySwapChain::GetMatrixTransform(
    DXGI_MATRIX_3X2_F* pMatrix,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->GetMatrixTransform(pMatrix);
}

UINT CDXGIAffinitySwapChain::GetCurrentBackBufferIndex()
{
    return mCurrentBackBufferIndex;
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::CheckColorSpaceSupport(
    DXGI_COLOR_SPACE_TYPE ColorSpace,
    UINT* pColorSpaceSupport,
    UINT AffinityIndex)
{
    if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        AffinityIndex = 0;
    }

    return mSwapChains[AffinityIndex]->CheckColorSpaceSupport(ColorSpace, pColorSpaceSupport);
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::SetColorSpace1(
    DXGI_COLOR_SPACE_TYPE ColorSpace,
    UINT AffinityMask)
{
    UINT EffectiveAffinityMask = (AffinityMask == 0) ? GetNodeMask() : AffinityMask & GetNodeMask();
    if (mMode == EAffinitySwapChainMode::MultiWindow)
    {
        for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES;i++)
        {
            if (((1 << i) & EffectiveAffinityMask) != 0)
            {
                IDXGISwapChain3* SwapChain = mSwapChains[i];

                HRESULT const hr = SwapChain->SetColorSpace1(ColorSpace);

                if (hr != S_OK)
                {
                    return hr;
                }
            }
        }
        return S_OK;
    }
    else if (mMode == EAffinitySwapChainMode::LDA || mMode == EAffinitySwapChainMode::SingleWindow)
    {
        return mSwapChains[0]->SetColorSpace1(ColorSpace);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT STDMETHODCALLTYPE CDXGIAffinitySwapChain::ResizeBuffers1(
    UINT BufferCount,
    UINT Width,
    UINT Height,
    DXGI_FORMAT Format,
    UINT SwapChainFlags,
    const UINT* pCreationNodeMask,
    IUnknown* const* ppPresentQueue)
{
    HRESULT_METHOD_NOT_YET_IMPLEMENTED(CDXGIAffinitySwapChain::ResizeBuffers1);
}

CDXGIAffinitySwapChain::CDXGIAffinitySwapChain(CD3DX12AffinityDevice* device, CD3DX12AffinityCommandQueue* queue, IDXGISwapChain3** swapChains, UINT Count)
    : CD3DX12AffinityObject(device, reinterpret_cast<IUnknown**>(swapChains), Count)
    , mCurrentBackBufferIndex(0)
{
    for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
    {
        if (i < Count)
        {
            mSwapChains[i] = swapChains[i];
        }
        else
        {
            mSwapChains[i] = nullptr;
        }
    }
#ifdef DEBUG_OBJECT_NAME
    mObjectTypeName = L"SwapChain";
#endif
    mParentQueue = queue;
}

HRESULT CDXGIAffinitySwapChain::CreateDummyRenderTargetsAndCrossAdapterCopyBuffers(
    UINT const Index,
    CD3DX12AffinityDevice* AffinityDevice,
    ID3D12Device* const HostDevice,
    CDXGIAffinitySwapChain::SDeviceContext& DeviceContext,
    UINT const Width,
    UINT const Height,
    DXGI_FORMAT const Format,
    UINT const NumBackBuffers)
{
    for (UINT b = 0; b < NumBackBuffers; ++b)
    {
        // Create the render target itself.

        ID3D12Resource* RenderTarget = nullptr;

        CD3DX12_HEAP_PROPERTIES renderTargetHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        if (AffinityDevice->GetAffinityMode() == EAffinityMode::LDA)
        {
            renderTargetHeapProps.CreationNodeMask = AffinityDevice->AffinityIndexToNodeMask(Index);
            renderTargetHeapProps.VisibleNodeMask = AffinityDevice->LDAAllNodeMasks();
        }
        RETURN_IF_FAILED_WITH_ERROR_LOG(
            DeviceContext.mDevice->CreateCommittedResource(
                &renderTargetHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(
                    Format,
                    Width,
                    Height,
                    1, 1,
                    1, 0,
                    D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
                    D3D12_TEXTURE_LAYOUT_UNKNOWN),
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&RenderTarget)
            ),
            "Failed to create render target committed resource during affinity swap chain setup.", AffinityDevice);
        WCHAR name[256] = {};
        swprintf_s(name, L"SwapChain RenderTarget: %u", b);
        RenderTarget->SetName(name);
        DeviceContext.mRenderTargets.push_back(RenderTarget);

        // Next create a heap for the cross adapter copy.

        ID3D12Resource* CrossAdapterCopyResource = nullptr;
        ID3D12Heap* CrossAdapterCopyBufferHeap = nullptr;

        CD3DX12_RESOURCE_DESC const ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            Format,
            Width,
            Height,
            1, 1,
            1, 0,
            D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER,
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR);

        D3D12_RESOURCE_ALLOCATION_INFO const AllocationInfo = DeviceContext.mDevice->GetResourceAllocationInfo(
            0,
            1, &ResourceDesc);
        CD3DX12_HEAP_DESC heapDesc(AllocationInfo.SizeInBytes, D3D12_HEAP_TYPE_DEFAULT, 0, D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);
        if (AffinityDevice->GetAffinityMode() == EAffinityMode::LDA)
        {
            heapDesc.Properties.CreationNodeMask = AffinityDevice->AffinityIndexToNodeMask(Index);
            heapDesc.Properties.VisibleNodeMask = AffinityDevice->LDAAllNodeMasks();
        }

        RETURN_IF_FAILED_WITH_ERROR_LOG(
            DeviceContext.mDevice->CreateHeap(
                &heapDesc,
                IID_PPV_ARGS(&CrossAdapterCopyBufferHeap)),
            "Failed to create cross-adapter heap during affinity swap chain setup.", AffinityDevice);

        DeviceContext.mCrossAdapterCopyBufferHeaps.push_back(CrossAdapterCopyBufferHeap);

        // Create a placed resource with that heap on the current device.

        RETURN_IF_FAILED_WITH_ERROR_LOG(
            DeviceContext.mDevice->CreatePlacedResource(
                CrossAdapterCopyBufferHeap, 0,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&CrossAdapterCopyResource)
            ),
            "Failed to create cross-adapter placed resource during affinity swap chain setup.", AffinityDevice);

        std::wstring Name = std::wstring(L"CrossAdapterCopyBuffer") + std::to_wstring(Index) + L"-" + std::to_wstring(b);
        CrossAdapterCopyResource->SetName(Name.c_str());
        DeviceContext.mCrossAdapterCopyBuffers.push_back(CrossAdapterCopyResource);

        // Now open the heap up on the host device.

        HANDLE CrossAdapterCopyBufferSharedHandle = nullptr;
        RETURN_IF_FAILED_WITH_ERROR_LOG(
            DeviceContext.mDevice->CreateSharedHandle(
                CrossAdapterCopyBufferHeap,
                nullptr,
                GENERIC_ALL,
                nullptr,
                &CrossAdapterCopyBufferSharedHandle),
            "Failed to create shared handle to cross-adapter copy buffer heap during affinity swap chain setup.", AffinityDevice);

        CrossAdapterCopyBufferHeap = nullptr;
        RETURN_IF_FAILED_WITH_ERROR_LOG(
            HostDevice->OpenSharedHandle(CrossAdapterCopyBufferSharedHandle, IID_PPV_ARGS(&CrossAdapterCopyBufferHeap)),
            "Failed to open shared handle to cross-adapter copy buffer heap during affinity swap chain setup.", AffinityDevice);

        DeviceContext.mCrossAdapterCopyBufferHeapsOnHost.push_back(CrossAdapterCopyBufferHeap);

        // And finally use that heap to create placed resource on the host device.

        CrossAdapterCopyResource = nullptr;
        RETURN_IF_FAILED_WITH_ERROR_LOG(
            HostDevice->CreatePlacedResource(
                CrossAdapterCopyBufferHeap, 0,
                &CD3DX12_RESOURCE_DESC::Tex2D(
                    Format,
                    Width,
                    Height,
                    1, 1,
                    1, 0,
                    D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER,
                    D3D12_TEXTURE_LAYOUT_ROW_MAJOR),
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&CrossAdapterCopyResource)),
            "Failed to create cross-adapter placed resource on host device during affinity swap chain setup.", AffinityDevice);

        CrossAdapterCopyResource->SetName(L"CrossAdapterCopyBufferOnHost");
        DeviceContext.mCrossAdapterCopyBuffersOnHost.push_back(CrossAdapterCopyResource);

    }

    return S_OK;
}
