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

#include "DXSample.h"
#include "dxcapi.h"
#include <windows.h>
#include <iostream>
#include <atlbase.h>

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12HelloMeshNodes : public DXSample
{
public:
    D3D12HelloMeshNodes(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    struct WorkGraphContext
    {
        ComPtr<ID3D12WorkGraphProperties1> spWGProps;
        ComPtr<ID3D12Resource> spBackingMemory;
        D3D12_GPU_VIRTUAL_ADDRESS_RANGE BackingMemory = {};
        D3D12_PROGRAM_IDENTIFIER hWorkGraph = {};
        D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS MemReqs = {};

        ComPtr<ID3D12Resource> spLocalRootArgumentsTable;
        D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE LocalRootArgumentsTable = {};

        UINT NumEntrypoints = 0;
        UINT NumNodes = 0;
        UINT WorkGraphIndex = 0;
        UINT MaxInputRecords = 0;
        UINT MaxNodeInputs = 0;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device14> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_globalRootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12StateObject> m_stateObject;
    WorkGraphContext m_workGraphContext;
    ComPtr<ID3D12GraphicsCommandList10> m_commandList;
    UINT m_rtvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void GeneralSetup();
    void CreateWorkGraph();
    void PopulateCommandList();
    void WaitForPreviousFrame();

    void FlushAndFinish();
    void MakeBuffer(
        ID3D12Resource** ppResource,
        UINT64 SizeInBytes,
        D3D12_RESOURCE_FLAGS ResourceMiscFlags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT);

    void UploadData(
        ID3D12Resource* pResource,
        const VOID* pData,
        SIZE_T Size,
        ID3D12Resource** ppStagingResource, // only used if doFlush == false
        D3D12_RESOURCE_STATES CurrentState,
        bool doFlush);

    void MakeBufferAndInitialize(
        ID3D12Resource** ppResource,
        const VOID* pInitialData,
        UINT64 SizeInBytes,
        ID3D12Resource** ppStagingResource = nullptr, // only used if doFlush == false
        bool doFlush = true,
        D3D12_RESOURCE_FLAGS ResourceMiscFlags = D3D12_RESOURCE_FLAG_NONE);

    void InitWorkGraphContext(
        WorkGraphContext* pCtx, 
        ID3D12StateObject* pSO, 
        LPCWSTR pWorkGraphName,
        void* pLocalRootArgumentsTable,
        UINT LocalRootArgumentsTableSizeInBytes,
        UINT MaxInputRecords,
        UINT MaxNodeInputs);

};

HRESULT CompileDxilLibraryFromFile(
    _In_ LPCWSTR pFile,
    _In_ LPCWSTR pEntry,
    _In_ LPCWSTR pTarget,
    _In_reads_(cArgs) LPCWSTR args[],
    _In_ UINT cArgs,
    _In_reads_(cDefines) DxcDefine* pDefines,
    _In_ UINT cDefines,
    _Out_ ID3DBlob** ppCode);



