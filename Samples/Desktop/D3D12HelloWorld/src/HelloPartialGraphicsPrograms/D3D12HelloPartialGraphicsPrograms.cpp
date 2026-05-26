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
    // Partial graphics programs: compile VS + PS once, then late-link three
    // blend permutations into three program identifiers that share the same
    // shader code.
    //
    // The block below is written against the API names in
    //   https://github.com/microsoft/DirectX-Specs/blob/master/d3d/PartialGraphicsPrograms.md
    // (D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM,
    //  D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC,
    //  D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS,
    //  D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS, etc.).
    // -------------------------------------------------------------------------
    {
        // First check device support.
        D3D12_FEATURE_DATA_PARTIAL_GRAPHICS_PROGRAMS pgp = {};
        ThrowIfFailed(m_device->CheckFeatureSupport(
            D3D12_FEATURE_PARTIAL_GRAPHICS_PROGRAMS, &pgp, sizeof(pgp)));
        if (pgp.PartialGraphicsProgramsTier < D3D12_PARTIAL_GRAPHICS_PROGRAMS_TIER_1_0)
        {
            OutputDebugStringA("Partial Graphics Programs Tier 1.0 is required.");
            ThrowIfFailed(E_FAIL);
        }

        // Compile VS + PS once.
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(),
            L"VSMain", L"vs_6_0", nullptr, 0, &vertexShader));
        ThrowIfFailed(CompileDxilLibraryFromFile(
            GetAssetFullPath(L"shaders.hlsl").c_str(),
            L"PSMain", L"ps_6_0", nullptr, 0, &pixelShader));

        // Input layout (baked into the pre-rasterization partial program).
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = { inputElementDescs, _countof(inputElementDescs) };

        // Primitive topology (shared by both partial programs).
        D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // Render target formats (baked into the PS partial program).
        D3D12_RT_FORMAT_ARRAY rtFormats = {};
        rtFormats.NumRenderTargets = 1;
        rtFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        // Pre-rasterization shaders partial program fields: input layout is
        // baked in (not late linked) since it doesn't vary across permutations.
        D3D12_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS_DESC preRastFields = {};
        preRastFields.ExcludePS = FALSE;
        preRastFields.LateLinkInputLayoutSubobject = FALSE;

        // Pixel shader partial program fields: blend will be late linked; all
        // other state is baked in / defaulted.
        D3D12_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS_DESC psFields = {};
        psFields.LineRasterizationMode             = D3D12_LINE_RASTERIZATION_MODE_ALIASED;
        psFields.ForcedSampleCount                 = 0;
        psFields.AlphaToCoverageEnable             = FALSE;
        psFields.DualSourceBlendEnable             = FALSE;
        psFields.LateLinkRasterizerSubobject       = FALSE;
        psFields.LateLinkBlendSubobject            = TRUE;   // <-- the whole point
        psFields.LateLinkSampleMaskSubobject       = FALSE;
        psFields.LateLinkSampleDescSubobject       = FALSE;
        psFields.LateLinkDepthStencilFormatSubobject = FALSE;
        psFields.LateLinkRenderTargetFormatSubobject = FALSE;
        psFields.LateLinkDepthStencilSubobject     = FALSE;

        // Global root signature.
        D3D12_GLOBAL_ROOT_SIGNATURE globalRS = { m_rootSignature.Get() };

        // DXIL library descs (one per shader).
        D3D12_EXPORT_DESC vsExport = { L"VSMain", nullptr, D3D12_EXPORT_FLAG_NONE };
        D3D12_DXIL_LIBRARY_DESC vsLib = {};
        vsLib.DXILLibrary.pShaderBytecode = vertexShader->GetBufferPointer();
        vsLib.DXILLibrary.BytecodeLength  = vertexShader->GetBufferSize();
        vsLib.NumExports = 1;
        vsLib.pExports   = &vsExport;

        D3D12_EXPORT_DESC psExport = { L"PSMain", nullptr, D3D12_EXPORT_FLAG_NONE };
        D3D12_DXIL_LIBRARY_DESC psLib = {};
        psLib.DXILLibrary.pShaderBytecode = pixelShader->GetBufferPointer();
        psLib.DXILLibrary.BytecodeLength  = pixelShader->GetBufferSize();
        psLib.NumExports = 1;
        psLib.pExports   = &psExport;

        // Three blend descs (the only thing that varies between the three
        // generic programs we'll link).
        auto MakeBlend = [](BOOL enable, D3D12_BLEND src, D3D12_BLEND dst)
        {
            D3D12_BLEND_DESC b = {};
            b.AlphaToCoverageEnable  = FALSE;
            b.IndependentBlendEnable = FALSE;
            b.RenderTarget[0].BlendEnable           = enable;
            b.RenderTarget[0].LogicOpEnable         = FALSE;
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
        D3D12_BLEND_DESC blendDescs[NumBlendPermutations] =
        {
            MakeBlend(FALSE, D3D12_BLEND_ONE,       D3D12_BLEND_ZERO),         // opaque
            MakeBlend(TRUE,  D3D12_BLEND_ONE,       D3D12_BLEND_ONE),          // additive
            MakeBlend(TRUE,  D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA),// alpha
        };

        // Build the flat subobject list. Indices are referenced below when
        // wiring up the partial program and generic program descs.
        enum SubobjectIndex
        {
            SO_Config = 0,
            SO_RootSig,
            SO_VSLib,
            SO_PSLib,
            SO_InputLayout,
            SO_Topology,
            SO_RTFormats,
            SO_PreRastFields,
            SO_PSFields,
            SO_PreRastProgram,
            SO_PSProgram,
            SO_Blend0,
            SO_Blend1,
            SO_Blend2,
            SO_GenericProgram0,
            SO_GenericProgram1,
            SO_GenericProgram2,
            SO_Count
        };

        D3D12_STATE_OBJECT_CONFIG soConfig = {};
        soConfig.Flags = D3D12_STATE_OBJECT_FLAG_NONE;

        D3D12_STATE_SUBOBJECT subobjects[SO_Count] = {};
        subobjects[SO_Config]       = { D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG,   &soConfig };
        subobjects[SO_RootSig]      = { D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalRS };
        subobjects[SO_VSLib]        = { D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,          &vsLib };
        subobjects[SO_PSLib]        = { D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,          &psLib };
        subobjects[SO_InputLayout]  = { D3D12_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT,          &inputLayoutDesc };
        subobjects[SO_Topology]     = { D3D12_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY,    &topology };
        subobjects[SO_RTFormats]    = { D3D12_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, &rtFormats };
        subobjects[SO_PreRastFields]= { D3D12_STATE_SUBOBJECT_TYPE_PRERASTERIZATION_SHADERS_PARTIAL_PROGRAM_FIELDS, &preRastFields };
        subobjects[SO_PSFields]     = { D3D12_STATE_SUBOBJECT_TYPE_PIXEL_SHADER_PARTIAL_PROGRAM_FIELDS,             &psFields };

        // Pre-rasterization partial program: references VS export and includes
        // input layout, primitive topology, and the pre-raster fields desc.
        LPCWSTR preRastExports[] = { L"VSMain" };
        const D3D12_STATE_SUBOBJECT* preRastChildren[] =
        {
            &subobjects[SO_InputLayout],
            &subobjects[SO_Topology],
            &subobjects[SO_PreRastFields],
        };
        D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC preRastDesc = {};
        preRastDesc.ProgramName    = L"VSPartial";
        preRastDesc.NumExports     = _countof(preRastExports);
        preRastDesc.pExports       = preRastExports;
        preRastDesc.NumSubobjects  = _countof(preRastChildren);
        preRastDesc.ppSubobjects   = preRastChildren;
        preRastDesc.ProgramType    = D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE_PRERASTERIZATION_SHADER;
        subobjects[SO_PreRastProgram] = { D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM, &preRastDesc };

        // Pixel shader partial program: references PS export and bakes in
        // primitive topology + RT formats; blend is late linked.
        LPCWSTR psExports2[] = { L"PSMain" };
        const D3D12_STATE_SUBOBJECT* psChildren[] =
        {
            &subobjects[SO_Topology],
            &subobjects[SO_RTFormats],
            &subobjects[SO_PSFields],
        };
        D3D12_PARTIAL_GRAPHICS_PROGRAM_DESC psDesc = {};
        psDesc.ProgramName    = L"PSPartial";
        psDesc.NumExports     = _countof(psExports2);
        psDesc.pExports       = psExports2;
        psDesc.NumSubobjects  = _countof(psChildren);
        psDesc.ppSubobjects   = psChildren;
        psDesc.ProgramType    = D3D12_PARTIAL_GRAPHICS_PROGRAM_TYPE_PIXEL_SHADER;
        subobjects[SO_PSProgram] = { D3D12_STATE_SUBOBJECT_TYPE_PARTIAL_GRAPHICS_PROGRAM, &psDesc };

        subobjects[SO_Blend0] = { D3D12_STATE_SUBOBJECT_TYPE_BLEND, &blendDescs[0] };
        subobjects[SO_Blend1] = { D3D12_STATE_SUBOBJECT_TYPE_BLEND, &blendDescs[1] };
        subobjects[SO_Blend2] = { D3D12_STATE_SUBOBJECT_TYPE_BLEND, &blendDescs[2] };

        // Three generic programs, each referencing the two partial programs by
        // name and supplying a different blend subobject.
        LPCWSTR programExports[] = { L"VSPartial", L"PSPartial" };
        const D3D12_STATE_SUBOBJECT* gp0Children[] = { &subobjects[SO_Blend0] };
        const D3D12_STATE_SUBOBJECT* gp1Children[] = { &subobjects[SO_Blend1] };
        const D3D12_STATE_SUBOBJECT* gp2Children[] = { &subobjects[SO_Blend2] };

        D3D12_GENERIC_PROGRAM_DESC genericPrograms[NumBlendPermutations] = {};
        const LPCWSTR programNames[NumBlendPermutations] = { L"OpaqueProgram", L"AdditiveProgram", L"AlphaProgram" };
        const D3D12_STATE_SUBOBJECT* const* perProgramChildren[NumBlendPermutations] =
        {
            gp0Children, gp1Children, gp2Children,
        };
        for (UINT i = 0; i < NumBlendPermutations; ++i)
        {
            genericPrograms[i].ProgramName   = programNames[i];
            genericPrograms[i].NumExports    = _countof(programExports);
            genericPrograms[i].pExports      = programExports;
            genericPrograms[i].NumSubobjects = 1;
            genericPrograms[i].ppSubobjects  = perProgramChildren[i];
        }
        subobjects[SO_GenericProgram0] = { D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM, &genericPrograms[0] };
        subobjects[SO_GenericProgram1] = { D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM, &genericPrograms[1] };
        subobjects[SO_GenericProgram2] = { D3D12_STATE_SUBOBJECT_TYPE_GENERIC_PROGRAM, &genericPrograms[2] };

        D3D12_STATE_OBJECT_DESC soDesc = {};
        soDesc.Type          = D3D12_STATE_OBJECT_TYPE_EXECUTABLE;
        soDesc.NumSubobjects = SO_Count;
        soDesc.pSubobjects   = subobjects;
        ThrowIfFailed(m_device->CreateStateObject(&soDesc, IID_PPV_ARGS(&m_stateObject)));

        ComPtr<ID3D12StateObjectProperties1> soProps;
        ThrowIfFailed(m_stateObject->QueryInterface(IID_PPV_ARGS(&soProps)));
        for (UINT i = 0; i < NumBlendPermutations; ++i)
        {
            m_blendPrograms[i] = soProps->GetProgramIdentifier(programNames[i]);
        }
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
