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
 * Primary included header for the affinity layer. You should change all
 * #include <d3d12.h> in your engine source to include this file instead.
 */

#pragma once

#pragma pack(push,16)


#include <d3d12.h>

class CD3DX12AffinityObject;
class CD3DX12AffinityDeviceChild;
class CD3DX12AffinityPageable;
class CD3DX12AffinityRootSignature;
class CD3DX12AffinityRootSignatureDeserializer;
class CD3DX12AffinityHeap;
class CD3DX12AffinityResource;
class CD3DX12AffinityCommandAllocator;
class CD3DX12AffinityFence;
class CD3DX12AffinityPipelineState;
class CD3DX12AffinityDescriptorHeap;
class CD3DX12AffinityQueryHeap;
class CD3DX12AffinityCommandSignature;
class CD3DX12AffinityCommandList;
class CD3DX12AffinityGraphicsCommandList;
class CD3DX12AffinityCommandQueue;
class CD3DX12AffinityDevice;
class CDXGIAffinitySwapChain;

#include "CD3DX12AffinityObject.h"
#include "CD3DX12AffinityDeviceChild.h"
#include "CD3DX12AffinityPageable.h"
#include "CD3DX12AffinityRootSignature.h"
#include "CD3DX12AffinityHeap.h"
#include "CD3DX12AffinityResource.h"
#include "CD3DX12AffinityCommandAllocator.h"
#include "CD3DX12AffinityFence.h"
#include "CD3DX12AffinityPipelineState.h"
#include "CD3DX12AffinityDescriptorHeap.h"
#include "CD3DX12AffinityQueryHeap.h"
#include "CD3DX12AffinityCommandSignature.h"
#include "CD3DX12AffinityCommandList.h"
#include "CD3DX12AffinityGraphicsCommandList.h"
#include "CD3DX12AffinityCommandQueue.h"
#include "CD3DX12AffinityDevice.h"
#include "CDXGIAffinitySwapChain.h"
#include "d3dx12affinity_structs.h"


HRESULT WINAPI D3DX12AffinityCreateLDADevice(
    _In_ ID3D12Device* Device,
    _Outptr_  CD3DX12AffinityDevice** ppDevice);

HRESULT STDMETHODCALLTYPE DXGIXAffinityCreateSingleWindowSwapChain(
    _In_  IDXGISwapChain3* pSwapChain,
    _In_  CD3DX12AffinityCommandQueue* pQueue,
    _In_  CD3DX12AffinityDevice* pDevice,
    _In_  DXGI_SWAP_CHAIN_DESC1* pDesc,
    _Outptr_  CDXGIAffinitySwapChain** ppSwapChain);

HRESULT STDMETHODCALLTYPE DXGIXAffinityCreateLDASwapChain(
    _In_  IDXGISwapChain* pSwapChain,
    _In_  CD3DX12AffinityCommandQueue* pQueue,
    _In_  CD3DX12AffinityDevice* pDevice,
    _Outptr_  CDXGIAffinitySwapChain** ppSwapChain);

HRESULT STDMETHODCALLTYPE DXGIXAffinityCreateLDASwapChain(
    _In_  IDXGISwapChain3* pSwapChain,
    _In_  CD3DX12AffinityCommandQueue* pQueue,
    _In_  CD3DX12AffinityDevice* pDevice,
    _In_  DXGI_SWAP_CHAIN_DESC1* pDesc,
    _Outptr_  CDXGIAffinitySwapChain** ppSwapChain);

#pragma pack(pop)
