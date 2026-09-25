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

//*********************************************************
//
// D3D12AsyncCommands
//
// Demonstrates the D3D12 Batched Asynchronous Command List APIs (Async
// Commands), part of the retail surface from Agility SDK 620 on
// ID3D12GraphicsCommandList12.
//
// The legacy Copy*/Clear*/Resolve* commands execute strictly in series,
// because the old ResourceBarrier model cannot express a dependency between
// two operations of the same type. The async commands drop that implicit
// serialization contract, so independent work can overlap, and the caller
// expresses real data hazards with enhanced barriers instead.
//
// The sample renders a triangle whose render target is cleared with the async
// ClearBoundRenderTargetViews, then benchmarks all ten async commands against
// their legacy counterparts using GPU timestamp queries.
//
// Usage: D3D12AsyncCommands [-warp] [-fallback] [-e2e]
//
//*********************************************************

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <array>
#include <chrono>
#include <functional>
#include <initguid.h>
#include <atlbase.h>
#include "d3d12.h"
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Version 620 is the first Agility SDK with async commands on the retail surface.
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 620; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }
extern "C" { __declspec(dllexport) extern const char* WarpPath = u8".\\WARP\\"; }

//======================================================================================================================
// Helpers
//======================================================================================================================
#define PRINT(text) do { std::cout << (text) << "\n" << std::flush; } while(0)
#define VERIFY_SUCCEEDED(hr) { HRESULT _hr = (hr); if (FAILED(_hr)) { \
    std::ostringstream _os; _os << "Error at " << __FILE__ << ":" << __LINE__ << " HRESULT=0x" << std::hex << _hr; \
    PRINT(_os.str()); throw _hr; } }

bool g_useWarpDevice = false;
bool g_forceFallback = false;
bool g_listAdapters = false;
int  g_adapterIndex = -1;
bool g_runEndToEnd = false;

static std::string Narrow(const WCHAR* text)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (count <= 1)
        return std::string();
    std::string out(static_cast<size_t>(count) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), count, nullptr, nullptr);
    return out;
}

static const char* VendorName(UINT vendorId)
{
    switch (vendorId)
    {
    case 0x10DE: return "NVIDIA";
    case 0x1002: return "AMD";
    case 0x8086: return "Intel";
    case 0x1414: return "Microsoft";
    default:     return "unknown vendor";
    }
}

// The user-mode driver version, which is what identifies a specific IHV driver build.
static std::string DriverVersion(IDXGIAdapter1* adapter)
{
    LARGE_INTEGER umd = {};
    if (FAILED(adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umd)))
        return "not available";
    std::ostringstream os;
    os << HIWORD(umd.HighPart) << "." << LOWORD(umd.HighPart) << "."
       << HIWORD(umd.LowPart) << "." << LOWORD(umd.LowPart);
    return os.str();
}

static std::string AdapterLuidString(const LUID& luid)
{
    std::ostringstream os;
    os << "0x" << std::hex << std::uppercase << std::setfill('0')
       << std::setw(8) << static_cast<UINT>(luid.HighPart) << ":"
       << std::setw(8) << luid.LowPart;
    return os.str();
}

static void PrintAdapterDetails(IDXGIAdapter1* adapter)
{
    DXGI_ADAPTER_DESC1 desc = {};
    if (FAILED(adapter->GetDesc1(&desc)))
        return;

    std::ostringstream os;
    os << std::hex << std::uppercase;
    PRINT("   Adapter        : " + Narrow(desc.Description));
    os << "   Vendor / Device: " << VendorName(desc.VendorId) << " (0x" << desc.VendorId
       << ")  device 0x" << desc.DeviceId << "  rev 0x" << desc.Revision;
    PRINT(os.str());
    PRINT("   Driver (UMD)   : " + DriverVersion(adapter));
    PRINT("   Adapter LUID   : " + AdapterLuidString(desc.AdapterLuid));

    std::ostringstream mem;
    mem << "   Dedicated VRAM : " << (desc.DedicatedVideoMemory >> 20) << " MB";
    PRINT(mem.str());
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        PRINT("   Type           : software adapter (WARP)");
}

static void ListAdapters()
{
    CComPtr<IDXGIFactory4> factory;
    VERIFY_SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    PRINT(" Available adapters:\n");
    for (UINT i = 0; ; ++i)
    {
        CComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(i, &adapter) == DXGI_ERROR_NOT_FOUND)
            break;

        std::ostringstream os;
        os << " [" << i << "]";
        PRINT(os.str());
        PrintAdapterDetails(adapter);
        PRINT("");
    }
}


struct D3DContext
{
    CComPtr<ID3D12Device10>              spDevice;
    CComPtr<IDXGIAdapter3>               spAdapter;
    CComPtr<ID3D12CommandQueue>          spQueue;
    CComPtr<ID3D12CommandAllocator>      spAllocator;
    CComPtr<ID3D12GraphicsCommandList12> spList;
    CComPtr<ID3D12Fence>                 spFence;
    CHandle                              hFenceEvent;
    UINT64                               fenceValue = 0;
    UINT64                               gpuTimestampFrequency = 0;
    CComPtr<ID3D12QueryHeap>             spTimestampHeap;
    CComPtr<ID3D12Resource>              spTimestampReadback;
    D3D12_ASYNC_COMMANDS_IMPL            asyncImpl = D3D12_ASYNC_COMMANDS_IMPL_NOT_SUPPORTED;

    // CComPtr asserts on rebinding a live pointer, so release everything between device attempts.
    void Reset()
    {
        spTimestampReadback.Release();
        spTimestampHeap.Release();
        spFence.Release();
        spList.Release();
        spAllocator.Release();
        spQueue.Release();
        spAdapter.Release();
        spDevice.Release();
        hFenceEvent.Close();
        fenceValue = 0;
        gpuTimestampFrequency = 0;
        asyncImpl = D3D12_ASYNC_COMMANDS_IMPL_NOT_SUPPORTED;
    }
};

// useWarp selects WARP, adapterIndex >= 0 selects that adapter, otherwise the default is used.
static bool InitDeviceAndContext(D3DContext& D3D, bool useWarp, int adapterIndex)
{
    D3D.Reset();

    D3D_FEATURE_LEVEL FL = D3D_FEATURE_LEVEL_11_0;
    CComPtr<ID3D12Device> spBaseDevice;
    CComPtr<IDXGIFactory4> factory;
    VERIFY_SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    if (useWarp)
    {
        if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&D3D.spAdapter))))
            return false;
    }
    else if (adapterIndex >= 0)
    {
        CComPtr<IDXGIAdapter1> selected;
        if (factory->EnumAdapters1(static_cast<UINT>(adapterIndex), &selected) == DXGI_ERROR_NOT_FOUND)
        {
            PRINT(" No adapter at that index. Run with -list to see the available adapters.");
            return false;
        }
        if (FAILED(selected.QueryInterface(&D3D.spAdapter)))
            return false;
    }

    if (D3D.spAdapter)
    {
        if (FAILED(D3D12CreateDevice(D3D.spAdapter, FL, IID_PPV_ARGS(&spBaseDevice))))
            return false;
    }
    else
    {
        if (FAILED(D3D12CreateDevice(nullptr, FL, IID_PPV_ARGS(&spBaseDevice))))
            return false;
        LUID luid = spBaseDevice->GetAdapterLuid();
        if (FAILED(factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&D3D.spAdapter))))
            return false;
    }

    if (FAILED(spBaseDevice.QueryInterface(&D3D.spDevice)))
        return false;

    D3D12_FEATURE_DATA_ASYNC_COMMANDS asyncData = {};
    if (SUCCEEDED(D3D.spDevice->CheckFeatureSupport(D3D12_FEATURE_ASYNC_COMMANDS, &asyncData, sizeof(asyncData))))
        D3D.asyncImpl = asyncData.Impl;

    if (D3D.asyncImpl == D3D12_ASYNC_COMMANDS_IMPL_NOT_SUPPORTED)
        return false;

    D3D12_COMMAND_QUEUE_DESC qDesc = {};
    qDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommandQueue(&qDesc, IID_PPV_ARGS(&D3D.spQueue)));
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D3D.spAllocator)));

    CComPtr<ID3D12GraphicsCommandList> spBaseList;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D.spAllocator, nullptr, IID_PPV_ARGS(&spBaseList)));
    VERIFY_SUCCEEDED(spBaseList->Close());
    if (FAILED(spBaseList.QueryInterface(&D3D.spList)))
    {
        PRINT(" ID3D12GraphicsCommandList12 is not available on this command list.");
        return false;
    }

    VERIFY_SUCCEEDED(D3D.spDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D3D.spFence)));
    D3D.hFenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
    if (!D3D.hFenceEvent)
        VERIFY_SUCCEEDED(HRESULT_FROM_WIN32(GetLastError()));

    VERIFY_SUCCEEDED(D3D.spQueue->GetTimestampFrequency(&D3D.gpuTimestampFrequency));
    D3D12_QUERY_HEAP_DESC qhDesc = {};
    qhDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    qhDesc.Count = 2;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateQueryHeap(&qhDesc, IID_PPV_ARGS(&D3D.spTimestampHeap)));

    CD3DX12_HEAP_PROPERTIES readbackProps(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC tsBuf = CD3DX12_RESOURCE_DESC::Buffer(2 * sizeof(UINT64));
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&readbackProps, D3D12_HEAP_FLAG_NONE, &tsBuf,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&D3D.spTimestampReadback)));

    return true;
}

static void ResetList(D3DContext& D3D)
{
    VERIFY_SUCCEEDED(D3D.spAllocator->Reset());
    VERIFY_SUCCEEDED(D3D.spList->Reset(D3D.spAllocator, nullptr));
}

static void ExecuteAndWait(D3DContext& D3D)
{
    VERIFY_SUCCEEDED(D3D.spList->Close());
    ID3D12CommandList* lists[] = { D3D.spList };
    D3D.spQueue->ExecuteCommandLists(1, lists);
    const UINT64 v = ++D3D.fenceValue;
    VERIFY_SUCCEEDED(D3D.spQueue->Signal(D3D.spFence, v));
    if (D3D.spFence->GetCompletedValue() < v)
    {
        VERIFY_SUCCEEDED(D3D.spFence->SetEventOnCompletion(v, D3D.hFenceEvent));
        WaitForSingleObject(D3D.hFenceEvent, INFINITE);
    }
}

// Records the work produced by 'record' inside a GPU-timestamped, isolated submission.
// Returns GPU elapsed milliseconds; optionally reports CPU recording milliseconds.
template <typename TRecord>
static double TimeGpu(D3DContext& D3D, TRecord&& record, double* cpuRecordMs = nullptr)
{
    ResetList(D3D);

    auto cpuStart = std::chrono::high_resolution_clock::now();
    D3D.spList->EndQuery(D3D.spTimestampHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0);
    record(D3D);
    D3D.spList->EndQuery(D3D.spTimestampHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);
    auto cpuEnd = std::chrono::high_resolution_clock::now();
    if (cpuRecordMs)
        *cpuRecordMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();

    D3D.spList->ResolveQueryData(D3D.spTimestampHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, D3D.spTimestampReadback, 0);
    ExecuteAndWait(D3D);

    UINT64 timestamps[2] = {};
    D3D12_RANGE readRange = { 0, sizeof(timestamps) };
    void* pData = nullptr;
    VERIFY_SUCCEEDED(D3D.spTimestampReadback->Map(0, &readRange, &pData));
    memcpy(timestamps, pData, sizeof(timestamps));
    D3D12_RANGE emptyRange = { 0, 0 };
    D3D.spTimestampReadback->Unmap(0, &emptyRange);

    const double ticks = static_cast<double>(timestamps[1] - timestamps[0]);
    return (ticks / static_cast<double>(D3D.gpuTimestampFrequency)) * 1000.0;
}

struct BenchResult
{
    double legacyGpuMs = 0, asyncGpuMs = 0;
    double legacyCpuMs = 0, asyncCpuMs = 0;
};

static void PrintBench(const std::string& title, const char* legacyName, const char* asyncName, const BenchResult& r)
{
    std::ostringstream os;
    os << std::fixed << std::setprecision(4);
    os << "  " << title << "\n";
    os << "    Legacy (" << legacyName << "):  GPU " << std::setw(9) << r.legacyGpuMs
       << " ms   CPU-record " << std::setw(9) << r.legacyCpuMs << " ms\n";
    os << "    Async  (" << asyncName << "):  GPU " << std::setw(9) << r.asyncGpuMs
       << " ms   CPU-record " << std::setw(9) << r.asyncCpuMs << " ms\n";
    const double gpuSpeedup = r.asyncGpuMs > 0 ? r.legacyGpuMs / r.asyncGpuMs : 0;
    const double cpuSpeedup = r.asyncCpuMs > 0 ? r.legacyCpuMs / r.asyncCpuMs : 0;
    os << "    => GPU speedup:  " << std::setprecision(2) << gpuSpeedup << "x"
       << "   CPU-record speedup: " << cpuSpeedup << "x";
    PRINT(os.str());
}

// Records 'record' in an isolated submission with no timestamps (used for untimed setup/layout flips).
static void RunUntimed(D3DContext& D3D, const std::function<void(D3DContext&)>& record)
{
    ResetList(D3D);
    record(D3D);
    ExecuteAndWait(D3D);
}

// Averages a legacy-vs-async comparison over 'iterations'. Optional untimed prep lambdas run before
// each timed measurement (e.g. to flip resource layouts between the two paths).
static BenchResult RunLoop(D3DContext& D3D, UINT iterations,
    const std::function<void(D3DContext&)>& legacy,
    const std::function<void(D3DContext&)>& async,
    const std::function<void(D3DContext&)>& legacyPrep = nullptr,
    const std::function<void(D3DContext&)>& asyncPrep = nullptr)
{
    // Warm-up (untimed).
    if (legacyPrep) RunUntimed(D3D, legacyPrep);
    TimeGpu(D3D, legacy);
    if (asyncPrep) RunUntimed(D3D, asyncPrep);
    TimeGpu(D3D, async);

    BenchResult r; double lg = 0, ag = 0, lc = 0, ac = 0, tmp = 0;
    for (UINT it = 0; it < iterations; ++it)
    {
        if (legacyPrep) RunUntimed(D3D, legacyPrep);
        lg += TimeGpu(D3D, legacy, &tmp); lc += tmp;
        if (asyncPrep) RunUntimed(D3D, asyncPrep);
        ag += TimeGpu(D3D, async, &tmp);  ac += tmp;
    }
    r.legacyGpuMs = lg / iterations; r.asyncGpuMs = ag / iterations;
    r.legacyCpuMs = lc / iterations; r.asyncCpuMs = ac / iterations;
    return r;
}

//======================================================================================================================
// Part 1 - Draw a triangle, clearing the bound RTV with the async ClearBoundRenderTargetViews.
//======================================================================================================================
static const char* g_triangleShader =
"struct PSInput { float4 position : SV_POSITION; float4 color : COLOR; };\n"
"PSInput VSMain(float3 position : POSITION, float4 color : COLOR) {\n"
"  PSInput r; r.position = float4(position, 1.0f); r.color = color; return r; }\n"
"float4 PSMain(PSInput input) : SV_TARGET { return input.color; }\n";

static bool RenderTriangleWithAsyncClear(D3DContext& D3D)
{
    PRINT("[1] Rendering a triangle into an offscreen render target...");
    PRINT("    The RTV is cleared with the async ClearBoundRenderTargetViews (an in-render-pass raster clear),");
    PRINT("    replacing the legacy ClearRenderTargetView.\n");

    const UINT width = 256, height = 256;
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Offscreen render target created for the enhanced-barrier model (UNDEFINED initial layout).
    CD3DX12_HEAP_PROPERTIES defaultProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC1 rtDesc = CD3DX12_RESOURCE_DESC1::Tex2D(format, width, height, 1, 1);
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE optClear = {};
    optClear.Format = format;
    optClear.Color[0] = 0.0f; optClear.Color[1] = 0.2f; optClear.Color[2] = 0.4f; optClear.Color[3] = 1.0f;

    CComPtr<ID3D12Resource> rt;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&defaultProps, D3D12_HEAP_FLAG_NONE, &rtDesc,
        D3D12_BARRIER_LAYOUT_UNDEFINED, &optClear, nullptr, 0, nullptr, IID_PPV_ARGS(&rt)));

    CComPtr<ID3D12DescriptorHeap> rtvHeap;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D.spDevice->CreateRenderTargetView(rt, nullptr, rtvHandle);

    CComPtr<ID3D12RootSignature> rootSig;
    {
        CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
        rsDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        CComPtr<ID3DBlob> sig, err;
        VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &err));
        VERIFY_SUCCEEDED(D3D.spDevice->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&rootSig)));
    }

    CComPtr<ID3DBlob> vs, ps, err;
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    VERIFY_SUCCEEDED(D3DCompile(g_triangleShader, strlen(g_triangleShader), "triangle", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vs, &err));
    VERIFY_SUCCEEDED(D3DCompile(g_triangleShader, strlen(g_triangleShader), "triangle", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &ps, &err));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = rootSig;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = format;
    psoDesc.SampleDesc.Count = 1;

    CComPtr<ID3D12PipelineState> pso;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

    struct Vertex { float pos[3]; float color[4]; };
    Vertex verts[] =
    {
        { {  0.0f,  0.75f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.75f, -0.75f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.75f, -0.75f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
    };
    CComPtr<ID3D12Resource> vb;
    CD3DX12_HEAP_PROPERTIES uploadProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(verts));
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vb)));
    {
        void* p = nullptr; D3D12_RANGE empty = { 0, 0 };
        VERIFY_SUCCEEDED(vb->Map(0, &empty, &p));
        memcpy(p, verts, sizeof(verts));
        vb->Unmap(0, nullptr);
    }
    D3D12_VERTEX_BUFFER_VIEW vbv = { vb->GetGPUVirtualAddress(), sizeof(verts), sizeof(Vertex) };

    UINT64 rbSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    CD3DX12_RESOURCE_DESC rtDesc0 = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1);
    D3D.spDevice->GetCopyableFootprints(&rtDesc0, 0, 1, 0, &footprint, nullptr, nullptr, &rbSize);
    CComPtr<ID3D12Resource> readback;
    CD3DX12_HEAP_PROPERTIES readbackProps(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC rbDesc = CD3DX12_RESOURCE_DESC::Buffer(rbSize);
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&readbackProps, D3D12_HEAP_FLAG_NONE, &rbDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)));

    ResetList(D3D);

    // A texture's first use needs DISCARD to initialize its metadata.
    D3D12_TEXTURE_BARRIER toRt = {};
    toRt.SyncBefore = D3D12_BARRIER_SYNC_NONE;
    toRt.SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET;
    toRt.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS;
    toRt.AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET;
    toRt.LayoutBefore = D3D12_BARRIER_LAYOUT_UNDEFINED;
    toRt.LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    toRt.pResource = rt;
    toRt.Subresources.IndexOrFirstMipLevel = 0xffffffff;
    toRt.Flags = D3D12_TEXTURE_BARRIER_FLAG_DISCARD;
    D3D12_BARRIER_GROUP bgToRt = {};
    bgToRt.Type = D3D12_BARRIER_TYPE_TEXTURE;
    bgToRt.NumBarriers = 1;
    bgToRt.pTextureBarriers = &toRt;
    D3D.spList->Barrier(1, &bgToRt);

    D3D12_VIEWPORT vp = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    D3D12_RECT sc = { 0, 0, (LONG)width, (LONG)height };
    D3D.spList->RSSetViewports(1, &vp);
    D3D.spList->RSSetScissorRects(1, &sc);
    D3D.spList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // The async clear of the bound RTV, in place of ClearRenderTargetView.
    D3D12_CLEAR_DATA clearValues[8] = {};
    clearValues[0].Floats[0] = 0.0f;
    clearValues[0].Floats[1] = 0.2f;
    clearValues[0].Floats[2] = 0.4f;
    clearValues[0].Floats[3] = 1.0f;
    D3D.spList->ClearBoundRenderTargetViews(0x1 /* slot 0 */, clearValues, nullptr, nullptr);

    D3D.spList->SetGraphicsRootSignature(rootSig);
    D3D.spList->SetPipelineState(pso);
    D3D.spList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D.spList->IASetVertexBuffers(0, 1, &vbv);
    D3D.spList->DrawInstanced(3, 1, 0, 0);

    D3D12_TEXTURE_BARRIER toCopy = {};
    toCopy.SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
    toCopy.SyncAfter = D3D12_BARRIER_SYNC_COPY;
    toCopy.AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
    toCopy.AccessAfter = D3D12_BARRIER_ACCESS_COPY_SOURCE;
    toCopy.LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    toCopy.LayoutAfter = D3D12_BARRIER_LAYOUT_COPY_SOURCE;
    toCopy.pResource = rt;
    toCopy.Subresources.IndexOrFirstMipLevel = 0xffffffff;
    D3D12_BARRIER_GROUP bgToCopy = {};
    bgToCopy.Type = D3D12_BARRIER_TYPE_TEXTURE;
    bgToCopy.NumBarriers = 1;
    bgToCopy.pTextureBarriers = &toCopy;
    D3D.spList->Barrier(1, &bgToCopy);

    CD3DX12_TEXTURE_COPY_LOCATION dst(readback, footprint);
    CD3DX12_TEXTURE_COPY_LOCATION src(rt, 0);
    D3D.spList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    ExecuteAndWait(D3D);

    void* pData = nullptr;
    D3D12_RANGE readRange = { 0, (SIZE_T)rbSize };
    VERIFY_SUCCEEDED(readback->Map(0, &readRange, &pData));
    auto pixelAt = [&](UINT x, UINT y)
    {
        const BYTE* p = reinterpret_cast<const BYTE*>(pData) + footprint.Footprint.RowPitch * y + x * 4;
        return std::array<BYTE, 4>{ p[0], p[1], p[2], p[3] };
    };
    // The triangle spans the middle of the target, so a corner still shows the cleared background.
    const std::array<BYTE, 4> corner = pixelAt(2, 2);
    const std::array<BYTE, 4> center = pixelAt(width / 2, height / 2);
    D3D12_RANGE emptyRange = { 0, 0 };
    readback->Unmap(0, &emptyRange);

    const std::array<BYTE, 4> expectedClear = { 0, 51, 102, 255 };
    auto matches = [](const std::array<BYTE, 4>& a, const std::array<BYTE, 4>& b)
    {
        for (size_t i = 0; i < 4; ++i)
            if (abs(static_cast<int>(a[i]) - static_cast<int>(b[i])) > 2)
                return false;
        return true;
    };
    auto rgba = [](const std::array<BYTE, 4>& c)
    {
        std::ostringstream os;
        os << "(" << (int)c[0] << ", " << (int)c[1] << ", " << (int)c[2] << ", " << (int)c[3] << ")";
        return os.str();
    };

    const bool clearApplied = matches(corner, expectedClear);
    const bool triangleDrawn = !matches(center, expectedClear);

    PRINT("    Corner pixel RGBA = " + rgba(corner) + (clearApplied ? "  (cleared)" : "  EXPECTED " + rgba(expectedClear)));
    PRINT("    Center pixel RGBA = " + rgba(center) + (triangleDrawn ? "  (triangle)" : "  EXPECTED not the clear color"));

    const bool pass = clearApplied && triangleDrawn;
    PRINT(pass ? "    PASS: triangle rendered over the async-cleared background.\n"
               : "    FAIL: render target contents are not as expected.\n");
    return pass;
}

//======================================================================================================================
// Part 2 - Benchmarks: async batched commands vs legacy serialized commands.
//======================================================================================================================
static const UINT   kNumResources = 256;          // independent resources per batch
static const UINT64 kBufferSize = 1u << 20;      // 1 MiB each
static const UINT   kIterations = 16;            // averaging runs

// ---- Buffer fill: FillBuffers vs ClearUnorderedAccessViewUint ----
static void BenchmarkFillVsUavClear(D3DContext& D3D, BenchResult& out)
{
    std::vector<CComPtr<ID3D12Resource>> buffers(kNumResources);
    CD3DX12_HEAP_PROPERTIES defaultProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(kBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    for (UINT i = 0; i < kNumResources; ++i)
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&buffers[i])));

    // The legacy path needs a shader-visible and a non-shader-visible descriptor per resource.
    const UINT descSize = D3D.spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CComPtr<ID3D12DescriptorHeap> gpuHeap, cpuHeap;
    D3D12_DESCRIPTOR_HEAP_DESC hd = {};
    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    hd.NumDescriptors = kNumResources;
    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&gpuHeap)));
    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&cpuHeap)));

    D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};
    uav.Format = DXGI_FORMAT_R32_TYPELESS;
    uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uav.Buffer.NumElements = (UINT)(kBufferSize / 4);
    uav.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    for (UINT i = 0; i < kNumResources; ++i)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE g(gpuHeap->GetCPUDescriptorHandleForHeapStart(), i, descSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE c(cpuHeap->GetCPUDescriptorHandleForHeapStart(), i, descSize);
        D3D.spDevice->CreateUnorderedAccessView(buffers[i], nullptr, &uav, g);
        D3D.spDevice->CreateUnorderedAccessView(buffers[i], nullptr, &uav, c);
    }

    const UINT clearVals[4] = { 0xAABBCCDD, 0xAABBCCDD, 0xAABBCCDD, 0xAABBCCDD };
    auto legacy = [&](D3DContext& d)
    {
        ID3D12DescriptorHeap* heaps[] = { gpuHeap };
        d.spList->SetDescriptorHeaps(1, heaps);
        for (UINT i = 0; i < kNumResources; ++i)
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE g(gpuHeap->GetGPUDescriptorHandleForHeapStart(), i, descSize);
            CD3DX12_CPU_DESCRIPTOR_HANDLE c(cpuHeap->GetCPUDescriptorHandleForHeapStart(), i, descSize);
            d.spList->ClearUnorderedAccessViewUint(g, c, buffers[i], clearVals, 0, nullptr);
        }
    };

    // FillBuffers takes resource pointers, so it needs no descriptors at all.
    std::vector<D3D12_FILL_BUFFER_DESC> fillDescs(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
    {
        D3D12_FILL_BUFFER_DESC& f = fillDescs[i];
        f.pBuffer = buffers[i];
        f.Offset = 0;
        f.FillValue.Uints[0] = 0xAABBCCDD;
        f.Format = DXGI_FORMAT_R32_UINT;
        f.RawPatternSizeInBytes = 0;
        f.RepeatCount = (UINT)(kBufferSize / 4);
    }
    auto async = [&](D3DContext& d)
    {
        d.spList->FillBuffers(kNumResources, fillDescs.data());
    };

    TimeGpu(D3D, legacy); TimeGpu(D3D, async);
    double lg = 0, ag = 0, lc = 0, ac = 0, tmp = 0;
    for (UINT it = 0; it < kIterations; ++it)
    {
        lg += TimeGpu(D3D, legacy, &tmp); lc += tmp;
        ag += TimeGpu(D3D, async, &tmp);  ac += tmp;
    }
    out.legacyGpuMs = lg / kIterations; out.asyncGpuMs = ag / kIterations;
    out.legacyCpuMs = lc / kIterations; out.asyncCpuMs = ac / kIterations;
}

// ---- Buffer copy: CopyBufferRegions vs CopyBufferRegion ----
static void BenchmarkCopy(D3DContext& D3D, BenchResult& out)
{
    CD3DX12_HEAP_PROPERTIES defaultProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(kBufferSize);

    CComPtr<ID3D12Resource> source;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&source)));

    std::vector<CComPtr<ID3D12Resource>> dests(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&dests[i])));

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < kNumResources; ++i)
            d.spList->CopyBufferRegion(dests[i], 0, source, 0, kBufferSize);
    };

    std::vector<ID3D12Resource*> destPtrs(kNumResources), srcPtrs(kNumResources);
    std::vector<UINT64> destOffsets(kNumResources, 0), srcOffsets(kNumResources, 0), sizes(kNumResources, kBufferSize);
    for (UINT i = 0; i < kNumResources; ++i) { destPtrs[i] = dests[i]; srcPtrs[i] = source; }
    auto async = [&](D3DContext& d)
    {
        d.spList->CopyBufferRegions(kNumResources, destPtrs.data(), destOffsets.data(),
            srcPtrs.data(), srcOffsets.data(), sizes.data());
    };

    TimeGpu(D3D, legacy); TimeGpu(D3D, async);
    double lg = 0, ag = 0, lc = 0, ac = 0, tmp = 0;
    for (UINT it = 0; it < kIterations; ++it)
    {
        lg += TimeGpu(D3D, legacy, &tmp); lc += tmp;
        ag += TimeGpu(D3D, async, &tmp);  ac += tmp;
    }
    out.legacyGpuMs = lg / kIterations; out.asyncGpuMs = ag / kIterations;
    out.legacyCpuMs = lc / kIterations; out.asyncCpuMs = ac / kIterations;
}

// ---- Texture clear: ClearTextureSubresources vs ClearRenderTargetView ----
static void BenchmarkTextureClear(D3DContext& D3D, BenchResult& out)
{
    const UINT texDim = 512;
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    std::vector<CComPtr<ID3D12Resource>> textures(kNumResources);
    CD3DX12_HEAP_PROPERTIES defaultProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC1 desc = CD3DX12_RESOURCE_DESC1::Tex2D(format, texDim, texDim, 1, 1);
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE optClear = {}; optClear.Format = format;

    for (UINT i = 0; i < kNumResources; ++i)
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&defaultProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_BARRIER_LAYOUT_RENDER_TARGET, &optClear, nullptr, 0, nullptr, IID_PPV_ARGS(&textures[i])));

    const UINT rtvSize = D3D.spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CComPtr<ID3D12DescriptorHeap> rtvHeap;
    D3D12_DESCRIPTOR_HEAP_DESC hd = {};
    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    hd.NumDescriptors = kNumResources;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap)));
    for (UINT i = 0; i < kNumResources; ++i)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE h(rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, rtvSize);
        D3D.spDevice->CreateRenderTargetView(textures[i], nullptr, h);
    }

    // Barrier helper to flip all textures between RENDER_TARGET and COPY_DEST layouts.
    auto transitionAll = [&](D3DContext& d, D3D12_BARRIER_LAYOUT before, D3D12_BARRIER_LAYOUT after,
                             D3D12_BARRIER_SYNC syncB, D3D12_BARRIER_SYNC syncA,
                             D3D12_BARRIER_ACCESS accB, D3D12_BARRIER_ACCESS accA)
    {
        std::vector<D3D12_TEXTURE_BARRIER> barriers(kNumResources);
        for (UINT i = 0; i < kNumResources; ++i)
        {
            D3D12_TEXTURE_BARRIER& b = barriers[i];
            b.SyncBefore = syncB; b.SyncAfter = syncA;
            b.AccessBefore = accB; b.AccessAfter = accA;
            b.LayoutBefore = before; b.LayoutAfter = after;
            b.pResource = textures[i];
            b.Subresources.IndexOrFirstMipLevel = 0xffffffff;
        }
        D3D12_BARRIER_GROUP bg = {};
        bg.Type = D3D12_BARRIER_TYPE_TEXTURE;
        bg.NumBarriers = kNumResources;
        bg.pTextureBarriers = barriers.data();
        d.spList->Barrier(1, &bg);
    };

    const float clearColor[4] = { 0.1f, 0.2f, 0.3f, 1.0f };

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < kNumResources; ++i)
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE h(rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, rtvSize);
            d.spList->ClearRenderTargetView(h, clearColor, 0, nullptr);
        }
    };

    // ClearTextureSubresources clears by resource pointer and format, so it needs no RTVs, but it
    // requires the COPY_DEST layout rather than RENDER_TARGET.
    std::vector<D3D12_CLEAR_TEXTURE_DESC> clearDescs(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
    {
        D3D12_CLEAR_TEXTURE_DESC& c = clearDescs[i];
        c.pTexture = textures[i];
        c.SubresourceIndex = 0;
        c.ClearValue.Floats[0] = clearColor[0];
        c.ClearValue.Floats[1] = clearColor[1];
        c.ClearValue.Floats[2] = clearColor[2];
        c.ClearValue.Floats[3] = clearColor[3];
        c.Format = format;
        c.pRegion = nullptr;
    }
    auto async = [&](D3DContext& d)
    {
        d.spList->ClearTextureSubresources(kNumResources, clearDescs.data());
    };

    // Warm-up.
    {
        double dummy;
        TimeGpu(D3D, [&](D3DContext& d) { legacy(d); }, &dummy);
        TimeGpu(D3D, [&](D3DContext& d)
        {
            transitionAll(d, D3D12_BARRIER_LAYOUT_RENDER_TARGET, D3D12_BARRIER_LAYOUT_COPY_DEST,
                D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_SYNC_COPY,
                D3D12_BARRIER_ACCESS_RENDER_TARGET, D3D12_BARRIER_ACCESS_COPY_DEST);
        }, &dummy);
        TimeGpu(D3D, [&](D3DContext& d) { async(d); }, &dummy);
        TimeGpu(D3D, [&](D3DContext& d)
        {
            transitionAll(d, D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_RENDER_TARGET,
                D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_RENDER_TARGET,
                D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_RENDER_TARGET);
        }, &dummy);
    }

    double lg = 0, ag = 0, lc = 0, ac = 0, tmp = 0;
    for (UINT it = 0; it < kIterations; ++it)
    {
        lg += TimeGpu(D3D, legacy, &tmp); lc += tmp;

        // The layout flips run in their own submissions so they stay out of the measurements.
        TimeGpu(D3D, [&](D3DContext& d)
        {
            transitionAll(d, D3D12_BARRIER_LAYOUT_RENDER_TARGET, D3D12_BARRIER_LAYOUT_COPY_DEST,
                D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_SYNC_COPY,
                D3D12_BARRIER_ACCESS_RENDER_TARGET, D3D12_BARRIER_ACCESS_COPY_DEST);
        });

        ag += TimeGpu(D3D, async, &tmp); ac += tmp;

        TimeGpu(D3D, [&](D3DContext& d)
        {
            transitionAll(d, D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_RENDER_TARGET,
                D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_RENDER_TARGET,
                D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_RENDER_TARGET);
        });
    }
    out.legacyGpuMs = lg / kIterations; out.asyncGpuMs = ag / kIterations;
    out.legacyCpuMs = lc / kIterations; out.asyncCpuMs = ac / kIterations;
}

// ---- Whole-resource copy: CopyResources vs CopyResource ----
static void BenchmarkCopyResources(D3DContext& D3D, BenchResult& out, UINT64 bufferSize)
{
    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    CComPtr<ID3D12Resource> source;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&dp, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&source)));

    std::vector<CComPtr<ID3D12Resource>> dests(kNumResources);
    std::vector<ID3D12Resource*> destPtrs(kNumResources), srcPtrs(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
    {
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&dp, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&dests[i])));
        destPtrs[i] = dests[i]; srcPtrs[i] = source;
    }

    auto legacy = [&](D3DContext& d) { for (UINT i = 0; i < kNumResources; ++i) d.spList->CopyResource(dests[i], source); };
    auto async = [&](D3DContext& d) { d.spList->CopyResources(kNumResources, destPtrs.data(), srcPtrs.data()); };
    out = RunLoop(D3D, kIterations, legacy, async);
}

// ---- Texture region copy: CopyTextureRegions vs CopyTextureRegion ----
static void BenchmarkCopyTextureRegions(D3DContext& D3D, BenchResult& out)
{
    const UINT dim = 512;
    const DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC1 desc = CD3DX12_RESOURCE_DESC1::Tex2D(fmt, dim, dim, 1, 1);

    std::vector<CComPtr<ID3D12Resource>> src(kNumResources), dst(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
    {
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&dp, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_BARRIER_LAYOUT_COPY_SOURCE, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&src[i])));
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&dp, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&dst[i])));
    }

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < kNumResources; ++i)
        {
            CD3DX12_TEXTURE_COPY_LOCATION dl(dst[i], 0), sl(src[i], 0);
            d.spList->CopyTextureRegion(&dl, 0, 0, 0, &sl, nullptr);
        }
    };

    D3D12_TEXTURE_COPY_REGION region = { 0, 0, 0, nullptr }; // full source subresource
    std::vector<D3D12_TEXTURE_COPY_DESC> descs(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
    {
        D3D12_TEXTURE_COPY_DESC& c = descs[i];
        c.Type = D3D12_TEXTURE_COPY_DESC_TYPE_TEXTURE_TO_TEXTURE;
        c.NumRegions = 1;
        c.pCopyRegions = &region;
        c.TextureTextureCopy.pDest = dst[i];
        c.TextureTextureCopy.DestSubresourceIndex = 0;
        c.TextureTextureCopy.pSource = src[i];
        c.TextureTextureCopy.SourceSubresourceIndex = 0;
    }
    auto async = [&](D3DContext& d) { d.spList->CopyTextureRegions(kNumResources, descs.data()); };
    out = RunLoop(D3D, kIterations, legacy, async);
}

// ---- MSAA resolve: ResolveSubresourceRegionAsync vs ResolveSubresourceRegion ----
static void BenchmarkResolve(D3DContext& D3D, BenchResult& out)
{
    const UINT N = 64, iters = 8, dim = 512;
    const DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);

    CD3DX12_RESOURCE_DESC1 msDesc = CD3DX12_RESOURCE_DESC1::Tex2D(fmt, dim, dim, 1, 1);
    msDesc.SampleDesc.Count = 4;
    msDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    CD3DX12_RESOURCE_DESC1 dstDesc = CD3DX12_RESOURCE_DESC1::Tex2D(fmt, dim, dim, 1, 1);
    dstDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    std::vector<CComPtr<ID3D12Resource>> ms(N), dst(N);
    for (UINT i = 0; i < N; ++i)
    {
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&dp, D3D12_HEAP_FLAG_NONE, &msDesc,
            D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&ms[i])));
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&dp, D3D12_HEAP_FLAG_NONE, &dstDesc,
            D3D12_BARRIER_LAYOUT_RESOLVE_DEST, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&dst[i])));
    }

    // One-time: initialize MSAA metadata (DISCARD) and move to RESOLVE_SOURCE layout.
    RunUntimed(D3D, [&](D3DContext& d)
    {
        std::vector<D3D12_TEXTURE_BARRIER> bs(N);
        for (UINT i = 0; i < N; ++i)
        {
            D3D12_TEXTURE_BARRIER& b = bs[i];
            b.SyncBefore = D3D12_BARRIER_SYNC_NONE; b.SyncAfter = D3D12_BARRIER_SYNC_RESOLVE;
            b.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS; b.AccessAfter = D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
            b.LayoutBefore = D3D12_BARRIER_LAYOUT_UNDEFINED; b.LayoutAfter = D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
            b.pResource = ms[i]; b.Subresources.IndexOrFirstMipLevel = 0xffffffff;
            b.Flags = D3D12_TEXTURE_BARRIER_FLAG_DISCARD;
        }
        D3D12_BARRIER_GROUP g = {}; g.Type = D3D12_BARRIER_TYPE_TEXTURE; g.NumBarriers = N; g.pTextureBarriers = bs.data();
        d.spList->Barrier(1, &g);
    });

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < N; ++i)
            d.spList->ResolveSubresourceRegion(dst[i], 0, 0, 0, ms[i], 0, nullptr, fmt, D3D12_RESOLVE_MODE_AVERAGE);
    };
    auto async = [&](D3DContext& d)
    {
        for (UINT i = 0; i < N; ++i)
            d.spList->ResolveSubresourceRegionAsync(dst[i], 0, 0, 0, ms[i], 0, nullptr, fmt, D3D12_RESOLVE_MODE_AVERAGE);
    };
    out = RunLoop(D3D, iters, legacy, async);
}

// ---- Query resolve: ResolveQueryDataAsync vs ResolveQueryData ----
static void BenchmarkResolveQueryData(D3DContext& D3D, BenchResult& out)
{
    const UINT N = kNumResources;
    CComPtr<ID3D12QueryHeap> qh;
    D3D12_QUERY_HEAP_DESC qd = {}; qd.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP; qd.Count = N;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateQueryHeap(&qd, IID_PPV_ARGS(&qh)));

    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bd = CD3DX12_RESOURCE_DESC::Buffer((UINT64)N * sizeof(UINT64));
    CComPtr<ID3D12Resource> dst;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&dp, D3D12_HEAP_FLAG_NONE, &bd,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&dst)));

    // Populate all queries once so there is valid data to resolve.
    RunUntimed(D3D, [&](D3DContext& d) { for (UINT i = 0; i < N; ++i) d.spList->EndQuery(qh, D3D12_QUERY_TYPE_TIMESTAMP, i); });

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < N; ++i)
            d.spList->ResolveQueryData(qh, D3D12_QUERY_TYPE_TIMESTAMP, i, 1, dst, (UINT64)i * sizeof(UINT64));
    };
    auto async = [&](D3DContext& d)
    {
        for (UINT i = 0; i < N; ++i)
            d.spList->ResolveQueryDataAsync(qh, D3D12_QUERY_TYPE_TIMESTAMP, i, 1, dst, (UINT64)i * sizeof(UINT64));
    };
    out = RunLoop(D3D, kIterations, legacy, async);
}

// ---- Bound RTV clear: ClearBoundRenderTargetViews vs ClearRenderTargetView ----
static void BenchmarkClearBoundRTVs(D3DContext& D3D, BenchResult& out)
{
    const UINT dim = 512;
    const DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC1 desc = CD3DX12_RESOURCE_DESC1::Tex2D(fmt, dim, dim, 1, 1);
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE cv = {}; cv.Format = fmt;

    std::vector<CComPtr<ID3D12Resource>> tex(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&dp, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_BARRIER_LAYOUT_RENDER_TARGET, &cv, nullptr, 0, nullptr, IID_PPV_ARGS(&tex[i])));

    const UINT rtvSize = D3D.spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CComPtr<ID3D12DescriptorHeap> heap;
    D3D12_DESCRIPTOR_HEAP_DESC hd = {}; hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; hd.NumDescriptors = kNumResources;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&heap)));
    for (UINT i = 0; i < kNumResources; ++i)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE h(heap->GetCPUDescriptorHandleForHeapStart(), i, rtvSize);
        D3D.spDevice->CreateRenderTargetView(tex[i], nullptr, h);
    }

    const float col[4] = { 0.1f, 0.2f, 0.3f, 1.0f };
    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < kNumResources; ++i)
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE h(heap->GetCPUDescriptorHandleForHeapStart(), i, rtvSize);
            d.spList->ClearRenderTargetView(h, col, 0, nullptr);
        }
    };

    // ClearBoundRenderTargetViews clears every bound RTV in one call, up to the 8-slot limit.
    D3D12_CLEAR_DATA cd[8] = {};
    for (int k = 0; k < 8; ++k) { cd[k].Floats[0] = col[0]; cd[k].Floats[1] = col[1]; cd[k].Floats[2] = col[2]; cd[k].Floats[3] = col[3]; }
    auto async = [&](D3DContext& d)
    {
        for (UINT base = 0; base < kNumResources; base += 8)
        {
            const UINT count = (kNumResources - base < 8) ? (kNumResources - base) : 8;
            D3D12_CPU_DESCRIPTOR_HANDLE handles[8];
            for (UINT k = 0; k < count; ++k)
                handles[k] = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), base + k, rtvSize);
            d.spList->OMSetRenderTargets(count, handles, FALSE, nullptr);
            const UINT mask = (count >= 8) ? 0xFF : ((1u << count) - 1);
            d.spList->ClearBoundRenderTargetViews(mask, cd, nullptr, nullptr);
        }
    };
    out = RunLoop(D3D, kIterations, legacy, async);
}

// ---- Bound DSV clear: ClearBoundDepthStencilView vs ClearDepthStencilView ----
static void BenchmarkClearBoundDSV(D3DContext& D3D, BenchResult& out)
{
    const UINT dim = 512;
    const DXGI_FORMAT fmt = DXGI_FORMAT_D32_FLOAT;
    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC1 desc = CD3DX12_RESOURCE_DESC1::Tex2D(fmt, dim, dim, 1, 1);
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE cv = {}; cv.Format = fmt; cv.DepthStencil.Depth = 1.0f;

    std::vector<CComPtr<ID3D12Resource>> tex(kNumResources);
    for (UINT i = 0; i < kNumResources; ++i)
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&dp, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_BARRIER_LAYOUT_UNDEFINED, &cv, nullptr, 0, nullptr, IID_PPV_ARGS(&tex[i])));

    const UINT dsvSize = D3D.spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CComPtr<ID3D12DescriptorHeap> heap;
    D3D12_DESCRIPTOR_HEAP_DESC hd = {}; hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; hd.NumDescriptors = kNumResources;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&heap)));
    for (UINT i = 0; i < kNumResources; ++i)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE h(heap->GetCPUDescriptorHandleForHeapStart(), i, dsvSize);
        D3D.spDevice->CreateDepthStencilView(tex[i], nullptr, h);
    }

    // One-time: initialize depth metadata (DISCARD) and move to DEPTH_STENCIL_WRITE.
    RunUntimed(D3D, [&](D3DContext& d)
    {
        std::vector<D3D12_TEXTURE_BARRIER> bs(kNumResources);
        for (UINT i = 0; i < kNumResources; ++i)
        {
            D3D12_TEXTURE_BARRIER& b = bs[i];
            b.SyncBefore = D3D12_BARRIER_SYNC_NONE; b.SyncAfter = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
            b.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS; b.AccessAfter = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
            b.LayoutBefore = D3D12_BARRIER_LAYOUT_UNDEFINED; b.LayoutAfter = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
            b.pResource = tex[i]; b.Subresources.IndexOrFirstMipLevel = 0xffffffff;
            b.Flags = D3D12_TEXTURE_BARRIER_FLAG_DISCARD;
        }
        D3D12_BARRIER_GROUP g = {}; g.Type = D3D12_BARRIER_TYPE_TEXTURE; g.NumBarriers = kNumResources; g.pTextureBarriers = bs.data();
        d.spList->Barrier(1, &g);
    });

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < kNumResources; ++i)
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE h(heap->GetCPUDescriptorHandleForHeapStart(), i, dsvSize);
            d.spList->ClearDepthStencilView(h, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }
    };

    auto async = [&](D3DContext& d)
    {
        for (UINT i = 0; i < kNumResources; ++i)
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE h(heap->GetCPUDescriptorHandleForHeapStart(), i, dsvSize);
            d.spList->OMSetRenderTargets(0, nullptr, FALSE, &h);
            d.spList->ClearBoundDepthStencilView(D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }
    };
    out = RunLoop(D3D, kIterations, legacy, async);
}

// ---- Tiled copy: CopyTilesAsync vs CopyTiles. Returns false if tiled resources are unsupported. ----
static bool BenchmarkCopyTiles(D3DContext& D3D, BenchResult& out)
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS opt = {};
    D3D.spDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &opt, sizeof(opt));
    if (opt.TiledResourcesTier == D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED)
        return false;

    const UINT tileBytes = 65536; // 64 KiB per tile (D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES)
    const UINT numTiles = 256;

    CD3DX12_RESOURCE_DESC rdesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)numTiles * tileBytes);
    CComPtr<ID3D12Resource> tiled;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateReservedResource(&rdesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&tiled)));

    CComPtr<ID3D12Heap> heap;
    CD3DX12_HEAP_DESC heapDesc((UINT64)numTiles * tileBytes, D3D12_HEAP_TYPE_DEFAULT);
    heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

    D3D12_TILED_RESOURCE_COORDINATE startCoord = { 0, 0, 0, 0 };
    D3D12_TILE_REGION_SIZE regionSize = {}; regionSize.NumTiles = numTiles; regionSize.UseBox = FALSE;
    D3D12_TILE_RANGE_FLAGS rangeFlags = D3D12_TILE_RANGE_FLAG_NONE;
    UINT heapStart = 0, rangeCount = numTiles;
    D3D.spQueue->UpdateTileMappings(tiled, 1, &startCoord, &regionSize, heap, 1, &rangeFlags, &heapStart, &rangeCount, D3D12_TILE_MAPPING_FLAG_NONE);

    CD3DX12_HEAP_PROPERTIES dp(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC srcDesc = CD3DX12_RESOURCE_DESC::Buffer(tileBytes);
    CComPtr<ID3D12Resource> srcBuf;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&dp, D3D12_HEAP_FLAG_NONE, &srcDesc,
        D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&srcBuf)));

    auto legacy = [&](D3DContext& d)
    {
        for (UINT i = 0; i < numTiles; ++i)
        {
            D3D12_TILED_RESOURCE_COORDINATE c = { i, 0, 0, 0 };
            D3D12_TILE_REGION_SIZE sz = {}; sz.NumTiles = 1; sz.UseBox = FALSE;
            d.spList->CopyTiles(tiled, &c, &sz, srcBuf, 0, D3D12_TILE_COPY_FLAG_NONE);
        }
    };
    auto async = [&](D3DContext& d)
    {
        for (UINT i = 0; i < numTiles; ++i)
        {
            D3D12_TILED_RESOURCE_COORDINATE c = { i, 0, 0, 0 };
            D3D12_TILE_REGION_SIZE sz = {}; sz.NumTiles = 1; sz.UseBox = FALSE;
            d.spList->CopyTilesAsync(tiled, &c, &sz, srcBuf, 0, D3D12_TILE_COPY_FLAG_NONE);
        }
    };
    out = RunLoop(D3D, kIterations, legacy, async);
    return true;
}

// ---- End-to-end mixed frame: independent data updates + real rendering in one submission. ----
static void BenchmarkEndToEndMixedFrame(D3DContext& D3D, BenchResult& out, bool& includesNativeOnlyOps)
{
    const UINT kE2EResources = 128;
    const UINT kE2EIterations = 12;
    const UINT kE2EDrawCalls = 256;
    const UINT64 kE2EBufferBytes = 512u * 1024u;

    includesNativeOnlyOps = (D3D.asyncImpl == D3D12_ASYNC_COMMANDS_IMPL_NATIVE);

    CD3DX12_HEAP_PROPERTIES defaultProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Buffer(kE2EBufferBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    std::vector<CComPtr<ID3D12Resource>> fillBuffers;
    std::vector<D3D12_FILL_BUFFER_DESC> fillDescs;
    CComPtr<ID3D12DescriptorHeap> fillGpuHeap;
    CComPtr<ID3D12DescriptorHeap> fillCpuHeap;
    UINT fillDescSize = 0;
    if (includesNativeOnlyOps)
    {
        fillBuffers.resize(kE2EResources);
        for (UINT i = 0; i < kE2EResources; ++i)
            VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &uavDesc,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&fillBuffers[i])));

        fillDescSize = D3D.spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_DESCRIPTOR_HEAP_DESC hd = {};
        hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        hd.NumDescriptors = kE2EResources;
        hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&fillGpuHeap)));
        hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&fillCpuHeap)));

        D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};
        uav.Format = DXGI_FORMAT_R32_TYPELESS;
        uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uav.Buffer.NumElements = static_cast<UINT>(kE2EBufferBytes / 4);
        uav.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        for (UINT i = 0; i < kE2EResources; ++i)
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE g(fillGpuHeap->GetCPUDescriptorHandleForHeapStart(), i, fillDescSize);
            CD3DX12_CPU_DESCRIPTOR_HANDLE c(fillCpuHeap->GetCPUDescriptorHandleForHeapStart(), i, fillDescSize);
            D3D.spDevice->CreateUnorderedAccessView(fillBuffers[i], nullptr, &uav, g);
            D3D.spDevice->CreateUnorderedAccessView(fillBuffers[i], nullptr, &uav, c);
        }

        fillDescs.resize(kE2EResources);
        for (UINT i = 0; i < kE2EResources; ++i)
        {
            D3D12_FILL_BUFFER_DESC& f = fillDescs[i];
            f.pBuffer = fillBuffers[i];
            f.Offset = 0;
            f.FillValue.Uints[0] = 0xDEADBEEF;
            f.Format = DXGI_FORMAT_R32_UINT;
            f.RawPatternSizeInBytes = 0;
            f.RepeatCount = static_cast<UINT>(kE2EBufferBytes / 4);
        }
    }

    CD3DX12_RESOURCE_DESC copyDesc = CD3DX12_RESOURCE_DESC::Buffer(kE2EBufferBytes);
    CComPtr<ID3D12Resource> regionSource;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &copyDesc,
        D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&regionSource)));
    std::vector<CComPtr<ID3D12Resource>> regionDests(kE2EResources);
    for (UINT i = 0; i < kE2EResources; ++i)
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &copyDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&regionDests[i])));
    std::vector<ID3D12Resource*> regionDestPtrs(kE2EResources), regionSrcPtrs(kE2EResources, regionSource);
    std::vector<UINT64> regionDestOffsets(kE2EResources, 0), regionSrcOffsets(kE2EResources, 0), regionSizes(kE2EResources, kE2EBufferBytes);
    for (UINT i = 0; i < kE2EResources; ++i)
        regionDestPtrs[i] = regionDests[i];

    CComPtr<ID3D12Resource> wholeSource;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &copyDesc,
        D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&wholeSource)));
    std::vector<CComPtr<ID3D12Resource>> wholeDests(kE2EResources);
    std::vector<ID3D12Resource*> wholeDestPtrs(kE2EResources), wholeSrcPtrs(kE2EResources, wholeSource);
    for (UINT i = 0; i < kE2EResources; ++i)
    {
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &copyDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&wholeDests[i])));
        wholeDestPtrs[i] = wholeDests[i];
    }

    CComPtr<ID3D12QueryHeap> resolveQh;
    D3D12_QUERY_HEAP_DESC qd = {};
    qd.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    qd.Count = kE2EResources;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateQueryHeap(&qd, IID_PPV_ARGS(&resolveQh)));
    CComPtr<ID3D12Resource> resolveDst;
    CD3DX12_RESOURCE_DESC resolveDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)kE2EResources * sizeof(UINT64));
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &resolveDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resolveDst)));

    const UINT width = 1280;
    const UINT height = 720;
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3DX12_RESOURCE_DESC1 rtDesc = CD3DX12_RESOURCE_DESC1::Tex2D(format, width, height, 1, 1);
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE optClear = {};
    optClear.Format = format;
    CComPtr<ID3D12Resource> rt;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource3(&defaultProps, D3D12_HEAP_FLAG_NONE, &rtDesc,
        D3D12_BARRIER_LAYOUT_RENDER_TARGET, &optClear, nullptr, 0, nullptr, IID_PPV_ARGS(&rt)));

    CComPtr<ID3D12DescriptorHeap> rtvHeap;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D.spDevice->CreateRenderTargetView(rt, nullptr, rtvHandle);

    CComPtr<ID3D12RootSignature> rootSig;
    {
        CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
        rsDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        CComPtr<ID3DBlob> sig, err;
        VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &err));
        VERIFY_SUCCEEDED(D3D.spDevice->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&rootSig)));
    }

    CComPtr<ID3DBlob> vs, ps, err;
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    VERIFY_SUCCEEDED(D3DCompile(g_triangleShader, strlen(g_triangleShader), "triangle", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vs, &err));
    VERIFY_SUCCEEDED(D3DCompile(g_triangleShader, strlen(g_triangleShader), "triangle", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &ps, &err));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = rootSig;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = format;
    psoDesc.SampleDesc.Count = 1;
    CComPtr<ID3D12PipelineState> pso;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

    struct Vertex { float pos[3]; float color[4]; };
    Vertex verts[] =
    {
        { {  0.0f,  0.75f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.75f, -0.75f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.75f, -0.75f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
    };
    CComPtr<ID3D12Resource> vb;
    CD3DX12_HEAP_PROPERTIES uploadProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(verts));
    VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vb)));
    {
        void* p = nullptr;
        D3D12_RANGE empty = { 0, 0 };
        VERIFY_SUCCEEDED(vb->Map(0, &empty, &p));
        memcpy(p, verts, sizeof(verts));
        vb->Unmap(0, nullptr);
    }
    D3D12_VERTEX_BUFFER_VIEW vbv = { vb->GetGPUVirtualAddress(), sizeof(verts), sizeof(Vertex) };

    D3D12_VIEWPORT vp = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
    D3D12_RECT sc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    const UINT legacyFillVals[4] = { 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF };
    const float clearColor[4] = { 0.01f, 0.01f, 0.08f, 1.0f };

    auto recordRender = [&](D3DContext& d)
    {
        d.spList->RSSetViewports(1, &vp);
        d.spList->RSSetScissorRects(1, &sc);
        d.spList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        d.spList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        d.spList->SetGraphicsRootSignature(rootSig);
        d.spList->SetPipelineState(pso);
        d.spList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d.spList->IASetVertexBuffers(0, 1, &vbv);
        for (UINT draw = 0; draw < kE2EDrawCalls; ++draw)
            d.spList->DrawInstanced(3, 1, 0, 0);
    };

    auto legacy = [&](D3DContext& d)
    {
        if (includesNativeOnlyOps)
        {
            ID3D12DescriptorHeap* heaps[] = { fillGpuHeap };
            d.spList->SetDescriptorHeaps(1, heaps);
            for (UINT i = 0; i < kE2EResources; ++i)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE g(fillGpuHeap->GetGPUDescriptorHandleForHeapStart(), i, fillDescSize);
                CD3DX12_CPU_DESCRIPTOR_HANDLE c(fillCpuHeap->GetCPUDescriptorHandleForHeapStart(), i, fillDescSize);
                d.spList->ClearUnorderedAccessViewUint(g, c, fillBuffers[i], legacyFillVals, 0, nullptr);
            }
        }

        for (UINT i = 0; i < kE2EResources; ++i)
            d.spList->CopyBufferRegion(regionDests[i], 0, regionSource, 0, kE2EBufferBytes);

        for (UINT i = 0; i < kE2EResources; ++i)
            d.spList->CopyResource(wholeDests[i], wholeSource);

        for (UINT i = 0; i < kE2EResources; ++i)
            d.spList->EndQuery(resolveQh, D3D12_QUERY_TYPE_TIMESTAMP, i);
        for (UINT i = 0; i < kE2EResources; ++i)
            d.spList->ResolveQueryData(resolveQh, D3D12_QUERY_TYPE_TIMESTAMP, i, 1, resolveDst, static_cast<UINT64>(i) * sizeof(UINT64));

        recordRender(d);
    };

    auto async = [&](D3DContext& d)
    {
        if (includesNativeOnlyOps)
            d.spList->FillBuffers(kE2EResources, fillDescs.data());

        d.spList->CopyBufferRegions(kE2EResources, regionDestPtrs.data(), regionDestOffsets.data(),
            regionSrcPtrs.data(), regionSrcOffsets.data(), regionSizes.data());
        d.spList->CopyResources(kE2EResources, wholeDestPtrs.data(), wholeSrcPtrs.data());

        for (UINT i = 0; i < kE2EResources; ++i)
            d.spList->EndQuery(resolveQh, D3D12_QUERY_TYPE_TIMESTAMP, i);
        for (UINT i = 0; i < kE2EResources; ++i)
            d.spList->ResolveQueryDataAsync(resolveQh, D3D12_QUERY_TYPE_TIMESTAMP, i, 1, resolveDst, static_cast<UINT64>(i) * sizeof(UINT64));

        recordRender(d);
    };

    out = RunLoop(D3D, kE2EIterations, legacy, async);
}

//======================================================================================================================
int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_stricmp(argv[i], "-warp") == 0 || _stricmp(argv[i], "/warp") == 0)
            g_useWarpDevice = true;
        else if (_stricmp(argv[i], "-fallback") == 0 || _stricmp(argv[i], "/fallback") == 0)
            g_forceFallback = true;
        else if (_stricmp(argv[i], "-list") == 0 || _stricmp(argv[i], "/list") == 0)
            g_listAdapters = true;
        else if (_stricmp(argv[i], "-e2e") == 0 || _stricmp(argv[i], "/e2e") == 0)
            g_runEndToEnd = true;
        else if ((_stricmp(argv[i], "-adapter") == 0 || _stricmp(argv[i], "/adapter") == 0) && i + 1 < argc)
            g_adapterIndex = atoi(argv[++i]);
        else
        {
            PRINT(std::string(" Unrecognized option: ") + argv[i]);
            PRINT(" Usage: D3D12AsyncCommands [-list] [-adapter <index>] [-warp] [-fallback] [-e2e]");
            return -1;
        }
    }

    try
    {
        PRINT("==================================================================================");
        PRINT(" D3D12 Async Commands (Batched Asynchronous Command List APIs)");
        PRINT(" Agility SDK 620");
        PRINT("==================================================================================\n");

        if (g_listAdapters)
        {
            ListAdapters();
            return 0;
        }

        // Opting into the runtime fallback makes the async commands lower onto their legacy
        // counterparts, which is how they behave on a driver without native support.
        if (g_forceFallback)
        {
            UUID fallbackFeature[] = { D3D12ExperimentalForceAsyncCommandsFallback };
            if (FAILED(D3D12EnableExperimentalFeatures(1, fallbackFeature, nullptr, nullptr)))
            {
                PRINT(" Could not enable D3D12ExperimentalForceAsyncCommandsFallback (is Developer Mode on?).");
                return -1;
            }
            PRINT(" Forcing the runtime async-commands fallback.\n");
        }

#if defined(_DEBUG)
        CComPtr<ID3D12Debug1> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
            debug->EnableDebugLayer();
#endif

        D3DContext D3D;

        // An explicitly named adapter is never silently swapped for WARP; that would hide a driver
        // that does not implement the feature, which is the thing a driver test needs to catch.
        const bool adapterWasRequested = g_useWarpDevice || g_adapterIndex >= 0;
        bool ok = InitDeviceAndContext(D3D, g_useWarpDevice, g_adapterIndex);

        if (!ok && adapterWasRequested)
        {
            PRINT(" The requested adapter does not support async commands. Aborting.");
            PRINT(" Run with -list to see the available adapters.");
            return -1;
        }
        if (!ok)
        {
            PRINT(" Async commands not supported on the default adapter. Falling back to WARP...\n");
            ok = InitDeviceAndContext(D3D, true, -1);
        }
        if (!ok)
        {
            PRINT(" Async commands are not supported on this driver or the installed WARP. Aborting.");
            PRINT(" (Requires WARP or a driver that implements async commands.)");
            return -1;
        }

        PRINT(" Device under test:");
        PrintAdapterDetails(D3D.spAdapter);

        if (D3D.asyncImpl == D3D12_ASYNC_COMMANDS_IMPL_NATIVE)
        {
            PRINT("   Async commands : NATIVE (driver-implemented)\n");
        }
        else
        {
            PRINT("   Async commands : FALLBACK - the runtime lowers them onto the legacy serialized");
            PRINT("                    path, so the comparisons below measure the same work twice.\n");
        }

        bool pass = true;
        if (!g_forceFallback)
            pass = RenderTriangleWithAsyncClear(D3D);

        PRINT("[2] Benchmarking async batched commands vs legacy serialized commands");
        {
            std::ostringstream os;
            os << "    (" << kNumResources << " independent resources, "
               << (kBufferSize >> 10) << " KiB buffers, " << kIterations << " averaged iterations)\n";
            PRINT(os.str());
        }

        // Only these four async commands have a runtime lowering onto their legacy counterparts;
        // the rest reject the call when the fallback is forced.
        struct Benchmark
        {
            const char* title;
            const char* legacyName;
            const char* asyncName;
            bool        hasRuntimeFallback;
            const char* note;
            std::function<bool(D3DContext&, BenchResult&)> run;
        };

        const Benchmark benchmarks[] =
        {
            { "Buffer fill", "ClearUnorderedAccessViewUint x N", "FillBuffers (batched)", false, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkFillVsUavClear(d, r); return true; } },
            { "Buffer region copy", "CopyBufferRegion x N", "CopyBufferRegions (batched)", true, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkCopy(d, r); return true; } },
            { "Whole-resource copy", "CopyResource x N", "CopyResources (batched)", true, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkCopyResources(d, r, kBufferSize); return true; } },
            { "Texture region copy", "CopyTextureRegion x N", "CopyTextureRegions (batched)", true, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkCopyTextureRegions(d, r); return true; } },
            { "MSAA resolve", "ResolveSubresourceRegion x N", "ResolveSubresourceRegionAsync x N", false, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkResolve(d, r); return true; } },
            { "Query resolve", "ResolveQueryData x N", "ResolveQueryDataAsync x N", true, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkResolveQueryData(d, r); return true; } },
            { "Texture clear", "ClearRenderTargetView x N", "ClearTextureSubresources (batched)", false, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkTextureClear(d, r); return true; } },
            { "Bound RTV clear", "ClearRenderTargetView x N", "ClearBoundRenderTargetViews (8/call)", false,
              "  ClearBound* are raster-ordered (serialized with Draw* by design), so their benefit is\n"
              "  ergonomics - in/mid-render-pass clears, batching bound targets, lower CPU-record cost -\n"
              "  rather than GPU overlap.",
              [](D3DContext& d, BenchResult& r) { BenchmarkClearBoundRTVs(d, r); return true; } },
            { "Bound DSV clear", "ClearDepthStencilView x N", "ClearBoundDepthStencilView x N", false, nullptr,
              [](D3DContext& d, BenchResult& r) { BenchmarkClearBoundDSV(d, r); return true; } },
            { "Tiled copy", "CopyTiles x N", "CopyTilesAsync x N", false, nullptr,
              [](D3DContext& d, BenchResult& r)
              {
                  try { return BenchmarkCopyTiles(d, r); }
                  catch (HRESULT) { return false; }
              } },
        };

        for (const Benchmark& b : benchmarks)
        {
            if (g_forceFallback && !b.hasRuntimeFallback)
                continue;
            if (b.note)
                PRINT(b.note);

            BenchResult r;
            if (b.run(D3D, r))
                PrintBench(b.title, b.legacyName, b.asyncName, r);
            else
                PRINT(std::string("  ") + b.title + ": skipped (not supported on this adapter).");
            PRINT("");
        }

        if (g_forceFallback)
        {
            PRINT("  Skipped - no runtime fallback: FillBuffers, ClearTextureSubresources,");
            PRINT("  ClearBoundRenderTargetViews, ClearBoundDepthStencilView,");
            PRINT("  ResolveSubresourceRegionAsync, CopyTilesAsync.\n");
        }

        if (g_runEndToEnd)
        {
            PRINT("[3] End-to-end mixed frame: data updates + rendering in one submission");
            PRINT("    This path models practical frame work (resource updates plus draw calls),");
            PRINT("    then compares serialized legacy recording against batched async APIs.\n");

            bool includesNativeOnlyOps = false;
            BenchResult e2e = {};
            BenchmarkEndToEndMixedFrame(D3D, e2e, includesNativeOnlyOps);
            PrintBench("End-to-end mixed frame",
                includesNativeOnlyOps ? "Legacy: ClearUAV + Copy + ResolveQuery + Draw"
                                      : "Legacy: Copy + ResolveQuery + Draw",
                includesNativeOnlyOps ? "Async: FillBuffers + Copy* + ResolveQueryAsync + Draw"
                                      : "Async: Copy* + ResolveQueryAsync + Draw",
                e2e);
            if (!includesNativeOnlyOps)
            {
                PRINT("    Note: FillBuffers has no runtime fallback lowering, so fallback mode");
                PRINT("          uses the shared copy/resolve/render subset for a fair comparison.\n");
            }
            else
            {
                PRINT("");
            }
        }

        PRINT(pass ? " Done." : " Done, with failures.");
        return pass ? 0 : -1;
    }
    catch (HRESULT)
    {
        PRINT(" Aborting due to a fatal error.");
        return -1;
    }
    return 0;
}
