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

// This sample is the same as the D3D12HelloTriangle except it uses
// a work graph with mesh nodes instead of the previously existing PSO path. 
// 
// The graph has a root node and routes input across a small number of
// "material" mesh nodes in an array.  The mesh nodes happen to use the 
// same mesh node shader but vary other properties like pixel shader
// or local root argument data.  Each work item draws a triangle with 
// position or color varying depending on a combination of input record data,
// "material" selection, local root argument data and global root argument data.
// 
// The mesh nodes are also marked as graph entry points in addition to the 
// root node above them.  So some input records are fed directly to the leaf mesh 
// nodes from the command list, just to illustrate that possibility. In fact
// one could have a graph consisting of only leaf nodes, so that graph inputs
// are basically directly selecting which mesh nodes to run.
// 
// The interesting functions to compare vs D3D12HelloTriangle are:
// LoadPipeline() and PopulateCommandList().
// 

#include "stdafx.h"
#include "D3D12HelloMeshNodes.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 715; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

void D3D12HelloMeshNodes::InitWorkGraphContext(
    WorkGraphContext* pCtx,
    ID3D12StateObject* pSO,
    LPCWSTR pWorkGraphName,
    void* pLocalRootArgumentsTable,
    UINT LocalRootArgumentsTableSizeInBytes,
    UINT MaxInputRecords,
    UINT MaxNodeInputs)
{
    ComPtr<ID3D12StateObjectProperties1> spSOProps;
    pSO->QueryInterface(IID_PPV_ARGS(&spSOProps));
    pCtx->hWorkGraph = spSOProps->GetProgramIdentifier(pWorkGraphName);
    pSO->QueryInterface(IID_PPV_ARGS(&pCtx->spWGProps));
    pCtx->WorkGraphIndex = pCtx->spWGProps->GetWorkGraphIndex(pWorkGraphName);

    // Work graphs with mesh nodes require the max number of input records that will 
    // be sent to a given DispatchGraph() call to be specified before retrieving 
    // backing memory.
    // Hopefully by the end of the preview phase this annoyance will no longer be necessary.
    pCtx->spWGProps->SetMaximumInputRecords(
        pCtx->WorkGraphIndex, MaxInputRecords, MaxNodeInputs);
    pCtx->MaxInputRecords = MaxInputRecords;
    pCtx->MaxNodeInputs = MaxNodeInputs;

    pCtx->spWGProps->GetWorkGraphMemoryRequirements(
        pCtx->WorkGraphIndex,
        &pCtx->MemReqs);
    pCtx->BackingMemory.SizeInBytes = pCtx->MemReqs.MaxSizeInBytes;
    MakeBuffer(
        &pCtx->spBackingMemory,
        pCtx->BackingMemory.SizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    pCtx->BackingMemory.StartAddress =
        pCtx->spBackingMemory->GetGPUVirtualAddress();
    pCtx->NumEntrypoints =
        pCtx->spWGProps->GetNumEntrypoints(pCtx->WorkGraphIndex);
    pCtx->NumNodes = pCtx->spWGProps->GetNumNodes(pCtx->WorkGraphIndex);

    if (pLocalRootArgumentsTable && LocalRootArgumentsTableSizeInBytes)
    {
        pCtx->LocalRootArgumentsTable.SizeInBytes =
            LocalRootArgumentsTableSizeInBytes;
        MakeBufferAndInitialize(
            &pCtx->spLocalRootArgumentsTable,
            pLocalRootArgumentsTable,
            pCtx->LocalRootArgumentsTable.SizeInBytes);
        pCtx->LocalRootArgumentsTable.StartAddress =
            pCtx->spLocalRootArgumentsTable->GetGPUVirtualAddress();
        pCtx->LocalRootArgumentsTable.StrideInBytes = sizeof(UINT);
    }
}


D3D12HelloMeshNodes::D3D12HelloMeshNodes(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{
    m_viewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) );
}

void D3D12HelloMeshNodes::OnInit()
{
    GeneralSetup();
    CreateWorkGraph();
}

// Load the rendering pipeline dependencies.
void D3D12HelloMeshNodes::GeneralSetup()
{
    UINT dxgiFactoryFlags = 0;

    UUID Features[2] = { D3D12ExperimentalShaderModels, D3D12StateObjectsExperiment };
    HRESULT hr;
    hr = D3D12EnableExperimentalFeatures(_countof(Features), Features, nullptr, nullptr);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to enable experimental features required for mesh nodes to run.");
        ThrowIfFailed(E_FAIL);
    }

#if defined(_DEBUG)
    // Enable the debug layer
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS21 Options;
    ThrowIfFailed(m_device->CheckFeatureSupport(
        D3D12_FEATURE_D3D12_OPTIONS21, &Options, sizeof(Options)));
    if (Options.WorkGraphsTier < D3D12_WORK_GRAPHS_TIER_1_1) {
        OutputDebugStringA("Device does not report support for work graphs tier 1.1 (mesh nodes).");
        ThrowIfFailed(E_FAIL);
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(
        factory->MakeWindowAssociation(Win32Application::GetHwnd(), 
            DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = 
            m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        IID_PPV_ARGS(&m_commandAllocator)));
}

void D3D12HelloMeshNodes::CreateWorkGraph()
{
    ThrowIfFailed(m_device->CreateCommandList(0, 
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        m_commandAllocator.Get(), 
        nullptr, 
        IID_PPV_ARGS(&m_commandList)));

    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fenceValue = 1;
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    // Compile shaders and create a state object
    {
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> pixelShader2;
        ComPtr<ID3DBlob> libShaders;

        // Compile shaders
        
        // Pixel shaders must be compiled via non-lib shader target, e.g. ps_6_0:
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(), 
            L"PSMain", L"ps_6_0", nullptr, 0, nullptr, 0, &pixelShader));

        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(), 
            L"PSMain2", L"ps_6_0", nullptr, 0, nullptr, 0, &pixelShader2));

        LPCWSTR cDefines[] = { 
            L"-D LIB_TARGET", 
            L"-select-validator internal", 
            L"-enable-16bit-types"};

        // Node shaders use lib target, lib_6_9 here for mesh node support:
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(), 
            nullptr, L"lib_6_9", cDefines, _countof(cDefines), nullptr, 0, &libShaders));

        ThrowIfFailed(m_device->CreateRootSignatureFromSubobjectInLibrary(0, 
            libShaders->GetBufferPointer(), 
            libShaders->GetBufferSize(), 
            L"MeshNodesGlobalRS", 
            IID_PPV_ARGS(&m_globalRootSignature)));

        // Create state object

        CD3DX12_STATE_OBJECT_DESC SODesc;
        SODesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

        // Work graphs with mesh nodes need to use graphics global root arguments
        // (as opposed to compute):
        auto pSOConfig = SODesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
        pSOConfig->SetFlags(
            D3D12_STATE_OBJECT_FLAG_WORK_GRAPHS_USE_GRAPHICS_STATE_FOR_GLOBAL_ROOT_SIGNATURE);

        // Add global root signature
        // This could also be added to the state object the same way as the 
        // local root signature further below, referring to the DXIL subobject
        // by name.  The alternative shown here works equivalently 
        // since there happens to be an API version of the global root 
        // signature created.
        auto pGlobalRootSig = 
            SODesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        pGlobalRootSig->SetRootSignature(m_globalRootSignature.Get());

        // Add DXIL library with node shaders and local root signature definition
        auto pLib = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        CD3DX12_SHADER_BYTECODE bcLib(libShaders.Get());
        pLib->SetDXILLibrary(&bcLib);
        pLib->DefineExport(L"Root");
        pLib->DefineExport(L"Materials");
        pLib->DefineExport(L"MeshNodesLocalRS");

        // Add pixel shaders
        auto pPS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        CD3DX12_SHADER_BYTECODE bcPS(pixelShader.Get());
        pPS->SetDXILLibrary(&bcPS); // by not listing exports, 
                                    // just taking whatever is in the library

        auto pPS2 = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        CD3DX12_SHADER_BYTECODE bcPS2(pixelShader2.Get());
        pPS2->SetDXILLibrary(&bcPS2);

        // Associate global root signature with pixel shaders 
        // (the only shaders that reference it).
        //
        // This is for illustration and commented out because the global root 
        // signature added above with no associations means it gets associated 
        // to all exports by default, which is just fine in this sample.
        // 
        //auto pGlobalRSAssoc = 
        //    SODesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        //pGlobalRSAssoc->SetSubobjectToAssociate(*pGlobalRootSig);
        //pGlobalRSAssoc->AddExport(L"PSMain");
        //pGlobalRSAssoc->AddExport(L"PSMain2");

        // Associate dxil local root signature with pixel shaders 
        // (the only shaders that reference it)
        auto pLocalRSAssoc = 
            SODesc.CreateSubobject<CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION>();
        pLocalRSAssoc->SetSubobjectNameToAssociate(L"MeshNodesLocalRS");
        // Could omit the lines below and it would associate with all exports,
        // which would be just fine in this sample
        pLocalRSAssoc->AddExport(L"PSMain");
        pLocalRSAssoc->AddExport(L"PSMain2");

        // Add necessary building block subobjects for the mesh nodes
        auto pPrimitiveTopology = 
            SODesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
        pPrimitiveTopology->SetPrimitiveTopologyType(
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        auto pRTFormats = 
            SODesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
        pRTFormats->SetNumRenderTargets(1);
        pRTFormats->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
        // Don't need to add the following descs since they're all just default
        //auto pRast = SODesc.CreateSubobject<CD3DX12_RASTERIZER_SUBOBJECT>();
        //auto pBlend = SODesc.CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();
        //auto pDepth = SODesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_DESC2>();
        //auto pSampleMask = SODesc.CreateSubobject<CD3DX12_SAMPLE_MASK_SUBOBJECT>();
        //auto pSampleDesc = SODesc.CreateSubobject<CD3DX12_SAMPLE_DESC_SUBOBJECT>();

        // Define generic programs out of the building blocks:
        // name, list of shaders, list of subobjects:
        // (Can define multiple generic programs in the same state object, 
        // each picking the building blocks it wants)
        auto pGenericProgram = 
            SODesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
        pGenericProgram->SetProgramName(L"myMeshNode0");
        pGenericProgram->AddExport(L"Materials");
        pGenericProgram->AddExport(L"PSMain");
        pGenericProgram->AddSubobject(*pPrimitiveTopology);
        pGenericProgram->AddSubobject(*pRTFormats);

        // Notice the root signature isn't added to the list here.  
        // Root signatures are associated with shader exports directly, not programs.  

        // Second mesh node definition with just a different pixel shader
        auto pGenericProgram2 = 
            SODesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
        pGenericProgram2->SetProgramName(L"myMeshNode1");
        pGenericProgram2->AddExport(L"Materials");
        pGenericProgram2->AddExport(L"PSMain2");
        pGenericProgram2->AddSubobject(*pPrimitiveTopology);
        pGenericProgram2->AddSubobject(*pRTFormats);

        // Define a work graph
        auto pWorkGraph = 
            SODesc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
        pWorkGraph->SetProgramName(L"myWorkGraph");
        pWorkGraph->IncludeAllAvailableNodes();

        // Add root node to work graph
        auto pRootNode = pWorkGraph->CreateShaderNode(L"Root");

        // Add array of 3 nodes: {"MaterialID",0/1/2}

        auto pMeshNode0 = 
            pWorkGraph->CreateProgramNode(L"myMeshNode0");

        // Second node uses a different program "myMeshNode1" 
        // and different local root argument data
        auto pMeshNode1 = 
            pWorkGraph->CreateCommonProgramNodeOverrides(L"myMeshNode1");
        pMeshNode1->NewName({ L"Materials",1 });
        pMeshNode1->LocalRootArgumentsTableIndex(1);

        // Third node uses the same program as the previous, 
        // only difference being local root argument data
        auto pMeshNode2 = 
            pWorkGraph->CreateCommonProgramNodeOverrides(L"myMeshNode1");
        pMeshNode2->NewName({ L"Materials",2 });
        pMeshNode2->LocalRootArgumentsTableIndex(2);

        ThrowIfFailed(
            m_device->CreateStateObject(SODesc, 
                IID_PPV_ARGS(&m_stateObject)));

        // Define local root argument data
        struct MeshNodesLocalStruct {
            float blueChannel;
        };

        MeshNodesLocalStruct LocalRootArgs[3] = { 0.f, 0.5f, 1.f };

        UINT MaxInputRecords = 10; // max input records per DispatchGraph call
        UINT MaxNodeInputs = 3; // max node inputs per DispatchGraph call 
                                // (that the records are split across)
        InitWorkGraphContext(
            &m_workGraphContext, 
            m_stateObject.Get(), 
            L"myWorkGraph", 
            LocalRootArgs, 
            _countof(LocalRootArgs)*sizeof(MeshNodesLocalStruct), 
            MaxInputRecords,
            MaxNodeInputs);
    }

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());
}

// Update frame-based values.
void D3D12HelloMeshNodes::OnUpdate()
{
}

// Render the scene.
void D3D12HelloMeshNodes::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void D3D12HelloMeshNodes::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloMeshNodes::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_globalRootSignature.Get());
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), 
            D3D12_RESOURCE_STATE_PRESENT, 
            D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), 
        m_frameIndex, 
        m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    m_commandList->RSSetViewports(1, &m_viewport);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    float greenChannel = 0.5f;
    m_commandList->SetGraphicsRoot32BitConstant(0, *(UINT*)&greenChannel, 0);

    D3D12_SET_PROGRAM_DESC SP;
    SP.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
    SP.WorkGraph.BackingMemory = m_workGraphContext.BackingMemory;
    SP.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
    SP.WorkGraph.ProgramIdentifier = m_workGraphContext.hWorkGraph;
    SP.WorkGraph.NodeLocalRootArgumentsTable = 
        m_workGraphContext.LocalRootArgumentsTable;
    m_commandList->SetProgram(&SP);

    struct BinningRecord
    {
        uint16_t materialID;
        float position[4];
        float redChannel;
    };

    struct MeshNodeRecord
    {
        float position[4];
        float redChannel;
    };


    std::vector<BinningRecord> rootInputRecord;

    // Send some records to the root of the graph

    rootInputRecord.push_back({0, {-0.5f, 0.2f, 0.0f, 1.0f}, 0.0f});
    rootInputRecord.push_back({0, {-0.5f, 0.0f, 0.0f, 1.0f}, 0.5f});
    rootInputRecord.push_back({0, {-0.5f, -0.2f, 0.0f, 1.0f}, 1.0f});
    UINT totalRecords = (UINT)rootInputRecord.size();

    std::vector<D3D12_NODE_CPU_INPUT> multiNodeInput(3);
    assert(m_workGraphContext.spWGProps->GetEntrypointIndex(
        m_workGraphContext.WorkGraphIndex, 
        { L"Root",0 }) == 0);

    multiNodeInput[0].EntrypointIndex = 0;
    multiNodeInput[0].NumRecords = (UINT)rootInputRecord.size();
    multiNodeInput[0].pRecords = rootInputRecord.data();
    multiNodeInput[0].RecordStrideInBytes = sizeof(BinningRecord);

    std::vector<MeshNodeRecord> meshInputRecord[2];

    // Send some records directly to a couple of the mesh nodes

    meshInputRecord[0].push_back({ {0.f, 0.2f, 0.0f, 1.0f}, 0.f });
    meshInputRecord[0].push_back({ {0.f, 0.0f, 0.0f, 1.0f}, 0.5f });
    meshInputRecord[0].push_back({ {0.f, -0.2f, 0.0f, 1.0f}, 1.f });
    totalRecords += (UINT)meshInputRecord[0].size();

    meshInputRecord[1].push_back({ {0.5f, 0.2f, 0.0f, 1.0f}, 0.f });
    meshInputRecord[1].push_back({ {0.5f, 0.0f, 0.0f, 1.0f}, 0.5f });
    meshInputRecord[1].push_back({ {0.5f, -0.2f, 0.0f, 1.0f}, 1.0f });
    totalRecords += (UINT)meshInputRecord[1].size();

    for (UINT i = 0; i < 2; i++)
    {
        assert(m_workGraphContext.spWGProps->GetEntrypointIndex(
            m_workGraphContext.WorkGraphIndex, 
            { L"Materials",i+1 }) == i+2);
        multiNodeInput[i+1].EntrypointIndex = i+2;
        multiNodeInput[i+1].NumRecords = (UINT)meshInputRecord[i].size();
        multiNodeInput[i+1].pRecords = meshInputRecord[i].data();
        multiNodeInput[i+1].RecordStrideInBytes = sizeof(MeshNodeRecord);
    }

    assert(totalRecords <= m_workGraphContext.MaxInputRecords);
    assert((UINT)multiNodeInput.size() <= m_workGraphContext.MaxInputRecords);

    // Kick off work graph
    D3D12_DISPATCH_GRAPH_DESC DG;
    DG.Mode = D3D12_DISPATCH_MODE_MULTI_NODE_CPU_INPUT;
    DG.MultiNodeCPUInput.NumNodeInputs = (UINT)multiNodeInput.size();
    DG.MultiNodeCPUInput.pNodeInputs = multiNodeInput.data();
    DG.MultiNodeCPUInput.NodeInputStrideInBytes = sizeof(D3D12_NODE_CPU_INPUT);

    m_commandList->DispatchGraph(&DG);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(), 
            D3D12_RESOURCE_STATE_RENDER_TARGET, 
            D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloMeshNodes::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12HelloMeshNodes::FlushAndFinish()
{
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
    m_fenceValue++;

    DWORD waitResult = WaitForSingleObject(m_fenceEvent, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        ThrowIfFailed(E_FAIL);
    }
    ThrowIfFailed(m_device->GetDeviceRemovedReason());

    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
}

void D3D12HelloMeshNodes::MakeBuffer(
    ID3D12Resource** ppResource,
    UINT64 SizeInBytes,
    D3D12_RESOURCE_FLAGS ResourceMiscFlags,
    D3D12_HEAP_TYPE HeapType)
{
    CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(SizeInBytes);
    rd.Flags = ResourceMiscFlags;
    CD3DX12_HEAP_PROPERTIES hp(HeapType);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &hp,
        D3D12_HEAP_FLAG_NONE,
        &rd,
        D3D12_RESOURCE_STATE_COMMON,
        NULL,
        __uuidof(ID3D12Resource),
        (void**)ppResource));
}

void D3D12HelloMeshNodes::UploadData(
    ID3D12Resource* pResource,
    const VOID* pData,
    SIZE_T Size,
    ID3D12Resource** ppStagingResource, // only used if doFlush == false
    D3D12_RESOURCE_STATES CurrentState,
    bool doFlush)
{
    CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_UPLOAD);
    UINT64 IntermediateSize = GetRequiredIntermediateSize(pResource, 0, 1);
    if (Size != IntermediateSize)
    {
        OutputDebugStringA(
            "Provided Size of pData needs to account for the whole buffer (i.e. equal to GetRequiredIntermediateSize() output)");
        throw E_FAIL;
    }

    CD3DX12_RESOURCE_DESC BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(IntermediateSize);
    CComPtr<ID3D12Resource> pStagingResource;
    if (ppStagingResource == nullptr)
    {
        ppStagingResource = &pStagingResource;
    }
    ThrowIfFailed(m_device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &BufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(ppStagingResource)));

    bool NeedTransition = (CurrentState & D3D12_RESOURCE_STATE_COPY_DEST) == 0;
    D3D12_RESOURCE_BARRIER BarrierDesc; ZeroMemory(&BarrierDesc, sizeof(BarrierDesc));
    if (NeedTransition)
    {
        // Transition to COPY_DEST
        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = pResource;
        BarrierDesc.Transition.Subresource = 0;
        BarrierDesc.Transition.StateBefore = CurrentState;
        BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        m_commandList->ResourceBarrier(1, &BarrierDesc);
        std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter); // ensure StateBefore represents current state
    }

    // Execute upload
    D3D12_SUBRESOURCE_DATA SubResourceData = { pData, static_cast<LONG_PTR>(Size), static_cast<LONG_PTR>(Size) };
    if (Size != UpdateSubresources(m_commandList.Get(), pResource, *ppStagingResource, 0, 0, 1, &SubResourceData))
    {
        OutputDebugStringA("UpdateSubresources returns the number of bytes updated, so 0 if nothing was updated");
        throw E_FAIL;
    }
    if (NeedTransition)
    {
        // Transition back to whatever the app had
        m_commandList->ResourceBarrier(1, &BarrierDesc);
        std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter); // ensure StateBefore represents current state
    }
    if (doFlush == true)
    {
        // Finish Upload
        FlushAndFinish();
    }
}

void D3D12HelloMeshNodes::MakeBufferAndInitialize(
    ID3D12Resource** ppResource,
    const VOID* pInitialData,
    UINT64 SizeInBytes,
    ID3D12Resource** ppStagingResource, // only used if doFlush == false
    bool doFlush,
    D3D12_RESOURCE_FLAGS ResourceMiscFlags)
{
    MakeBuffer(ppResource, SizeInBytes, ResourceMiscFlags, D3D12_HEAP_TYPE_DEFAULT);
    UploadData(*ppResource, pInitialData, SizeInBytes, ppStagingResource, D3D12_RESOURCE_STATE_COMMON, doFlush);
}

HRESULT CompileDxilLibraryFromFile(
    _In_ LPCWSTR pFile,
    _In_ LPCWSTR pEntry,
    _In_ LPCWSTR pTarget,
    _In_reads_(cArgs) LPCWSTR args[],
    _In_ UINT cArgs,
    _In_reads_(cDefines) DxcDefine* pDefines,
    _In_ UINT cDefines,
    _Out_ ID3DBlob** ppCode)
{
    HRESULT hr = S_OK;
    *ppCode = nullptr;

    static HMODULE s_hmod = 0;
    static DxcCreateInstanceProc s_pDxcCreateInstanceProc = nullptr;
    if (s_hmod == 0)
    {
        s_hmod = LoadLibrary(L"dxcompiler.dll");
        if (s_hmod == 0)
        {
            OutputDebugStringA("dxcompiler.dll missing or wrong architecture");
            return E_FAIL;
        }

        if (s_pDxcCreateInstanceProc == nullptr)
        {
            s_pDxcCreateInstanceProc = (DxcCreateInstanceProc)GetProcAddress(s_hmod, "DxcCreateInstance");
            if (s_pDxcCreateInstanceProc == nullptr)
            {
                OutputDebugStringA("Unable to find dxcompiler!DxcCreateInstance");
                return E_FAIL;
            }
        }
    }

    ComPtr<IDxcCompiler> compiler;
    ComPtr<IDxcLibrary> library;
    ComPtr<IDxcBlobEncoding> source;
    ComPtr<IDxcOperationResult> operationResult;
    ComPtr<IDxcIncludeHandler> includeHandler;
    hr = s_pDxcCreateInstanceProc(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to instantiate compiler.");
        return hr;
    }

    HRESULT createBlobHr = library->CreateBlobFromFile(pFile, nullptr, &source);
    if (createBlobHr != S_OK)
    {
        OutputDebugStringA("Create Blob From File Failed - perhaps file is missing?");
        return E_FAIL;
    }

    hr = library->CreateIncludeHandler(&includeHandler);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create include handler.");
        return hr;
    }
    hr = s_pDxcCreateInstanceProc(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to instantiate compiler.");
        return hr;
    }

    hr = compiler->Compile(
        source.Get(),
        nullptr,
        pEntry,
        pTarget,
        args, cArgs,
        pDefines, cDefines,
        includeHandler.Get(),
        &operationResult);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to compile.");
        return hr;
    }

    operationResult->GetStatus(&hr);
    if (SUCCEEDED(hr))
    {
        hr = operationResult->GetResult((IDxcBlob**)ppCode);
        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to retrieve compiled code.");
        }
    }
    CComPtr<IDxcBlobEncoding> pErrors;
    if (SUCCEEDED(operationResult->GetErrorBuffer(&pErrors)))
    {
        OutputDebugStringA((LPCSTR)pErrors->GetBufferPointer());
    }

    return hr;
}
