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
// generic programs in state objects to define pipelines (programs) instaed of
// the previously existing PSO path. 
// 
// This sample also shows how to add an additional program permutation to an existing
// state object using the AddToStateObject() API.  The addition is the program used for shading
// a second triangle.
// 
// The interesting functions to compare vs D3D12HelloTriangle are:
// LoadPipeline() - PSO equivalent shown commented out in there;
// and PopulateCommandList() - SetProgram() is called instead of SetPipelineState()
// 

#include "stdafx.h"
#include "D3D12HelloGenericPrograms.h"
#include <windows.h>
#include "dxcapi.h"
#include <iostream>
#include <atlbase.h>

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 618; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

HRESULT CompileDxilLibraryFromFile(
    _In_ LPCWSTR pFile,
    _In_ LPCWSTR pEntry,
    _In_ LPCWSTR pTarget,
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

    CComPtr<IDxcCompiler> compiler;
    CComPtr<IDxcLibrary> library;
    CComPtr<IDxcBlobEncoding> source;
    CComPtr<IDxcOperationResult> operationResult;
    CComPtr<IDxcIncludeHandler> includeHandler;
    hr = s_pDxcCreateInstanceProc(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<LPVOID*>(&library));
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
    hr = s_pDxcCreateInstanceProc(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<LPVOID*>(&compiler));
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to instantiate compiler.");
        return hr;
    }

    LPCWSTR args[] = { L"" };
    UINT cArgs = 0;
    hr = compiler->Compile(
        source,
        nullptr,
        pEntry,
        pTarget,
        args, cArgs,
        pDefines, cDefines,
        includeHandler,
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

D3D12HelloGenericPrograms::D3D12HelloGenericPrograms(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{
    m_viewport[0] = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>(width/2), static_cast<float>(height) );
    m_viewport[1] = CD3DX12_VIEWPORT((float)width/2, 0.0f, static_cast<float>(width/2), static_cast<float>(height));

}

void D3D12HelloGenericPrograms::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12HelloGenericPrograms::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

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

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel;
    shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_8;
    ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)));
    if (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_8)
    {
        OutputDebugStringA("Generic Programs require a device with shader model 6.8 support (though the shaders used don't have be 6.8 shaders).");
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
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

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

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void D3D12HelloGenericPrograms::LoadAssets()
{
    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create a generic program (like a PSO) in a state object, including first compiling shaders
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

        ThrowIfFailed(CompileDxilLibraryFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), L"VSMain", L"vs_6_0", nullptr, 0, &vertexShader));
        ThrowIfFailed(CompileDxilLibraryFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), L"PSMain", L"ps_6_0", nullptr, 0, &pixelShader));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline as a generic program.

        /* Here is what it would have looked like as a PSO:
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
        */

        CD3DX12_STATE_OBJECT_DESC SODesc;
        SODesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

        // Optional flag to allow state object additions
        auto pConfig = SODesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
        pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

        // Define the building blocks for the program - the individual subobjects / shaders
        auto pIL = SODesc.CreateSubobject<CD3DX12_INPUT_LAYOUT_SUBOBJECT>();

        for (UINT i = 0; i < _countof(inputElementDescs); i++)
        {
            pIL->AddInputLayoutElementDesc(inputElementDescs[i]);
        }
        auto pRootSig = SODesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        pRootSig->SetRootSignature(m_rootSignature.Get());
        auto pVS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        CD3DX12_SHADER_BYTECODE bcVS(vertexShader.Get());
        pVS->SetDXILLibrary(&bcVS);
        pVS->DefineExport(L"myRenamedVS", L"*"); // Take whatever the shader is and rename it to myVS.  Instead of "*" could have used the actual name of the shader.
                                          // Also could have omitted this line completely, which would just import all exports in the binary (just one shader here), 
                                          // using the name of the shader in the lib. Could also have listed the name of the shader in the lib on its own for the same effect
                                          // (would ensure that the shader you are expecting is actually there)

        auto pPS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        CD3DX12_SHADER_BYTECODE bcPS(pixelShader.Get());
        pPS->SetDXILLibrary(&bcPS); // by not listing exports, just taking whatever is in the library

        // Don't need to add the following descs since they're all just default
        //auto pRast = SODesc.CreateSubobject<CD3DX12_RASTERIZER_SUBOBJECT>();
        //auto pBlend = SODesc.CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();
        //auto pDepth = SODesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_DESC2>();
        //auto pSampleMask = SODesc.CreateSubobject<CD3DX12_SAMPLE_MASK_SUBOBJECT>();
        //auto pSampleDesc = SODesc.CreateSubobject<CD3DX12_SAMPLE_DESC_SUBOBJECT>();
        auto pPrimitiveTopology = SODesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
        pPrimitiveTopology->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        auto pRTFormats = SODesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
        pRTFormats->SetNumRenderTargets(1);
        pRTFormats->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

        // Then define a generic program out of the building blocks:  name, list of shaders, list of subobjects:
        // (Can define multiple generic programs in the same state object, each picking the building blocks it wants)

        auto pGenericProgram = SODesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
        pGenericProgram->SetProgramName(L"myGenericProgram");
        pGenericProgram->AddExport(L"myRenamedVS");
        pGenericProgram->AddExport(L"PSMain");
        pGenericProgram->AddSubobject(*pIL);
        pGenericProgram->AddSubobject(*pPrimitiveTopology);
        pGenericProgram->AddSubobject(*pRTFormats);
        // Notice the root signature isn't being added to the list here.  Root signatures are associated with
        // shader exports directly, not programs.  The single root sig in the state objcet above with no associations defined automatically
        // becomes a default root sig that applies to all exports, so myVS and myPS get it.

        ThrowIfFailed(m_device->CreateStateObject(SODesc, IID_PPV_ARGS(&m_stateObject[0])));

        ComPtr<ID3D12StateObjectProperties1> pSOProperties;
        ThrowIfFailed(m_stateObject[0]->QueryInterface(IID_PPV_ARGS(&pSOProperties)));
        m_genericProgram[0] = pSOProperties->GetProgramIdentifier(L"myGenericProgram");
    }

    // Add an additional program thatMake an additional permutationCreate the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> pixelShader2;

        ThrowIfFailed(CompileDxilLibraryFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), L"PSMain2", L"ps_6_0", nullptr, 0, &pixelShader2));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        CD3DX12_STATE_OBJECT_DESC SODesc;
        SODesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

        // Optional flag to allow state object additions
        auto pConfig = SODesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
        pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

        // First define the building blocks for the program - the individual subobjects / shaders
        // Subobjects like input layout etc. need to be redefined, as there currently isn't a way to define them in DXIL such that 
        // they can be reused from an existing state object being added to.  Root signatures can be defined in DXIL and reused (by referering to them
        // by name), but that isn't shown here.
        auto pIL = SODesc.CreateSubobject<CD3DX12_INPUT_LAYOUT_SUBOBJECT>();

        for (UINT i = 0; i < _countof(inputElementDescs); i++)
        {
            pIL->AddInputLayoutElementDesc(inputElementDescs[i]);
        }
        auto pRootSig = SODesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        pRootSig->SetRootSignature(m_rootSignature.Get());

        // not listing any exports means what's in the bytecode will be used as is, which is "PSMain2"
        auto pPS = SODesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

        CD3DX12_SHADER_BYTECODE bcPS(pixelShader2.Get());
        pPS->SetDXILLibrary(&bcPS);

        auto pPrimitiveTopology = SODesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
        pPrimitiveTopology->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        auto pRTFormats = SODesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
        pRTFormats->SetNumRenderTargets(1);
        pRTFormats->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);


        auto pGenericProgram = SODesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
        pGenericProgram->SetProgramName(L"myGenericProgram2");
        pGenericProgram->AddExport(L"myRenamedVS");
        pGenericProgram->AddExport(L"PSMain2");
        pGenericProgram->AddSubobject(*pIL);
        pGenericProgram->AddSubobject(*pPrimitiveTopology);
        pGenericProgram->AddSubobject(*pRTFormats);
        // Notice the root signature isn't being added to the list here.  Root signatures are associated with
        // shader exports directly, not programs.  The single root sig in the state objcet above with no associations defined automatically
        // becomes a default root sig that applies to all exports, so myVS and myPS get it.

        ThrowIfFailed(m_device->AddToStateObject(SODesc, m_stateObject[0].Get(), IID_PPV_ARGS(&m_stateObject[1])));

        ComPtr<ID3D12StateObjectProperties1> pSOProperties;
        ThrowIfFailed(m_stateObject[1]->QueryInterface(IID_PPV_ARGS(&pSOProperties)));
        m_genericProgram[1] = pSOProperties->GetProgramIdentifier(L"myGenericProgram2");
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}

// Update frame-based values.
void D3D12HelloGenericPrograms::OnUpdate()
{
}

// Render the scene.
void D3D12HelloGenericPrograms::OnRender()
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

void D3D12HelloGenericPrograms::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloGenericPrograms::PopulateCommandList()
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
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    for (UINT i = 0; i < 2; i++)
    {
        D3D12_SET_PROGRAM_DESC SP;
        SP.Type = D3D12_PROGRAM_TYPE_GENERIC_PIPELINE;
        SP.GenericPipeline.ProgramIdentifier = m_genericProgram[i];
        m_commandList->SetProgram(&SP);
        m_commandList->RSSetViewports(1, &m_viewport[i]);
        m_commandList->DrawInstanced(3, 1, 0, 0);
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloGenericPrograms::WaitForPreviousFrame()
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
