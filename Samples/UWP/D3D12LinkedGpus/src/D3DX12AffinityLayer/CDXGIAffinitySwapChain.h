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
#include "CD3DX12AffinityObject.h"

#include <dxgi1_4.h>

inline UINT gcd(UINT a, UINT b)
{
    for (;;)
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

inline UINT lcm(UINT a, UINT b)
{
    UINT temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
}

enum EAffinitySwapChainMode
{
    SingleWindow,
    MultiWindow,
    LDA
};

// Implements up to IDXGISwapChain3.
class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CDXGIAffinitySwapChain : public CD3DX12AffinityObject
{
public:
    virtual HRESULT QueryInterface(CDXGIAffinitySwapChain** ppvObject)
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetParent(
        _In_  REFIID riid,
        _COM_Outptr_  void** ppParent,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE Present(
        UINT SyncInterval,
        UINT Flags,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetBuffer(
        UINT Buffer,
        _In_ REFIID riid,
        _Outptr_  void** ppSurface,
        _In_opt_  UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE SetFullscreenState(
        BOOL Fullscreen,
        _In_opt_  IDXGIOutput* pTarget,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetFullscreenState(
        _Out_opt_  BOOL* pFullscreen,
        _COM_Outptr_opt_result_maybenull_  IDXGIOutput** ppTarget,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetDesc(
        _Out_  DXGI_SWAP_CHAIN_DESC* pDesc,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE ResizeBuffers(
        UINT BufferCount,
        UINT Width,
        UINT Height,
        DXGI_FORMAT NewFormat,
        UINT SwapChainFlags,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE ResizeTarget(
        _In_  const DXGI_MODE_DESC* pNewTargetParameters,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetContainingOutput(
        _COM_Outptr_  IDXGIOutput** ppOutput,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetFrameStatistics(
        _Out_  DXGI_FRAME_STATISTICS* pStats,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetLastPresentCount(
        _Out_  UINT* pLastPresentCount,
        UINT AffinityIndex = 0);

    // IDXGISwapChain1

    HRESULT STDMETHODCALLTYPE GetDesc1(
        _Out_  DXGI_SWAP_CHAIN_DESC1* pDesc,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetFullscreenDesc(
        _Out_  DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetHwnd(
        _Out_  HWND* pHwnd,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetCoreWindow(
        _In_  REFIID refiid,
        _COM_Outptr_  void** ppUnk,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE Present1(
        UINT SyncInterval,
        UINT PresentFlags,
        _In_  const DXGI_PRESENT_PARAMETERS* pPresentParameters,
        UINT AffinityMask = EAffinityMask::AllNodes);

    BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported(UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE GetRestrictToOutput(
        _Out_  IDXGIOutput** ppRestrictToOutput,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetBackgroundColor(
        _In_  const DXGI_RGBA* pColor,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetBackgroundColor(
        _Out_  DXGI_RGBA* pColor,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetRotation(
        _In_  DXGI_MODE_ROTATION Rotation,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetRotation(
        _Out_  DXGI_MODE_ROTATION* pRotation,
        UINT AffinityIndex = 0);

    // IDXGISwapChain2

    HRESULT STDMETHODCALLTYPE SetSourceSize(
        UINT Width,
        UINT Height,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetSourceSize(
        _Out_  UINT* pWidth,
        _Out_  UINT* pHeight,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(
        UINT MaxLatency,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(
        _Out_  UINT* pMaxLatency,
        UINT AffinityIndex = 0);

    HANDLE STDMETHODCALLTYPE GetFrameLatencyWaitableObject(
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetMatrixTransform(
        const DXGI_MATRIX_3X2_F* pMatrix,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE GetMatrixTransform(
        _Out_  DXGI_MATRIX_3X2_F* pMatrix,
        UINT AffinityIndex = 0);

    // IDXGISwapChain3

    UINT STDMETHODCALLTYPE GetCurrentBackBufferIndex();

    HRESULT STDMETHODCALLTYPE CheckColorSpaceSupport(
        _In_  DXGI_COLOR_SPACE_TYPE ColorSpace,
        _Out_  UINT* pColorSpaceSupport,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetColorSpace1(
        _In_  DXGI_COLOR_SPACE_TYPE ColorSpace,
        UINT AffinityMask = EAffinityMask::AllNodes);

    HRESULT STDMETHODCALLTYPE ResizeBuffers1(
        _In_  UINT BufferCount,
        _In_  UINT Width,
        _In_  UINT Height,
        _In_  DXGI_FORMAT Format,
        _In_  UINT SwapChainFlags,
        _In_reads_(BufferCount)  const UINT* pCreationNodeMask,
        _In_reads_(BufferCount)  IUnknown* const* ppPresentQueue);

    CDXGIAffinitySwapChain(CD3DX12AffinityDevice* device, CD3DX12AffinityCommandQueue* queue, IDXGISwapChain3** swapChains, UINT Count);

    struct SDeviceContext
    {
        std::vector<ID3D12Resource*> mRenderTargets;
        std::vector<ID3D12Resource*> mCrossAdapterCopyBuffers;
        std::vector<ID3D12Heap*> mCrossAdapterCopyBufferHeaps;
        std::vector<ID3D12Resource*> mCrossAdapterCopyBuffersOnHost;
        std::vector<ID3D12Heap*> mCrossAdapterCopyBufferHeapsOnHost;
        ID3D12Fence* mCrossAdapterCopyFence;
        ID3D12Fence* mCrossAdapterCopyFenceOnHost;

        ID3D12CommandQueue* mDisplayCommandQueue;
        ID3D12CommandAllocator* mDisplayCommandAllocator;
        ID3D12GraphicsCommandList* mDisplayCommandList;

        UINT mFenceValue;
        ID3D12Device* mDevice;

        SDeviceContext()
        {
            mCrossAdapterCopyFence = nullptr;
            mCrossAdapterCopyFenceOnHost = nullptr;

            mDisplayCommandQueue = nullptr;
            mDisplayCommandAllocator = nullptr;
            mDisplayCommandList = nullptr;

            mFenceValue = 0;
            mDevice = nullptr;
        }
    };

    static HRESULT CreateDummyRenderTargetsAndCrossAdapterCopyBuffers(
        UINT const Index,
        CD3DX12AffinityDevice* AffinityDevice,
        ID3D12Device* const HostDevice,
        CDXGIAffinitySwapChain::SDeviceContext& DeviceContext,
        UINT const Width,
        UINT const Height,
        DXGI_FORMAT const Format,
        UINT const NumBackBuffers);

    CD3DX12AffinityCommandQueue* mParentQueue;
    std::vector<SDeviceContext> mDeviceContexts;

    UINT mCurrentBackBufferIndex;
    UINT mNumBackBuffers;
    UINT mNumRequestedBackBuffers;
    EAffinitySwapChainMode mMode;

    ID3D12Device* mHostDevice;

protected:
    virtual bool IsD3D()
    {
        return false;
    }

private:
    IDXGISwapChain3* mSwapChains[D3DX12_MAX_ACTIVE_NODES];
};
