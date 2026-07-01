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

// This sample demonstrates "partial graphics programs" driving a mesh shader:
// compile a mesh shader and a pixel shader exactly once, then late-link three
// different blend states to produce three pipeline programs that share the same
// shader code.
//
// To exercise both features at once, the mesh shader procedurally generates a
// solid, spinning 3D spur gear every frame - one thread group builds the whole
// gear (front/back faces, outer tooth wall, inner hub-hole wall) with per-vertex
// lighting, and no vertex or index buffer (mesh-shader geometry amplification).
// A depth buffer resolves the 3D self-occlusion. That same gear is drawn three
// times, layered front-to-back and overlapping, each time with a different
// late-linked blend mode (partial graphics programs):
//
//   * red   gear (left)   -> opaque blend
//   * green gear (center) -> additive blend
//   * blue  gear (right)  -> source-alpha / inv-source-alpha blend
//
// Unlike the vertex-shader based sibling sample, there is no input assembler or
// vertex buffer: the mesh shader (MSMain) is used as the pre-rasterization
// partial program and generates the geometry itself.

#include "stdafx.h"
#include "D3D12HelloMeshShaderPartialPrograms.h"
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

D3D12HelloMeshShaderPartialPrograms::D3D12HelloMeshShaderPartialPrograms(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0),
    m_rotation(0.0f)
{
}

void D3D12HelloMeshShaderPartialPrograms::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12HelloMeshShaderPartialPrograms::LoadPipeline()
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

    // Check mesh shader support. The pre-rasterization partial program used by
    // this sample is a mesh shader, so the device must support mesh shaders.
    D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
    ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)));
    if (options7.MeshShaderTier < D3D12_MESH_SHADER_TIER_1)
    {
        OutputDebugStringA("Mesh Shader Tier 1 is required.");
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

        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
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

    // Create the depth buffer and its view. The 3D gears are solids, so a depth
    // buffer is required for them to occlude themselves (and each other) correctly.
    {
        D3D12_CLEAR_VALUE depthClear = {};
        depthClear.Format = DXGI_FORMAT_D32_FLOAT;
        depthClear.DepthStencil.Depth = 1.0f;
        depthClear.DepthStencil.Stencil = 0;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClear,
            IID_PPV_ARGS(&m_depthStencil)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void D3D12HelloMeshShaderPartialPrograms::LoadAssets()
{
    // Create a root signature with three 32-bit root constants at b0. The mesh
    // shader reads them to select which disc to emit (index), to keep the disc
    // from stretching (aspect ratio), and to spin it (rotation angle). There is
    // no input assembler input layout because mesh shaders generate their own
    // geometry.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[1];
        rootParameters[0].InitAsConstants(3, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // -------------------------------------------------------------------------
    // Partial graphics programs split across:
    //   1. A collection state object holding the shaders and the two partial
    //      programs (MSPartial + PSPartial). These are compiled once.
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
        // Compile MS + PS once. The mesh shader must target shader model 6.5 or
        // higher; the pixel shader has no such requirement.
        ComPtr<ID3DBlob> meshShader;
        ComPtr<ID3DBlob> pixelShader;
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(),
            L"MSMain", L"ms_6_5", nullptr, 0, &meshShader));
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

        // ---------------------------------------------------------------------
        // 1. Collection state object: shaders + MSPartial + PSPartial.
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

            auto pMS = collectionDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            CD3DX12_SHADER_BYTECODE bcMS(meshShader.Get());
            pMS->SetDXILLibrary(&bcMS);

            auto pPS = collectionDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            CD3DX12_SHADER_BYTECODE bcPS(pixelShader.Get());
            pPS->SetDXILLibrary(&bcPS);

            auto pRTFormats = collectionDesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
            pRTFormats->SetNumRenderTargets(1);
            pRTFormats->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

            // A mesh-shader pre-rasterization partial program is required to
            // declare its output linkage signature: the non-system-value
            // outputs (here a single COLOR varying) that feed the pixel shader.
            auto pMSOutputLinkage = collectionDesc.CreateSubobject<CD3DX12_OUTPUT_LINKAGE_SIGNATURE_SUBOBJECT>();
            pMSOutputLinkage->AddOutputLinkageElementDesc({ "COLOR", 0, 0, 4 });

            // Pixel shader partial program fields: blend will be late linked.
            // AlphaToCoverageEnable and DualSourceBlendEnable affect PS
            // compilation, so they must be specified here (the late-linked
            // blend subobject must agree with these values).
            auto pPSFields = collectionDesc.CreateSubobject<CD3DX12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_SUBOBJECT>();
            pPSFields->SetAlphaToCoverageEnable(FALSE);
            pPSFields->SetDualSourceBlendEnable(FALSE);
            pPSFields->SetLateLinkBlendSubobject(TRUE);

            // Depth is used so the 3D gears occlude correctly. The depth-stencil
            // state and format are baked into the PS partial program (shared by
            // all three blend permutations); only blend is late linked. The
            // default depth-stencil desc enables depth test LESS with depth writes.
            auto pDepthFormat = collectionDesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT>();
            pDepthFormat->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);

            auto pDepthStencil = collectionDesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_SUBOBJECT>();
            pDepthStencil->SetDepthEnable(TRUE);
            pDepthStencil->SetDepthWriteMask(D3D12_DEPTH_WRITE_MASK_ALL);
            pDepthStencil->SetDepthFunc(D3D12_COMPARISON_FUNC_LESS);

            // Pre-rasterization partial program: the mesh shader. Mesh shaders
            // do not use an input layout, and their output topology is baked
            // into the shader, so no input-layout or primitive-topology
            // subobjects are added here.
            auto pMSProgram = collectionDesc.CreateSubobject<CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT>();
            pMSProgram->SetProgramName(L"MSPartial");
            pMSProgram->SetPartialGraphicsProgramType(D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE_PRERASTERIZATION_SHADER);
            pMSProgram->AddExport(L"MSMain");
            pMSProgram->AddSubobject(*pMSOutputLinkage);

            auto pPSProgram = collectionDesc.CreateSubobject<CD3DX12_PARTIAL_GRAPHICS_PROGRAM_SUBOBJECT>();
            pPSProgram->SetProgramName(L"PSPartial");
            pPSProgram->SetPartialGraphicsProgramType(D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE_PIXEL_SHADER);
            pPSProgram->AddExport(L"PSMain");
            pPSProgram->AddSubobject(*pRTFormats);
            pPSProgram->AddSubobject(*pPSFields);
            pPSProgram->AddSubobject(*pDepthFormat);
            pPSProgram->AddSubobject(*pDepthStencil);

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
                pProgram->AddExport(L"MSPartial");
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
            pProgram->AddExport(L"MSPartial");
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
void D3D12HelloMeshShaderPartialPrograms::OnUpdate()
{
    // Advance the rotation angle; the mesh shader rebuilds the gears from it
    // each frame. Wrap to keep the value bounded over long runs.
    m_rotation += 0.01f;
    if (m_rotation > XM_2PI)
    {
        m_rotation -= XM_2PI;
    }
}

// Render the scene.
void D3D12HelloMeshShaderPartialPrograms::OnRender()
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

void D3D12HelloMeshShaderPartialPrograms::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloMeshShaderPartialPrograms::PopulateCommandList()
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
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // -------------------------------------------------------------------------
    // Draw the gear three times, each with its own blend-permutation program.
    // The three program identifiers were produced by linking the same MS + PS
    // partial programs with three different blend subobjects in LoadAssets().
    // The mesh shader generates all of the geometry, so we dispatch it (there is
    // no input assembler / vertex buffer). Root constants select which gear to
    // build, provide the aspect ratio, and spin each gear (alternating direction
    // so neighbors turn opposite ways). The gears are layered in depth by
    // instance and drawn back-to-front so the blend compositing is correct.
    // -------------------------------------------------------------------------
    struct DrawConstants
    {
        UINT  instanceIndex;
        float aspectRatio;
        float rotation;
    } drawConstants;
    drawConstants.aspectRatio = m_aspectRatio;

    for (UINT i = 0; i < NumBlendPermutations; ++i)
    {
        drawConstants.instanceIndex = i;
        drawConstants.rotation = (i % 2 == 0) ? m_rotation : -m_rotation;
        m_commandList->SetGraphicsRoot32BitConstants(0, 3, &drawConstants, 0);

        D3D12_SET_PROGRAM_DESC sp = {};
        sp.Type = D3D12_PROGRAM_TYPE_GENERIC_PIPELINE;
        sp.GenericPipeline.ProgramIdentifier = m_blendPrograms[i];
        m_commandList->SetProgram(&sp);
        m_commandList->DispatchMesh(1, 1, 1);
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloMeshShaderPartialPrograms::WaitForPreviousFrame()
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
