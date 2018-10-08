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
#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void EnableShaderBasedValidation()
{
    CComPtr<ID3D12Debug> spDebugController0;
    CComPtr<ID3D12Debug1> spDebugController1;
    AssertSucceeded(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
    AssertSucceeded(spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
    spDebugController1->SetEnableGPUBasedValidation(true);
}

static const GUID D3D12ExperimentalShaderModelsID = { /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
    0x76f5573e,
    0xf13a,
    0x40f5,
    { 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
};

using namespace DirectX;

// A more recent Windows SDK than currently required is needed for these.
typedef HRESULT(WINAPI *D3D12EnableExperimentalFeaturesFn)(
    UINT                                    NumFeatures,
    __in_ecount(NumFeatures) const IID*     pIIDs,
    __in_ecount_opt(NumFeatures) void*      pConfigurationStructs,
    __in_ecount_opt(NumFeatures) UINT*      pConfigurationStructSizes);

static HRESULT EnableExperimentalShaderModels() {
    HMODULE hRuntime = LoadLibraryW(L"d3d12.dll");
    if (hRuntime == NULL) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    D3D12EnableExperimentalFeaturesFn pD3D12EnableExperimentalFeatures =
        (D3D12EnableExperimentalFeaturesFn)GetProcAddress(hRuntime, "D3D12EnableExperimentalFeatures");
    if (pD3D12EnableExperimentalFeatures == nullptr) {
        FreeLibrary(hRuntime);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return pD3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModelsID, nullptr, nullptr);
}

D3D12Context::D3D12Context(CreationFlags flags)
{
    EnableExperimentalShaderModels();

    CComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)))) debugInterface->EnableDebugLayer();
    //EnableShaderBasedValidation(); 

    CComPtr<IDXGIFactory4> factory;
    AssertSucceeded(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    CComPtr<IDXGIAdapter> pAdapter;

    bool bUseWarp = ((UINT)flags & (UINT)CreationFlags::ForceHardware) == 0;
    if (bUseWarp)
    {
        AssertSucceeded(factory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter)));
    }

    AssertSucceeded(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)));

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueDesc.NodeMask = 0;
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CComPtr<ID3D12CommandQueue> pCommandQueue;
    AssertSucceeded(m_pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_pCommandQueue)));

    AssertSucceeded(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pAllocator)));

    m_lastSignaledValue = 1;
    AssertSucceeded(m_pDevice->CreateFence(m_lastSignaledValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
}


void D3D12Context::GetGraphicsCommandList(ID3D12GraphicsCommandList **ppCommandList)
{
    AssertSucceeded(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pAllocator, nullptr, IID_PPV_ARGS(ppCommandList)));
}

void D3D12Context::ExecuteCommandList(ID3D12GraphicsCommandList *pGraphicsCommandList)
{
    ID3D12CommandList *commandLists[] = { pGraphicsCommandList };
    m_pCommandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);

    m_lastSignaledValue++;
    m_pCommandQueue->Signal(m_pFence, m_lastSignaledValue);

    m_commandListsToDelete.push_back(pGraphicsCommandList);
}

void D3D12Context::WaitForGpuWork()
{
    HANDLE gpuWaitEvent = CreateEvent(nullptr, false, false, nullptr);
    Assert::AreNotEqual(gpuWaitEvent, INVALID_HANDLE_VALUE);

    AssertSucceeded(m_pFence->SetEventOnCompletion(m_lastSignaledValue, gpuWaitEvent));
    DWORD waitResult = WaitForSingleObject(gpuWaitEvent, INFINITE);
    Assert::AreEqual(waitResult, WAIT_OBJECT_0);
    CloseHandle(gpuWaitEvent);

    m_commandListsToDelete.empty();
    AssertSucceeded(m_pAllocator->Reset());
}
void D3D12Context::CreateResourceWithInitialData(const void *pInputData, UINT64 dataSize, ID3D12Resource **ppResource)
{
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(std::max(dataSize, (UINT64)D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT));
    auto heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    AssertSucceeded(m_pDevice->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(ppResource)));

    void *pMappedResource;
    AssertSucceeded((*ppResource)->Map(0, nullptr, &pMappedResource));
    memcpy(pMappedResource, pInputData, dataSize);
    (*ppResource)->Unmap(0, nullptr);
}

void D3D12Context::ReadbackTexture(ID3D12Resource *pResource, std::vector<byte> &data, UINT64 &pitch)
{
    UINT64 sizeInBytes;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    m_pDevice->GetCopyableFootprints(&pResource->GetDesc(), 0, 1, 0, &footprint, nullptr, nullptr, &sizeInBytes);

    auto readbackResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);
    auto readbackHeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

    CComPtr<ID3D12Resource> pReadbackResource;
    AssertSucceeded(m_pDevice->CreateCommittedResource(&readbackHeapDesc, D3D12_HEAP_FLAG_NONE, &readbackResourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pReadbackResource)));

    CComPtr<ID3D12GraphicsCommandList> pCopyCommandList;
    GetGraphicsCommandList(&pCopyCommandList);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    pCopyCommandList->ResourceBarrier(1, &barrier);

    D3D12_TEXTURE_COPY_LOCATION dst = CD3DX12_TEXTURE_COPY_LOCATION(pReadbackResource, footprint);
    D3D12_TEXTURE_COPY_LOCATION src = CD3DX12_TEXTURE_COPY_LOCATION(pResource, 0);
    pCopyCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    AssertSucceeded(pCopyCommandList->Close());

    ExecuteCommandList(pCopyCommandList);
    WaitForGpuWork();

    void *pReadbackData;
    pReadbackResource->Map(0, nullptr, &pReadbackData);

    data.resize(sizeInBytes);
    memcpy(data.data(), pReadbackData, sizeInBytes);
    pitch = footprint.Footprint.RowPitch;
}


void D3D12Context::ReadbackResource(ID3D12Resource *pResource, void *pOutputData, UINT dataSize)
{
    D3D12_RESOURCE_DESC desc = pResource->GetDesc();
    
    Assert::IsTrue(desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);

    auto readbackResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Width);
    auto readbackHeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

    CComPtr<ID3D12Resource> pReadbackResource;
    AssertSucceeded(m_pDevice->CreateCommittedResource(&readbackHeapDesc, D3D12_HEAP_FLAG_NONE, &readbackResourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pReadbackResource)));

    CComPtr<ID3D12GraphicsCommandList> pCopyCommandList;
    GetGraphicsCommandList(&pCopyCommandList);

    pCopyCommandList->CopyResource(pReadbackResource, pResource);
    AssertSucceeded(pCopyCommandList->Close());
    ExecuteCommandList(pCopyCommandList);
    WaitForGpuWork();

    void *pReadbackData;
    pReadbackResource->Map(0, nullptr, &pReadbackData);

    memcpy(pOutputData, pReadbackData, dataSize);
}

D3D12Context::~D3D12Context()
{
    WaitForGpuWork();
}


UINT D3D12Context::GetTotalLaneCount()
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 waveData;
    AssertSucceeded(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &waveData, sizeof(waveData)));
    return waveData.TotalLaneCount;
}


