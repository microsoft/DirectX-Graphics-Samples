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

// This sample demonstrates "partial graphics programs": compile a vertex
// shader and a pixel shader exactly once, then late-link three different
// blend states to produce three pipeline programs that share the same
// shader code. The result is three overlapping translucent triangles
// rendered in a single frame, each drawn with a different blend mode:
//
//   * red   triangle (left)   -> opaque blend
//   * green triangle (center) -> additive blend
//   * blue  triangle (right)  -> source-alpha / inv-source-alpha blend

#include "stdafx.h"
#include "D3D12HelloPartialGraphicsPrograms.h"
#include <windows.h>
#include "dxcapi.h"
#include <iostream>
#include <atlbase.h>

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 721; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

// Helper for compiling an HLSL file into a DXIL library blob using dxcompiler.dll.
// Useful when populating CD3DX12_DXIL_LIBRARY_SUBOBJECTs inside a state object.
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

D3D12HelloPartialGraphicsPrograms::D3D12HelloPartialGraphicsPrograms(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{
}

void D3D12HelloPartialGraphicsPrograms::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12HelloPartialGraphicsPrograms::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

    // Partial graphics programs are exposed via the D3D12 state objects
    // experiment. Enable it before any device is created.
    UUID experimentalFeatures[] = { D3D12StateObjectsExperiment };
    if (FAILED(D3D12EnableExperimentalFeatures(_countof(experimentalFeatures), experimentalFeatures, nullptr, nullptr)))
    {
        OutputDebugStringA("Failed to enable D3D12StateObjectsExperiment, required for partial graphics programs.");
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

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel;
    shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_8;
    ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)));
    if (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_8)
    {
        OutputDebugStringA("Partial Graphics Programs require a device with shader model 6.8 support (though the shaders used don't have to be 6.8 shaders).");
        ThrowIfFailed(E_FAIL);
    }


    // Check partial graphics program support.
    D3D12_FEATURE_DATA_PARTIAL_GRAPHICS_PROGRAMS partialGraphicsProgramTier = {};
    ThrowIfFailed(m_device->CheckFeatureSupport(
        D3D12_FEATURE_PARTIAL_GRAPHICS_PROGRAMS, &partialGraphicsProgramTier, sizeof(partialGraphicsProgramTier)));
    if (partialGraphicsProgramTier.PartialGraphicsProgramsTier < D3D12_PARTIAL_GRAPHICS_PROGRAMS_TIER_1_0)
    {
        OutputDebugStringA("Partial Graphics Programs Tier 1.0 is required.");
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
void D3D12HelloPartialGraphicsPrograms::LoadAssets()
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

    // -------------------------------------------------------------------------
    // Partial graphics programs split across:
    //   1. A collection state object holding the shaders and the two partial
    //      programs (VSPartial + PSPartial). These are compiled once.
    //   2. An executable state object that references that collection and
    //      late-links the first two blend permutations (opaque + additive)
    //      into two generic programs.
    //   3. An AddToStateObject call that adds the third blend permutation
    //      (alpha-blend) as a third generic program -- still no shader code
    //      is recompiled, only the link step runs for the added program.
    //
    // See:
    //   https://github.com/microsoft/DirectX-Specs/blob/master/d3d/PartialGraphicsPrograms.md
    // -------------------------------------------------------------------------
    {
        // Compile VS + PS once.
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(),
            L"VSMain", L"vs_6_0", nullptr, 0, &vertexShader));
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(),
            L"PSMain", L"ps_6_0", nullptr, 0, &pixelShader));

        // Three blend descs (the only state that varies across the three
        // generic programs we link).
        auto MakeBlend = [](BOOL enable, D3D12_BLEND src, D3D12_BLEND dst)
        {
            D3D12_BLEND_DESC b = {};
            b.RenderTarget[0].BlendEnable           = enable;
            b.RenderTarget[0].SrcBlend              = src;
            b.RenderTarget[0].DestBlend             = dst;
            b.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
            b.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
            b.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
            b.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
            b.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
            b.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            return b;
        };
        const D3D12_BLEND_DESC blendDescs[NumBlendPermutations] =
        {
            MakeBlend(FALSE, D3D12_BLEND_ONE,       D3D12_BLEND_ZERO),         // opaque
            MakeBlend(TRUE,  D3D12_BLEND_ONE,       D3D12_BLEND_ONE),          // additive
            MakeBlend(TRUE,  D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA),// alpha
        };
        const LPCWSTR programNames[NumBlendPermutations] =
        {
            L"OpaqueProgram", L"AdditiveProgram", L"AlphaProgram"
        };

        // Input layout, shared by the pre-rasterization partial program.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // ---------------------------------------------------------------------
        // 1. Collection state object: shaders + VSPartial + PSPartial.
        //    Partial graphics programs in a collection are compiled when
        //    the collection is created, so the executable state object that
        //    references it only has to perform a cheap link step.
        // ---------------------------------------------------------------------
        {
            CD3DX12_STATE_OBJECT_DESC collectionDesc;
            collectionDesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_COLLECTION);

            auto pConfig = collectionDesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
            pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

            auto pRootSig = collectionDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
            pRootSig->SetRootSignature(m_rootSignature.Get());

            auto pVS = collectionDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            CD3DX12_SHADER_BYTECODE bcVS(vertexShader.Get());
            pVS->SetDXILLibrary(&bcVS);

            auto pPS = collectionDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            CD3DX12_SHADER_BYTECODE bcPS(pixelShader.Get());
            pPS->SetDXILLibrary(&bcPS);

            auto pIL = collectionDesc.CreateSubobject<CD3DX12_INPUT_LAYOUT_SUBOBJECT>();
            for (UINT i = 0; i < _countof(inputElementDescs); ++i)
            {
                pIL->AddInputLayoutElementDesc(inputElementDescs[i]);
            }

            auto pTopology = collectionDesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
            pTopology->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

            auto pRTFormats = collectionDesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
            pRTFormats->SetNumRenderTargets(1);
            pRTFormats->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

            // Pixel shader partial program fields: blend will be late linked.
            // AlphaToCoverageEnable and DualSourceBlendEnable affect PS
            // compilation, so they must be specified here (the late-linked
            // blend subobject must agree with these values).
            auto pPSFields = collectionDesc.CreateSubobject<CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT>();
            pPSFields->SetAlphaToCoverageEnable(FALSE);
            pPSFields->SetDualSourceBlendEnable(FALSE);
            pPSFields->SetLateLinkBlendSubobject(TRUE);

            auto pPreRastProgram = collectionDesc.CreateSubobject<CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT>();
            pPreRastProgram->SetProgramName(L"VSPartial");
            pPreRastProgram->SetPartialGraphicsProgramType(D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE_PRERASTERIZATION_SHADER);
            pPreRastProgram->AddExport(L"VSMain");
            pPreRastProgram->AddSubobject(*pIL);
            pPreRastProgram->AddSubobject(*pTopology);

            auto pPSProgram = collectionDesc.CreateSubobject<CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT>();
            pPSProgram->SetProgramName(L"PSPartial");
            pPSProgram->SetPartialGraphicsProgramType(D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE_PIXEL_SHADER);
            pPSProgram->AddExport(L"PSMain");
            pPSProgram->AddSubobject(*pRTFormats);
            pPSProgram->AddSubobject(*pPSFields);

            ThrowIfFailed(m_device->CreateStateObject(collectionDesc, IID_PPV_ARGS(&m_collection)));
        }

        // ---------------------------------------------------------------------
        // 2. Executable state object containing the first two blend
        //    permutations. Allows additions so we can append the third later.
        // ---------------------------------------------------------------------
        {
            CD3DX12_STATE_OBJECT_DESC executableDesc;
            executableDesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

            auto pConfig = executableDesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
            pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

            auto pCollectionRef = executableDesc.CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
            pCollectionRef->SetExistingCollection(m_collection.Get());

            for (UINT i = 0; i < 2; ++i)
            {
                auto pBlend = executableDesc.CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();
                static_cast<D3D12_BLEND_DESC&>(*pBlend) = blendDescs[i];

                auto pProgram = executableDesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
                pProgram->SetProgramName(programNames[i]);
                pProgram->AddExport(L"VSPartial");
                pProgram->AddExport(L"PSPartial");
                pProgram->AddSubobject(*pBlend);
            }

            ThrowIfFailed(m_device->CreateStateObject(executableDesc, IID_PPV_ARGS(&m_stateObject[0])));
        }

        // ---------------------------------------------------------------------
        // 3. AddToStateObject: append the third blend permutation. The result
        //    is a new state object handle that is a superset of the first;
        //    program identifiers from m_stateObject[0] remain valid.
        // ---------------------------------------------------------------------
        {
            CD3DX12_STATE_OBJECT_DESC additionDesc;
            additionDesc.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

            auto pConfig = additionDesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
            pConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

            auto pBlend = additionDesc.CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();
            static_cast<D3D12_BLEND_DESC&>(*pBlend) = blendDescs[2];

            auto pProgram = additionDesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
            pProgram->SetProgramName(programNames[2]);
            pProgram->AddExport(L"VSPartial");
            pProgram->AddExport(L"PSPartial");
            pProgram->AddSubobject(*pBlend);

            ThrowIfFailed(m_device->AddToStateObject(additionDesc, m_stateObject[0].Get(), IID_PPV_ARGS(&m_stateObject[1])));
        }

        // Query program identifiers. The first two come from the original
        // executable state object; the third comes from the one produced by
        // AddToStateObject.
        ComPtr<ID3D12StateObjectProperties1> baseProps;
        ThrowIfFailed(m_stateObject[0]->QueryInterface(IID_PPV_ARGS(&baseProps)));
        m_blendPrograms[0] = baseProps->GetProgramIdentifier(programNames[0]);
        m_blendPrograms[1] = baseProps->GetProgramIdentifier(programNames[1]);

        ComPtr<ID3D12StateObjectProperties1> addedProps;
        ThrowIfFailed(m_stateObject[1]->QueryInterface(IID_PPV_ARGS(&addedProps)));
        m_blendPrograms[2] = addedProps->GetProgramIdentifier(programNames[2]);
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    // Create the vertex buffer with three overlapping translucent triangles.
    // Each triangle is ~0.4 NDC tall / wide. They are offset along X so that
    // they visibly overlap in the middle of the window, which makes the per-
    // triangle blend modes (opaque / additive / alpha) easy to see at a glance.
    {
        const float kAlpha = 0.7f;
        // Per-triangle horizontal offsets in NDC.
        const float xOffset[NumBlendPermutations] = { -0.30f, 0.0f, 0.30f };
        const XMFLOAT4 triColor[NumBlendPermutations] =
        {
            { 1.0f, 0.0f, 0.0f, kAlpha }, // opaque (red)
            { 0.0f, 1.0f, 0.0f, kAlpha }, // additive (green)
            { 0.0f, 0.0f, 1.0f, kAlpha }, // alpha blend (blue)
        };

        Vertex triangleVertices[NumBlendPermutations * 3];
        for (UINT i = 0; i < NumBlendPermutations; ++i)
        {
            const float x = xOffset[i];
            triangleVertices[i * 3 + 0] = { { x,           0.30f * m_aspectRatio, 0.0f }, triColor[i] };
            triangleVertices[i * 3 + 1] = { { x + 0.25f, -0.30f * m_aspectRatio, 0.0f }, triColor[i] };
            triangleVertices[i * 3 + 2] = { { x - 0.25f, -0.30f * m_aspectRatio, 0.0f }, triColor[i] };
        }

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
void D3D12HelloPartialGraphicsPrograms::OnUpdate()
{
}

// Render the scene.
void D3D12HelloPartialGraphicsPrograms::OnRender()
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

void D3D12HelloPartialGraphicsPrograms::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloPartialGraphicsPrograms::PopulateCommandList()
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
    m_commandList->RSSetViewports(1, &m_viewport);
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

    // -------------------------------------------------------------------------
    // Draw each triangle with its own blend-permutation program. The three
    // program identifiers were produced by linking the same VS + PS partial
    // programs with three different blend subobjects in LoadAssets().
    // -------------------------------------------------------------------------
    for (UINT i = 0; i < NumBlendPermutations; ++i)
    {
        D3D12_SET_PROGRAM_DESC sp = {};
        sp.Type = D3D12_PROGRAM_TYPE_GENERIC_PIPELINE;
        sp.GenericPipeline.ProgramIdentifier = m_blendPrograms[i];
        m_commandList->SetProgram(&sp);
        m_commandList->DrawInstanced(3, 1, i * 3, 0);
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloPartialGraphicsPrograms::WaitForPreviousFrame()
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
