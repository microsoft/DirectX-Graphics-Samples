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
#include <dxgi1_2.h>
#include <string>
#include "Utils.h"

HRESULT STDMETHODCALLTYPE DXGIXAffinityCreateLDASwapChain(
    IDXGISwapChain* pSwapChain,
    CD3DX12AffinityCommandQueue* pQueue,
    CD3DX12AffinityDevice* pDevice,
    CDXGIAffinitySwapChain** ppSwapChain)
{
    IDXGISwapChain3* pSwapChain3;
    if (SUCCEEDED(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3))))
    {
        DXGI_SWAP_CHAIN_DESC1 desc;
        if (SUCCEEDED(pSwapChain3->GetDesc1(&desc)))
        {
            HRESULT hr = DXGIXAffinityCreateLDASwapChain(pSwapChain3, pQueue, pDevice, &desc, ppSwapChain);
            pSwapChain3->Release();
            return hr;
        }
    }
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE DXGIXAffinityCreateLDASwapChain(
    IDXGISwapChain3* pSwapChain,
    CD3DX12AffinityCommandQueue* pQueue,
    CD3DX12AffinityDevice* pDevice,
    DXGI_SWAP_CHAIN_DESC1* pDesc,
    CDXGIAffinitySwapChain** ppSwapChain)
{
    CD3DX12AffinityCommandQueue* AffinityQueue = static_cast<CD3DX12AffinityCommandQueue*>(pQueue);
    CD3DX12AffinityDevice* AffinityDevice = static_cast<CD3DX12AffinityDevice*>(pDevice);
    ID3D12Device* HostDevice = AffinityDevice->GetChildObject(0);

    std::vector<CDXGIAffinitySwapChain::SDeviceContext> NodeContexts;
    NodeContexts.reserve(AffinityDevice->GetNodeCount());

    for (UINT i = 0; i < AffinityDevice->GetNodeCount(); ++i)
    {
        NodeContexts.push_back(CDXGIAffinitySwapChain::SDeviceContext());
        CDXGIAffinitySwapChain::SDeviceContext& NodeContext = NodeContexts.back();

        NodeContext.mDevice = AffinityDevice->GetChildObject(0); //single device in LDA case
        NodeContext.mDisplayCommandQueue = AffinityQueue->GetChildObject(i);
    }

    CDXGIAffinitySwapChain* AffinityChain = new CDXGIAffinitySwapChain(AffinityDevice, AffinityQueue, &pSwapChain, 1);
    AffinityChain->mMode = EAffinitySwapChainMode::LDA;

    AffinityChain->mDeviceContexts = NodeContexts;
    AffinityChain->mNumBackBuffers = lcm(pDesc->BufferCount, AffinityDevice->GetNodeCount());
    AffinityChain->mNumRequestedBackBuffers = pDesc->BufferCount;
    AffinityChain->mHostDevice = HostDevice;
    *ppSwapChain = AffinityChain;

    std::vector<IUnknown*> ppCommandQueues;
    std::vector<UINT> pCreationNodes;
    for (UINT i = 0; i < AffinityChain->mNumBackBuffers; ++i)
    {
        UINT node = i % AffinityDevice->GetNodeCount();
        ppCommandQueues.push_back(AffinityQueue->GetChildObject(node));
        pCreationNodes.push_back(AffinityDevice->AffinityIndexToNodeMask(node));
    }

    pSwapChain->AddRef();
    const HRESULT hr = pSwapChain->ResizeBuffers1(AffinityChain->mNumBackBuffers, pDesc->Width, pDesc->Height, pDesc->Format, pDesc->Flags,
        pCreationNodes.data(), ppCommandQueues.data());

    return hr;
}
