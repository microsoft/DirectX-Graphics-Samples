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
 * This is your standard "SFR or AFR or SPR (split pipeline rendering)" scenario where
 * there is just ONE actual swapchain and Present()s from any other adapter means we
 * have to copy the render target to the adapter which owns the actual swapchain.
 *
 * As such this method does a lot of the heavy lifting in terms of creating all the
 * resources and objects we need to perform that copy.
 */

#include "d3dx12affinity.h"
#include <dxgi1_2.h>
#include <string>
#include "Utils.h"

HRESULT STDMETHODCALLTYPE DXGIXAffinityCreateSingleWindowSwapChain(
    IDXGISwapChain3* pSwapChain,
    CD3DX12AffinityCommandQueue* pQueue,
    CD3DX12AffinityDevice* pDevice,
    DXGI_SWAP_CHAIN_DESC1* pDesc,
    CDXGIAffinitySwapChain** ppSwapChain)
{
    CD3DX12AffinityCommandQueue* AffinityQueue = static_cast<CD3DX12AffinityCommandQueue*>(pQueue);
    CD3DX12AffinityDevice* AffinityDevice = static_cast<CD3DX12AffinityDevice*>(pDevice);
    ID3D12Device* HostDevice = AffinityDevice->GetChildObject(0);

    std::vector<CDXGIAffinitySwapChain::SDeviceContext> DeviceContexts;
    DeviceContexts.reserve(AffinityDevice->GetDeviceCount());

    for (UINT i = 0; i < AffinityDevice->GetDeviceCount(); ++i)
    {
        DeviceContexts.push_back(CDXGIAffinitySwapChain::SDeviceContext());
        CDXGIAffinitySwapChain::SDeviceContext& DeviceContext = DeviceContexts.back();

        DeviceContext.mDevice = AffinityDevice->GetChildObject(i);
        DeviceContext.mDisplayCommandQueue = AffinityQueue->GetChildObject(i);

        // Each device needs a command list and respective command allocator.
        // This list will be used for submitting resource barriers and copies from the "dummy" rendertarget.

        RETURN_IF_FAILED_WITH_ERROR_LOG(
            DeviceContext.mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&DeviceContext.mDisplayCommandAllocator)),
            "Failed to create command allocator during affinity swap chain setup.", AffinityDevice);

        RETURN_IF_FAILED_WITH_ERROR_LOG(
            DeviceContext.mDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, DeviceContext.mDisplayCommandAllocator,
                nullptr, IID_PPV_ARGS(&DeviceContext.mDisplayCommandList)),
            "Failed to create command list during affinity swap chain setup.", AffinityDevice);

        // The device at index 0 is the "host" device, it uses it's own actual render targets
        // and presents from this device occur in the usual fashion.
        //
        // For all devices index 1 and up, we need to create some fences and intermediate buffers
        // to facilitate presents.

        {
            // Create the cross-adapter copy fence, used to control access to the cross-adapter copy buffer

            RETURN_IF_FAILED_WITH_ERROR_LOG(
                DeviceContext.mDevice->CreateFence(
                    0,
                    D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER,
                    IID_PPV_ARGS(&DeviceContext.mCrossAdapterCopyFence)),
                "Failed to create fence during affinity swap chain setup.", AffinityDevice);

            std::wstring Name = std::wstring(L"CrossAdapterCopyFence") + std::to_wstring(i);
            RETURN_IF_FAILED(DeviceContext.mCrossAdapterCopyFence->SetName(Name.c_str()));

            {
                HANDLE CrossAdapterCopyFenceSharedHandle = nullptr;
                RETURN_IF_FAILED_WITH_ERROR_LOG(
                    DeviceContext.mDevice->CreateSharedHandle(
                        DeviceContext.mCrossAdapterCopyFence,
                        nullptr,
                        GENERIC_ALL,
                        Name.c_str(),
                        &CrossAdapterCopyFenceSharedHandle),
                    "Failed to create shared handle to cross adapter copy fence during affinity swap chain setup.", AffinityDevice);

                RETURN_IF_FAILED_WITH_ERROR_LOG(
                    HostDevice->OpenSharedHandle(CrossAdapterCopyFenceSharedHandle, IID_PPV_ARGS(&DeviceContext.mCrossAdapterCopyFenceOnHost)),
                    "Failed to open shared handle to cross adapter copy fence during affinity swap chain setup.", AffinityDevice);

                Name = std::wstring(L"CrossAdapterCopyFenceOnHost") + std::to_wstring(i);
                RETURN_IF_FAILED(DeviceContext.mCrossAdapterCopyFenceOnHost->SetName(L"CrossAdapterCopyFenceOnHost"));
            }
            // Create the "dummy" render targets and cross-adapter copy buffers.
            // For a swap chain with (n) back buffers, we create (n) render targets and (n) cross-adapter copy buffers.

            DeviceContext.mRenderTargets.reserve(pDesc->BufferCount);
            DeviceContext.mCrossAdapterCopyBuffers.reserve(pDesc->BufferCount);

            RETURN_IF_FAILED(CDXGIAffinitySwapChain::CreateDummyRenderTargetsAndCrossAdapterCopyBuffers(
                i,
                AffinityDevice,
                HostDevice,
                DeviceContext,
                pDesc->Width,
                pDesc->Height,
                pDesc->Format,
                pDesc->BufferCount
            ));
        }
    }

    pSwapChain->AddRef();

    CDXGIAffinitySwapChain* AffinityChain = new CDXGIAffinitySwapChain(AffinityDevice, AffinityQueue, &pSwapChain, 1);
    if (AffinityDevice->GetAffinityMode() == EAffinityMode::LDA)
    {
        AffinityChain->mMode = EAffinitySwapChainMode::LDA;
    }
    else
    {
        AffinityChain->mMode = EAffinitySwapChainMode::SingleWindow;
    }

    AffinityChain->mDeviceContexts = DeviceContexts;
    AffinityChain->mNumBackBuffers = pDesc->BufferCount;
    AffinityChain->mHostDevice = HostDevice;
    *ppSwapChain = AffinityChain;

    return S_OK;
}
