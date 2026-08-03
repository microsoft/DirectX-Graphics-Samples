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
#include "D3D12RaytracingClusteredGeometry.h"
#include "DirectXRaytracingHelper.h"
#include "CompiledShaders\\Raytracing.hlsl.h"
#include "CompiledShaders\\AnimateBall.hlsl.h"
#include "CompiledShaders\\FillInstantiateArgs.hlsl.h"
#include "CompiledShaders\\FillMoveClusterArgs.hlsl.h"
#include "CompiledShaders\\FillBlasFromClasArgs.hlsl.h"
#include "CompiledShaders\\FillClasFromTrianglesArgs.hlsl.h"
#include "CompiledShaders\\FillClusterTemplateArgs.hlsl.h"
#include "SceneData.h"
#include "MaterialData.h"

#include <DirectXMath.h>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cmath>
#include <random>
#include <fstream>
#include <sstream>

using namespace std;
using namespace DX;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// These layouts are emitted manually by GPU shaders.  A d3d12.h revision
// must fail the host build until the matching HLSL byte offsets are updated.
static_assert(sizeof(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS) == 80,
    "Update FillClasFromTrianglesArgs.hlsl for the current d3d12.h layout");
static_assert(sizeof(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS) == 96,
    "Update FillClusterTemplateArgs.hlsl for the current d3d12.h layout");
static_assert(sizeof(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS) == 32,
    "Update FillInstantiateArgs.hlsl for the current d3d12.h layout");
static_assert(sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS) == 16,
    "Update FillBlasFromClasArgs.hlsl for the current d3d12.h layout");
static_assert(sizeof(D3D12_RTAS_OPERATION_MOVE_CLUSTER_OBJECTS_ARGS) == 8,
    "Update FillMoveClusterArgs.hlsl for the current d3d12.h layout");

// Size checks alone miss same-size field reorders. Lock every offset written by
// the raw ByteAddressBuffer shaders to the corresponding public C++ member.
#define CHECK_FIELD_OFFSET(type, member, expected) \
    static_assert(offsetof(type, member) == expected, \
        "Update the GPU argument writer for " #type "::" #member)
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, ClusterID, 0);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, ClusterFlags, 4);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, TriangleCount, 8);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, VertexCount, 10);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, BaseGeometryIndexAndFlags, 12);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, OpacityMicromapBaseLocation, 16);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, VertexBufferStride, 20);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, IndexBufferStride, 22);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, OpacityMicromapIndexBufferStride, 24);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, GeometryIndexAndFlagsArrayStride, 26);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, PositionTruncateBitCount, 28);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, ReservedPadding, 30);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, VertexBuffer, 32);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, IndexBuffer, 40);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, GeometryIndexAndFlagsArray, 48);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, GeometryIndexAndFlagsIndexBuffer, 56);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, OpacityMicromapArray, 64);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS, OpacityMicromapIndexBuffer, 72);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS, TrianglesArgs, 0);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS, InstantiationBoundingBoxLimit, 80);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS, Compressed1, 88);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS, GeometryIndexOffset, 0);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS, ClusterIdOffset, 4);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS, ClusterTemplate, 8);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS, VertexBuffer, 16);
CHECK_FIELD_OFFSET(D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE, StartAddress, 0);
CHECK_FIELD_OFFSET(D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE, StrideInBytes, 8);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS, ClasAddressCount, 0);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS, ClasAddressStride, 4);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS, ClasAddressArray, 8);
CHECK_FIELD_OFFSET(D3D12_RTAS_OPERATION_MOVE_CLUSTER_OBJECTS_ARGS, SourceAccelerationStructure, 0);
#undef CHECK_FIELD_OFFSET

static_assert(sizeof(ClusterMeta) == 48,
    "Update Raytracing.hlsl LoadClusterMeta for the current CPU layout");
static_assert(offsetof(ClusterMeta, colorIndex) == 0);
static_assert(offsetof(ClusterMeta, flags) == 4);
static_assert(offsetof(ClusterMeta, overrideRefl) == 8);
static_assert(offsetof(ClusterMeta, overrideRefr) == 12);
static_assert(offsetof(ClusterMeta, overrideIor) == 16);
static_assert(offsetof(ClusterMeta, baseColorScale) == 20);
static_assert(offsetof(ClusterMeta, surfTintMul) == 24);
static_assert(offsetof(ClusterMeta, refrTintMul) == 28);
static_assert(offsetof(ClusterMeta, reflTintMul) == 32);
static_assert(offsetof(ClusterMeta, materialSlot) == 36);
static_assert(offsetof(ClusterMeta, _pad0) == 40);
static_assert(offsetof(ClusterMeta, _pad1) == 44);

// ==== Shader entry-point names (must match Raytracing.hlsl) ====
const wchar_t* D3D12RaytracingClusteredGeometry::c_raygenName            = L"RayGen";
// Two specialised closesthit shaders + two primary hit groups:
//   OpaqueHitGroup binds OpaqueHit, NO any-hit (any-hit dispatch is
//   skipped entirely for opaque-only instances - significantly cheaper
//   for chrome / pure-matte traversals).
//   GlassHitGroup binds GlassHit + GlassAnyHit (anyhit handles
//   stochastic translucency; closesthit handles Fresnel + refraction).
const wchar_t* D3D12RaytracingClusteredGeometry::c_opaqueClosestHitName  = L"OpaqueHit";
const wchar_t* D3D12RaytracingClusteredGeometry::c_glassClosestHitName   = L"GlassHit";
const wchar_t* D3D12RaytracingClusteredGeometry::c_glassAnyHitName       = L"GlassAnyHit";
const wchar_t* D3D12RaytracingClusteredGeometry::c_missName              = L"Miss";
const wchar_t* D3D12RaytracingClusteredGeometry::c_shadowMissName        = L"ShadowMiss";
const wchar_t* D3D12RaytracingClusteredGeometry::c_opaqueHitGroupName    = L"OpaqueHitGroup";
const wchar_t* D3D12RaytracingClusteredGeometry::c_glassHitGroupName     = L"GlassHitGroup";
const wchar_t* D3D12RaytracingClusteredGeometry::c_shadowHitGroupName    = L"ShadowHitGroup";

// =====================================================================================
// Construction + command-line + lifecycle
// =====================================================================================
D3D12RaytracingClusteredGeometry::D3D12RaytracingClusteredGeometry(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name)
{
    UpdateForSizeChange(width, height);
}


// =============================================================================
// Input handling moved to InputHandling.cpp:
//   ParseCommandLineArgs, OnKeyDown
// =============================================================================


void D3D12RaytracingClusteredGeometry::OnInit()
{
    // Enable DRED (Device Removed Extended Data) BEFORE device creation so
    // any TDR/device-removed events capture GPU breadcrumbs + page-fault
    // info into the log.  Costs ~0 perf and is invaluable for diagnosing
    // GPU hangs during TLAS/BLAS work.
    {
        ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
        HRESULT hrDred = D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings));
        if (SUCCEEDED(hrDred) && dredSettings) {
            dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            SampleLog::Write(L"OnInit: DRED enabled (breadcrumbs + page-fault)\n");
        } else {
            SampleLog::LogF(L"OnInit: DRED unavailable hr=0x%08X\n", (unsigned)hrDred);
        }
    }
    // ---- DXR2 experimental-features opt-in ----
    // The runtime requires D3D12RaytracingExperiment to be enabled BEFORE any
    // D3D12CreateDevice call in this process. Without it, ClustersAndPTLAS
    // GetRTASOperationPrebuildInfo() calls silently return zero sizes even when
    // the cap reports YES. (D3D12ExperimentalShaderModels is needed for the
    // SM 6.10 raygen/closesthit shaders.)
    {
        UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels, D3D12RaytracingExperiment };
        HRESULT hrExp = D3D12EnableExperimentalFeatures(_countof(experimentalFeatures),
                                                        experimentalFeatures, nullptr, nullptr);
        SampleLog::LogF(L"OnInit: D3D12EnableExperimentalFeatures -> hr=0x%08X (%s)\n",
                        (unsigned)hrExp, SUCCEEDED(hrExp) ? L"OK" : L"FAIL");
        ThrowIfFailed(hrExp,
            L"D3D12EnableExperimentalFeatures failed. Is Windows Developer Mode enabled?\n");
    }

    SampleLog::Write(L"OnInit: creating DeviceResources\n");
    // 5th arg = device-resources options.  0 here means VSYNC ON
    // (Present(1, 0) inside DeviceResources::Present).  Tearing is visible at
    // low framerates without vsync -- this scene's clusters can dip to <60
    // fps under heavy reflection/refraction bounces, so we lock to refresh.
    // If you want VRR / unlocked framerate to measure perf, swap this to
    // DeviceResources::c_RequireTearingSupport (Present(0, ALLOW_TEARING)).
    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        /*options*/0,
        m_adapterIDoverride);
    m_deviceResources->RegisterDeviceNotify(this);
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);
    m_deviceResources->InitializeDXGIAdapter();

    SampleLog::Write(L"OnInit: checking DXR support\n");
    {
        ComPtr<ID3D12Device> testDevice;
        HRESULT hrCreate = D3D12CreateDevice(m_deviceResources->GetAdapter(),
                                             D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice));
        SampleLog::LogF(L"  D3D12CreateDevice(adapter, FL11_0) -> hr=0x%08X (%s)\n",
                        (unsigned)hrCreate, SUCCEEDED(hrCreate) ? L"OK" : L"FAIL");
        ThrowIfFailed(hrCreate);
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts5 = {};
        HRESULT hrFeat = testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts5, sizeof(opts5));
        SampleLog::LogF(L"  CheckFeatureSupport(OPTIONS5) hr=0x%08X, RaytracingTier=0x%X\n",
                        (unsigned)hrFeat, (unsigned)opts5.RaytracingTier);
        ThrowIfFalse(SUCCEEDED(hrFeat) && opts5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED,
            L"ERROR: DirectX Raytracing tier 1.0+ is required.\n\n");
    }

    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();
    CreateDeviceDependentResources();
    SampleLog::Write(L"OnInit: complete\n");
}

void D3D12RaytracingClusteredGeometry::CreateDeviceDependentResources()
{
    auto device      = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)),
        L"ERROR: ID3D12Device5 not available.\n");
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)),
        L"ERROR: ID3D12GraphicsCommandList4 not available.\n");
    ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxr2Device)),
        L"ERROR: ID3D12DeviceRaytracing2 not available - is the experimental D3D12Core loaded?\n");
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxr2CommandList)),
        L"ERROR: ID3D12CommandListRaytracing2 not available.\n");

    // Register an info-queue callback so D3D12 debug-layer messages get
    // mirrored into our SampleLog file before any break-on-severity fires.
    // Without this, the debug layer calls __debugbreak() and (if no debugger
    // is attached) the process exits with STATUS_BREAKPOINT (0xC0000005-ish)
    // without us ever seeing WHY - which makes WARP and other layered failures
    // very hard to diagnose. The callback runs in-process before the break.
    //
    // Also mute id=1328 (CREATERESOURCE_STATE_IGNORED) - this is a benign
    // D3D12 quirk: buffer resources are always created in COMMON state
    // regardless of pInitialState, and the debug layer reminds you per
    // CreateCommittedResource. Filtering it via SetMessageFilter() leaves
    // every other warning category active, so we still see real bugs.
    {
        ComPtr<ID3D12InfoQueue> infoQueue0;
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue0))))
        {
            D3D12_MESSAGE_ID denied[] = {
                D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED, // 1328
            };
            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs  = _countof(denied);
            filter.DenyList.pIDList = denied;
            HRESULT hrFilter = infoQueue0->AddStorageFilterEntries(&filter);
            SampleLog::LogF(L"InfoQueue: muted id=1328 hr=0x%08X\n", (unsigned)hrFilter);
        }

        ComPtr<ID3D12InfoQueue1> infoQueue1;
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue1))))
        {
            DWORD cookie = 0;
            HRESULT hrReg = infoQueue1->RegisterMessageCallback(
                [](D3D12_MESSAGE_CATEGORY /*cat*/,
                   D3D12_MESSAGE_SEVERITY sev,
                   D3D12_MESSAGE_ID id,
                   LPCSTR desc, void* /*ctx*/)
                {
                    // Mute the persistent benign id=1328 in the callback path
                    // too (storage-filter doesn't reach the callback).
                    if (id == D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED) return;
                    const wchar_t* sevStr =
                        sev == D3D12_MESSAGE_SEVERITY_CORRUPTION ? L"CORRUPTION" :
                        sev == D3D12_MESSAGE_SEVERITY_ERROR      ? L"ERROR" :
                        sev == D3D12_MESSAGE_SEVERITY_WARNING    ? L"WARNING" :
                        sev == D3D12_MESSAGE_SEVERITY_INFO       ? L"INFO" :
                                                                   L"MESSAGE";
                    // Convert UTF8 description to wide for SampleLog.
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, desc, -1, nullptr, 0);
                    std::wstring wdesc(wlen > 0 ? wlen - 1 : 0, L'\0');
                    if (wlen > 0)
                        MultiByteToWideChar(CP_UTF8, 0, desc, -1, wdesc.data(), wlen);
                    SampleLog::LogF(L"[D3D12 %s id=%d] %s\n", sevStr, (int)id, wdesc.c_str());
                },
                D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                nullptr, &cookie);
            SampleLog::LogF(L"InfoQueue1 callback registered: hr=0x%08X cookie=%u\n",
                            (unsigned)hrReg, (unsigned)cookie);
        }
        else
        {
            SampleLog::Write(L"InfoQueue1 not available - D3D12 messages won't be mirrored to log\n");
        }
    }

    QueryDXR2Support();
    SampleLog::Write(L">>> BuildScene\n");
    BuildScene();
    SampleLog::Write(L">>> BuildMaterials\n");
    BuildMaterials();
    // CreateAnimationComputePipeline must run BEFORE BuildAccelerationStructures
    // because the latter invokes UpdateAnimatedObjectPerFrame on the init path,
    // which dispatches the AnimateBall compute shader -- needs the PSO + RS.
    SampleLog::Write(L">>> CreateAnimationComputePipeline\n");
    CreateAnimationComputePipeline();
    SampleLog::Write(L">>> CreateFillInstantiateArgsPipeline\n");
    CreateFillInstantiateArgsPipeline();
    SampleLog::Write(L">>> CreateFillMoveArgsPipeline\n");
    CreateFillMoveArgsPipeline();
    SampleLog::Write(L">>> CreateFillBlasArgsPipeline\n");
    CreateFillBlasArgsPipeline();
    SampleLog::Write(L">>> CreateFillClasTriArgsPipeline\n");
    CreateFillClasTriArgsPipeline();
    SampleLog::Write(L">>> CreateFillTemplateArgsPipeline\n");
    CreateFillTemplateArgsPipeline();
    SampleLog::Write(L">>> BuildAccelerationStructures\n");
    BuildAccelerationStructures();
    SampleLog::Write(L">>> BuildClusterShaderSideBuffers\n");
    BuildClusterShaderSideBuffers();
    SampleLog::Write(L">>> BuildTradCidLookup\n");
    BuildTradCidLookup();   // built unconditionally for root-sig binding; cluster path doesn't read it
    SampleLog::Write(L">>> BuildPerInstGeomMaterialTable\n");
    BuildPerInstGeomMaterialTable();
    SampleLog::Write(L">>> BuildClusterMetadata\n");
    BuildClusterMetadata();
    SampleLog::Write(L">>> CreateRaytracingPipelineAndShaderTables\n");
    CreateRaytracingPipelineAndShaderTables();
    SampleLog::Write(L">>> CreateDescriptorHeapAndRaytracingOutput\n");
    CreateDescriptorHeapAndRaytracingOutput();
    SampleLog::Write(L">>> CreateUIFont\n");
    CreateUIFont();
    SampleLog::Write(L">>> CreateDeviceDependentResources DONE\n");

    // Apply --extra-instances <N>= 100/1k/10k from the CLI if it was set.
    // BuildScene + BuildAccelerationStructures above don't read
    // m_extraInstancesMode; the workload-clone path is wired through
    // RebuildStaticAccelerationStructures which the [N] key calls.  Do
    // the same one-shot here so headless / scripted runs honour the
    // CLI flag without needing a scheduled [N] press.
    //
    // The initial BuildAccelerationStructures captured GPU timestamps for
    // m_clasBuildMs / m_blasBuildMs / m_tlasBuildMs / m_totalBuildMs --
    // but that was the BASELINE 8-object build, not the with-clones build
    // we're about to do.  Wall-clock time the rebuild and OVERWRITE
    // m_totalBuildMs with the bigger number so the benchmark JSON reports
    // a build time representative of what was actually built.  The
    // per-pass breakdown (clas/blas/tlas) stays as the GPU-timestamp
    // measurement of the initial build -- those would need timestamp
    // queries in RebuildStaticAccelerationStructures to be accurate at
    // scale, and the headline metric users care about is total time.
    if (m_extraInstancesMode != ExtraInstancesMode::None)
    {
        SampleLog::LogF(L">>> applying --extra-instances %ls (one-shot rebuild)\n",
                        ExtraInstancesModeName());
        auto t0 = std::chrono::steady_clock::now();
        RebuildStaticAccelerationStructures(L"--extra-instances startup");
        auto t1 = std::chrono::steady_clock::now();
        const double rebuildMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        SampleLog::LogF(L"  rebuild wall-clock: %.1f ms (was %.1f ms for baseline; "
                        L"using rebuild as build_times_ms.total)\n",
                        rebuildMs, m_totalBuildMs);
        m_totalBuildMs = rebuildMs;
    }
}

void D3D12RaytracingClusteredGeometry::QueryDXR2Support()
{
    auto device = m_deviceResources->GetD3DDevice();
    D3D12_FEATURE_DATA_D3D12_OPTIONS_EXPERIMENTAL opts = {};
    HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS_EXPERIMENTAL, &opts, sizeof(opts));
    m_clustersAndPtlasSupported = SUCCEEDED(hr) && opts.ClustersAndPTLASSupported;

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts5 = {};
    device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts5, sizeof(opts5));

    SampleLog::LogF(L"\n[DXR2] adapter: %s\n", m_deviceResources->GetAdapterDescription());
    SampleLog::LogF(L"  driver: %s   VID:%04X  PID:%04X   dedicatedVRAM: %.0f MB\n",
                    *m_deviceResources->GetDriverVersionString() ? m_deviceResources->GetDriverVersionString() : L"(unknown)",
                    m_deviceResources->GetAdapterVendorId(),
                    m_deviceResources->GetAdapterDeviceId(),
                    (double)m_deviceResources->GetDedicatedVideoMemoryBytes() / (1024.0 * 1024.0));
    SampleLog::LogF(L"  D3D12_RAYTRACING_TIER: 0x%X\n", (unsigned)opts5.RaytracingTier);
    SampleLog::LogF(L"  ClustersAndPTLASSupported: %s   (hr=0x%08X)\n",
                    m_clustersAndPtlasSupported ? L"YES" : L"NO", (unsigned)hr);

    // Resolve the auto-default for m_aaSamplesPerPixel now that the adapter
    // description is known.  Sentinel 0 in the header means "not set by
    // CLI"; an explicit --aa-samples N takes precedence and skips this
    // branch.  WARP / Basic Render -> 1 (4x on a software rasterizer
    // pushes a 1280x720 frame to seconds-per-frame); HW -> 4.
    if (m_aaSamplesPerPixel == 0)
    {
        const wchar_t* desc = m_deviceResources->GetAdapterDescription();
        const bool isSoftware = (wcsstr(desc, L"WARP") != nullptr) ||
                                (wcsstr(desc, L"Basic Render") != nullptr);
        m_aaSamplesPerPixel = isSoftware ? 1u : 4u;
        SampleLog::LogF(L"  auto AA samples-per-pixel: %u  (%ls adapter)\n",
                        m_aaSamplesPerPixel, isSoftware ? L"software" : L"hardware");
    }
    // Same auto-resolve for the per-frame timing snapshot window.  60
    // samples on HW (~1 s at 60 fps), 5 samples on WARP (~30-50 s at
    // 6-10 s/frame).  Without this, the PER-FRAME overlay line shows
    // "recalculating..." for 10+ minutes on WARP after each toggle.
    if (m_pfSnapTargetCount == 0)
    {
        const wchar_t* desc = m_deviceResources->GetAdapterDescription();
        const bool isSoftware = (wcsstr(desc, L"WARP") != nullptr) ||
                                (wcsstr(desc, L"Basic Render") != nullptr);
        m_pfSnapTargetCount = isSoftware ? 5 : 60;
        SampleLog::LogF(L"  auto per-frame snap samples: %d  (%ls adapter)\n",
                        m_pfSnapTargetCount, isSoftware ? L"software" : L"hardware");
    }

    // Wall-clock FPS rolling-average window size.  Same adapter-aware
    // sentinel pattern.  60-frame window on HW (~0.5-1 s, smooths vsync
    // jitter), 3-frame window on WARP (frames are seconds long, a
    // large window would lag behind state changes by minutes).
    if (m_frameTimeWindow == 0)
    {
        const wchar_t* desc = m_deviceResources->GetAdapterDescription();
        const bool isSoftware = (wcsstr(desc, L"WARP") != nullptr) ||
                                (wcsstr(desc, L"Basic Render") != nullptr);
        m_frameTimeWindow = isSoftware ? 3u : 60u;
        m_frameTimeRing.assign(m_frameTimeWindow, 0.0);
        m_frameTimeRingIdx   = 0;
        m_frameTimeRingCount = 0;
        m_frameTimeRingSum   = 0.0;
        SampleLog::LogF(L"  auto FPS rolling window: %u frames  (%ls adapter)\n",
                        m_frameTimeWindow, isSoftware ? L"software" : L"hardware");
    }

    if (!m_clustersAndPtlasSupported)
    {
        // Hardware doesn't support clusters -- lock the sample into
        // Traditional (DXR1) mode.  The [T] toggle becomes a no-op, the
        // overlay shows a "(locked, clusters unsupported)" hint, and the
        // window title flags the fallback so it's obvious before the
        // overlay even renders.  All cluster-path init / per-frame work
        // is gated on m_clustersAndPtlasSupported below.
        m_geometryMode = GeometryMode::Traditional;
        SampleLog::Write(L"  -> clusters unsupported; falling back to traditional BLAS, [T] toggle disabled.\n");
        SetCustomWindowText(L"clusters not supported on this adapter -- traditional BLAS fallback");
    }
}


// =============================================================================
// Scene definition moved to SceneSetup.cpp:
//   BuildScene, BuildMaterials
// =============================================================================


// ---------------------------------------------------------------------------------
// Re-encode every cluster's positions into Compressed1 blobs using the live
// m_compressedBitsPerComponent.  Called once from BuildScene at startup and
// again from RebuildStaticAccelerationStructures when the user cycles the
// precision slider ('[' / ']' in COMPRESSED1 mode).
//
// We always populate obj.encoded even in FLOAT32_3 mode so the 'v' toggle
// can flip vertex format without re-running scene build.  Cost is pure CPU
// and ~50 ms one-time at scene scale; re-encode is comparable.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::EncodeCompressedClusters()
{
    const UINT bitsPerComp = m_compressedBitsPerComponent;

    // Pick a single shared compressed1 exponent across the WHOLE scene (all
    // clusters in all objects).  Same-grid quantization keeps adjacent
    // clusters' shared-edge vertices bit-identical -> watertight at the
    // quantization level (BVH-leaf precision is a separate matter).
    int sharedExponent;
    {
        float maxExtent = 0.f;
        for (const auto& obj : m_objects)
        for (const auto& c   : obj.mesh.clusters)
        {
            float mn[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX };
            float mx[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX };
            for (const auto& p : c.positions)
            {
                const float ps[3] = { p.x, p.y, p.z };
                for (int k = 0; k < 3; ++k) { mn[k] = std::min(mn[k], ps[k]); mx[k] = std::max(mx[k], ps[k]); }
            }
            for (int k = 0; k < 3; ++k) maxExtent = std::max(maxExtent, mx[k] - mn[k]);
        }
        const float minUnit = maxExtent / float((1ull << bitsPerComp) - 1);
        sharedExponent = (int)std::ceil(std::log2(minUnit)) + 127;
        if (sharedExponent < 1)   sharedExponent = 1;
        if (sharedExponent > 232) sharedExponent = 232;
        SampleLog::LogF(L"[compressed1] %u bits/comp; shared exponent (biased): %d  (unit=%.6f)\n",
                        bitsPerComp, sharedExponent, std::ldexp(1.0f, sharedExponent - 127));
    }

    size_t totalCompressedBytes = 0, totalUncompressedBytes = 0;
    float  maxRoundtripError = 0.f;
    for (auto& obj : m_objects)
    {
        obj.encoded.clear();                                 // re-encode wipes any prior blobs
        obj.encoded.reserve(obj.mesh.clusters.size());
        for (const auto& c : obj.mesh.clusters)
        {
            auto enc = Compressed1::Encode(c.positions, (int)bitsPerComp, sharedExponent);
            auto decoded = Compressed1::Decode(enc);
            for (size_t i = 0; i < c.positions.size(); ++i)
            {
                float dx = decoded[i].x - c.positions[i].x;
                float dy = decoded[i].y - c.positions[i].y;
                float dz = decoded[i].z - c.positions[i].z;
                maxRoundtripError = std::max(maxRoundtripError, std::sqrt(dx*dx + dy*dy + dz*dz));
            }
            totalCompressedBytes   += enc.TotalBytes();
            totalUncompressedBytes += c.positions.size() * sizeof(ProceduralGeometry::float3);
            obj.encoded.push_back(std::move(enc));
        }
    }
    SampleLog::LogF(L"[compressed1] %u clusters @ %u bits/comp -> %zu bytes (vs %zu uncompressed = %.2fx); "
                    L"worst-case roundtrip error %.6f units\n",
                    m_totalClusterCount, bitsPerComp,
                    totalCompressedBytes, totalUncompressedBytes,
                    (double)totalUncompressedBytes / (double)totalCompressedBytes,
                    maxRoundtripError);
}



// =====================================================================================
// Acceleration structure build orchestration.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildAccelerationStructures()
{
    auto device           = m_deviceResources->GetD3DDevice();
    auto commandList      = m_deviceResources->GetCommandList();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();
    auto commandQueue     = m_deviceResources->GetCommandQueue();

    // ---- Create the build-timestamp query heap + readback buffer. We straddle
    // each ExecuteIndirectRTASOperations / classic TLAS build with two EndQuery
    // calls so we can report per-operation wall-clock times to the title bar. ----
    {
        D3D12_QUERY_HEAP_DESC qhd = {};
        qhd.Type     = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        qhd.Count    = kBuildTimestampCount;
        qhd.NodeMask = 0;
        ThrowIfFailed(device->CreateQueryHeap(&qhd, IID_PPV_ARGS(&m_buildQueryHeap)));
        m_buildQueryHeap->SetName(L"AS build timestamp query heap");

        auto rbHeap  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64) * kBuildTimestampCount);
        ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE,
            &bufDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_buildQueryReadback)));
        m_buildQueryReadback->SetName(L"AS build timestamp readback");

        ThrowIfFailed(commandQueue->GetTimestampFrequency(&m_timestampFrequency));
        SampleLog::LogF(L"[stats] timestamp frequency: %llu ticks/sec\n",
                        (unsigned long long)m_timestampFrequency);
    }

    // Per-frame timestamp heap + readback. 3 ring slots * 6 timestamps each.
    // Per-frame writes to one slot, resolves to its matching range in the
    // readback buffer; CPU reads from the slot ~3 frames behind the write
    // (safe past GPU completion).
    {
        D3D12_QUERY_HEAP_DESC qhd = {};
        qhd.Type  = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        qhd.Count = kPerFrameTsPerSlot * kPerFrameRingSlots;
        ThrowIfFailed(device->CreateQueryHeap(&qhd, IID_PPV_ARGS(&m_pfQueryHeap)));
        m_pfQueryHeap->SetName(L"AS per-frame timestamp heap");

        auto rbHeap  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer(
            sizeof(UINT64) * kPerFrameTsPerSlot * kPerFrameRingSlots);
        ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE,
            &bufDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pfQueryReadback)));
        m_pfQueryReadback->SetName(L"AS per-frame timestamp readback");
    }

    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

    if (m_geometryMode == GeometryMode::Clusters)
    {
        UploadClusterInputs();

        // Stamp slot 0 (before CLAS build).
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
        BuildClasIndirect();
        // Stamp slot 1 (after CLAS UAV barrier).
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);

        // Stamp slot 2 (before BLAS build).
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 2);
        BuildBlasFromClasIndirect();
        // Stamp slot 3 (after BLAS UAV barrier).
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 3);

        // Animated object: build cluster templates once, then do a first
        // per-frame instantiate+BLAS so the TLAS instance points at valid BVH
        // contents from frame 0. (Subsequent frames re-run UpdateAnimatedObjectPerFrame
        // from DoRender.) NOTE: BuildAnimatedObjectSetup itself flushes the cmd
        // list partway through (CPU readback of the template GVA array), so it
        // must run BEFORE the per-build EndQuery pair below or the resolve at
        // bottom would resolve mid-flushed timestamps.
        BuildAnimatedObjectSetup();
        UpdateAnimatedObjectPerFrame();
    }
    else
    {
        // Traditional (DXR1) per-object BLAS path.  Slots 0/1 (CLAS) are
        // stamped with a no-op pair so the ResolveQueryData below has a
        // complete range to resolve; overlay shows them as 0 build time
        // (the truth -- trad mode has no CLAS).  Slot 2/3 bracket the
        // traditional BLAS build for the wall-clock readout.
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 2);
        BuildTraditionalStaticAS();
        commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 3);
        // Traditional path's animated ball: build the mesh + per-frame VB +
        // restPos + flat IB + BLAS storage.  Subsequent frames just rebuild/
        // refit tradBlasStorage in UpdateAnimatedTradPerFrame -- this init
        // pass also kicks one rebuild so the BLAS contents are valid before
        // the TLAS build below references it.
        BuildAnimatedObjectSetup();
        UpdateAnimatedTradPerFrame();
    }

    // Stamp slot 4 (before TLAS build).
    commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 4);
    BuildTlasClassic();
    // Stamp slot 5 (after TLAS UAV barrier).
    commandList->EndQuery(m_buildQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 5);

    // Resolve into the readback buffer. NumQueries=6 because we only wrote slots 0..5
    // (the heap has spare capacity for future per-frame timestamps).
    commandList->ResolveQueryData(m_buildQueryHeap.Get(),
        D3D12_QUERY_TYPE_TIMESTAMP, 0, 6,
        m_buildQueryReadback.Get(), 0);

    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();

    ReadBuildTimestamps();
    if (m_geometryMode == GeometryMode::Clusters)
        DumpClusterStatsAsync();

    // Initial snapshot of overlay stats so the first frame of rendering shows
    // the correct numbers (subsequent config changes refresh via the
    // CaptureOverlayStatsSnapshot call at the bottom of
    // RebuildStaticAccelerationStructures).
    CaptureOverlayStatsSnapshot();
}

// ---------------------------------------------------------------------------------
// Hook for Win32Application::Run -- decide between SW_NORMAL and SW_MAXIMIZE
// on the initial ShowWindow.  Two opt-outs:
//   1. Software adapter (WARP / Basic Render) -- maximizing on a CPU
//      rasterizer turns each frame into a seconds-long affair; the window
//      stays at the launched 1280x720 so the app is at least interactive.
//   2. Headless run (any of --screenshot-at, --screenshot-frame,
//      --exit-after-frames is set) -- the back-buffer size is significant
//      because captured screenshots use it; maximizing would silently
//      change the screenshot resolution from 1280x720 to the user's
//      desktop res, breaking any reference-image diff.
// Otherwise (interactive HW run) -- maximize, since the 4K canvas is
// where the cluster geometry actually showcases.
// ---------------------------------------------------------------------------------
bool D3D12RaytracingClusteredGeometry::ShouldMaximizeWindowOnLaunch() const
{
    if (m_screenshotAtSeconds >= 0.0 || m_screenshotFrame >= 0 || m_exitAfterFrames > 0)
        return false;
    const wchar_t* desc = m_deviceResources->GetAdapterDescription();
    const bool isSoftware = (wcsstr(desc, L"WARP") != nullptr) ||
                            (wcsstr(desc, L"Basic Render") != nullptr);
    return !isSoftware;
}


// =============================================================================
// Scene-specific clone generation moved to ScenePopulation.cpp:
//   EnsureCloneSourceLodMeshes, RegenerateWorkloadCloneInstances
// =============================================================================



// ---------------------------------------------------------------------------------
// Runtime tear-down + rebuild of the STATIC half of the AS pipeline (CLAS,
// BLAS-from-CLAS, TLAS).  Animated object's per-frame INSTANTIATE + BLAS
// rebuild uses its own buffers + scratch and is intentionally untouched -
// it continues to render correctly across the rebuild because:
//   1. m_animatedObject.blasGPUVA is stable (EXPLICIT_DESTINATIONS).
//   2. The TLAS instance desc array we rebuild references that same GVA.
//
// Drives the 'a' (cycle CLAS alloc mode) and 'v' (cycle vertex format)
// keyboard shortcuts in OnKeyDown.  Per-frame cost is one full GPU flush
// per press; perfectly fine for a tech-demo toggle but not something you
// would hook into a hot path in production code.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::RebuildStaticAccelerationStructures(const wchar_t* reason)
{
    SampleLog::LogF(L"\n>>> RebuildStaticAccelerationStructures (%s)\n", reason ? reason : L"?");

    // Thread-safety note: in async-display mode (WARP), this function MUST
    // be invoked on the worker thread, not the UI thread.  Win32Application's
    // WM_KEYDOWN handler enqueues OnKeyDown into the worker's pending-action
    // queue (DeviceResources::EnqueueAsyncAction); the worker drains the
    // queue at the top of each frame and calls OnKeyDown on its own thread,
    // which then lands here.  Previous attempt: hold a mutex during the
    // teardown so UI thread could safely call here -- caused UI freeze on
    // WARP because the mutex blocked the message pump for the worker's
    // multi-second frame.  See DeviceResources.h m_pendingAsyncActions
    // comment for the full design rationale.

    auto commandList      = m_deviceResources->GetCommandList();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    // 1) Flush the GPU so the in-flight frame finishes reading from CLAS/BLAS/TLAS
    //    before we tear them down.  ComPtr<>::Reset() releases the underlying
    //    ID3D12Resource - if the GPU is still touching it the runtime errors out.
    m_deviceResources->WaitForGpu();

    // 2) Stash the PRE-rebuild overlay snapshot as the delta-colour baseline.
    //    Must happen BEFORE the build code below -- specifically before
    //    BuildAnimatedObjectSetup -> MeasureAnimatedClasBytesOneShot writes
    //    s.animatedPerFrameClasActualBytes.  If we stash AFTER, prev gets
    //    the new actual-CLAS value and the per-frame-CLAS-actual field
    //    silently stops lighting up green/red on precision changes.
    StashOverlayStatsAsPrev();

    // [N] workload-scaling: truncate previous clones, then regenerate
    // for the current m_extraInstancesMode.  MUST happen BEFORE the
    // per-object resource reset loop below so that the reset iterates
    // the new full m_objects (clones have null ComPtrs so reset is a
    // no-op for them) and the subsequent build pipeline allocates
    // fresh CLAS+BLAS per clone.
    RegenerateWorkloadCloneInstances();

    // LOD clone meshes have their own ClusterID-indexed normals, indices, and
    // metadata.  Refresh these tables after clone regeneration so the rebuilt
    // shader bindings match the active meshes.
    BuildClusterShaderSideBuffers();
    BuildClusterMetadata();

    // 2) Drop all CLAS-related GPU resources.  Each mode's BuildClas* will
    //    reallocate these via ComPtr assignment (which releases stale slots).
    //    Explicitly clearing here makes the tear-down explicit and is the
    //    correct hygiene for the Compact mode (which keeps m_clasMoveArgsBuffer
    //    alive across function returns).
    m_clasResultBuffer.Reset();
    m_clasScratchBuffer.Reset();
    m_clasAddressArray.Reset();
    m_clasSizeArray.Reset();
    m_clasMoveArgsBuffer.Reset();
    m_clasArgsBuffer.Reset();
    m_clasArgsMetaBuffer.Reset();
    m_clasArgsArrayGPUVA = 0;
    m_clasArgsStride     = 0;
    m_totalClasBytes     = 0;
    m_clasMemStats       = ClasMemStats{};

    // 3) Drop static-object BLAS storage (BOTH paths' per-object resources --
    //    we may be transitioning between modes and want to release
    //    whichever side is currently holding GPU memory).  The active path
    //    re-allocates whichever subset it needs below.
    for (auto& obj : m_objects)
    {
        obj.blasStorage.Reset();   obj.blasGPUVA      = 0;
        obj.tradVertexBuffer.Reset();
        obj.tradIndexBuffer.Reset();
        obj.tradNormalsBuffer.Reset();
        obj.tradBlasStorage.Reset();
        obj.tradBlasScratch.Reset();
        obj.tradBlasGPUVA  = 0;
        obj.tradVertexCount   = 0;
        obj.tradTriangleCount = 0;
        obj.tradBlasResultBytes  = 0;
        obj.tradBlasScratchBytes = 0;
    }
    m_blasScratchBuffer.Reset();
    m_blasArgsBuffer.Reset();
    m_blasArgsMeta.Reset();
    m_blasResultAddrBuffer.Reset();
    m_clusterBlasPoolBuffer.Reset();
    // Pool buffer holding all clones' BLAS storage -- released so
    // the next BuildBlasFromClasIndirect call gets a fresh
    // appropriately-sized allocation for the new clone count + LOD
    // distribution.
    // Phase-2 anim clones: pool gets re-allocated by
    // BuildAnimatedClonesSetup() below if N_animClones > 0.  Reset
    // forward an over-sized clone count.
    m_animClonesBlasPool.Reset();
    m_animClonesBlasArgsBuffer.Reset();
    m_animClonesBlasResultAddrBuffer.Reset();
    m_animClonesBlasScratchBuffer.Reset();
    // Trad-path VB+IB pools: re-allocated by BuildTraditionalStaticAS.
    // Reset here so the next build starts from empty pools (sizes
    // depend on the current clone count + LOD distribution).
    m_tradVertexPool.Reset();
    m_tradIndexPool.Reset();
    // Trad-mode phase-2 anim-clones pool + shared scratch.  Re-allocated
    // by BuildAnimatedClonesTradSetup() if N_animClones > 0 in trad mode.
    m_animClonesTradBlasPool.Reset();
    m_animClonesTradBlasScratch.Reset();
    m_traditionalStaticTotalResultBytes  = 0;
    m_traditionalStaticTotalActualBytes  = 0;
    m_traditionalStaticTotalScratchBytes = 0;
    m_traditionalStaticBuildMs           = 0.0;

    // 4) Drop TLAS storage.  Per-frame rebuild path keys off these, so they
    //    MUST exist before the next OnRender; BuildTlasClassic recreates them.
    m_tlasBuffer.Reset();
    m_tlasScratchBuffer.Reset();
    m_tlasInstanceDescs.Reset();

    // 5) Reset the command list/allocator and re-run the static build chain
    //    for the currently-active geometry mode.
    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

    if (m_geometryMode == GeometryMode::Clusters)
    {
        EncodeCompressedClusters();
        UploadClusterInputs();
        BuildClasIndirect();
        BuildBlasFromClasIndirect();
        if (m_animatedObjectEnabled)
        {
            BuildAnimatedObjectSetup();
            BuildAnimatedClonesSetup();
            UpdateAnimatedObjectPerFrame();
        }
    }
    else
    {
        // Traditional path: per-object DXR1 BLAS + per-frame animated
        // ball (DXR1 rebuild/refit via [F]).
        BuildTraditionalStaticAS();
        if (m_animatedObjectEnabled)
        {
            BuildAnimatedObjectSetup();
            // Trad-mode phase-2 anim clones: separate per-clone BLAS pool +
            // per-clone per-frame BuildRaytracingAccelerationStructure
            // calls.  Sets ac.blasGPUVA so BuildTlasClassic picks them up.
            BuildAnimatedClonesTradSetup();
            UpdateAnimatedTradPerFrame();
            // Refresh the tri->cid table now that the animated mesh is
            // part of the trad scene -- BuildTraditionalStaticAS already
            // ran it for static-only, this picks up the animated tris.
            BuildTradCidLookup();
        }
    }
    // Per-(InstIdx, GeomIdx) material table must track the animated-
    // enabled state (which flips between modes).
    BuildPerInstGeomMaterialTable();
    BuildTlasClassic();

    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();

    // 6) Refresh the stats that feed the title bar.  m_totalClasBytes is the
    //    live result buffer size; the per-build CPU-wall stats already landed
    //    in m_clasMemStats via the relevant BuildClas* function.
    if (m_clasResultBuffer) m_totalClasBytes = m_clasResultBuffer->GetDesc().Width;

    SampleLog::LogF(L"<<< RebuildStaticAccelerationStructures done.  CLAS=%.1f MB, scratch=%.1f MB\n",
                    m_totalClasBytes / (1024.0 * 1024.0),
                    m_clasMemStats.scratchBytesPhase1 / (1024.0 * 1024.0));

    // Snapshot all the displayed numbers (except per-frame ms) into
    // m_overlayStats so the overlay reads from a cached struct instead of
    // poking GetDesc().Width every frame.  Per-frame timing gets snapped
    // a few frames later, once the ring buffer refills - see Tick().
    // RefreshOverlayStatsCurrent (NOT CaptureOverlayStatsSnapshot) -- the
    // prev-stash already happened at the top of this function before
    // BuildAnimatedObjectSetup wrote the new actual-CLAS value into s.
    RefreshOverlayStatsCurrent();
}



void D3D12RaytracingClusteredGeometry::BuildClusterShaderSideBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Animated sphere clusters are reachable via templated INSTANTIATE with
    // m_animatedClusterIdOffset, so register the same IDs in the side tables.
    // Pass 1: figure out the offset-table size.
    UINT maxClusterID = 0;
    for (const auto& obj : m_objects)
        for (const auto& c : obj.mesh.clusters)
            maxClusterID = std::max(maxClusterID, c.clusterID);
    if (m_animatedObjectEnabled)
        for (const auto& c : m_animatedObject.mesh.clusters)
            maxClusterID = std::max(maxClusterID, c.clusterID + m_animatedClusterIdOffset);

    ThrowIfFalse(m_animatedClusterIdOffset >= m_totalClusterCount,
        L"Animated ClusterID range overlaps static CLAS IDs.\n");
    const UINT offsetTableSize = maxClusterID + 1;

    std::vector<XMFLOAT4> normals;            // 16 bytes per normal (.w padding) to match HLSL's
                                              // ByteAddressBuffer.Load3((off.x + i_k) * 16) stride.
                                              // See Raytracing.hlsl declaration of g_clusterNormals.
    std::vector<UINT>     indices;
    std::vector<XMUINT2>  offsets(offsetTableSize, XMUINT2(0u, 0u));

    // Helper: append one cluster's data and stamp its offset entry.
    auto appendCluster = [&](const ProceduralGeometry::Cluster& c, UINT cidForOffsetTable)
    {
        offsets[cidForOffsetTable] = XMUINT2((UINT)normals.size(), (UINT)indices.size());
        for (const auto& n : c.normals)
            normals.push_back(XMFLOAT4(n.x, n.y, n.z, 0.0f));
        for (uint8_t i : c.indices)
            indices.push_back((UINT)i);
    };

    for (const auto& obj : m_objects)
        for (const auto& c : obj.mesh.clusters)
            appendCluster(c, c.clusterID);

    if (m_animatedObjectEnabled)
        for (const auto& c : m_animatedObject.mesh.clusters)
            appendCluster(c, c.clusterID + m_animatedClusterIdOffset);

    AllocateUploadBuffer(device, normals.data(), normals.size() * sizeof(DirectX::XMFLOAT4),
                         &m_clusterNormalsBuffer, L"Cluster vertex normals (per-vertex side channel)");
    AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(UINT),
                         &m_clusterIndicesBuffer, L"Cluster indices (uint32-widened side channel)");
    AllocateUploadBuffer(device, offsets.data(), offsets.size() * sizeof(XMUINT2),
                         &m_clusterOffsetsBuffer, L"Cluster vert/idx offset table");

    m_clusterNormalsCount = (UINT)normals.size();
    m_clusterIndicesCount = (UINT)indices.size();
    m_clusterOffsetsCount = (UINT)offsets.size();

    SampleLog::LogF(L"[normals] side channel: %u vertex normals, %u indices, %u offset slots (max cid=%u, ~%u KB total)\n",
                    m_clusterNormalsCount, m_clusterIndicesCount, m_clusterOffsetsCount,
                    maxClusterID,
                    (UINT)((normals.size() * sizeof(XMFLOAT3) +
                            indices.size() * sizeof(UINT) +
                            offsets.size() * sizeof(XMUINT2)) / 1024));
}


// =====================================================================================
// BuildClusterMetadata - generic per-cluster metadata pass.
//
// Walks every cluster of every object (including the animated sphere) and
// emits one ClusterMeta entry per cluster ID.  All per-object material /
// colour decisions live in the ClusterObject::checker config and the per-
// object tint multipliers - the GPU shader is fully data-driven and has
// ZERO per-instance / per-cid-range branches.
//
// The fields populated come straight from CPU-side scene data:
//   - colorIndex       = cluster.matchedColorCid  (set by generator;
//                                                  matched top tile for
//                                                  slab bottom/wall)
//   - flags            = cluster.flags            (INTERIOR_SURFACE etc.)
//   - overrideRefl/Refr/Ior = picked by parity (cluster.gridU+gridV)&1
//                             from obj.checker.{even,odd}Parity
//   - baseColorScale   = same (from CheckerOverride.baseColorScale)
//   - surfTintMul / refrTintMul / reflTintMul = per-object knobs
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildClusterMetadata()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Pass 1: same size discovery as BuildClusterShaderSideBuffers.
    UINT maxClusterID = 0;
    for (const auto& obj : m_objects)
        for (const auto& c : obj.mesh.clusters)
            maxClusterID = std::max(maxClusterID, c.clusterID);
    if (m_animatedObjectEnabled)
        for (const auto& c : m_animatedObject.mesh.clusters)
            maxClusterID = std::max(maxClusterID, c.clusterID + m_animatedClusterIdOffset);

    ThrowIfFalse(m_animatedClusterIdOffset >= m_totalClusterCount,
        L"Animated ClusterID range overlaps static CLAS IDs.\n");
    const UINT metaCount = maxClusterID + 1;
    std::vector<ClusterMeta> meta(metaCount, ClusterMeta{});

    // Initialize all slots with neutral defaults: no override, colorIndex=cid,
    // baseColorScale=1, tint mul defaults that reproduce the legacy look.
    for (UINT i = 0; i < metaCount; ++i)
    {
        meta[i].colorIndex     = i;
        meta[i].flags          = 0u;
        meta[i].overrideRefl   = -1.0f;
        meta[i].overrideRefr   = -1.0f;
        meta[i].overrideIor    = -1.0f;
        meta[i].baseColorScale = 1.0f;
        meta[i].surfTintMul    = 1.0f;
        meta[i].refrTintMul    = 0.50f;
        meta[i].reflTintMul    = 1.08f;   // = 0.70 / 0.65 (default clusterTint)
    }

    // Generic per-cluster fill - works on any object type that has a
    // mesh + optional checker config + tint multipliers (since the
    // animated sphere uses a different storage class than ClusterObject
    // but otherwise carries the same per-cluster info on its mesh).
    auto fillFromMesh = [&](const ProceduralGeometry::Mesh& mesh,
                            UINT cidOffset,
                            const CheckerConfig& checker,
                            float surfTintMul, float refrTintMul, float reflTintMul,
                            UINT defaultMaterialSlot,
                            const std::vector<UINT>* perRegionMaterialSlot = nullptr)
    {
        for (const auto& c : mesh.clusters)
        {
            const UINT id = c.clusterID + cidOffset;
            ClusterMeta& m = meta[id];

            m.colorIndex     = c.matchedColorCid + cidOffset;
            m.flags          = c.flags;
            m.surfTintMul    = surfTintMul;
            m.refrTintMul    = refrTintMul;
            m.reflTintMul    = reflTintMul;
            // Per-cluster material slot for the traditional path's mixed-region
            // fallback.  Cluster mode uses GeometryIndex() +
            // g_perInstGeomMaterial; keeping this CPU-baked slot costs no extra
            // stride (the struct remains 48 bytes) and avoids regressing the
            // separately diagnosed traditional multi-geometry behavior.
            m.materialSlot   = (perRegionMaterialSlot && c.matRegionIdx < perRegionMaterialSlot->size())
                                ? (*perRegionMaterialSlot)[c.matRegionIdx]
                                : defaultMaterialSlot;

            if (checker.enabled)
            {
                const bool isOdd = ((c.gridU + c.gridV) & 1u) != 0u;
                const auto& side = isOdd ? checker.oddParity
                                         : checker.evenParity;
                m.overrideRefl   = side.overrideRefl;
                m.overrideRefr   = side.overrideRefr;
                m.overrideIor    = side.overrideIor;
                m.baseColorScale = side.baseColorScale;
            }
        }
    };

    for (const auto& obj : m_objects)
        fillFromMesh(obj.mesh, 0u, obj.checker,
                     obj.surfTintMul, obj.refrTintMul, obj.reflTintMul,
                     obj.instanceID,
                     obj.perRegionMaterialSlot.empty() ? nullptr : &obj.perRegionMaterialSlot);
    if (m_animatedObjectEnabled)
    {
        // Animated object: alternating cluster checker - even parity
        // is OPAQUE SHINY (chrome-like; refl=0.95, refr=0), odd parity
        // keeps the baseline refractive glass.  Plus cranked tint
        // multipliers so the refractive clusters' cluster colours read
        // prominently.
        CheckerConfig checker;
        checker.enabled = true;
        checker.evenParity.overrideRefl = 0.95f;   // chrome-ish reflectivity
        checker.evenParity.overrideRefr = 0.0f;    // opaque (no refraction)
        checker.evenParity.baseColorScale = 1.0f;
        checker.oddParity.overrideRefl  = 0.0f;    // translucent clusters: no reflection
        checker.oddParity.overrideRefr  = -1.0f;   // keep baseline refractivity (0.78)
        checker.oddParity.overrideIor   = -1.0f;   // keep baseline ior (2.4)
        fillFromMesh(m_animatedObject.mesh, m_animatedClusterIdOffset,
                     checker, /*surf*/1.0f, /*refr*/0.75f, /*refl*/1.20f,
                     m_animatedObject.instanceID);
    }

    AllocateUploadBuffer(device, meta.data(), meta.size() * sizeof(ClusterMeta),
                         &m_clusterMetaBuffer, L"Per-cluster GENERIC metadata (ClusterMeta[])");
    m_clusterMetaCount = (UINT)meta.size();

    SampleLog::LogF(L"[clustermeta] %u entries (~%u KB)\n",
                    m_clusterMetaCount,
                    (UINT)((meta.size() * sizeof(ClusterMeta)) / 1024));
}



//
//   [aligned 16] cluster[0].vertex_blob
//   [aligned 16] cluster[0].index_buffer
//   ...
//   [aligned 16] cluster[N-1].vertex_blob
//   [aligned 16] cluster[N-1].index_buffer
//   [aligned 16] BUILD_CLAS_FROM_TRIANGLES_ARGS[0..N-1]
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::UploadClusterInputs()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Use a generous per-cluster alignment for COMPRESSED1 vertex data; some
    // drivers appear to read past the spec'd end of a cluster's compressed
    // blob (probably prefetch), so leaving real padding between clusters
    // avoids stepping into the next cluster's bits. 16 bytes is fine for the
    // FLOAT32_3 path which has a fixed per-vertex stride.
    const bool       useFloat = (m_vertexMode == VertexMode::Float32_3);
    const size_t     kAlign   = useFloat ? 16 : 256;
    auto alignTo = [](size_t x, size_t a) { return (x + (a - 1)) & ~(a - 1); };

    auto vbSizeForCluster = [&](const ClusterObject& obj, size_t i) -> size_t
    {
        if (useFloat)
            return obj.mesh.clusters[i].positions.size() * sizeof(ProceduralGeometry::float3);
        else
            return obj.encoded[i].TotalBytes();
    };

    struct Slot { size_t vbOffset, vbSize, ibOffset, ibSize; };
    std::vector<Slot> slots(m_totalClusterCount);
    size_t cursor = 0;
    UINT gIdx = 0;
    for (const auto& obj : m_objects)
    for (size_t i = 0; i < obj.mesh.clusters.size(); ++i, ++gIdx)
    {
        const auto& src = obj.mesh.clusters[i];
        cursor = alignTo(cursor, kAlign);
        slots[gIdx].vbOffset = cursor;
        slots[gIdx].vbSize   = vbSizeForCluster(obj, i);
        cursor += slots[gIdx].vbSize;
        cursor = alignTo(cursor, kAlign);
        slots[gIdx].ibOffset = cursor;
        slots[gIdx].ibSize   = src.indices.size();
        cursor += slots[gIdx].ibSize;
    }
    // Layout the per-cluster vertex+index slots in the upload buffer.  The
    // args used to live in this same buffer at `argsOffset` -- now they
    // live in a separate DEFAULT-heap UAV (m_clasArgsBuffer below) so a
    // compute shader can write them GPU-side.  m_clusterInputBuffer is
    // strictly vertex+index data now.
    const size_t totalDataSize = alignTo(cursor, 256);
    SampleLog::LogF(L"[upload] cluster input buffer: %zu bytes (vert+idx; args now in separate UAV)\n",
                    totalDataSize);

    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufDesc    = CD3DX12_RESOURCE_DESC::Buffer(totalDataSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeap, D3D12_HEAP_FLAG_NONE, &bufDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_clusterInputBuffer)));
    m_clusterInputBuffer->SetName(L"ClusterInputBuffer (vertex+index data only)");

    uint8_t* mapped = nullptr;
    CD3DX12_RANGE noRead(0, 0);
    ThrowIfFailed(m_clusterInputBuffer->Map(0, &noRead, reinterpret_cast<void**>(&mapped)));
    const D3D12_GPU_VIRTUAL_ADDRESS baseGPUVA = m_clusterInputBuffer->GetGPUVirtualAddress();

    // Per-cluster vertex + index data.
    gIdx = 0;
    for (const auto& obj : m_objects)
    for (size_t i = 0; i < obj.mesh.clusters.size(); ++i, ++gIdx)
    {
        const auto& src = obj.mesh.clusters[i];
        if (useFloat)
        {
            // float32_3: just copy the positions array. Same byte-exact value
            // for shared edge vertices across adjacent clusters -> watertight
            // to the vertex-precision level.
            memcpy(mapped + slots[gIdx].vbOffset,
                   src.positions.data(),
                   src.positions.size() * sizeof(ProceduralGeometry::float3));
        }
        else
        {
            // compressed1: encoded header + bitstream blob.
            const auto& enc = obj.encoded[i];
            memcpy(mapped + slots[gIdx].vbOffset, &enc.header, sizeof(enc.header));
            if (!enc.bitstream.empty())
                memcpy(mapped + slots[gIdx].vbOffset + sizeof(enc.header),
                       enc.bitstream.data(), enc.bitstream.size());
        }
        if (!src.indices.empty())
            memcpy(mapped + slots[gIdx].ibOffset, src.indices.data(), src.indices.size());
    }
    m_clusterInputBuffer->Unmap(0, nullptr);

    // ------------------------------------------------------------------
    // Per-cluster metadata for the FillClasFromTrianglesArgs CS.
    // 28 bytes/cluster: see ClasArgsMeta layout in FillClasFromTrianglesArgs.hlsl.
    //   { clusterID, triCount, vertCount, vbOff, ibOff, opaqueFlag, matRegionIdx }
    // matRegionIdx becomes BaseGeometryIndex in the CLAS (bits 0..23 of
    // BaseGeometryIndexAndFlags), so GeometryIndex() in the closest-
    // hit returns this per-cluster value -- the same identifier the
    // traditional path's per-material-region geom-desc layout produces.
    // ------------------------------------------------------------------
    {
        struct ClasArgsMeta {
            UINT clusterID, triCount, vertCount, vbOff, ibOff, opaqueFlag, matRegionIdx;
        };
        static_assert(sizeof(ClasArgsMeta) == 28, "must match the HLSL load offsets");

        std::vector<ClasArgsMeta> meta(m_totalClusterCount);
        gIdx = 0;
        for (const auto& obj : m_objects)
        {
            // Per-cluster OPAQUE flag.  Must reflect the cluster's ACTUAL
            // material -- look up via perRegionMaterialSlot[matRegionIdx]
            // when the object has per-region material assignments (e.g.
            // mixed sphere), otherwise fall back to obj.instanceID.  A
            // per-cluster checker override that may introduce refr makes
            // EVERY cluster of the object non-opaque (any cluster could
            // land on the odd-parity tile and need any-hit).
            const bool checkerCouldRefract = obj.checker.enabled &&
                (obj.checker.evenParity.overrideRefr > 0.0f ||
                 obj.checker.oddParity.overrideRefr  > 0.0f);

            for (size_t i = 0; i < obj.mesh.clusters.size(); ++i, ++gIdx)
            {
                const auto& src = obj.mesh.clusters[i];
                const UINT clusterMatSlot = (src.matRegionIdx < (UINT)obj.perRegionMaterialSlot.size())
                    ? obj.perRegionMaterialSlot[src.matRegionIdx]
                    : obj.instanceID;
                const auto& mat = m_materials[clusterMatSlot];
                const bool isOpaqueLike =
                    (mat.translucency == 0.0f) && (mat.refractivity == 0.0f) &&
                    !checkerCouldRefract;
                meta[gIdx].clusterID    = src.clusterID;
                meta[gIdx].triCount     = (UINT)(src.indices.size() / 3);
                meta[gIdx].vertCount    = (UINT)src.positions.size();
                meta[gIdx].vbOff        = (UINT)slots[gIdx].vbOffset;
                meta[gIdx].ibOff        = (UINT)slots[gIdx].ibOffset;
                meta[gIdx].opaqueFlag   = isOpaqueLike
                    ? (UINT)D3D12_RTAS_CLUSTERED_GEOMETRY_FLAG_OPAQUE
                    : 0u;
                meta[gIdx].matRegionIdx = src.matRegionIdx;
            }
        }
        AllocateUploadBuffer(device, meta.data(),
                             meta.size() * sizeof(ClasArgsMeta),
                             &m_clasArgsMetaBuffer, L"Cluster args metadata");
    }

    // GPU-written args buffer (DEFAULT/UAV).
    {
        const UINT64 argsBytes = (UINT64)m_totalClusterCount * sizeof(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS);
        AllocateUAVBuffer(device, argsBytes,
            &m_clasArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            L"CLAS-from-triangles args (GPU-filled)");
    }

    // Dispatch FillClasFromTrianglesArgs.
    {
        auto cmdList = m_deviceResources->GetCommandList();
        cmdList->SetComputeRootSignature(m_fillClasTriArgsRS.Get());
        cmdList->SetPipelineState(m_fillClasTriArgsPSO.Get());
        struct {
            UINT baseLo, baseHi;
            UINT count;
            UINT vbStride;
            UINT posTruncBits;
        } cb;
        cb.baseLo       = (UINT)(baseGPUVA & 0xFFFFFFFFu);
        cb.baseHi       = (UINT)(baseGPUVA >> 32);
        cb.count        = m_totalClusterCount;
        cb.vbStride     = useFloat ? (UINT)sizeof(ProceduralGeometry::float3) : 0u;
        cb.posTruncBits = useFloat ? m_positionTruncateBits : 0u;
        cmdList->SetComputeRoot32BitConstants(0, sizeof(cb) / 4, &cb, 0);
        cmdList->SetComputeRootShaderResourceView(1, m_clasArgsMetaBuffer->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(2, m_clasArgsBuffer->GetGPUVirtualAddress());
        const UINT groups = (m_totalClusterCount + 63) / 64;
        cmdList->Dispatch(groups, 1, 1);
        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(m_clasArgsBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(m_clasArgsBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cmdList->ResourceBarrier(_countof(barriers), barriers);
    }

    m_clasArgsArrayGPUVA = m_clasArgsBuffer->GetGPUVirtualAddress();
    m_clasArgsStride     = (UINT)sizeof(D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS);
}

// =====================================================================================
// CLAS allocation strategies.
//
// All three modes share the same per-cluster inputs (vertex/index buffers,
// triangle counts, args array, etc.). They differ only in HOW the per-cluster
// output CLAS bytes get placed in GPU memory:
//
//   Implicit  - one IMPLICIT_DESTINATIONS build; allocate worst-case storage
//               from prebuild.ResultDataMaxSizeInBytes. Simplest path. Wastes
//               memory when actual per-cluster size << worst case (typical
//               5x over-alloc for this scene on NVIDIA).
//
//   GetSizes  - two passes:
//                 1. MODE_GET_SIZES: no result-buffer alloc, driver writes
//                    per-cluster bytes-needed into a UAV size array.
//                 2. CPU readback of sizes, sum to get the exact total,
//                    compute per-cluster destination offsets, allocate an
//                    exact-sized result buffer.
//                 3. MODE_EXPLICIT_DESTINATIONS build into that buffer.
//               No over-allocation at any point. Cost: one extra GPU pass +
//               a CPU stall on the size readback.
//
//   Compact   - two passes, opposite tradeoff:
//                 1. MODE_IMPLICIT_DESTINATIONS build (worst-case alloc),
//                    with ResultSizeArray attached so per-cluster actual
//                    sizes are written as a side effect of the build.
//                 2. CPU readback of sizes -> compacted total.
//                 3. MOVE_CLUSTER_OBJECTS in MODE_IMPLICIT_DESTINATIONS into
//                    a tightly-sized compacted result buffer.
//                 4. Old (worst-case) result buffer is released.
//               Peak GPU memory = worst-case + compacted (briefly, during
//               step 3). Final = compacted. No CPU-blocking required if the
//               size info is consumed by a GPU shader -> only useful if you
//               want EXACT post-build alloc on a single GPU timeline.
//
// All three end with m_clasResultBuffer, m_clasAddressArray, m_clasSizeArray
// in their canonical post-CLAS state so the subsequent BLAS-from-CLAS build
// can proceed identically. The captured stats (m_clasMemStats) let the
// user see the tradeoff in numbers via the [CLAS mem] log line.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildClasIndirect()
{
    SampleLog::LogF(L"[CLAS alloc-mode] %s; position-truncate-bits=%u (FLOAT32_3 only)\n",
                    ClasAllocModeName(), m_positionTruncateBits);
    m_clasMemStats = ClasMemStats{};        // reset stats for this run
    switch (m_clasAllocMode)
    {
    case ClasAllocMode::Implicit: BuildClasImplicit(); break;
    case ClasAllocMode::GetSizes: BuildClasGetSizes(); break;
    case ClasAllocMode::Compact:  BuildClasCompact();  break;
    }
}

// ---------------------------------------------------------------------------------
// Build the shared D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC + ClusterLimits.
// All three modes need the same input description; only their Mode field and
// the surrounding batched-op-data setup differ. Keeping this in one helper
// avoids drift between the three paths.
// ---------------------------------------------------------------------------------
static void BuildSharedClusterTrianglesInputs(
    UINT N, bool useFloat, UINT maxTris, UINT maxVerts, UINT totalTris, UINT totalVerts,
    UINT maxCompressedSize, UINT maxGeometryIndexValue, UINT positionTruncateBits,
    D3D12_RTAS_OPERATION_FLAGS buildFlags,
    D3D12_RTAS_CLUSTER_LIMITS& outLimits,
    D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC& outClasDesc)
{
    outLimits = {};
    outLimits.MaxArgCount                                   = N;
    // matRegionIdx is stamped into each CLAS BaseGeometryIndex.  Keep the
    // declared prebuild bound aligned with the flat material-table stride so
    // GeometryIndex() in cluster hit shaders can legally address every slot.
    outLimits.MaxGeometryIndexValue                         = maxGeometryIndexValue;
    outLimits.MaxUniqueGeometryIndexAndFlagsCountPerCluster = 1;
    outLimits.MaxTriangleCountPerCluster                    = maxTris;
    // Use the exact scene bound required by the current API contract.  The
    // final-tree FLOAT32_3 and COMPRESSED1 paths are validated on WARP.
    outLimits.MaxVertexCountPerCluster                      = maxVerts;
    outLimits.MaxTotalTriangleCount                         = totalTris;
    outLimits.MaxTotalVertexCount                           = totalVerts;
    outLimits.MaxOpacityMicromapIndicesPerCluster           = 0;

    outClasDesc = {};
    outClasDesc.ClusterLimits                       = outLimits;
    // FAST_TRACE: optimise for trace performance over build speed (the typical
    // static-asset choice; use FAST_OPERATION instead for streaming rebuilds).
    // Smooth normals come from the explicit shader-side normal/index buffers;
    // no shader reads optional built-in triangle-position data.
    outClasDesc.Flags                               = buildFlags;
    outClasDesc.VertexFormat                        = useFloat ? D3D12_VERTEX_FORMAT_FLOAT32_3
                                                               : D3D12_VERTEX_FORMAT_COMPRESSED1;
    outClasDesc.IndexFormat                         = D3D12_INDEX_FORMAT_UINT8;
    outClasDesc.GeometryIndexAndFlagsIndexFormat    = D3D12_INDEX_FORMAT_NONE;
    outClasDesc.OpacityMicromapIndexFormat          = D3D12_INDEX_FORMAT_NONE;
    // The last field is a union: in COMPRESSED1 mode it's
    // MaxCompressedClusterPositionsSize (the max compressed-blob bytes per
    // cluster), in FLOAT32_3 mode it's MinPositionTruncateBitCount (the
    // floor on per-cluster mantissa-truncation; per-cluster
    // PositionTruncateBitCount in the args struct must be >= this).
    if (useFloat)
        outClasDesc.MinPositionTruncateBitCount = positionTruncateBits;
    else
        outClasDesc.MaxCompressedClusterPositionsSize = maxCompressedSize;
    // .Mode set by each caller.
}

// ---------------------------------------------------------------------------------
// Implicit mode (default). Single IMPLICIT_DESTINATIONS build covering all
// clusters. Allocates the worst-case result-buffer size from prebuild info;
// per-cluster GVAs land in m_clasAddressArray; per-cluster sizes land in
// m_clasSizeArray (as a side effect of the build, useful for stats).
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::BuildClasImplicit()
{
    auto device = m_deviceResources->GetD3DDevice();
    const UINT N = m_totalClusterCount;
    const bool useFloat = (m_vertexMode == VertexMode::Float32_3);

    UINT maxTris = 0, maxVerts = 0, totalTris = 0, totalVerts = 0;
    UINT maxCompressedSize = 0;
    for (const auto& obj : m_objects)
    for (size_t i = 0; i < obj.mesh.clusters.size(); ++i)
    {
        UINT t = (UINT)(obj.mesh.clusters[i].indices.size() / 3);
        UINT v = (UINT)obj.mesh.clusters[i].positions.size();
        maxTris  = std::max(maxTris,  t);  totalTris  += t;
        maxVerts = std::max(maxVerts, v);  totalVerts += v;
        if (!useFloat)
            maxCompressedSize = std::max(maxCompressedSize, (UINT)obj.encoded[i].TotalBytes());
    }

    D3D12_RTAS_CLUSTER_LIMITS limits;
    D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC clasDesc;
    BuildSharedClusterTrianglesInputs(N, useFloat, maxTris, maxVerts,
                                      totalTris, totalVerts, maxCompressedSize,
                                      kMaxGeomsPerInstance - 1,
                                      m_positionTruncateBits, BuildFlagModeRtas(),
                                      limits, clasDesc);
    clasDesc.Mode = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;

    D3D12_RTAS_OPERATION_INPUTS opInputs = {};
    opInputs.Type                  = D3D12_RTAS_OPERATION_TYPE_BUILD_CLAS_FROM_TRIANGLES;
    opInputs.pClusterTrianglesDesc = &clasDesc;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuild = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputs, &prebuild);
    SampleLog::LogF(L"[CLAS prebuild] %u clusters: result max=%llu bytes, scratch=%llu bytes\n",
                    N,
                    (unsigned long long)prebuild.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuild.ScratchDataSizeInBytes);

    m_clasMemStats.resultPrebuildMax  = prebuild.ResultDataMaxSizeInBytes;
    m_clasMemStats.resultInitialBytes = prebuild.ResultDataMaxSizeInBytes;
    m_clasMemStats.resultFinalBytes   = prebuild.ResultDataMaxSizeInBytes;
    m_clasMemStats.peakResidentBytes  = prebuild.ResultDataMaxSizeInBytes;
    m_clasMemStats.scratchBytesPhase1 = prebuild.ScratchDataSizeInBytes;

    AllocateUAVBuffer(device, prebuild.ResultDataMaxSizeInBytes,
                      &m_clasResultBuffer, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"All-cluster CLAS result buffer (implicit)");
    AllocateUAVBuffer(device, std::max<UINT64>(prebuild.ScratchDataSizeInBytes, 256ull),
                      &m_clasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS scratch");
    AllocateUAVBuffer(device, (UINT64)N * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                      &m_clasAddressArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS address array");
    AllocateUAVBuffer(device, (UINT64)N * sizeof(UINT64),
                      &m_clasSizeArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS size array");

    D3D12_RTAS_BATCHED_OPERATION_DATA batched = {};
    batched.AddressResolutionFlags    = D3D12_RTAS_OPERATION_ADDRESS_RESOLUTION_FLAG_NONE;
    batched.BatchResultData           = m_clasResultBuffer->GetGPUVirtualAddress();
    batched.BatchScratchData          = m_clasScratchBuffer->GetGPUVirtualAddress();
    batched.ResultAddressArray        = { m_clasAddressArray->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batched.ResultSizeArray           = { m_clasSizeArray->GetGPUVirtualAddress(),    sizeof(UINT64) };
    batched.IndirectArgumentArray     = { m_clasArgsArrayGPUVA, m_clasArgsStride };
    batched.IndirectArgumentArraySize = 0;
    batched.ToolsInfo                 = 0;

    D3D12_RTAS_OPERATION_DESC opDesc = {};
    opDesc.Inputs                = opInputs;
    opDesc.pBatchedOperationData = &batched;

    const auto t0 = std::chrono::steady_clock::now();
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDesc, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasAddressArray.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasSizeArray.Get()),
    };
    m_dxrCommandList->ResourceBarrier(_countof(barriers), barriers);
    const auto t1 = std::chrono::steady_clock::now();
    m_clasMemStats.cpuWallMsPhase1 = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Read back the per-cluster sizes the driver wrote into m_clasSizeArray
    // as a side effect of the build, so the [CLAS mem] stats line can show
    // the actual irreducible storage (sum_actual). This is a one-time CPU
    // stall - production code that picks implicit mode would skip it. The
    // sample does it so users can see the over-allocation at a glance.
    auto cmdList = m_deviceResources->GetCommandList();
    ComPtr<ID3D12Resource> sizesReadback;
    {
        auto rbHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto rbDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)N * sizeof(UINT64));
        ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sizesReadback)));
        auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        cmdList->ResourceBarrier(1, &toCopy);
        cmdList->CopyBufferRegion(sizesReadback.Get(), 0, m_clasSizeArray.Get(), 0,
            (UINT64)N * sizeof(UINT64));
        auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        cmdList->ResourceBarrier(1, &fromCopy);
    }
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();
    {
        void* mapped = nullptr;
        D3D12_RANGE rr = { 0, (SIZE_T)N * sizeof(UINT64) };
        ThrowIfFailed(sizesReadback->Map(0, &rr, &mapped));
        auto* sizes = reinterpret_cast<const UINT64*>(mapped);
        UINT64 sum = 0;
        for (UINT i = 0; i < N; ++i) sum += sizes[i];
        D3D12_RANGE noWrite = {0, 0};
        sizesReadback->Unmap(0, &noWrite);
        m_clasMemStats.sumActualBytes = sum;
    }
    auto cmdAlloc = m_deviceResources->GetCommandAllocator();
    ThrowIfFailed(cmdAlloc->Reset());
    ThrowIfFailed(cmdList->Reset(cmdAlloc, nullptr));
}

// ---------------------------------------------------------------------------------
// GetSizes mode. Two-pass: first a GET_SIZES pass to learn per-cluster bytes,
// then an EXPLICIT_DESTINATIONS build into an exact-sized result buffer.
// CPU stalls between passes on the size-array readback.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::BuildClasGetSizes()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto cmdList = m_deviceResources->GetCommandList();
    const UINT N = m_totalClusterCount;
    const bool useFloat = (m_vertexMode == VertexMode::Float32_3);

    UINT maxTris = 0, maxVerts = 0, totalTris = 0, totalVerts = 0;
    UINT maxCompressedSize = 0;
    for (const auto& obj : m_objects)
    for (size_t i = 0; i < obj.mesh.clusters.size(); ++i)
    {
        UINT t = (UINT)(obj.mesh.clusters[i].indices.size() / 3);
        UINT v = (UINT)obj.mesh.clusters[i].positions.size();
        maxTris  = std::max(maxTris,  t);  totalTris  += t;
        maxVerts = std::max(maxVerts, v);  totalVerts += v;
        if (!useFloat)
            maxCompressedSize = std::max(maxCompressedSize, (UINT)obj.encoded[i].TotalBytes());
    }

    D3D12_RTAS_CLUSTER_LIMITS limits;
    D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC clasDescBase;
    BuildSharedClusterTrianglesInputs(N, useFloat, maxTris, maxVerts,
                                      totalTris, totalVerts, maxCompressedSize,
                                      kMaxGeomsPerInstance - 1,
                                      m_positionTruncateBits, BuildFlagModeRtas(),
                                      limits, clasDescBase);

    // ---- Phase 1: GET_SIZES pass ----
    // No result buffer needed; driver writes per-cluster bytes-needed into a
    // UAV size buffer. m_clasSizeArray is reused as the destination.
    auto clasDescSizes = clasDescBase;
    clasDescSizes.Mode = D3D12_RTAS_OPERATION_MODE_GET_SIZES;

    D3D12_RTAS_OPERATION_INPUTS opInputsSizes = {};
    opInputsSizes.Type                  = D3D12_RTAS_OPERATION_TYPE_BUILD_CLAS_FROM_TRIANGLES;
    opInputsSizes.pClusterTrianglesDesc = &clasDescSizes;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuildSizes = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputsSizes, &prebuildSizes);
    SampleLog::LogF(L"[CLAS prebuild get-sizes] result max=%llu bytes, scratch=%llu bytes\n",
                    (unsigned long long)prebuildSizes.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuildSizes.ScratchDataSizeInBytes);

    // Worst-case result alloc for stats reference - we DON'T allocate it.
    D3D12_RTAS_OPERATION_INPUTS opInputsImplicitProbe = opInputsSizes;
    auto clasDescProbe = clasDescBase;
    clasDescProbe.Mode = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;
    opInputsImplicitProbe.pClusterTrianglesDesc = &clasDescProbe;
    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuildImplicitProbe = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputsImplicitProbe, &prebuildImplicitProbe);
    m_clasMemStats.resultPrebuildMax  = prebuildImplicitProbe.ResultDataMaxSizeInBytes;
    m_clasMemStats.scratchBytesPhase1 = prebuildSizes.ScratchDataSizeInBytes;

    AllocateUAVBuffer(device, std::max<UINT64>(prebuildSizes.ScratchDataSizeInBytes, 256ull),
                      &m_clasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS scratch (get-sizes phase)");
    AllocateUAVBuffer(device, (UINT64)N * sizeof(UINT64),
                      &m_clasSizeArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS size array");

    D3D12_RTAS_BATCHED_OPERATION_DATA batchedSizes = {};
    batchedSizes.BatchScratchData      = m_clasScratchBuffer->GetGPUVirtualAddress();
    batchedSizes.ResultSizeArray       = { m_clasSizeArray->GetGPUVirtualAddress(), sizeof(UINT64) };
    batchedSizes.IndirectArgumentArray = { m_clasArgsArrayGPUVA, m_clasArgsStride };

    D3D12_RTAS_OPERATION_DESC opDescSizes = {};
    opDescSizes.Inputs                = opInputsSizes;
    opDescSizes.pBatchedOperationData = &batchedSizes;

    const auto p1_t0 = std::chrono::steady_clock::now();
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDescSizes,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    auto sizesUav = CD3DX12_RESOURCE_BARRIER::UAV(m_clasSizeArray.Get());
    m_dxrCommandList->ResourceBarrier(1, &sizesUav);

    // Copy size array to a readback heap so the CPU can sum them.
    ComPtr<ID3D12Resource> sizesReadback;
    {
        auto rbHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto rbDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)N * sizeof(UINT64));
        ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sizesReadback)));
        auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        cmdList->ResourceBarrier(1, &toCopy);
        cmdList->CopyBufferRegion(sizesReadback.Get(), 0, m_clasSizeArray.Get(), 0,
            (UINT64)N * sizeof(UINT64));
        auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        cmdList->ResourceBarrier(1, &fromCopy);
    }
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();
    const auto p1_t1 = std::chrono::steady_clock::now();
    m_clasMemStats.cpuWallMsPhase1 = std::chrono::duration<double, std::milli>(p1_t1 - p1_t0).count();

    // Sum sizes + compute per-cluster destination offsets in the about-to-be-
    // allocated exact-sized result buffer. CLAS alignment must be respected
    // for every cluster's start address.
    constexpr UINT64 kClasAlign = D3D12_RAYTRACING_CLAS_BYTE_ALIGNMENT;
    auto alignUp = [](UINT64 x, UINT64 a) { return (x + (a - 1)) & ~(a - 1); };

    void* mapped = nullptr;
    D3D12_RANGE rr = { 0, (SIZE_T)N * sizeof(UINT64) };
    ThrowIfFailed(sizesReadback->Map(0, &rr, &mapped));
    auto* sizes = reinterpret_cast<const UINT64*>(mapped);

    std::vector<UINT64> destOffsets(N);
    UINT64 sumActual = 0, packedTotal = 0;
    for (UINT i = 0; i < N; ++i)
    {
        destOffsets[i] = packedTotal;
        packedTotal   += alignUp(sizes[i], kClasAlign);
        sumActual     += sizes[i];
    }
    D3D12_RANGE noWrite = { 0, 0 };
    sizesReadback->Unmap(0, &noWrite);
    SampleLog::LogF(L"[CLAS get-sizes] sum actual=%llu bytes, packed (%lluB-aligned)=%llu bytes\n",
                    (unsigned long long)sumActual, (unsigned long long)kClasAlign,
                    (unsigned long long)packedTotal);
    m_clasMemStats.sumActualBytes  = sumActual;
    m_clasMemStats.resultInitialBytes = packedTotal;
    m_clasMemStats.resultFinalBytes   = packedTotal;
    m_clasMemStats.peakResidentBytes  = packedTotal;

    // ---- Phase 2: EXPLICIT_DESTINATIONS build into exact-sized buffer ----
    auto commandAllocator = m_deviceResources->GetCommandAllocator();
    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(cmdList->Reset(commandAllocator, nullptr));

    AllocateUAVBuffer(device, packedTotal,
                      &m_clasResultBuffer, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"All-cluster CLAS result buffer (get-sizes exact)");

    // Per-cluster destination GVAs go into m_clasAddressArray. EXPLICIT mode
    // reads from this buffer; the driver places each result at the address we
    // specify here. After the build, m_clasAddressArray's contents (which we
    // just wrote on CPU) are the canonical post-build per-cluster GVAs that
    // the BLAS-from-CLAS step needs.
    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> destAddrs(N);
    const D3D12_GPU_VIRTUAL_ADDRESS baseGVA = m_clasResultBuffer->GetGPUVirtualAddress();
    for (UINT i = 0; i < N; ++i) destAddrs[i] = baseGVA + destOffsets[i];

    AllocateUploadBuffer(device, destAddrs.data(),
                         destAddrs.size() * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                         &m_clasAddressArray, L"CLAS address array (explicit destinations)");

    auto clasDescBuild = clasDescBase;
    clasDescBuild.Mode = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;

    D3D12_RTAS_OPERATION_INPUTS opInputsBuild = {};
    opInputsBuild.Type                  = D3D12_RTAS_OPERATION_TYPE_BUILD_CLAS_FROM_TRIANGLES;
    opInputsBuild.pClusterTrianglesDesc = &clasDescBuild;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuildBuild = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputsBuild, &prebuildBuild);
    SampleLog::LogF(L"[CLAS prebuild explicit-build] scratch=%llu bytes\n",
                    (unsigned long long)prebuildBuild.ScratchDataSizeInBytes);
    m_clasMemStats.scratchBytesPhase2 = prebuildBuild.ScratchDataSizeInBytes;

    AllocateUAVBuffer(device, std::max<UINT64>(prebuildBuild.ScratchDataSizeInBytes, 256ull),
                      &m_clasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS scratch (explicit-build phase)");

    D3D12_RTAS_BATCHED_OPERATION_DATA batchedBuild = {};
    batchedBuild.BatchScratchData      = m_clasScratchBuffer->GetGPUVirtualAddress();
    batchedBuild.ResultAddressArray    = { m_clasAddressArray->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batchedBuild.ResultSizeArray       = { m_clasSizeArray->GetGPUVirtualAddress(),    sizeof(UINT64) };
    batchedBuild.IndirectArgumentArray = { m_clasArgsArrayGPUVA, m_clasArgsStride };

    D3D12_RTAS_OPERATION_DESC opDescBuild = {};
    opDescBuild.Inputs                = opInputsBuild;
    opDescBuild.pBatchedOperationData = &batchedBuild;

    const auto p2_t0 = std::chrono::steady_clock::now();
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDescBuild,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    // NB: m_clasAddressArray was allocated as an UPLOAD-heap buffer in this
    // mode (read-only INPUT to the EXPLICIT_DESTINATIONS build), so it has no
    // UAV bind flags and must NOT be UAV-barriered. Only the result/size
    // buffers get the barrier.
    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasSizeArray.Get()),
    };
    m_dxrCommandList->ResourceBarrier(_countof(barriers), barriers);
    const auto p2_t1 = std::chrono::steady_clock::now();
    m_clasMemStats.cpuWallMsPhase2 = std::chrono::duration<double, std::milli>(p2_t1 - p2_t0).count();
}

// ---------------------------------------------------------------------------------
// Compact mode. Single IMPLICIT build (worst-case alloc) -- with the size
// array attached so we get per-cluster actual sizes for free -- then a CPU
// readback + MOVE_CLUSTER_OBJECTS in IMPLICIT mode that compacts the live
// CLAS into a tightly-sized buffer. After the move the old (worst-case)
// result buffer is released; what remains is the compacted buffer.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::BuildClasCompact()
{
    auto device  = m_deviceResources->GetD3DDevice();
    auto cmdList = m_deviceResources->GetCommandList();
    const UINT N = m_totalClusterCount;
    const bool useFloat = (m_vertexMode == VertexMode::Float32_3);

    UINT maxTris = 0, maxVerts = 0, totalTris = 0, totalVerts = 0;
    UINT maxCompressedSize = 0;
    for (const auto& obj : m_objects)
    for (size_t i = 0; i < obj.mesh.clusters.size(); ++i)
    {
        UINT t = (UINT)(obj.mesh.clusters[i].indices.size() / 3);
        UINT v = (UINT)obj.mesh.clusters[i].positions.size();
        maxTris  = std::max(maxTris,  t);  totalTris  += t;
        maxVerts = std::max(maxVerts, v);  totalVerts += v;
        if (!useFloat)
            maxCompressedSize = std::max(maxCompressedSize, (UINT)obj.encoded[i].TotalBytes());
    }

    D3D12_RTAS_CLUSTER_LIMITS limits;
    D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC clasDescBase;
    BuildSharedClusterTrianglesInputs(N, useFloat, maxTris, maxVerts,
                                      totalTris, totalVerts, maxCompressedSize,
                                      kMaxGeomsPerInstance - 1,
                                      m_positionTruncateBits, BuildFlagModeRtas(),
                                      limits, clasDescBase);
    clasDescBase.Mode = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;

    D3D12_RTAS_OPERATION_INPUTS opInputs = {};
    opInputs.Type                  = D3D12_RTAS_OPERATION_TYPE_BUILD_CLAS_FROM_TRIANGLES;
    opInputs.pClusterTrianglesDesc = &clasDescBase;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuild = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputs, &prebuild);
    SampleLog::LogF(L"[CLAS prebuild compact phase1] result max=%llu, scratch=%llu bytes\n",
                    (unsigned long long)prebuild.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuild.ScratchDataSizeInBytes);
    m_clasMemStats.resultPrebuildMax  = prebuild.ResultDataMaxSizeInBytes;
    m_clasMemStats.resultInitialBytes = prebuild.ResultDataMaxSizeInBytes;
    m_clasMemStats.scratchBytesPhase1 = prebuild.ScratchDataSizeInBytes;

    // Phase 1 buffers - this is the implicit/worst-case build.
    ComPtr<ID3D12Resource> phase1ResultBuffer;
    AllocateUAVBuffer(device, prebuild.ResultDataMaxSizeInBytes,
                      &phase1ResultBuffer, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"CLAS phase1 (worst-case) result buffer");
    AllocateUAVBuffer(device, std::max<UINT64>(prebuild.ScratchDataSizeInBytes, 256ull),
                      &m_clasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS scratch (compact phase1)");
    ComPtr<ID3D12Resource> phase1AddressArray;
    AllocateUAVBuffer(device, (UINT64)N * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                      &phase1AddressArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS phase1 address array");
    AllocateUAVBuffer(device, (UINT64)N * sizeof(UINT64),
                      &m_clasSizeArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS size array");

    D3D12_RTAS_BATCHED_OPERATION_DATA batched = {};
    batched.BatchResultData           = phase1ResultBuffer->GetGPUVirtualAddress();
    batched.BatchScratchData          = m_clasScratchBuffer->GetGPUVirtualAddress();
    batched.ResultAddressArray        = { phase1AddressArray->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batched.ResultSizeArray           = { m_clasSizeArray->GetGPUVirtualAddress(),    sizeof(UINT64) };
    batched.IndirectArgumentArray     = { m_clasArgsArrayGPUVA, m_clasArgsStride };

    D3D12_RTAS_OPERATION_DESC opDesc = {};
    opDesc.Inputs                = opInputs;
    opDesc.pBatchedOperationData = &batched;

    const auto p1_t0 = std::chrono::steady_clock::now();
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDesc,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);

    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(phase1ResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(phase1AddressArray.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasSizeArray.Get()),
    };
    m_dxrCommandList->ResourceBarrier(_countof(barriers), barriers);

    // Read sizes back so we know how big the compacted buffer needs to be.
    ComPtr<ID3D12Resource> sizesReadback;
    {
        auto rbHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto rbDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)N * sizeof(UINT64));
        ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sizesReadback)));
        auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        cmdList->ResourceBarrier(1, &toCopy);
        cmdList->CopyBufferRegion(sizesReadback.Get(), 0, m_clasSizeArray.Get(), 0,
            (UINT64)N * sizeof(UINT64));
        auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        cmdList->ResourceBarrier(1, &fromCopy);
    }
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();
    const auto p1_t1 = std::chrono::steady_clock::now();
    m_clasMemStats.cpuWallMsPhase1 = std::chrono::duration<double, std::milli>(p1_t1 - p1_t0).count();

    // Sum sizes -> compacted total. The move op packs sequentially with CLAS
    // alignment per element.
    constexpr UINT64 kClasAlign = D3D12_RAYTRACING_CLAS_BYTE_ALIGNMENT;
    auto alignUp = [](UINT64 x, UINT64 a) { return (x + (a - 1)) & ~(a - 1); };

    void* mapped = nullptr;
    D3D12_RANGE rr = { 0, (SIZE_T)N * sizeof(UINT64) };
    ThrowIfFailed(sizesReadback->Map(0, &rr, &mapped));
    auto* sizes = reinterpret_cast<const UINT64*>(mapped);
    UINT64 sumActual = 0, packedTotal = 0;
    for (UINT i = 0; i < N; ++i)
    {
        sumActual   += sizes[i];
        packedTotal += alignUp(sizes[i], kClasAlign);
    }
    D3D12_RANGE noWrite = { 0, 0 };
    sizesReadback->Unmap(0, &noWrite);
    SampleLog::LogF(L"[CLAS compact] sum actual=%llu bytes, packed (%lluB-aligned)=%llu bytes (vs worst-case %llu)\n",
                    (unsigned long long)sumActual, (unsigned long long)kClasAlign,
                    (unsigned long long)packedTotal,
                    (unsigned long long)prebuild.ResultDataMaxSizeInBytes);
    m_clasMemStats.sumActualBytes  = sumActual;
    m_clasMemStats.resultFinalBytes = packedTotal;
    // Peak = worst-case + compacted (both live during MOVE).
    m_clasMemStats.peakResidentBytes =
        prebuild.ResultDataMaxSizeInBytes + packedTotal;

    // ---- Phase 2: MOVE_CLUSTER_OBJECTS in IMPLICIT_DESTINATIONS ----
    // Pack into a fresh tightly-sized buffer; driver writes new per-cluster
    // addresses into a new m_clasAddressArray. After the move, phase1 buffers
    // are released - only the compacted result remains live.
    auto commandAllocator = m_deviceResources->GetCommandAllocator();
    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(cmdList->Reset(commandAllocator, nullptr));

    // Compacted result buffer + new address array (output of MOVE).
    AllocateUAVBuffer(device, packedTotal,
                      &m_clasResultBuffer, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"All-cluster CLAS result buffer (compacted)");
    AllocateUAVBuffer(device, (UINT64)N * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                      &m_clasAddressArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS address array (post-compact)");

    // MOVE_CLUSTER_OBJECTS args (one u64 GVA per cluster) are GPU-filled by
    // FillMoveClusterArgs CS from phase1AddressArray.  Previously the CPU
    // round-tripped these addresses through a readback + upload; the CS
    // copies them GPU-to-GPU.  m_clasMoveArgsBuffer is a member so it
    // outlives this function call (the queued cmd list reads from it until
    // BuildAccelerationStructures flushes at the bottom).
    AllocateUAVBuffer(device, (UINT64)N * sizeof(D3D12_RTAS_OPERATION_MOVE_CLUSTER_OBJECTS_ARGS),
                      &m_clasMoveArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS move args (GPU-filled)");
    {
        cmdList->SetComputeRootSignature(m_fillMoveArgsRS.Get());
        cmdList->SetPipelineState(m_fillMoveArgsPSO.Get());
        cmdList->SetComputeRoot32BitConstants(0, 1, &N, 0);
        cmdList->SetComputeRootUnorderedAccessView(1, phase1AddressArray->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(2, m_clasMoveArgsBuffer->GetGPUVirtualAddress());
        const UINT groups = (N + 63) / 64;
        cmdList->Dispatch(groups, 1, 1);
        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(m_clasMoveArgsBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(m_clasMoveArgsBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cmdList->ResourceBarrier(_countof(barriers), barriers);
    }

    D3D12_RTAS_CLUSTER_MOVES_DESC movesDesc = {};
    movesDesc.MaxArgCount    = N;
    movesDesc.Mode           = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;
    movesDesc.Type           = D3D12_RTAS_MOVE_OPERATION_TYPE_CLUSTER_LEVEL_ACCELERATION_STRUCTURE;
    movesDesc.MaxBytesMoved  = packedTotal;
    movesDesc.Flags          = D3D12_RTAS_CLUSTER_MOVE_OPERATION_FLAG_NONE;

    D3D12_RTAS_OPERATION_INPUTS moveInputs = {};
    moveInputs.Type              = D3D12_RTAS_OPERATION_TYPE_MOVE_CLUSTER_OBJECTS;
    moveInputs.pClusterMovesDesc = &movesDesc;

    D3D12_RTAS_OPERATION_PREBUILD_INFO movePrebuild = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&moveInputs, &movePrebuild);
    SampleLog::LogF(L"[CLAS prebuild compact phase2 (move)] result max=%llu, scratch=%llu bytes\n",
                    (unsigned long long)movePrebuild.ResultDataMaxSizeInBytes,
                    (unsigned long long)movePrebuild.ScratchDataSizeInBytes);
    m_clasMemStats.scratchBytesPhase2 = movePrebuild.ScratchDataSizeInBytes;

    AllocateUAVBuffer(device, std::max<UINT64>(movePrebuild.ScratchDataSizeInBytes, 256ull),
                      &m_clasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"CLAS scratch (compact move)");

    D3D12_RTAS_BATCHED_OPERATION_DATA batchedMove = {};
    batchedMove.BatchResultData       = m_clasResultBuffer->GetGPUVirtualAddress();
    batchedMove.BatchScratchData      = m_clasScratchBuffer->GetGPUVirtualAddress();
    batchedMove.ResultAddressArray    = { m_clasAddressArray->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batchedMove.ResultSizeArray       = { m_clasSizeArray->GetGPUVirtualAddress(),    sizeof(UINT64) };
    batchedMove.IndirectArgumentArray = { m_clasMoveArgsBuffer->GetGPUVirtualAddress(),
                                          sizeof(D3D12_RTAS_OPERATION_MOVE_CLUSTER_OBJECTS_ARGS) };

    D3D12_RTAS_OPERATION_DESC moveOpDesc = {};
    moveOpDesc.Inputs                = moveInputs;
    moveOpDesc.pBatchedOperationData = &batchedMove;

    const auto p2_t0 = std::chrono::steady_clock::now();
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &moveOpDesc,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    D3D12_RESOURCE_BARRIER moveBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasAddressArray.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasSizeArray.Get()),
    };
    m_dxrCommandList->ResourceBarrier(_countof(moveBarriers), moveBarriers);

    // Flush + wait the move op before this function returns. Without this,
    // the queued move work + the still-OPEN cmd list interact badly with the
    // next mid-flush in BuildAnimatedObjectSetup (we get an
    // ID3D12CommandAllocator::Reset error - "previous executions associated
    // with the allocator have not completed"), even with a UAV barrier
    // between the move and the BLAS-from-CLAS that consumes its output.
    // Treating the compact-pass as a self-contained init step (build+move+
    // wait) is the natural shape and matches how a streaming asset pipeline
    // would issue compactions anyway. Cost: roughly 1ms wall-clock on this
    // hardware - this is the price of doing exact-fit allocation under a
    // post-build compaction model.
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();
    auto cmdAlloc = m_deviceResources->GetCommandAllocator();
    ThrowIfFailed(cmdAlloc->Reset());
    ThrowIfFailed(cmdList->Reset(cmdAlloc, nullptr));

    const auto p2_t1 = std::chrono::steady_clock::now();
    m_clasMemStats.cpuWallMsPhase2 = std::chrono::duration<double, std::milli>(p2_t1 - p2_t0).count();

    // phase1ResultBuffer + phase1AddressArray go out of scope here; their
    // ComPtrs drop their last ref; the GPU is done with them (we just waited).
    // Final memory cost is m_clasResultBuffer (compacted) + scratch + addr/size.
}

// ---------------------------------------------------------------------------------
// One BLAS per object, all built in a single ExecuteIndirectRTASOperations call.
// Each per-object BLAS arg points at its slice of the global CLAS-address array
// via (m_clasAddressArray.GPUVA + obj.globalClusterStart * 8).
//
// EXPLICIT_DESTINATIONS mode so the BLAS GPU VAs end up in CPU-known buffers
// (one per object) - we bake them straight into the TLAS instance descs.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::BuildBlasFromClasIndirect()
{
    auto device = m_deviceResources->GetD3DDevice();
    const UINT  N_obj  = (UINT)m_objects.size();

    // Find per-result max BLAS size + worst-case CLAS count per BLAS for limits.
    UINT maxClasPerArg = 0;
    UINT totalClas     = 0;
    for (const auto& obj : m_objects)
    {
        maxClasPerArg = std::max(maxClasPerArg, obj.clusterCount);
        totalClas    += obj.clusterCount;
    }

    D3D12_RTAS_CLAS_INPUTS_DESC blasDesc = {};
    blasDesc.Flags              = BuildFlagModeRtas();
    blasDesc.MaxArgCount        = N_obj;
    blasDesc.Mode               = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;
    blasDesc.MaxTotalClasCount  = totalClas;
    blasDesc.MaxClasCountPerArg = maxClasPerArg;

    D3D12_RTAS_OPERATION_INPUTS opInputs = {};
    opInputs.Type      = D3D12_RTAS_OPERATION_TYPE_BUILD_BLAS_FROM_CLAS;
    opInputs.pClasDesc = &blasDesc;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuild = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputs, &prebuild);
    SampleLog::LogF(L"[BLAS prebuild] %u objects: per-result max=%llu bytes, scratch=%llu bytes\n",
                    N_obj,
                    (unsigned long long)prebuild.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuild.ScratchDataSizeInBytes);

    // ONE pool buffer holding every object's BLAS storage.  Per-object
    // BLAS sizes vary (LOD'd clones have far fewer clusters than full-
    // detail sources) so we re-prebuild PER UNIQUE cluster count via a
    // memoized helper -- typically only a handful of distinct cluster
    // counts across the whole scene (one per source + one per LOD
    // tier per cloneable source).  Per-object offset is the running
    // sum of aligned per-object sizes.
    //
    // EXPLICIT_DESTINATIONS BLAS-from-CLAS mode lets each per-arg
    // BUILD_BLAS_FROM_CLAS_ARGS specify its own DestAddress GPU VA --
    // so the driver writes each BLAS directly into its slice of the
    // pool with no per-clone CreateCommittedResource roundtrip.  This
    // collapses 10K driver allocation calls (which made cluster N=10K
    // hang) into one.
    std::unordered_map<UINT, UINT64> blasSizeByClusterCount;
    auto getBlasSizeForClusters = [&](UINT clusterCount) -> UINT64 {
        if (auto it = blasSizeByClusterCount.find(clusterCount); it != blasSizeByClusterCount.end())
            return it->second;
        D3D12_RTAS_CLAS_INPUTS_DESC d = {};
        d.Flags              = BuildFlagModeRtas();
        d.MaxArgCount        = 1;
        d.Mode               = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;
        d.MaxTotalClasCount  = clusterCount;
        d.MaxClasCountPerArg = clusterCount;
        D3D12_RTAS_OPERATION_INPUTS oi = {};
        oi.Type      = D3D12_RTAS_OPERATION_TYPE_BUILD_BLAS_FROM_CLAS;
        oi.pClasDesc = &d;
        D3D12_RTAS_OPERATION_PREBUILD_INFO pb = {};
        m_dxr2Device->GetRTASOperationPrebuildInfo(&oi, &pb);
        blasSizeByClusterCount[clusterCount] = pb.ResultDataMaxSizeInBytes;
        return pb.ResultDataMaxSizeInBytes;
    };

    constexpr UINT64 kBlasAlign = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;  // 256
    std::vector<UINT64> perObjOffset(N_obj);
    UINT64 poolBytes = 0;
    for (UINT i = 0; i < N_obj; ++i)
    {
        const auto& obj = m_objects[i];
        const UINT64 sz = getBlasSizeForClusters(obj.clusterCount);
        const UINT64 aligned = (sz + (kBlasAlign - 1)) & ~(kBlasAlign - 1);
        perObjOffset[i] = poolBytes;
        poolBytes += aligned;
    }
    SampleLog::LogF(L"[BLAS pool] %u objects, %zu unique cluster-counts, %.2f MB total (%.2f MB if per-obj worst-case)\n",
                    N_obj, blasSizeByClusterCount.size(),
                    poolBytes / (1024.0 * 1024.0),
                    (UINT64)N_obj * prebuild.ResultDataMaxSizeInBytes / (1024.0 * 1024.0));

    // Allocate the pool in ONE shot (drops 10K driver calls to 1).
    AllocateUAVBuffer(device, poolBytes, &m_clusterBlasPoolBuffer,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Cluster BLAS pool (all objects)");
    const D3D12_GPU_VIRTUAL_ADDRESS poolBaseGVA = m_clusterBlasPoolBuffer->GetGPUVirtualAddress();
    for (UINT i = 0; i < N_obj; ++i)
    {
        m_objects[i].blasStorage.Reset();  // not needed when pool owns the memory
        m_objects[i].blasGPUVA = poolBaseGVA + perObjOffset[i];
    }
    AllocateUAVBuffer(device, std::max<UINT64>(prebuild.ScratchDataSizeInBytes, 256ull),
                      &m_blasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Cluster BLAS scratch");

    // Build args array: one BUILD_BLAS_FROM_CLAS_ARGS per object, each
    // referencing a sub-range of the global CLAS-address array.  GPU-filled
    // by FillBlasFromClasArgs CS (shared with the animated path).  CPU
    // prepares a small per-object {count, gvaLo, gvaHi} input buffer.
    {
        struct BlasArgsMeta { UINT count, gvaLo, gvaHi; };
        static_assert(sizeof(BlasArgsMeta) == 12, "must match HLSL Load3 stride");

        const D3D12_GPU_VIRTUAL_ADDRESS clasArrayGPUVA = m_clasAddressArray->GetGPUVirtualAddress();
        std::vector<BlasArgsMeta> meta(N_obj);
        for (UINT i = 0; i < N_obj; ++i)
        {
            const D3D12_GPU_VIRTUAL_ADDRESS perObjArrayGva = clasArrayGPUVA
                + (UINT64)m_objects[i].globalClusterStart * sizeof(D3D12_GPU_VIRTUAL_ADDRESS);
            meta[i].count = m_objects[i].clusterCount;
            meta[i].gvaLo = (UINT)(perObjArrayGva & 0xFFFFFFFFu);
            meta[i].gvaHi = (UINT)(perObjArrayGva >> 32);
        }
        AllocateUploadBuffer(device, meta.data(), meta.size() * sizeof(BlasArgsMeta),
                             &m_blasArgsMeta, L"BLAS args metadata (multi-object)");

        AllocateUAVBuffer(device,
            (UINT64)N_obj * sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS),
            &m_blasArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            L"BLAS args (multi-object, GPU-filled)");

        auto cmdList = m_deviceResources->GetCommandList();
        cmdList->SetComputeRootSignature(m_fillBlasArgsRS.Get());
        cmdList->SetPipelineState(m_fillBlasArgsPSO.Get());
        const UINT cb[2] = { N_obj, (UINT)sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
        cmdList->SetComputeRoot32BitConstants(0, 2, cb, 0);
        cmdList->SetComputeRootShaderResourceView(1, m_blasArgsMeta->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(2, m_blasArgsBuffer->GetGPUVirtualAddress());
        const UINT groups = (N_obj + 63) / 64;
        cmdList->Dispatch(groups, 1, 1);
        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(m_blasArgsBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(m_blasArgsBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cmdList->ResourceBarrier(_countof(barriers), barriers);
    }

    // Implicit/compact CLAS builds produce this address array as a DEFAULT-heap
    // UAV.  BUILD_BLAS_FROM_CLAS reads it through each indirect argument.
    // GetSizes creates the same member on an UPLOAD heap in GENERIC_READ.
    if (m_clasAllocMode != ClasAllocMode::GetSizes)
    {
        auto toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_clasAddressArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_dxrCommandList->ResourceBarrier(1, &toRead);
    }
    {
        // Explicit destination addresses: one entry per object = its m_blasGPUVA.
        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> destAddrs(N_obj);
        for (UINT i = 0; i < N_obj; ++i) destAddrs[i] = m_objects[i].blasGPUVA;
        AllocateUploadBuffer(device, destAddrs.data(), destAddrs.size() * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                             &m_blasResultAddrBuffer, L"BLAS dest addrs (multi-object)");
    }

    D3D12_RTAS_BATCHED_OPERATION_DATA batched = {};
    batched.AddressResolutionFlags    = D3D12_RTAS_OPERATION_ADDRESS_RESOLUTION_FLAG_NONE;
    batched.BatchResultData           = 0;
    batched.BatchScratchData          = m_blasScratchBuffer->GetGPUVirtualAddress();
    batched.ResultAddressArray        = { m_blasResultAddrBuffer->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batched.ResultSizeArray           = { 0, 0 };
    batched.IndirectArgumentArray     = { m_blasArgsBuffer->GetGPUVirtualAddress(), sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS) };
    batched.IndirectArgumentArraySize = 0;

    D3D12_RTAS_OPERATION_DESC opDesc = {};
    opDesc.Inputs                = opInputs;
    opDesc.pBatchedOperationData = &batched;

    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDesc, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);

    // ONE UAV barrier covers every clone's BLAS because they all share
    // the m_clusterBlasPoolBuffer storage.  Replaces the per-object
    // barrier loop (which collapsed N=10K x 1 barrier into the same
    // count of structs the kernel had to walk).
    auto poolBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_clusterBlasPoolBuffer.Get());
    m_dxrCommandList->ResourceBarrier(1, &poolBarrier);
}

// =====================================================================================
// Per-frame static-AS rebuilds (driven by m_staticRebuildMode = [R] key)
// =====================================================================================
//
// Re-issues the corresponding RTAS op against the buffers built once at init
// (no reallocations).  Used purely to demonstrate the per-frame cost of an
// LOD-style AS-churn workload even though the inputs in this sample don't
// actually change between frames.
//
// IMPORTANT: these run on the same command list as everything else.  Caller
// is responsible for ordering relative to UpdateAnimatedObjectPerFrame +
// RebuildTlasPerFrame (the static rebuild must happen BEFORE the TLAS
// rebuild, so the TLAS sees the freshly-rebuilt BLAS contents -- BLAS GVAs
// stay constant via EXPLICIT_DESTINATIONS so TLAS instance descs are still
// valid).
// ---------------------------------------------------------------------------------

// =====================================================================================
// Traditional BLAS build path -- one classic BLAS per static object, with
// one D3D12_RAYTRACING_GEOMETRY_DESC per cluster inside that BLAS (so
// GeometryIndex() at hit time recovers the cluster index, matching the
// cluster path's ClusterID()).  Used when m_geometryMode == Traditional.
// TLAS hands these BLAS GVAs out via obj.tradBlasGPUVA.
//
// ------------------------------------------------------------------------
// WHY ONE GEOMETRY DESC PER CLUSTER (READ THIS BEFORE COPYING):
//
// The per-cluster geometry-desc layout we use here is a SAMPLE-ONLY
// choice driven by the demo's apples-to-apples comparison goal.  We
// want the traditional path's closest-hit to be able to recover the
// same per-cluster identity the cluster path gets from ClusterID(),
// so we lay one geometry desc per cluster inside the per-object BLAS
// and feed g_perInstanceFirstCid[InstanceIndex()] + GeometryIndex()
// back into the SAME g_clusterMeta / g_clusterNormals / etc. tables.
// That gets you visually identical output between the two paths with
// the smallest possible code delta, which is what makes the memory
// + build-cost numbers in the overlay directly comparable.
//
// A real app shipping the traditional path would almost certainly
// NOT subdivide a mesh into ~50 geometry descs per object just to
// preserve cluster scope -- it costs measurable BLAS bytes (the
// BVH carries per-geometry split metadata) and build scratch (see
// the overlay numbers).  The natural unit a real app picks is
// MATERIALS: one geometry desc per material region of the mesh,
// because that's the boundary at which the closest-hit needs to
// branch.  Material count is typically O(few) per object instead
// of O(hundreds) of clusters, so the BVH overhead is small.
//
// Worth noting: even in the cluster path, a real app would also
// usually use multiple geometry descs at the TLAS-instance level
// to express materials -- cluster boundaries are placed for spatial
// coherence and traversal efficiency, NOT material coherence, so a
// single cluster will routinely straddle multiple materials in
// real meshes.  The cluster path gives you finer-than-material
// granularity (per-cluster overrides, per-cluster opacity flags,
// per-cluster precision) AS WELL AS material-driven geometries --
// the two layers compose; they're not alternatives.  Our demo
// scene happens to be one-material-per-object so we don't exercise
// that, but the API supports it cleanly.
// ------------------------------------------------------------------------
//
// Two allocation strategies, cycled via [A] in traditional mode (mirrors
// the cluster path's [A] CLAS alloc-mode toggle):
//
//   Implicit: one-shot.  Prebuild reports worst-case size, allocate that
//             buffer, build into it, done.  obj.tradBlasActualBytes ==
//             obj.tradBlasResultBytes.
//
//   Compact:  two-pass.
//             Pass 1: build with ALLOW_COMPACTION into a worst-case
//             temp BLAS + emit POSTBUILD_INFO_COMPACTED_SIZE.
//             GPU flush, CPU readback of per-BLAS actual sizes.
//             Pass 2: allocate tight compacted buffers per object,
//             CopyRaytracingAccelerationStructure(COPY_MODE_COMPACT)
//             into them, drop the worst-case temp buffers.
//             obj.tradBlasActualBytes is the compacted size; the
//             persistent obj.tradBlasStorage IS the compacted buffer
//             (worst-case temp is freed at end of build).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildTraditionalStaticAS()
{
    auto device      = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    const bool wantCompact = (m_traditionalAllocMode == TraditionalAllocMode::Compact);
    const auto t0 = std::chrono::steady_clock::now();
    UINT64 totalResultBytes  = 0;   // sum of worst-case sizes (== "alloc bytes" in overlay)
    UINT64 totalActualBytes  = 0;   // sum of per-BLAS final-storage sizes (compacted in Compact mode)
    UINT64 totalScratchBytes = 0;
    UINT64 totalVbBytes      = 0;
    UINT64 totalIbBytes      = 0;
    UINT   totalGeomDescs    = 0;

    // Per-object build info we accumulate in PASS A so PASS C can run
    // the actual BuildRaytracingAccelerationStructure into pre-allocated
    // pool slots instead of per-object CreateCommittedResource calls.
    // Pass A also records per-object VB and IB BYTE OFFSETS into the
    // shared trad VB/IB pools (allocated between PASS A and the geom-desc
    // patch-up that finalises pool GVAs).
    struct PerObjInfo {
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs;
        UINT64 resultBytes  = 0;   // prebuild ResultDataMaxSizeInBytes
        UINT64 resultOffset = 0;   // running cumulative offset into the worst-case pool
        UINT64 scratchBytes = 0;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS)0;
        // Trad VB/IB pool slot offsets (vs the pool buffers'
        // GetGPUVirtualAddress() base).  Used to finalise the per-region
        // geom descs' VertexBuffer.StartAddress / IndexBuffer pointers
        // once the pool buffers are allocated.
        UINT64 vbOffset = 0;
        UINT64 ibOffset = 0;
    };
    std::vector<PerObjInfo> info(m_objects.size());

    constexpr UINT64 kAlign = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;  // 256
    UINT64 maxScratchBytes = 0;

    // Cumulative VB+IB pool data, built up during PASS A and uploaded as
    // ONE committed UPLOAD resource each after the loop.  Replaces 2*N
    // CreateCommittedResource calls (= 2*N driver round-trips at ~ms each)
    // with 2 allocations + 2 memcpys.  At [N]=10K trad mode this is the
    // single biggest startup-time saving on top of the existing BLAS pool:
    // ~16 seconds of CreateCommittedResource overhead disappears.
    std::vector<XMFLOAT3> poolVbData;
    std::vector<UINT32>   poolIbData;
    // Pre-allocate a reasonable hint (avoids quadratic-amortized
    // reallocations during the loop's verts.size() push-backs).  Each
    // object will adjust the actual sizes as it accumulates.
    poolVbData.reserve(1024u * 1024u);   // 1M verts initial; grows on demand
    poolIbData.reserve(2u * 1024u * 1024u); // 2M indices initial

    // -------- PASS A: per-object VB+IB + geom desc build + prebuild --------
    // per-object since they hold distinct mesh data) but DON'T yet
    // allocate the BLAS or scratch -- those go into shared pools
    // built after this pass completes and we know per-obj prebuild
    // sizes.
    for (size_t oi = 0; oi < m_objects.size(); ++oi)
    {
        auto& obj = m_objects[oi];

        // (Concatenate cluster VB + cluster IB into per-object buffers
        // in matRegionIdx-sorted order, so all clusters of region 0 land
        // contiguously first, then region 1, etc.  This lets us emit
        // one geom desc per material region spanning a contiguous
        // VB+IB sub-range.  Indices stay 0-based within each cluster's
        // vertex slice -- each geom desc's VertexBuffer.StartAddress is
        // biased to its first cluster's first vertex, and its
        // VertexCount covers all the verts in the region.)

        // Stable sort cluster indices by matRegionIdx so within-region
        // cluster order is preserved (matches the order
        // BuildTradCidLookup walks them in, so the per-tri cid table
        // and the geom-desc layout stay in sync).
        std::vector<UINT> clusterOrder(obj.clusterCount);
        std::iota(clusterOrder.begin(), clusterOrder.end(), 0u);
        std::stable_sort(clusterOrder.begin(), clusterOrder.end(),
            [&](UINT a, UINT b) {
                return obj.mesh.clusters[a].matRegionIdx <
                       obj.mesh.clusters[b].matRegionIdx;
            });

        // Walk in sorted order, building VB+IB and recording per-region
        // ranges (first-vertex-in-VB / first-tri-in-IB / vertex count /
        // tri count).
        std::vector<XMFLOAT3> verts;
        std::vector<UINT32>   idx32;
        struct RegionRange {
            UINT regionIdx;
            UINT firstVbIdx;   // first vertex of region's first cluster in verts[]
            UINT vertCount;    // sum of cluster vert counts in this region
            UINT firstTri;     // first triangle of region in idx32[] / 3
            UINT triCount;     // sum of cluster tri counts in this region
        };
        std::vector<RegionRange> regions;
        UINT prevRegion = 0xFFFFFFFFu;
        for (UINT c : clusterOrder)
        {
            const auto& cl = obj.mesh.clusters[c];
            // Indices are cluster-local 0..N_cluster_verts -- adjust to
            // be region-local 0..N_region_verts so the geom desc's
            // VertexBuffer.StartAddress can point at the region's first
            // vertex and the indices stay valid.
            const UINT clusterVertBase = (UINT)verts.size();
            if (cl.matRegionIdx != prevRegion)
            {
                regions.push_back({ cl.matRegionIdx,
                                    clusterVertBase,
                                    0u,
                                    (UINT)(idx32.size() / 3),
                                    0u });
                prevRegion = cl.matRegionIdx;
            }
            const UINT clusterVertBaseInRegion = clusterVertBase - regions.back().firstVbIdx;
            for (const auto& p : cl.positions)
                verts.push_back({ p.x, p.y, p.z });
            for (auto i : cl.indices)
                idx32.push_back((UINT32)i + clusterVertBaseInRegion);
            regions.back().vertCount += (UINT)cl.positions.size();
            regions.back().triCount  += (UINT)(cl.indices.size() / 3);
        }
        obj.tradVertexCount   = (UINT)verts.size();
        obj.tradTriangleCount = (UINT)(idx32.size() / 3);


        // Pool-mode: append this object's VB+IB data to the cumulative
        // pool buffers, record the per-object byte offset, and clear the
        // per-object ComPtrs (the pool owns the memory).  We'll allocate
        // the actual UPLOAD-heap pool buffers + memcpy + finalise per-
        // region GVAs after PASS A finishes.
        info[oi].vbOffset = poolVbData.size() * sizeof(XMFLOAT3);
        info[oi].ibOffset = poolIbData.size() * sizeof(UINT32);
        poolVbData.insert(poolVbData.end(), verts.begin(), verts.end());
        poolIbData.insert(poolIbData.end(), idx32.begin(), idx32.end());
        obj.tradVertexBuffer.Reset();   // pool owns the memory; per-obj GVA via obj.tradVbGPUVA set below
        obj.tradIndexBuffer.Reset();
        obj.tradVbGPUVA = 0;            // patched in after pool allocation
        obj.tradIbGPUVA = 0;
        totalVbBytes += verts.size() * sizeof(XMFLOAT3);
        totalIbBytes += idx32.size() * sizeof(UINT32);

        // One geom desc per material region (NOT per cluster).  In the
        // current scene every object has one region (matRegionIdx==0 for
        // all clusters) -> exactly 1 geom desc per object.  When we add
        // a mixed-material object (e.g. the [mixed sphere] scene tweak)
        // those clusters get matRegionIdx 0 or 1 -> 2 geom descs in that
        // object's BLAS.  GeometryIndex() in the closest-hit returns the
        // material-region slot, which the fixed-function shader-table
        // indexing then routes to per-region hit groups (chrome -> Opaque,
        // glass -> Glass) via MultiplierForGeometryContributionToHitGroupIndex.
        //
        // Per-region geom flag.  OPAQUE skips the any-hit dispatch at
        // traversal time, which is the cheap path -- but it MUST NOT be
        // set on a region whose material can refract or translucently
        // reject, or stochastic-translucency / refraction behaviour will
        // silently break.  Compute the flag PER REGION from the region's
        // own material slot (perRegionMaterialSlot[regionIdx] when set,
        // otherwise the object-wide fallback obj.instanceID).  A per-
        // cluster checker override that introduces refr applies to ANY
        // cluster regardless of region (centroid-based), so when checker
        // overrides may inject refraction we force NONE on every region.
        // (CheckerOverride has no translucency knob today -- translucency
        // is base-material-only -- so we only check overrideRefr.)
        const bool checkerCouldRefract = obj.checker.enabled &&
            (obj.checker.evenParity.overrideRefr > 0.0f ||
             obj.checker.oddParity.overrideRefr  > 0.0f);

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs(regions.size());
        // Geom descs use PLACEHOLDER addresses (= per-object pool byte
        // offset only); they get patched up with the actual pool-base GVA
        // after the trad VB/IB pools are allocated below.
        const D3D12_GPU_VIRTUAL_ADDRESS vbPlaceholder = info[oi].vbOffset;
        const D3D12_GPU_VIRTUAL_ADDRESS ibPlaceholder = info[oi].ibOffset;
        for (size_t gi = 0; gi < regions.size(); ++gi)
        {
            const auto& rg = regions[gi];
            // Look up the material slot for THIS region.
            const UINT regionMatSlot = (rg.regionIdx < (UINT)obj.perRegionMaterialSlot.size())
                ? obj.perRegionMaterialSlot[rg.regionIdx]
                : obj.instanceID;
            const auto& rgMat = m_materials[regionMatSlot];
            const bool regionIsOpaqueLike =
                (rgMat.translucency == 0.0f) && (rgMat.refractivity == 0.0f) &&
                !checkerCouldRefract;
            const auto geomFlag = regionIsOpaqueLike
                ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE
                : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
            auto& gd = geomDescs[gi];
            gd.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            gd.Flags = geomFlag;
            gd.Triangles.IndexFormat                 = DXGI_FORMAT_R32_UINT;
            gd.Triangles.IndexCount                  = rg.triCount * 3;
            gd.Triangles.IndexBuffer                 = ibPlaceholder + (UINT64)rg.firstTri * 3u * sizeof(UINT32);
            gd.Triangles.VertexFormat                = DXGI_FORMAT_R32G32B32_FLOAT;
            gd.Triangles.VertexCount                 = rg.vertCount;
            gd.Triangles.VertexBuffer.StartAddress   = vbPlaceholder + (UINT64)rg.firstVbIdx * sizeof(XMFLOAT3);
            gd.Triangles.VertexBuffer.StrideInBytes  = sizeof(XMFLOAT3);
        }
        totalGeomDescs += (UINT)geomDescs.size();

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
        inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.NumDescs       = (UINT)geomDescs.size();
        inputs.pGeometryDescs = geomDescs.data();
        // PREFER_FAST_TRACE for hit perf; ALLOW_UPDATE so the [F] refit
        // toggle (animated path, future) can update without rebuild;
        // ALLOW_COMPACTION only when we actually want to compact (the
        // flag has a small build-cost on some drivers).
        inputs.Flags          = BuildFlagModeDxr1()
                              | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
                              | (wantCompact
                                   ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION
                                   : (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS)0);

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

        // Stash everything PASS C will need to call BuildRTAS for this
        // obj.  geomDescs is move'd into the per-obj info struct so it
        // lives across passes (the build descriptor's pGeometryDescs
        // points at it).  We also record the per-obj BLAS slot offset
        // into the upcoming pool (cumulative running sum).
        info[oi].resultBytes  = prebuild.ResultDataMaxSizeInBytes;
        info[oi].scratchBytes = prebuild.ScratchDataSizeInBytes;
        info[oi].resultOffset = totalResultBytes;
        info[oi].flags        = inputs.Flags;
        info[oi].geomDescs    = std::move(geomDescs);
        const UINT64 alignedResult = (prebuild.ResultDataMaxSizeInBytes + kAlign - 1) & ~(kAlign - 1);
        totalResultBytes  += alignedResult;
        totalScratchBytes += prebuild.ScratchDataSizeInBytes;
        maxScratchBytes    = std::max(maxScratchBytes, prebuild.ScratchDataSizeInBytes);

        obj.tradBlasResultBytes   = prebuild.ResultDataMaxSizeInBytes;
        obj.tradBlasScratchBytes  = prebuild.ScratchDataSizeInBytes;
    }

    // -------- PASS A.5: allocate trad VB/IB pools, upload, finalise GVAs --------
    // Now that we know the total VB+IB byte counts (computed via the
    // poolVbData/poolIbData accumulators in PASS A), allocate the two
    // UPLOAD-heap pool buffers in ONE shot each and memcpy the
    // accumulated data in.  Per-object geom descs were emitted with
    // PLACEHOLDER addresses (= per-object byte offset only); patch them
    // up here by adding the pool-base GVA.
    if (!poolVbData.empty())
    {
        const UINT64 vbBytes = poolVbData.size() * sizeof(XMFLOAT3);
        const UINT64 ibBytes = poolIbData.size() * sizeof(UINT32);
        AllocateUploadBuffer(device, poolVbData.data(), vbBytes,
                             &m_tradVertexPool,
                             L"Traditional VB pool (all static objects)");
        AllocateUploadBuffer(device, poolIbData.data(), ibBytes,
                             &m_tradIndexPool,
                             L"Traditional IB pool (all static objects)");
        const D3D12_GPU_VIRTUAL_ADDRESS vbBase = m_tradVertexPool->GetGPUVirtualAddress();
        const D3D12_GPU_VIRTUAL_ADDRESS ibBase = m_tradIndexPool->GetGPUVirtualAddress();
        for (size_t oi = 0; oi < m_objects.size(); ++oi)
        {
            auto& obj = m_objects[oi];
            obj.tradVbGPUVA = vbBase + info[oi].vbOffset;
            obj.tradIbGPUVA = ibBase + info[oi].ibOffset;
            // Patch each geom desc -- its VertexBuffer.StartAddress and
            // IndexBuffer currently hold the OFFSET only (placeholders
            // from PASS A); add the pool base GVA in place.
            for (auto& gd : info[oi].geomDescs)
            {
                gd.Triangles.VertexBuffer.StartAddress += vbBase;
                gd.Triangles.IndexBuffer               += ibBase;
            }
        }
        SampleLog::LogF(L"[traditional VB+IB pool] %.2f MB VB + %.2f MB IB "
                        L"(was %u per-object CreateCommittedResource pairs)\n",
                        vbBytes / (1024.0 * 1024.0),
                        ibBytes / (1024.0 * 1024.0),
                        (unsigned)m_objects.size());

        // Free the CPU-side scratch -- the pool buffers now own the data.
        poolVbData = std::vector<XMFLOAT3>{};
        poolIbData = std::vector<UINT32>{};
    }

    // -------- PASS B: allocate worst-case BLAS pool + shared scratch --------
    // Replaces N x AllocateUAVBuffer (each a separate CreateCommittedResource
    // costing ~ms in driver overhead) with a SINGLE allocation that
    // every per-object BLAS slot points into.  At [N]=10K trad mode this
    // collapses 7500+ driver alloc calls (the cause of multi-second
    // toggle stalls) down to 1 BLAS pool + 1 scratch.
    AllocateUAVBuffer(device, totalResultBytes, &m_tradBlasWorstcasePool,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      wantCompact ? L"Traditional BLAS pool (worst-case, pre-compact)"
                                  : L"Traditional BLAS pool (implicit)");
    AllocateUAVBuffer(device, std::max<UINT64>(maxScratchBytes, 256ull),
                      &m_tradBlasSharedScratch,
                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Traditional BLAS scratch (shared)");
    const D3D12_GPU_VIRTUAL_ADDRESS worstcasePoolBase = m_tradBlasWorstcasePool->GetGPUVirtualAddress();
    const D3D12_GPU_VIRTUAL_ADDRESS sharedScratchGVA  = m_tradBlasSharedScratch->GetGPUVirtualAddress();

    // -------- PASS C: actually run the per-object builds into pool slots --------
    // Each build writes its BLAS into worstcasePoolBase + info[oi].resultOffset.
    // The scratch buffer is shared across builds; we emit a UAV barrier on
    // it between each build so the next build doesn't start scratch writes
    // before the previous build's reads complete (per-build BuildRTAS is
    // an indirect dispatch from D3D12's perspective; ordering between
    // dispatches on the same scratch needs the barrier).
    for (size_t oi = 0; oi < m_objects.size(); ++oi)
    {
        auto& obj = m_objects[oi];
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
        inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.NumDescs       = (UINT)info[oi].geomDescs.size();
        inputs.pGeometryDescs = info[oi].geomDescs.data();
        inputs.Flags          = info[oi].flags;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
        buildDesc.Inputs                             = inputs;
        buildDesc.DestAccelerationStructureData      = worstcasePoolBase + info[oi].resultOffset;
        buildDesc.ScratchAccelerationStructureData   = sharedScratchGVA;
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

        // UAV barrier on shared scratch so the next per-obj build sees
        // it idle.  The dest pool's own UAV barrier comes at the end
        // (one for the whole pool).
        auto scratchBar = CD3DX12_RESOURCE_BARRIER::UAV(m_tradBlasSharedScratch.Get());
        m_dxrCommandList->ResourceBarrier(1, &scratchBar);

        if (!wantCompact)
        {
            // Implicit: the pool slot IS the final storage; obj.tradBlasStorage
            // stays null (the per-obj ComPtr would be redundant -- pool
            // owns the memory).  obj.tradBlasGPUVA points into the pool.
            obj.tradBlasStorage.Reset();
            obj.tradBlasScratch.Reset();
            obj.tradBlasGPUVA       = worstcasePoolBase + info[oi].resultOffset;
            obj.tradBlasActualBytes = info[oi].resultBytes;
            totalActualBytes       += info[oi].resultBytes;
        }
    }
    // One pool-level UAV barrier so subsequent ops (TLAS build,
    // postbuild emit) see every per-obj BLAS write.
    {
        auto poolBar = CD3DX12_RESOURCE_BARRIER::UAV(m_tradBlasWorstcasePool.Get());
        m_dxrCommandList->ResourceBarrier(1, &poolBar);
    }

    // -------- Implicit mode: done. --------
    if (!wantCompact)
    {
        const double cpuMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - t0).count();
        SampleLog::LogF(L"[traditional BLAS][implicit] %zu objects, %u geom descs, "
                        L"result=%llu  scratch=%llu  vb=%llu  ib=%llu (bytes) -- CPU=%.3f ms\n",
                        m_objects.size(), totalGeomDescs,
                        (unsigned long long)totalResultBytes,
                        (unsigned long long)totalScratchBytes,
                        (unsigned long long)totalVbBytes,
                        (unsigned long long)totalIbBytes,
                        cpuMs);
        m_traditionalStaticTotalResultBytes  = totalResultBytes;
        m_traditionalStaticTotalActualBytes  = totalActualBytes;
        m_traditionalStaticTotalScratchBytes = totalScratchBytes;
        m_traditionalStaticBuildMs           = cpuMs;
        // Per-tri cid lookup (matches the cluster-sort order we just used).
        BuildTradCidLookup();
        return;
    }

    // -------- Compact mode: emit postbuild sizes, flush, readback, compact-copy. --------
    const UINT N = (UINT)m_objects.size();
    const UINT64 postbuildBytes = (UINT64)N * sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC);

    ComPtr<ID3D12Resource> postbuildBuf;     // DEFAULT-heap UAV the driver writes into
    AllocateUAVBuffer(device, postbuildBytes, &postbuildBuf,
                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Traditional BLAS postbuild-info (compacted-size)");
    ComPtr<ID3D12Resource> postbuildReadback;
    {
        auto rbHeap  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto rbDesc  = CD3DX12_RESOURCE_DESC::Buffer(postbuildBytes);
        ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&postbuildReadback)));
    }

    // Emit one COMPACTED_SIZE postbuild-info per BLAS.  Single API call --
    // EmitRaytracingAccelerationStructurePostbuildInfo takes an array of
    // BLAS GVAs and writes the descs back-to-back into postbuildBuf.
    // Source GVAs come from worstcasePoolBase + per-obj offset (each
    // BLAS lives in the worst-case pool, not in its own resource).
    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> srcGvas(N);
    for (UINT i = 0; i < N; ++i)
        srcGvas[i] = worstcasePoolBase + info[i].resultOffset;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC pbDesc = {};
    pbDesc.DestBuffer = postbuildBuf->GetGPUVirtualAddress();
    pbDesc.InfoType   = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE;
    m_dxrCommandList->EmitRaytracingAccelerationStructurePostbuildInfo(&pbDesc, N, srcGvas.data());

    // Copy postbuild info into the CPU-readable readback buffer.
    auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(postbuildBuf.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(1, &toCopy);
    commandList->CopyBufferRegion(postbuildReadback.Get(), 0, postbuildBuf.Get(), 0, postbuildBytes);
    auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(postbuildBuf.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &fromCopy);

    // Flush + wait for postbuild info.
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();

    // CPU-side: read compacted sizes.
    std::vector<UINT64> compactedSizes(N);
    {
        D3D12_RANGE readAll = { 0, postbuildBytes };
        void* mapped = nullptr;
        ThrowIfFailed(postbuildReadback->Map(0, &readAll, &mapped));
        auto* p = (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC*)mapped;
        for (UINT i = 0; i < N; ++i) compactedSizes[i] = p[i].CompactedSizeInBytes;
        D3D12_RANGE wrote = { 0, 0 };
        postbuildReadback->Unmap(0, &wrote);
    }

    // Pass 2: allocate the COMPACTED-pool buffer sized to the sum of
    // per-obj compacted sizes, then copy each per-obj BLAS from its
    // slot in the worst-case pool into its slot in the compact pool
    // via COPY_MODE_COMPACT.
    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

    std::vector<UINT64> compactOffsets(N);
    UINT64 totalCompactBytes = 0;
    for (UINT i = 0; i < N; ++i)
    {
        compactOffsets[i] = totalCompactBytes;
        totalCompactBytes += (compactedSizes[i] + kAlign - 1) & ~(kAlign - 1);
    }
    AllocateUAVBuffer(device, std::max<UINT64>(totalCompactBytes, 256ull),
                      &m_tradBlasCompactPool,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Traditional BLAS pool (compacted)");
    const D3D12_GPU_VIRTUAL_ADDRESS compactPoolBase = m_tradBlasCompactPool->GetGPUVirtualAddress();

    for (UINT i = 0; i < N; ++i)
    {
        auto& obj = m_objects[i];
        m_dxrCommandList->CopyRaytracingAccelerationStructure(
            compactPoolBase + compactOffsets[i],
            worstcasePoolBase + info[i].resultOffset,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT);

        // No per-resource ComPtr -- pool owns the memory.  Stats /
        // reset paths read these GVA/size fields instead.
        obj.tradBlasStorage.Reset();
        obj.tradBlasGPUVA       = compactPoolBase + compactOffsets[i];
        obj.tradBlasActualBytes = compactedSizes[i];
        totalActualBytes       += compactedSizes[i];
    }
    {
        auto compactBar = CD3DX12_RESOURCE_BARRIER::UAV(m_tradBlasCompactPool.Get());
        m_dxrCommandList->ResourceBarrier(1, &compactBar);
    }

    // Execute + wait HERE so the worst-case temp pool (m_tradBlasWorstcasePool)
    // stays alive until the GPU finishes copying out of it.  Same
    // motivation as the old per-obj temps[].srcBlas keep-alive: dropping
    // it pre-submit would fire D3D12 ERROR 921 ("resource deleted prior
    // to closing").  After the wait, the worst-case pool is no longer
    // needed (its contents are now copied into m_tradBlasCompactPool)
    // so we drop it to reclaim the temp memory.
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();
    m_tradBlasWorstcasePool.Reset();
    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

    const double cpuMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - t0).count();
    SampleLog::LogF(L"[traditional BLAS][compact] %zu objects, %u geom descs, "
                    L"worst-case=%llu  compacted=%llu (%.1f%% of wc)  scratch=%llu  "
                    L"vb=%llu  ib=%llu (bytes) -- CPU=%.3f ms\n",
                    m_objects.size(), totalGeomDescs,
                    (unsigned long long)totalResultBytes,
                    (unsigned long long)totalActualBytes,
                    100.0 * (double)totalActualBytes / (double)totalResultBytes,
                    (unsigned long long)totalScratchBytes,
                    (unsigned long long)totalVbBytes,
                    (unsigned long long)totalIbBytes,
                    cpuMs);
    m_traditionalStaticTotalResultBytes  = totalResultBytes;
    m_traditionalStaticTotalActualBytes  = totalActualBytes;
    m_traditionalStaticTotalScratchBytes = totalScratchBytes;
    m_traditionalStaticBuildMs           = cpuMs;

    // Build the per-triangle cid lookup + per-(InstIdx, GeomIdx) tri-base
    // table.  Cheap, but must be rebuilt whenever the static AS rebuilds
    // because the matRegionIdx-sorted cluster order in BuildTraditionalStaticAS
    // is what defines the geom layout the lookups have to match.
    BuildTradCidLookup();
}

// =====================================================================================
// Traditional-BLAS cid recovery tables.
//
// Cluster path's closest-hit uses ClusterID() (DXR2 intrinsic) so the
// cid is "free" at hit time.  Traditional path doesn't have that --
// GeometryIndex() returns the material-region slot now (one geom desc
// per material region), and PrimitiveIndex() returns the triangle slot
// within that geom.  Neither directly identifies the source cluster.
//
// So we precompute two small tables:
//
//   m_tradTriToCidBuffer: flat per-static-triangle uint = cluster ID
//                         of the cluster that triangle came from.
//                         Indexed by (geomTriBase + PrimitiveIndex()).
//
//   m_tradGeomTriBaseBuffer: per-(InstanceIdx, GeometryIdx) uint =
//                            first index into m_tradTriToCidBuffer for
//                            this instance's GeomIdx-th material region.
//                            Flat 2D layout with kMaxGeomsPerInstance
//                            entries per instance (padded; unused slots
//                            are 0).
//
// At hit time the traditional shader does
//   uint geomBase = g_tradGeomTriBase[InstanceIndex()*MaxGeoms + GeomIdx];
//   uint cid      = g_tradTriToCid[geomBase + PrimitiveIndex()];
// and then everything downstream (cluster-meta lookup, per-vertex
// normals, cluster colour palette, etc.) is identical to the cluster
// path.  Cost is one uint per scene triangle + a few hundred bytes
// for the per-geom base table -- negligible.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildTradCidLookup()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Walk every object in TLAS-instance order, walk every cluster within
    // each object IN THE ORDER THE BLAS BUILD CONSUMED THEM (which is the
    // matRegionIdx-sorted order BuildTraditionalStaticAS used), append
    // (cid, cluster-local-primIdx) per triangle.  Stamp a per-(instance,
    // geom) tri-base into g_tradGeomTriBase every time we cross a
    // matRegion boundary so GeometryIndex() at hit time can index it
    // directly.
    //
    // Why uint2 (cid + localPrim) rather than just uint cid?  In the
    // cluster path, each CLAS is its own implicit "geometry slot" and
    // PrimitiveIndex() at hit time is the triangle index WITHIN that
    // CLAS -- so the smooth-normal lookup (g_clusterIndices.Load(off.y
    // + primIdx*3)) works directly.  In the traditional path's new
    // per-region geom-desc layout, PrimitiveIndex() is the triangle
    // index WITHIN THE REGION (spanning many clusters), NOT within the
    // source cluster -- using it directly would OOB-walk into other
    // clusters' index buffers and read garbage (which on the 4090
    // cascades into a TDR-hang).  So we precompute the per-cluster
    // local prim alongside the cid and the shader uses BOTH values.
    std::vector<XMUINT2> triToCidLocal;   // .x = cid, .y = localPrimIdxWithinCluster
    // Size for static instances + the animated source instance + every
    // anim clone (they SHARE the source's BLAS / geom-desc layout so they
    // need an entry pointing at the same tri-base as the source).  Without
    // anim clone entries, the trad-mode shader's
    //   g_tradGeomTriBase.Load(InstanceIndex() * MAX_GEOMS + GeomIdx)
    // read for an anim clone (InstanceIndex >= N_static + 1) OOBs the
    // buffer, returns garbage, then (garbage + PrimitiveIndex) * 8 OOBs
    // g_tradTriToCid -> garbage cid -> ClusterMeta OOB -> GPU page-fault
    // -> DXGI_ERROR_DEVICE_HUNG (TDR).  Same OOB-read class of bug as the
    // anim-clone InstanceID/g_materials lookup fixed in commit 414c23a.
    const UINT animSlots      = m_animatedObjectEnabled ? 1u : 0u;
    const UINT animCloneCount = (UINT)m_animatedClones.size();
    const UINT instCount      = (UINT)m_objects.size() + animSlots + animCloneCount;
    std::vector<UINT>    geomTriBase(instCount * kMaxGeomsPerInstance, 0);
    triToCidLocal.reserve(m_totalTriangleCount);

    for (size_t oi = 0; oi < m_objects.size(); ++oi)
    {
        auto& obj = m_objects[oi];

        // Sort cluster indices within this object by matRegionIdx (stable
        // so within a region the original cluster order is preserved).
        std::vector<UINT> clusterOrder(obj.clusterCount);
        std::iota(clusterOrder.begin(), clusterOrder.end(), 0u);
        std::stable_sort(clusterOrder.begin(), clusterOrder.end(),
            [&](UINT a, UINT b) {
                return obj.mesh.clusters[a].matRegionIdx <
                       obj.mesh.clusters[b].matRegionIdx;
            });

        // Walk in that order, stamping the tri-base whenever we enter a
        // new matRegion.
        UINT prevRegion  = 0xFFFFFFFFu;
        for (UINT c : clusterOrder)
        {
            const auto& cl = obj.mesh.clusters[c];
            const UINT  region = cl.matRegionIdx;
            if (region != prevRegion)
            {
                if (region >= kMaxGeomsPerInstance)
                {
                    SampleLog::LogF(L"[traditional] WARNING: object %zu cluster %u has "
                                    L"matRegionIdx %u >= kMaxGeomsPerInstance %u -- skipping "
                                    L"the out-of-range region.\n",
                                    oi, c, region, kMaxGeomsPerInstance);
                    continue;
                }
                geomTriBase[oi * kMaxGeomsPerInstance + region] = (UINT)triToCidLocal.size();
                prevRegion = region;
            }
            const UINT triCount = (UINT)(cl.indices.size() / 3);
            for (UINT t = 0; t < triCount; ++t)
                triToCidLocal.push_back(XMUINT2(cl.clusterID, t));
        }
    }

    // Animated ball: single region (geom 0) at TLAS instance index N_static.
    // Cluster order matches the order BuildAnimatedTraditionalAS walks them
    // in (linear over m_animatedObject.mesh.clusters), so the per-tri lookup
    // stays in lock-step with the flat IB the trad BLAS sees.  cid is
    // offset by m_animatedClusterIdOffset to match BuildClusterMetadata's
    // layout, so g_clusterMeta lookups for the animated ball's per-cluster
    // checker overrides hit the right slots.
    if (m_animatedObjectEnabled)
    {
        const UINT animSourceTriBase = (UINT)triToCidLocal.size();
        geomTriBase[m_objects.size() * kMaxGeomsPerInstance + 0] = animSourceTriBase;
        for (const auto& cl : m_animatedObject.mesh.clusters)
        {
            const UINT triCount = (UINT)(cl.indices.size() / 3);
            for (UINT t = 0; t < triCount; ++t)
                triToCidLocal.push_back(XMUINT2(cl.clusterID + m_animatedClusterIdOffset, t));
        }
        // Anim clones share source's tradBlasGPUVA (or per-clone BLAS pool
        // slots that were built from the source's geometry).  Either way
        // the BLAS layout is identical to the source so anim clones use the
        // SAME tri-base entry as the source -- give them entries that point
        // at the source's tri-base so the trad-mode shader's
        //   g_tradGeomTriBase[InstanceIndex * MAX_GEOMS + GeomIdx]
        // lookup returns valid data and PrimitiveIndex resolves to the
        // correct (cid, localPrim) pair in g_tradTriToCid.
        const size_t baseAnimRow = m_objects.size() + 1;   // after static + source
        for (size_t k = 0; k < m_animatedClones.size(); ++k)
        {
            geomTriBase[(baseAnimRow + k) * kMaxGeomsPerInstance + 0] = animSourceTriBase;
        }
    }

    AllocateUploadBuffer(device, triToCidLocal.data(),
                         triToCidLocal.size() * sizeof(XMUINT2),
                         &m_tradTriToCidBuffer,
                         L"Traditional per-triangle (cid, localPrim) lookup");
    AllocateUploadBuffer(device, geomTriBase.data(),
                         geomTriBase.size() * sizeof(UINT),
                         &m_tradGeomTriBaseBuffer,
                         L"Traditional per-(InstIdx, GeomIdx) tri-base table");
    SampleLog::LogF(L"[traditional] cid-lookup tables: %zu (cid,localPrim) entries (%.1f KB) + "
                    L"%zu base entries (%zu bytes)\n",
                    triToCidLocal.size(), triToCidLocal.size() * sizeof(XMUINT2) / 1024.0,
                    geomTriBase.size(), geomTriBase.size() * sizeof(UINT));
}

// =====================================================================================
// Per-(InstanceIdx, GeometryIdx) -> material-slot lookup table.
//
// Replaces the in-shader `g_materials[InstanceID()]` lookup with one
// that's keyed on both the TLAS instance AND the per-region GeometryIndex().
// For single-region objects the table just has one meaningful entry per
// instance (set to obj.instanceID, so the closesthit gets the same
// material it used to).  For multi-region objects (e.g. the mixed-
// material small sphere with chrome upper / glass lower hemispheres)
// each region gets its own entry pointing at whichever g_materials[]
// slot the region should render as.
//
// Flat 2D layout: uint per (InstIdx, GeomIdx), padded to
// kMaxGeomsPerInstance entries per instance.  Unused slots are 0.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildPerInstGeomMaterialTable()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Include the animated instance + every anim clone (when enabled).
    // InstanceIndex() ranges 0..(N_static + 1 + N_animClone - 1) so the
    // table must cover ALL trad-instance rows.  Anim clones share the
    // source's single material region (slot 7) -- give them the same
    // entry as the animated source.  Without this, anim-clone shader
    // hits would OOB-read this buffer (same TDR-causing pattern as the
    // g_tradGeomTriBase OOB fixed in BuildTradCidLookup).
    const UINT animSlots      = m_animatedObjectEnabled ? 1u : 0u;
    const UINT animCloneCount = (UINT)m_animatedClones.size();
    const UINT N_inst         = (UINT)m_objects.size() + animSlots + animCloneCount;
    std::vector<UINT> table(N_inst * kMaxGeomsPerInstance, 0);
    for (size_t oi = 0; oi < m_objects.size(); ++oi)
    {
        const auto& obj = m_objects[oi];
        // Discover how many regions the object actually uses (== max
        // matRegionIdx + 1).  Skipped if perRegionMaterialSlot is set,
        // in which case its size IS the region count.
        UINT regionCount = (UINT)obj.perRegionMaterialSlot.size();
        if (regionCount == 0)
        {
            for (const auto& cl : obj.mesh.clusters)
                regionCount = std::max(regionCount, cl.matRegionIdx + 1);
            if (regionCount == 0) regionCount = 1;
        }
        for (UINT r = 0; r < std::min(regionCount, kMaxGeomsPerInstance); ++r)
        {
            const UINT slot = (r < (UINT)obj.perRegionMaterialSlot.size())
                ? obj.perRegionMaterialSlot[r]
                : obj.instanceID;
            table[oi * kMaxGeomsPerInstance + r] = slot;
        }
    }
    if (m_animatedObjectEnabled)
    {
        // Animated instance: single-region with the animated obj's material slot.
        table[m_objects.size() * kMaxGeomsPerInstance + 0] = m_animatedObject.instanceID;
        // Anim clones: same material slot as the source (slot 7 anim glass).
        const size_t baseAnimRow = m_objects.size() + 1;
        for (size_t k = 0; k < m_animatedClones.size(); ++k)
        {
            table[(baseAnimRow + k) * kMaxGeomsPerInstance + 0] = m_animatedObject.instanceID;
        }
    }
    AllocateUploadBuffer(device, table.data(),
                         table.size() * sizeof(UINT),
                         &m_perInstGeomMaterialBuffer,
                         L"Per-(InstIdx, GeomIdx) material-slot lookup");
    SampleLog::LogF(L"[per-inst-geom-material] %zu entries (%zu bytes)\n",
                    table.size(), table.size() * sizeof(UINT));
}


void D3D12RaytracingClusteredGeometry::RebuildStaticBlasPerFrame()
{
    if (!m_blasArgsBuffer || !m_blasResultAddrBuffer || !m_blasScratchBuffer) return;

    const UINT  N_obj  = (UINT)m_objects.size();
    UINT maxClasPerArg = 0;
    UINT totalClas     = 0;
    for (const auto& obj : m_objects)
    {
        maxClasPerArg = std::max(maxClasPerArg, obj.clusterCount);
        totalClas    += obj.clusterCount;
    }

    D3D12_RTAS_CLAS_INPUTS_DESC blasDesc = {};
    blasDesc.Flags              = BuildFlagModeRtas();
    blasDesc.MaxArgCount        = N_obj;
    blasDesc.Mode               = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;
    blasDesc.MaxTotalClasCount  = totalClas;
    blasDesc.MaxClasCountPerArg = maxClasPerArg;

    D3D12_RTAS_OPERATION_INPUTS opInputs = {};
    opInputs.Type      = D3D12_RTAS_OPERATION_TYPE_BUILD_BLAS_FROM_CLAS;
    opInputs.pClasDesc = &blasDesc;

    D3D12_RTAS_BATCHED_OPERATION_DATA batched = {};
    batched.AddressResolutionFlags    = D3D12_RTAS_OPERATION_ADDRESS_RESOLUTION_FLAG_NONE;
    batched.BatchResultData           = 0;
    batched.BatchScratchData          = m_blasScratchBuffer->GetGPUVirtualAddress();
    batched.ResultAddressArray        = { m_blasResultAddrBuffer->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batched.ResultSizeArray           = { 0, 0 };
    batched.IndirectArgumentArray     = { m_blasArgsBuffer->GetGPUVirtualAddress(), sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS) };
    batched.IndirectArgumentArraySize = 0;

    D3D12_RTAS_OPERATION_DESC opDesc = {};
    opDesc.Inputs                = opInputs;
    opDesc.pBatchedOperationData = &batched;

    // Order prior TLAS/DispatchRays reads and the preceding use of shared
    // scratch before overwriting the static BLAS pool again.
    auto preOverwrite = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    m_dxrCommandList->ResourceBarrier(1, &preOverwrite);
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDesc, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);

    // One pool-level UAV barrier covers all per-object BLAS writes
    // (they all live in m_clusterBlasPoolBuffer; see BuildBlasFromClasIndirect).
    auto poolBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_clusterBlasPoolBuffer.Get());
    m_dxrCommandList->ResourceBarrier(1, &poolBarrier);
}

void D3D12RaytracingClusteredGeometry::RebuildStaticClasPerFrame()
{
    // Only safe in Implicit mode -- m_clasResultBuffer in GetSizes/Compact
    // modes is sized for exact-fit/compacted output and an unguarded re-run
    // could (and likely would, given the spec doesn't promise size
    // monotonicity across rebuilds) overflow.  Caller's responsibility to
    // gate -- we just no-op out here.
    if (m_clasAllocMode != ClasAllocMode::Implicit) return;
    if (!m_clasArgsBuffer || !m_clasResultBuffer || !m_clasScratchBuffer ||
        !m_clasAddressArray || !m_clasSizeArray) return;

    const UINT N = m_totalClusterCount;
    const bool useFloat = (m_vertexMode == VertexMode::Float32_3);

    UINT maxTris = 0, maxVerts = 0, totalTris = 0, totalVerts = 0;
    UINT maxCompressedSize = 0;
    for (const auto& obj : m_objects)
    for (size_t i = 0; i < obj.mesh.clusters.size(); ++i)
    {
        UINT t = (UINT)(obj.mesh.clusters[i].indices.size() / 3);
        UINT v = (UINT)obj.mesh.clusters[i].positions.size();
        maxTris  = std::max(maxTris,  t);  totalTris  += t;
        maxVerts = std::max(maxVerts, v);  totalVerts += v;
        if (!useFloat)
            maxCompressedSize = std::max(maxCompressedSize, (UINT)obj.encoded[i].TotalBytes());
    }

    D3D12_RTAS_CLUSTER_LIMITS limits;
    D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC clasDesc;
    BuildSharedClusterTrianglesInputs(N, useFloat, maxTris, maxVerts,
                                      totalTris, totalVerts, maxCompressedSize,
                                      kMaxGeomsPerInstance - 1,
                                      m_positionTruncateBits, BuildFlagModeRtas(),
                                      limits, clasDesc);
    clasDesc.Mode = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;

    D3D12_RTAS_OPERATION_INPUTS opInputs = {};
    opInputs.Type                  = D3D12_RTAS_OPERATION_TYPE_BUILD_CLAS_FROM_TRIANGLES;
    opInputs.pClusterTrianglesDesc = &clasDesc;

    // Order the previous frame's CLAS/BLAS/TLAS/raytracing accesses and
    // shared-scratch reuse, then return the address output to UAV state.
    D3D12_RESOURCE_BARRIER preOverwrite[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(nullptr),
        CD3DX12_RESOURCE_BARRIER::Transition(m_clasAddressArray.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
    };
    m_dxrCommandList->ResourceBarrier(_countof(preOverwrite), preOverwrite);

    D3D12_RTAS_BATCHED_OPERATION_DATA batched = {};
    batched.AddressResolutionFlags    = D3D12_RTAS_OPERATION_ADDRESS_RESOLUTION_FLAG_NONE;
    batched.BatchResultData           = m_clasResultBuffer->GetGPUVirtualAddress();
    batched.BatchScratchData          = m_clasScratchBuffer->GetGPUVirtualAddress();
    batched.ResultAddressArray        = { m_clasAddressArray->GetGPUVirtualAddress(), sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batched.ResultSizeArray           = { m_clasSizeArray->GetGPUVirtualAddress(),    sizeof(UINT64) };
    batched.IndirectArgumentArray     = { m_clasArgsArrayGPUVA, m_clasArgsStride };
    batched.IndirectArgumentArraySize = 0;

    D3D12_RTAS_OPERATION_DESC opDesc = {};
    opDesc.Inputs                = opInputs;
    opDesc.pBatchedOperationData = &batched;

    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDesc, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasAddressArray.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(m_clasSizeArray.Get()),
        CD3DX12_RESOURCE_BARRIER::Transition(m_clasAddressArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
    };
    m_dxrCommandList->ResourceBarrier(_countof(barriers), barriers);
}


// ---------------------------------------------------------------------------------
// Classic TLAS holding one instance per object (unaffected by the cluster path -
// from the TLAS's perspective, a Cluster BLAS is just a BLAS).
// ---------------------------------------------------------------------------------
// =====================================================================================
// ANIMATED OBJECT - cluster templates path
//
// Demonstrates the second half of the DXR2 cluster build API:
//
//   ┌─ once at init ─────────────────────────────────────────────────────────┐
//   │ BuildAnimatedObjectSetup()                                             │
//   │  1. Generate a sphere mesh; compute "hint" positions = max-deformation │
//   │     envelope (rest * 1.15). The driver uses these to size the cluster  │
//   │     template BVHs so per-frame instantiated positions fit.             │
//   │  2. Upload hint vertex blob + index data + per-cluster                 │
//   │     BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS into one upload buffer │
//   │  3. ExecuteIndirectRTASOperations(BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES)│
//   │     → templateResultBuffer holds N "positionless" template CLAS, their │
//   │       GPU VAs land in templateAddressArray                             │
//   │  4. Allocate the per-frame resources (vertex upload, instantiate args  │
//   │     upload, CLAS result/scratch/addr, BLAS storage/scratch/args)       │
//   │  5. Set the BLAS GPU VA - this never changes; only the BVH inside it   │
//   │     gets overwritten every frame                                       │
//   └────────────────────────────────────────────────────────────────────────┘
//
//   ┌─ every frame ──────────────────────────────────────────────────────────┐
//   │ UpdateAnimatedObjectPerFrame()                                         │
//   │  1. CPU: compute new vertex positions (pulsating sphere); memcpy into  │
//   │     the persistently-mapped per-frame upload buffer                    │
//   │  2. ExecuteIndirectRTASOperations(INSTANTIATE_CLUSTER_TEMPLATES)       │
//   │     consumes (template GVA, new vertex slice GVA) per cluster          │
//   │     and writes fresh CLAS into perFrameClasResultBuffer                │
//   │  3. ExecuteIndirectRTASOperations(BUILD_BLAS_FROM_CLAS) → rewrites     │
//   │     the BLAS contents at the existing BLAS GVA (EXPLICIT_DESTINATIONS) │
//   │  TLAS rebuild happens elsewhere - it has to be re-run to pick up the   │
//   │  new BLAS root bounds even though the BLAS GVA is unchanged.           │
//   └────────────────────────────────────────────────────────────────────────┘
//
// CLUSTER ID OFFSET: the animated object's clusters get unique IDs by setting
// ClusterIdOffset=m_animatedClusterIdOffset in the instantiate args. The
// dynamic base is allocated after all active source/clone IDs, so animated
// template hits can never alias static shader-side table entries.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildAnimatedObjectSetup()
{
    auto device      = m_deviceResources->GetD3DDevice();
    auto& obj        = m_animatedObject;

    // ------------------------------------------------------------------
    // 1) Mesh + hints. Sphere with spatial cluster tiles. Hints = rest * envelopeScale
    //    so animation can squash/stretch within [1/envelope, envelope] without
    //    leaving the cluster's pre-built BVH bounds.
    //
    //    INVARIANT: kEnvelopeScale MUST stay above (1 + kAnimWobbleAmp).
    //    FLOAT32_3 templates use the hint vertices to describe their expected
    //    deformation envelope.  COMPRESSED1 templates independently derive a
    //    shared exponent below from the same worst-case animation amplitude.
    //    If that amplitude changes without updating this mirrored constant,
    //    the hints and the locked COMPRESSED1 range can both become invalid.
    //    The
    //    HLSL-side wobble amplitude lives in AnimateBall.hlsl::kWobbleAmp and
    //    is mirrored here ONLY for this assert.  Keep the two in sync.
    // ------------------------------------------------------------------
    constexpr float kRestRadius    = 1.00f;       // central showpiece ball, bigger
    constexpr float kEnvelopeScale = 1.30f;       // hint sphere radius = rest * 1.30 (was 1.18 -- bumped to give headroom over kAnimWobbleAmp 0.20)
    constexpr float kAnimWobbleAmp = 0.20f;       // MUST match AnimateBall.hlsl::kWobbleAmp (bumped from 0.08 so wave is visible on [N] clones at smaller scales / further distances)
    static_assert(1.0f + kAnimWobbleAmp < kEnvelopeScale,
        "kEnvelopeScale must leave headroom over the animation's max radial "
        "displacement (1 + kAnimWobbleAmp).  See AnimateBall.hlsl::kWobbleAmp -- "
        "if you raise the wobble amplitude there, bump kEnvelopeScale here.");
    // 2x cluster count per dim (192 -> 768 clusters: 24 lat x 32 long)
    // while KEEPING per-cluster tris count at 48 - resolution doubled
    // in each dim (48x96 -> 96x192), tile size kept (tileLat 4, tileLong 6).
    obj.mesh = ProceduralGeometry::GenerateUVSphereSpatialTiles(
        kRestRadius, /*numLat*/96, /*numLong*/192, /*tileLat*/4, /*tileLong*/6, 0);
    obj.clusterCount         = (UINT)obj.mesh.clusters.size();
    obj.worldPos             = XMFLOAT3(0.0f, 1.10f, 0.0f);        // hovers above the hex group, dropped lower so refractions through it pick up the floor + objects below
    obj.worldScale           = 1.0f;
    obj.instanceID           = 7;       // matches NUM_MATERIAL_SLOTS-1; this is the refractive glass slot
    obj.vertexBufferStride   = (UINT)sizeof(XMFLOAT3);

    // Per-cluster hint positions = rest position scaled outward by envelopeScale.
    obj.hintPositions.resize(obj.clusterCount);
    obj.totalVertexCount = 0;
    for (UINT c = 0; c < obj.clusterCount; ++c)
    {
        const auto& src = obj.mesh.clusters[c];
        obj.hintPositions[c].resize(src.positions.size());
        for (size_t v = 0; v < src.positions.size(); ++v)
        {
            obj.hintPositions[c][v] = {
                src.positions[v].x * kEnvelopeScale,
                src.positions[v].y * kEnvelopeScale,
                src.positions[v].z * kEnvelopeScale,
            };
        }
        obj.totalVertexCount += (UINT)src.positions.size();
        obj.maxVertsPerCluster = std::max(obj.maxVertsPerCluster, (UINT)src.positions.size());
        obj.maxTrisPerCluster  = std::max(obj.maxTrisPerCluster,  (UINT)(src.indices.size() / 3));
    }
    // The template's stored vertex format follows the sample's [V] mode.
    // Source and hint vertices remain FLOAT32_3; INSTANTIATE converts them to
    // the template's VertexInstantiationFormat.  COMPRESSED1 locks one shared
    // exponent and the selected bit count into every template header.  The
    // exponent must cover the largest possible per-cluster component span
    // across the full radial-animation range, not merely the rest pose.
    const bool useCompressedTemplate = (m_vertexMode == VertexMode::Compressed1);
    UINT compressedTemplateHeaderField0 = 0;
    if (useCompressedTemplate)
    {
        constexpr float kMinAnimatedScale = 1.0f - kAnimWobbleAmp;
        constexpr float kMaxAnimatedScale = 1.0f + kAnimWobbleAmp;
        float maxAnimatedExtent = 0.0f;
        for (const auto& cluster : obj.mesh.clusters)
        {
            float mn[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX };
            float mx[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX };
            for (const auto& p : cluster.positions)
            {
                const float value[3] = { p.x, p.y, p.z };
                for (int axis = 0; axis < 3; ++axis)
                {
                    const float a = value[axis] * kMinAnimatedScale;
                    const float b = value[axis] * kMaxAnimatedScale;
                    mn[axis] = std::min(mn[axis], std::min(a, b));
                    mx[axis] = std::max(mx[axis], std::max(a, b));
                }
            }
            for (int axis = 0; axis < 3; ++axis)
                maxAnimatedExtent = std::max(maxAnimatedExtent, mx[axis] - mn[axis]);
        }

        const UINT bitsPerComponent = m_compressedBitsPerComponent;
        const uint64_t maxDelta = (1ull << bitsPerComponent) - 1ull;
        const double minUnit = double(maxAnimatedExtent) / double(maxDelta);
        int templateSharedExponent = (minUnit > 0.0)
            ? (int)std::ceil(std::log2(minUnit)) + 127
            : 1;
        templateSharedExponent = std::clamp(templateSharedExponent, 1, 232);

        // Validate the actual quantized extrema at both animation endpoints,
        // matching Compressed1::Encode's round-to-nearest rule.  Increase the
        // locked exponent until every component span fits its bit budget.
        for (;;)
        {
            const double invScale = std::ldexp(1.0, 127 - templateSharedExponent);
            bool fits = true;
            for (const auto& cluster : obj.mesh.clusters)
            {
                int64_t quantizedMin[3] = { INT64_MAX, INT64_MAX, INT64_MAX };
                int64_t quantizedMax[3] = { INT64_MIN, INT64_MIN, INT64_MIN };
                for (const auto& p : cluster.positions)
                {
                    const double value[3] = { p.x, p.y, p.z };
                    for (int axis = 0; axis < 3; ++axis)
                    {
                        const double endpoints[2] = {
                            value[axis] * kMinAnimatedScale,
                            value[axis] * kMaxAnimatedScale,
                        };
                        for (double v : endpoints)
                        {
                            const int64_t q = (int64_t)std::floor(v * invScale + 0.5);
                            quantizedMin[axis] = std::min(quantizedMin[axis], q);
                            quantizedMax[axis] = std::max(quantizedMax[axis], q);
                        }
                    }
                }
                for (int axis = 0; axis < 3; ++axis)
                    fits &= uint64_t(quantizedMax[axis] - quantizedMin[axis]) <= maxDelta;
                if (!fits) break;
            }
            if (fits || templateSharedExponent == 232) break;
            ++templateSharedExponent;
        }

        D3D12_VERTEX_FORMAT_COMPRESSED1_TEMPLATE_HEADER header = {};
        ENCODE_D3D12_COMPRESSED1_TEMPLATE(
            header, (UINT)templateSharedExponent,
            bitsPerComponent - 1u, bitsPerComponent - 1u, bitsPerComponent - 1u);
        compressedTemplateHeaderField0 = header.field0;

        SampleLog::LogF(
            L"[animated template format] COMPRESSED1 %u bits/component; shared exponent (biased)=%d, unit=%.6f, header=0x%08X\n",
            bitsPerComponent, templateSharedExponent,
            std::ldexp(1.0f, templateSharedExponent - 127),
            compressedTemplateHeaderField0);
    }
    else
    {
        SampleLog::LogF(
            L"[animated template format] FLOAT32_3; PositionTruncateBitCount=%u\n",
            m_positionTruncateBits);
    }

    SampleLog::LogF(L"\n[animated] sphere mesh: %u clusters, %u verts, %u tris (envelope x%.2f)\n",
                    obj.clusterCount, obj.totalVertexCount,
                    obj.mesh.totalTriangles, kEnvelopeScale);

    // Roll the animated mesh's triangles into the scene-wide total now
    // that the mesh has been generated; the title bar reads from this.
    // Recompute from scratch each time because BuildAnimatedObjectSetup is
    // now re-invoked by RebuildStaticAccelerationStructures on A/V/[/]
    // toggles -- a naive '+=' would keep stacking the animated tri count.
    m_totalTriangleCount = obj.mesh.totalTriangles;
    for (const auto& o : m_objects)
        m_totalTriangleCount += o.mesh.totalTriangles;

    // Shared sizing helper used by both the cluster-template input layout
    // (below) and the trad-mode flat IB build (in BuildAnimatedTraditionalAS).
    constexpr size_t kAlign = 16;
    auto alignTo = [](size_t x, size_t a) { return (x + (a - 1)) & ~(a - 1); };
    // Shared upload heap.  Step 2 (cluster-only, below) uses it for the
    // template-input buffer; step 4 (shared) uses it again for the rest-
    // positions staging upload.
    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RANGE noRead(0, 0);

    // Cluster limits used by BOTH the BUILD_CLUSTER_TEMPLATES op and the
    // per-frame INSTANTIATE_CLUSTER_TEMPLATES prebuild later in this
    // function -- hoisted to shared scope so the second cluster-only
    // block can see it.  Harmless to compute always (just copies of
    // obj.* counts from the shared mesh-gen step).
    D3D12_RTAS_CLUSTER_LIMITS limits = {};
    limits.MaxArgCount                                   = obj.clusterCount;
    limits.MaxGeometryIndexValue                         = 0;
    limits.MaxUniqueGeometryIndexAndFlagsCountPerCluster = 1;
    limits.MaxTriangleCountPerCluster                    = obj.maxTrisPerCluster;
    limits.MaxVertexCountPerCluster                      = obj.maxVertsPerCluster;
    limits.MaxTotalTriangleCount                         = obj.mesh.totalTriangles;
    limits.MaxTotalVertexCount                           = obj.totalVertexCount;

    // ==================================================================
    // CLUSTER-MODE-ONLY: template build + per-frame CLAS/BLAS storage
    // ==================================================================
    // Skipped entirely in traditional mode -- the trad path builds a single
    // per-object DXR1 BLAS each frame from perFrameVertexBuffer + a flat
    // IB (allocated below in the shared block + later in
    // BuildAnimatedTraditionalAS), so it doesn't need templates, INSTANTIATE
    // args, or per-cluster CLAS results.
    const bool kClusterPath = (m_geometryMode == GeometryMode::Clusters);
    if (kClusterPath)
    {
    // ------------------------------------------------------------------
    // 2) Upload buffer: hint vertex data + index data + template build args.
    //    Layout (all aligned to 16):
    //        [cluster 0 hint vertices][cluster 0 indices] ... [cluster N-1 ...]
    //        [per-cluster D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS]
    // ------------------------------------------------------------------
    struct Slot { size_t vbOffset, vbSize, ibOffset, ibSize; };
    std::vector<Slot> slots(obj.clusterCount);
    size_t cursor = 0;
    for (UINT c = 0; c < obj.clusterCount; ++c)
    {
        const auto& src = obj.mesh.clusters[c];
        cursor = alignTo(cursor, kAlign);
        slots[c].vbOffset = cursor;
        slots[c].vbSize   = src.positions.size() * sizeof(XMFLOAT3);
        cursor += slots[c].vbSize;
        cursor = alignTo(cursor, kAlign);
        slots[c].ibOffset = cursor;
        slots[c].ibSize   = src.indices.size();
        cursor += slots[c].ibSize;
    }
    cursor = alignTo(cursor, kAlign);
    const size_t argsOffset = cursor;
    const size_t argStride  = sizeof(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS);
    cursor += obj.clusterCount * argStride;
    // Layout: vertex+index data lives in obj.templateInputBuffer (upload heap,
    // CPU-filled here).  Args live in a separate DEFAULT/UAV buffer
    // (obj.templateArgsBuffer below) filled by FillClusterTemplateArgs CS --
    // mirrors the static-CLAS-args refactor.
    const size_t totalDataSize = alignTo(cursor, 256);

    auto bufDesc    = CD3DX12_RESOURCE_DESC::Buffer(totalDataSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeap, D3D12_HEAP_FLAG_NONE, &bufDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&obj.templateInputBuffer)));
    obj.templateInputBuffer->SetName(L"Animated: cluster-template input buffer (vert+idx)");


    uint8_t* mapped = nullptr;
    ThrowIfFailed(obj.templateInputBuffer->Map(0, &noRead, reinterpret_cast<void**>(&mapped)));
    const D3D12_GPU_VIRTUAL_ADDRESS templateInputBaseGPUVA =
        obj.templateInputBuffer->GetGPUVirtualAddress();

    for (UINT c = 0; c < obj.clusterCount; ++c)
    {
        memcpy(mapped + slots[c].vbOffset, obj.hintPositions[c].data(), slots[c].vbSize);
        memcpy(mapped + slots[c].ibOffset, obj.mesh.clusters[c].indices.data(), slots[c].ibSize);
    }
    obj.templateInputBuffer->Unmap(0, nullptr);

    // Per-cluster metadata for FillClusterTemplateArgs CS.  16 bytes/cluster:
    //   { triCount, vertCount, vbOff, ibOff }
    // (ClusterID is implicit = thread index; opaque flag always 0 for
    // animated; stride is hardcoded to 12 in the shader.)
    {
        struct TplArgsMeta { UINT triCount, vertCount, vbOff, ibOff; };
        static_assert(sizeof(TplArgsMeta) == 16, "must match HLSL Load4 stride");

        std::vector<TplArgsMeta> meta(obj.clusterCount);
        for (UINT c = 0; c < obj.clusterCount; ++c)
        {
            const auto& src = obj.mesh.clusters[c];
            meta[c].triCount  = (UINT)(src.indices.size() / 3);
            meta[c].vertCount = (UINT)src.positions.size();
            meta[c].vbOff     = (UINT)slots[c].vbOffset;
            meta[c].ibOff     = (UINT)slots[c].ibOffset;
        }
        AllocateUploadBuffer(device, meta.data(), meta.size() * sizeof(TplArgsMeta),
            &obj.templateMetaBuffer, L"Animated: template args metadata");
    }

    // GPU-written args buffer (DEFAULT/UAV).
    static_assert(sizeof(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS) == 96,
        "FillClusterTemplateArgs.hlsl stride must match d3d12.h");
    {
        const UINT64 argsBytes = (UINT64)obj.clusterCount * sizeof(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS);
        AllocateUAVBuffer(device, argsBytes,
            &obj.templateArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            L"Animated: template args (GPU-filled)");
    }

    // Dispatch FillClusterTemplateArgs.
    {
        auto cmdList = m_deviceResources->GetCommandList();
        cmdList->SetComputeRootSignature(m_fillTemplateArgsRS.Get());
        cmdList->SetPipelineState(m_fillTemplateArgsPSO.Get());
        struct {
            UINT baseLo, baseHi;
            UINT count;
            UINT posTruncBits;
            UINT useCompressed;
            UINT compressedHeader;
        } cb;
        cb.baseLo           = (UINT)(templateInputBaseGPUVA & 0xFFFFFFFFu);
        cb.baseHi           = (UINT)(templateInputBaseGPUVA >> 32);
        cb.count            = obj.clusterCount;
        cb.posTruncBits     = m_positionTruncateBits;
        cb.useCompressed    = useCompressedTemplate ? 1u : 0u;
        cb.compressedHeader = compressedTemplateHeaderField0;
        cmdList->SetComputeRoot32BitConstants(0, sizeof(cb) / 4, &cb, 0);
        cmdList->SetComputeRootShaderResourceView(1, obj.templateMetaBuffer->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(2, obj.templateArgsBuffer->GetGPUVirtualAddress());
        const UINT groups = (obj.clusterCount + 63) / 64;
        cmdList->Dispatch(groups, 1, 1);
        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(obj.templateArgsBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.templateArgsBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cmdList->ResourceBarrier(_countof(barriers), barriers);
    }

    obj.templateArgsArrayGPUVA = obj.templateArgsBuffer->GetGPUVirtualAddress();
    obj.templateArgsStride     = (UINT)sizeof(D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS);

    // ------------------------------------------------------------------
    // 3) BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES. We compute prebuild info,
    //    allocate the result+scratch+address buffers, fire one batched op for
    //    all N clusters in IMPLICIT_DESTINATIONS mode.
    // ------------------------------------------------------------------
    // `limits` is hoisted to shared scope above so the cluster-only
    // INSTANTIATE prebuild block below can also see it.

    D3D12_RTAS_CLUSTER_TEMPLATE_TRIANGLES_INPUTS_DESC tplDesc = {};
    tplDesc.ClusterLimits             = limits;
    tplDesc.Flags                     = BuildFlagModeRtas();
    tplDesc.Mode                      = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;
    tplDesc.VertexHintFormat          = D3D12_VERTEX_FORMAT_FLOAT32_3;
    tplDesc.VertexInstantiationFormat = useCompressedTemplate
        ? D3D12_VERTEX_FORMAT_COMPRESSED1
        : D3D12_VERTEX_FORMAT_FLOAT32_3;
    tplDesc.IndexFormat               = D3D12_INDEX_FORMAT_UINT8;
    tplDesc.GeometryIndexAndFlagsIndexFormat = D3D12_INDEX_FORMAT_NONE;
    tplDesc.OpacityMicromapIndexFormat       = D3D12_INDEX_FORMAT_NONE;

    D3D12_RTAS_OPERATION_INPUTS opInputsTpl = {};
    opInputsTpl.Type                          = D3D12_RTAS_OPERATION_TYPE_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES;
    opInputsTpl.pClusterTemplateTrianglesDesc = &tplDesc;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuildTpl = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputsTpl, &prebuildTpl);
    SampleLog::LogF(L"[animated template prebuild] %u clusters: result=%llu, scratch=%llu bytes\n",
                    obj.clusterCount,
                    (unsigned long long)prebuildTpl.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuildTpl.ScratchDataSizeInBytes);

    AllocateUAVBuffer(device, prebuildTpl.ResultDataMaxSizeInBytes,
                      &obj.templateResultBuffer, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Animated: cluster templates result buffer");
    AllocateUAVBuffer(device, std::max<UINT64>(prebuildTpl.ScratchDataSizeInBytes, 256ull),
                      &obj.templateScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: cluster templates scratch");
    AllocateUAVBuffer(device, (UINT64)obj.clusterCount * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                      &obj.templateAddressArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: cluster templates address array");

    D3D12_RTAS_BATCHED_OPERATION_DATA batchedTpl = {};
    batchedTpl.BatchResultData       = obj.templateResultBuffer->GetGPUVirtualAddress();
    batchedTpl.BatchScratchData      = obj.templateScratchBuffer->GetGPUVirtualAddress();
    batchedTpl.ResultAddressArray    = { obj.templateAddressArray->GetGPUVirtualAddress(),
                                         sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batchedTpl.IndirectArgumentArray = { obj.templateArgsArrayGPUVA, obj.templateArgsStride };

    D3D12_RTAS_OPERATION_DESC opDescTpl = {};
    opDescTpl.Inputs                = opInputsTpl;
    opDescTpl.pBatchedOperationData = &batchedTpl;
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDescTpl,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);

    D3D12_RESOURCE_BARRIER tplBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(obj.templateResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(obj.templateAddressArray.Get()),
    };
    m_dxrCommandList->ResourceBarrier(_countof(tplBarriers), tplBarriers);

    }   // end cluster-only template-build block

    // ==================================================================
    // SHARED: per-frame vertex buffer + rest-pose positions
    // ==================================================================
    // BOTH paths need these.  AnimateBall.cs writes perFrameVertexBuffer
    // every frame regardless of mode -- the cluster path then feeds it to
    // INSTANTIATE_CLUSTER_TEMPLATES, the traditional path feeds it
    // straight into BuildRaytracingAccelerationStructure as a D3D12_RAYTRACING_GEOMETRY_DESC
    // VertexBuffer.
    // ------------------------------------------------------------------
    // 4) Per-frame resources (allocated once, contents rewritten every frame).
    //    perFrameVertexBuffer  - GPU-only DEFAULT-heap UAV.  Written by
    //                            AnimateBall.cs each frame, read by
    //                            INSTANTIATE_CLUSTER_TEMPLATES.  Cycles
    //                            UAV <-> NON_PIXEL_SHADER_RESOURCE inside
    //                            UpdateAnimatedObjectPerFrame.
    //    restPositionsBuffer   - DEFAULT-heap SRV containing the rest-pose
    //                            positions, uploaded once below.
    //    perFrameInstArgsBuffer- one INSTANTIATE_CLUSTER_TEMPLATES_ARGS per cluster
    //    perFrameClasResultBuffer + scratch + addresses (IMPLICIT_DESTINATIONS)
    //    blasStorage + scratch + args  (EXPLICIT_DESTINATIONS for fixed GPU VA)
    // ------------------------------------------------------------------
    const size_t vbBytes = (size_t)obj.totalVertexCount * sizeof(XMFLOAT3);
    {
        // GPU-only animated vertex buffer (UAV; the compute shader writes;
        // INSTANTIATE reads).  Starts in UNORDERED_ACCESS so the first frame's
        // compute pass works without an initial state transition.
        auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(alignTo(vbBytes, 256),
                                                    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeap, D3D12_HEAP_FLAG_NONE, &vbDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
            IID_PPV_ARGS(&obj.perFrameVertexBuffer)));
        obj.perFrameVertexBuffer->SetName(L"Animated: per-frame vertex buffer (UAV, compute-written)");
    }
    {
        // Rest-pose positions: upload once from CPU into a DEFAULT-heap buffer.
        // Read by AnimateBall.cs via a raw SRV bound as a root descriptor.
        // Done as a self-contained ExecuteCommandList+WaitForGpu pair so the
        // upload staging buffer can die at the end of this block (the existing
        // template-GVA readback flush further down would also do, but a
        // self-contained upload reads more straightforwardly).
        auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto restDesc = CD3DX12_RESOURCE_DESC::Buffer(alignTo(vbBytes, 256));
        ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeap, D3D12_HEAP_FLAG_NONE, &restDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
            IID_PPV_ARGS(&obj.restPositionsBuffer)));
        obj.restPositionsBuffer->SetName(L"Animated: rest-pose positions (SRV, compute-read)");

        Microsoft::WRL::ComPtr<ID3D12Resource> staging;
        auto stageDesc = CD3DX12_RESOURCE_DESC::Buffer(alignTo(vbBytes, 256));
        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeap, D3D12_HEAP_FLAG_NONE, &stageDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&staging)));
        staging->SetName(L"Animated: rest-pose staging upload");

        XMFLOAT3* mapped = nullptr;
        ThrowIfFailed(staging->Map(0, &noRead, reinterpret_cast<void**>(&mapped)));
        for (UINT c = 0; c < obj.clusterCount; ++c)
        {
            const auto& src = obj.mesh.clusters[c];
            for (size_t v = 0; v < src.positions.size(); ++v)
            {
                mapped->x = src.positions[v].x;
                mapped->y = src.positions[v].y;
                mapped->z = src.positions[v].z;
                ++mapped;
            }
        }
        staging->Unmap(0, nullptr);

        auto cl = m_deviceResources->GetCommandList();
        cl->CopyBufferRegion(obj.restPositionsBuffer.Get(), 0, staging.Get(), 0, vbBytes);
        auto toSrv = CD3DX12_RESOURCE_BARRIER::Transition(obj.restPositionsBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        cl->ResourceBarrier(1, &toSrv);
        m_deviceResources->ExecuteCommandList();
        m_deviceResources->WaitForGpu();
        // Re-open the command list for the template-GVA readback that follows.
        auto allocator = m_deviceResources->GetCommandAllocator();
        ThrowIfFailed(allocator->Reset());
        ThrowIfFailed(cl->Reset(allocator, nullptr));
        // staging goes out of scope here -- safe, GPU has consumed it.
    }

    // ==================================================================
    // CLUSTER-MODE-ONLY: per-frame INSTANTIATE args + CLAS + BLAS storage
    // ==================================================================
    if (kClusterPath)
    {
    // perFrameInstArgsBuffer: per-cluster INSTANTIATE_CLUSTER_TEMPLATES_ARGS.
    // GPU-written via FillInstantiateArgs CS below (no CPU mapping, no
    // CPU-side readback of template GVAs).  Stays in UNORDERED_ACCESS so
    // CS writes + INSTANTIATE reads are separated by a UAV barrier.
    {
        const size_t instArgsBytes = obj.clusterCount * sizeof(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS);
        AllocateUAVBuffer(device, alignTo(instArgsBytes, 256),
            &obj.perFrameInstArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            L"Animated: per-frame instantiate args (GPU-filled)");
    }

    // vertexOffsetArray: per-cluster byte offset into perFrameVertexBuffer.
    // CPU-computed once (cumulative prefix sum of per-cluster vertex sizes)
    // and uploaded; the fill CS reads from it to compute each cluster's
    // VertexBuffer.StartAddress = pfVbGpuVa + vertexOffsets[c].  The
    // template-GVAs input is obj.templateAddressArray (already on GPU --
    // we don't have to round-trip it through CPU readback any more).
    {
        std::vector<UINT> offsets(obj.clusterCount);
        UINT cursor = 0;
        for (UINT c = 0; c < obj.clusterCount; ++c)
        {
            offsets[c] = cursor;
            cursor += (UINT)(obj.mesh.clusters[c].positions.size() * sizeof(XMFLOAT3));
        }
        AllocateUploadBuffer(device, offsets.data(), offsets.size() * sizeof(UINT),
            &obj.vertexOffsetArray, L"Animated: per-cluster VB byte offsets");
    }

    // Dispatch the GPU args-fill CS.  obj.templateAddressArray is still in
    // UNORDERED_ACCESS from the BUILD_CLUSTER_TEMPLATES op above and the
    // UAV barrier we emitted right after that op is sufficient to make its
    // contents visible to this dispatch (we bind it as RWByteAddressBuffer
    // and only read).  After the dispatch we leave a UAV barrier on
    // perFrameInstArgsBuffer so the downstream INSTANTIATE_CLUSTER_TEMPLATES
    // op sees the written args.
    {
        auto cmdList = m_deviceResources->GetCommandList();
        cmdList->SetComputeRootSignature(m_fillInstArgsRS.Get());
        cmdList->SetPipelineState(m_fillInstArgsPSO.Get());

        const D3D12_GPU_VIRTUAL_ADDRESS pfVbGPUVA = obj.perFrameVertexBuffer->GetGPUVirtualAddress();
        struct {
            UINT pfVbLo, pfVbHi;
            UINT clusterCount;
            INT  clusterIdOffset;
            UINT vertexStride;
        } cb;
        cb.pfVbLo           = (UINT)(pfVbGPUVA & 0xFFFFFFFFu);
        cb.pfVbHi           = (UINT)(pfVbGPUVA >> 32);
        cb.clusterCount     = obj.clusterCount;
        cb.clusterIdOffset  = m_animatedClusterIdOffset;
        cb.vertexStride     = (UINT)sizeof(XMFLOAT3);
        cmdList->SetComputeRoot32BitConstants(0, sizeof(cb) / 4, &cb, 0);

        // u1 = templateGvas (input as RWByteAddressBuffer; UAV barrier above
        // already separates it from the producing template build).
        cmdList->SetComputeRootUnorderedAccessView(1, obj.templateAddressArray->GetGPUVirtualAddress());
        // t0 = vertexOffsets (upload heap, always GENERIC_READ, includes
        // NON_PIXEL_SHADER_RESOURCE).
        cmdList->SetComputeRootShaderResourceView(2, obj.vertexOffsetArray->GetGPUVirtualAddress());
        // u0 = argsOut (UAV).
        cmdList->SetComputeRootUnorderedAccessView(3, obj.perFrameInstArgsBuffer->GetGPUVirtualAddress());

        const UINT groups = (obj.clusterCount + 63) / 64;
        cmdList->Dispatch(groups, 1, 1);

        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameInstArgsBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameInstArgsBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.templateAddressArray.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cmdList->ResourceBarrier(_countof(barriers), barriers);
    }

    // ------------------------------------------------------------------
    // 5) Per-frame CLAS + BLAS storage. Sized via prebuild info.
    // ------------------------------------------------------------------
    D3D12_RTAS_INSTANTIATE_CLUSTER_TEMPLATE_INPUTS_DESC instDesc = {};
    instDesc.ClusterLimits     = limits;
    instDesc.Flags             = BuildFlagModeRtas();
    instDesc.Mode              = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;
    instDesc.VertexSourceFormat = D3D12_VERTEX_FORMAT_FLOAT32_3;

    D3D12_RTAS_OPERATION_INPUTS opInputsInst = {};
    opInputsInst.Type                            = D3D12_RTAS_OPERATION_TYPE_INSTANTIATE_CLUSTER_TEMPLATES;
    opInputsInst.pInstantiateClusterTemplateDesc = &instDesc;

    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuildInst = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputsInst, &prebuildInst);
    SampleLog::LogF(L"[animated instantiate prebuild] result=%llu, scratch=%llu bytes\n",
                    (unsigned long long)prebuildInst.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuildInst.ScratchDataSizeInBytes);

    AllocateUAVBuffer(device, prebuildInst.ResultDataMaxSizeInBytes,
                      &obj.perFrameClasResultBuffer, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Animated: per-frame CLAS result");
    AllocateUAVBuffer(device, std::max<UINT64>(prebuildInst.ScratchDataSizeInBytes, 256ull),
                      &obj.perFrameClasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: per-frame CLAS scratch");
    AllocateUAVBuffer(device, (UINT64)obj.clusterCount * sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
                      &obj.perFrameClasAddressArray, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: per-frame CLAS address array");
    // Per-cluster output-size buffer for the one-shot measurement INSTANTIATE
    // (only written by MeasureAnimatedClasBytesOneShot at the end of this
    // setup function -- the regular per-frame INSTANTIATE skips ResultSizeArray
    // so there's zero per-frame readback cost).
    AllocateUAVBuffer(device, (UINT64)obj.clusterCount * sizeof(UINT64),
                      &obj.perFrameClasSizesBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: per-frame CLAS sizes (measurement)");
    {
        const UINT64 rbBytes = (UINT64)obj.clusterCount * sizeof(UINT64);
        auto rbDesc = CD3DX12_RESOURCE_DESC::Buffer(rbBytes);
        auto rbHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        ThrowIfFailed(device->CreateCommittedResource(
            &rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
            IID_PPV_ARGS(&obj.perFrameClasSizesReadback)));
        obj.perFrameClasSizesReadback->SetName(L"Animated: per-frame CLAS sizes readback");
    }

    // BLAS-from-CLAS prebuild & alloc (single BLAS, EXPLICIT_DESTINATIONS).
    D3D12_RTAS_CLAS_INPUTS_DESC blasDesc = {};
    blasDesc.Flags              = BuildFlagModeRtas();
    blasDesc.MaxArgCount        = 1;
    blasDesc.Mode               = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;
    blasDesc.MaxTotalClasCount  = obj.clusterCount;
    blasDesc.MaxClasCountPerArg = obj.clusterCount;

    D3D12_RTAS_OPERATION_INPUTS opInputsBlas = {};
    opInputsBlas.Type      = D3D12_RTAS_OPERATION_TYPE_BUILD_BLAS_FROM_CLAS;
    opInputsBlas.pClasDesc = &blasDesc;
    D3D12_RTAS_OPERATION_PREBUILD_INFO prebuildBlas = {};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&opInputsBlas, &prebuildBlas);
    SampleLog::LogF(L"[animated BLAS prebuild] result=%llu, scratch=%llu bytes\n",
                    (unsigned long long)prebuildBlas.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuildBlas.ScratchDataSizeInBytes);

    AllocateUAVBuffer(device, prebuildBlas.ResultDataMaxSizeInBytes,
                      &obj.blasStorage, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Animated: BLAS storage");
    AllocateUAVBuffer(device, std::max<UINT64>(prebuildBlas.ScratchDataSizeInBytes, 256ull),
                      &obj.blasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: BLAS scratch");
    obj.blasGPUVA = obj.blasStorage->GetGPUVirtualAddress();

    // BLAS-from-CLAS args (1 entry, never changes).  GPU-filled by
    // FillBlasFromClasArgs CS (same shader as the static path -- the only
    // difference is dispatch count and the input metadata buffer).
    {
        struct BlasArgsMeta { UINT count, gvaLo, gvaHi; };
        const D3D12_GPU_VIRTUAL_ADDRESS clasArrayGva = obj.perFrameClasAddressArray->GetGPUVirtualAddress();
        BlasArgsMeta meta = {
            obj.clusterCount,
            (UINT)(clasArrayGva & 0xFFFFFFFFu),
            (UINT)(clasArrayGva >> 32),
        };
        AllocateUploadBuffer(device, &meta, sizeof(meta),
            &obj.blasArgsMeta, L"Animated: BLAS args metadata");

        AllocateUAVBuffer(device,
            sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS),
            &obj.blasArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            L"Animated: BLAS args (GPU-filled)");

        auto cmdList = m_deviceResources->GetCommandList();
        cmdList->SetComputeRootSignature(m_fillBlasArgsRS.Get());
        cmdList->SetPipelineState(m_fillBlasArgsPSO.Get());
        const UINT cb[2] = { 1u, (UINT)sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
        cmdList->SetComputeRoot32BitConstants(0, 2, cb, 0);
        cmdList->SetComputeRootShaderResourceView(1, obj.blasArgsMeta->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(2, obj.blasArgsBuffer->GetGPUVirtualAddress());
        cmdList->Dispatch(1, 1, 1);
        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(obj.blasArgsBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.blasArgsBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cmdList->ResourceBarrier(_countof(barriers), barriers);
    }
    {
        // BLAS dest-addrs array (1 entry = our fixed BLAS storage GVA).
        AllocateUploadBuffer(device, &obj.blasGPUVA, sizeof(obj.blasGPUVA),
            &obj.blasResultAddrBuffer, L"Animated: BLAS dest addrs");
    }
    SampleLog::LogF(L"[animated] cluster-path setup complete: BLAS at GVA 0x%llx, %u clusters per frame\n",
                    (unsigned long long)obj.blasGPUVA, obj.clusterCount);

    // Setting enabled flag BEFORE the one-shot measure: the measure func has
    // a defensive `if (!m_animatedObjectEnabled) return;` gate at its top.
    // Pre-bug-fix this flag was set at the BOTTOM of BuildAnimatedObjectSetup,
    // so the measure always early-returned and m_overlayStats.animatedPerFrame
    // ClasActualBytes stayed at 0 in both overlay and benchmark JSON.
    m_animatedObjectEnabled = true;

    // One-shot per-cluster CLAS size readback - runs synchronously, populates
    // m_overlayStats.animatedPerFrameClasActualBytes.  Costs ~1ms of extra
    // setup work per config change; zero per-frame cost.
    MeasureAnimatedClasBytesOneShot();

    }   // end cluster-only step 4b + 5 block

    // ==================================================================
    // TRADITIONAL-MODE-ONLY: per-frame DXR1 BLAS setup
    // ==================================================================
    if (!kClusterPath)
    {
        BuildAnimatedTraditionalAS();
    }

    m_animatedObjectEnabled = true;
}

// =====================================================================================
// Phase-2 anim-clones setup: allocate the per-clone BLAS pool and rewrite
// the source's per-frame BLAS args + dest arrays so the existing per-frame
// BUILD_BLAS_FROM_CLAS op produces 1 + N_animClones BLASes in one batched
// call (source + every clone).
//
// Anim clones SHARE the source's per-frame CLAS results (every M-arg
// reads the SAME source.perFrameClasAddressArray) -- this stresses the
// BLAS-from-CLAS path per-clone WITHOUT multiplying INSTANTIATE work.
//
// Each clone's BLAS lands at m_animClonesBlasPool + (k * perBlasBytes)
// where perBlasBytes is the worst-case per-build size.  Pool sized at
// N * perBlasBytes; allocated as ONE committed resource so we don't
// pay N CreateCommittedResource calls (same trick as the static BLAS
// pool).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildAnimatedClonesSetup()
{
    if (m_geometryMode != GeometryMode::Clusters) return;
    if (m_animatedClones.empty()) return;
    if (!m_animatedObjectEnabled) return;

    auto device  = m_deviceResources->GetD3DDevice();
    auto& obj    = m_animatedObject;
    const UINT N = (UINT)m_animatedClones.size();
    const UINT M = N + 1;  // 1 source + N clones

    // Two prebuilds, matching BuildBlasFromClasIndirect's static-pool
    // pattern.  pbSingle (MaxArgCount=1) gives per-BLAS size; pbBatch
    // (MaxArgCount=M) gives the batch-total scratch sizing.
    auto prebuild = [&](UINT maxArgCount, UINT totalClas) {
        D3D12_RTAS_CLAS_INPUTS_DESC d = {};
        d.Flags              = BuildFlagModeRtas();
        d.MaxArgCount        = maxArgCount;
        d.Mode               = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;
        d.MaxTotalClasCount  = totalClas;
        d.MaxClasCountPerArg = obj.clusterCount;
        D3D12_RTAS_OPERATION_INPUTS oi = {};
        oi.Type      = D3D12_RTAS_OPERATION_TYPE_BUILD_BLAS_FROM_CLAS;
        oi.pClasDesc = &d;
        D3D12_RTAS_OPERATION_PREBUILD_INFO pi = {};
        m_dxr2Device->GetRTASOperationPrebuildInfo(&oi, &pi);
        return pi;
    };
    const auto pbSingle = prebuild(1, obj.clusterCount);
    const auto pbBatch  = prebuild(M, obj.clusterCount * M);

    constexpr UINT64 kAlign = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;  // 256
    const UINT64 perBlasBytes = (pbSingle.ResultDataMaxSizeInBytes + kAlign - 1) & ~(kAlign - 1);
    const UINT64 poolBytes    = perBlasBytes * (UINT64)N;
    const UINT64 scratchBytes = std::max<UINT64>(pbBatch.ScratchDataSizeInBytes, 256ull);

    // ONE committed resource for all N clones' BLAS storage.
    AllocateUAVBuffer(device, poolBytes, &m_animClonesBlasPool,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Animated clones BLAS pool");
    const D3D12_GPU_VIRTUAL_ADDRESS poolBaseGVA = m_animClonesBlasPool->GetGPUVirtualAddress();
    for (UINT k = 0; k < N; ++k)
        m_animatedClones[k].blasGPUVA = poolBaseGVA + (UINT64)k * perBlasBytes;

    // SEPARATE scratch / args / dest buffers for the phase-2 per-frame
    // op.  We can NOT reuse / replace obj.blasScratchBuffer /
    // obj.blasArgsBuffer / obj.blasResultAddrBuffer in place because
    // BuildAnimatedObjectSetup already recorded GPU work referencing
    // those resources, and releasing them mid-record was causing an
    // access violation when the recorded command list executed.
    AllocateUAVBuffer(device, scratchBytes, &m_animClonesBlasScratchBuffer,
                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated clones: BLAS scratch (1+N batch)");

    // M-entry args buffer.  All M entries are IDENTICAL bytes -- they
    // all reference source.perFrameClasAddressArray (clones share the
    // source's animated CLAS results).  Per-frame BUILD_BLAS_FROM_CLAS
    // reads M entries; per-entry destination comes from the dest array
    // (NOT from args), so identical args + distinct dests is correct.
    {
        struct ArgsLayout {
            UINT  count;
            UINT  stride;
            UINT  gvaLo;
            UINT  gvaHi;
        };
        static_assert(sizeof(ArgsLayout) == sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS),
                      "BLAS-from-CLAS args layout mismatch");
        const D3D12_GPU_VIRTUAL_ADDRESS clasArrayGva =
            obj.perFrameClasAddressArray->GetGPUVirtualAddress();
        std::vector<ArgsLayout> argsCpu(M, ArgsLayout{
            obj.clusterCount,
            (UINT)sizeof(D3D12_GPU_VIRTUAL_ADDRESS),
            (UINT)(clasArrayGva & 0xFFFFFFFFu),
            (UINT)(clasArrayGva >> 32),
        });
        AllocateUploadBuffer(device, argsCpu.data(),
            argsCpu.size() * sizeof(ArgsLayout),
            &m_animClonesBlasArgsBuffer,
            L"Animated clones: BLAS args (1+N CPU-filled)");
    }

    // M-entry dest GVAs:  [0] = source.blasGPUVA;  [1..N] = pool slots.
    {
        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> dests(M);
        dests[0] = obj.blasGPUVA;
        for (UINT k = 0; k < N; ++k)
            dests[1 + k] = m_animatedClones[k].blasGPUVA;
        AllocateUploadBuffer(device, dests.data(),
            dests.size() * sizeof(dests[0]),
            &m_animClonesBlasResultAddrBuffer,
            L"Animated clones: BLAS dest addrs (1+N)");
    }

    SampleLog::LogF(L"[anim-clones phase2] N=%u, per-clone BLAS=%llu bytes, pool=%.2f MB, scratch=%.2f MB\n",
                    N, (unsigned long long)perBlasBytes,
                    poolBytes / (1024.0 * 1024.0),
                    scratchBytes / (1024.0 * 1024.0));
}


// =====================================================================================
// One-shot INSTANTIATE_CLUSTER_TEMPLATES with ResultSizeArray hooked up so we
// can read the per-cluster CLAS leaf sizes the driver actually emitted.  This
// is invoked ONCE at the end of BuildAnimatedObjectSetup (which itself runs
// at init + on every config change), so the steady-state per-frame INSTANTIATE
// in UpdateAnimatedObjectPerFrame stays free of readback overhead.
// Stalls the GPU briefly while it copies the size UAV into a readback buffer.
// =====================================================================================
// =====================================================================================
// Traditional-mode addendum to BuildAnimatedObjectSetup: builds the flat
// per-object IB (cluster-major, with each cluster's local indices biased
// by its vertex offset in the per-frame VB so a single
// D3D12_RAYTRACING_GEOMETRY_DESC covers the whole ball) and allocates
// the per-frame-rebuilt BLAS storage + scratch.  Run ONCE per
// m_animatedObjectEnabled transition; subsequent frames just call
// UpdateAnimatedTradPerFrame to rebuild/refit into tradBlasStorage.
//
// Why a flat IB rather than per-cluster geom descs?  The animated ball
// renders as a single material (the cluster path's checker pattern
// comes from per-cluster ClusterMeta overrides, not from per-geom-desc
// material slots), so we don't need per-region geom-desc routing here.
// ONE geom desc keeps the BLAS build scratch small and the BVH topology
// compact.  If the animated ball ever grows multi-material regions,
// mirror the static-trad path: split clusters by matRegionIdx, emit one
// geom desc per region, populate matching tri-to-cid lookup entries.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildAnimatedTraditionalAS()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto& obj   = m_animatedObject;

    // ------------------------------------------------------------------
    // 1) Flat IB: walk clusters in order, append each cluster's local
    //    indices biased by the cluster's running vertex offset in the
    //    per-frame VB so a single geom desc sees a contiguous IB.
    //    perFrameVertexBuffer's layout is cluster-major float3 (same
    //    layout AnimateBall.cs writes), so the bias is just the prefix
    //    sum of per-cluster vertex counts.
    // ------------------------------------------------------------------
    std::vector<UINT32> indices;
    indices.reserve((size_t)obj.mesh.totalTriangles * 3);
    UINT vbBase = 0;
    for (UINT c = 0; c < obj.clusterCount; ++c)
    {
        const auto& src = obj.mesh.clusters[c];
        for (auto i : src.indices)
            indices.push_back((UINT32)i + vbBase);
        vbBase += (UINT)src.positions.size();
    }
    obj.tradTriangleCount = (UINT)(indices.size() / 3);

    AllocateUploadBuffer(device, indices.data(),
                         indices.size() * sizeof(UINT32),
                         &obj.tradIndexBuffer,
                         L"Animated: traditional flat IB (cluster-major, biased)");

    // ------------------------------------------------------------------
    // 2) Prebuild + allocate BLAS storage + scratch.
    //    PREFER_FAST_TRACE for ray-hit perf, ALLOW_UPDATE so the [F]
    //    toggle can switch to PERFORM_UPDATE refits without reallocating.
    //    Single geom desc covering perFrameVertexBuffer + tradIndexBuffer.
    // ------------------------------------------------------------------
    D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
    geomDesc.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;  // glass material -> any-hit must run
    geomDesc.Triangles.IndexFormat                = DXGI_FORMAT_R32_UINT;
    geomDesc.Triangles.IndexCount                 = obj.tradTriangleCount * 3;
    geomDesc.Triangles.IndexBuffer                = obj.tradIndexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexFormat               = DXGI_FORMAT_R32G32B32_FLOAT;
    geomDesc.Triangles.VertexCount                = obj.totalVertexCount;
    geomDesc.Triangles.VertexBuffer.StartAddress  = obj.perFrameVertexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(XMFLOAT3);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs       = 1;
    inputs.pGeometryDescs = &geomDesc;
    inputs.Flags          = BuildFlagModeDxr1()
                          | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

    AllocateUAVBuffer(device, prebuild.ResultDataMaxSizeInBytes,
                      &obj.tradBlasStorage,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Animated: traditional BLAS storage");
    const UINT64 scratchBytes = std::max<UINT64>(
        prebuild.ScratchDataSizeInBytes,
        prebuild.UpdateScratchDataSizeInBytes);
    AllocateUAVBuffer(device, std::max<UINT64>(scratchBytes, 256ull),
                      &obj.tradBlasScratchBuffer,
                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated: traditional BLAS scratch");

    obj.tradBlasResultBytes  = prebuild.ResultDataMaxSizeInBytes;
    obj.tradBlasScratchBytes = scratchBytes;
    obj.tradBlasGPUVA        = obj.tradBlasStorage->GetGPUVirtualAddress();
    obj.tradBlasInitialized  = false;  // first per-frame build must be a fresh rebuild

    SampleLog::LogF(L"[animated trad] %u tris, result=%llu  scratch=%llu (build=%llu, update=%llu) bytes\n",
                    obj.tradTriangleCount,
                    (unsigned long long)prebuild.ResultDataMaxSizeInBytes,
                    (unsigned long long)scratchBytes,
                    (unsigned long long)prebuild.UpdateScratchDataSizeInBytes);
}

// =====================================================================================
// Trad-mode phase-2 anim clones: allocate per-clone DXR1 BLAS storage in a
// single committed UAV pool (sized to N x prebuild.ResultDataMaxSizeInBytes
// aligned to RTAS alignment) and a shared scratch buffer.  Per-clone GVAs
// land in m_animatedClones[k].blasGPUVA so BuildTlasClassic picks them up
// naturally (the TLAS code already prefers ac.blasGPUVA over the source's
// GVA when non-zero, regardless of mode).
//
// The actual per-frame work (N x BuildRaytracingAccelerationStructure) is
// scheduled by UpdateAnimatedTradPerFrame.
//
// Memory budget at the higher [N] tiers: per-clone BLAS = ~2 MB for the
// source's sphere mesh, so 2000 anim clones at N=10K = ~4 GB.  RTX 4090
// has 24 GB VRAM and swallows this fine; smaller GPUs would want to back
// off to N=1K (200 anim clones = ~400 MB).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildAnimatedClonesTradSetup()
{
    if (m_geometryMode != GeometryMode::Traditional) return;
    if (m_animatedClones.empty())                    return;
    if (!m_animatedObjectEnabled)                    return;

    auto device = m_deviceResources->GetD3DDevice();
    auto& obj   = m_animatedObject;
    const UINT N = (UINT)m_animatedClones.size();

    // SCALING GATE: DXR1 has no batched BuildRaytracingAccelerationStructure
    // op (unlike DXR2's ExecuteIndirectRTASOperations), so trad-mode phase-2
    // anim-clone BLAS rebuild is N serialised driver calls per frame.  At
    // N>~200 (= [N]=1K's anim-clone count) the per-frame CPU/GPU cost +
    // pool memory (each per-clone trad BLAS is ~2.5 MB, so N=2000 needs
    // ~5 GB pool) overwhelm the TDR budget and risk OOM on smaller GPUs.
    // Above the cap, anim clones fall back to sharing the source's
    // tradBlasGPUVA -- they still ANIMATE (source's BLAS is rebuilt per
    // frame from the deformed perFrameVertexBuffer) but every clone in
    // excess of the cap shares the SAME instantaneous deformation as the
    // source, identical wobble silhouettes.  Cluster mode's phase-2 path
    // (BUILD_BLAS_FROM_CLAS via ExecuteIndirectRTASOperations) IS batched
    // and continues to do per-clone BLASes at any N.
    constexpr UINT kTradPerClonePoolMax = 200u;   // moderate cap; high N falls back to shared source BLAS
    const UINT N_pooled = std::min(N, kTradPerClonePoolMax);
    if (N_pooled == 0)
    {
        // Nothing to pool -- all clones will share source's BLAS in the
        // TLAS instance writeup via the (ac.blasGPUVA == 0) -> animBlasGVA
        // fallback.
        SampleLog::LogF(L"[anim-clones trad phase2] N=%u: ALL clones share source's BLAS (N_pooled=0)\n", N);
        return;
    }

    // Same geom desc shape as BuildAnimatedTraditionalAS / UpdateAnimatedTradPerFrame
    // -- we just need the prebuild sizes here (driver doesn't actually
    // read VB/IB until the per-frame BuildRaytracingAccelerationStructure
    // call, which will refer to the live perFrameVertexBuffer + obj.tradIndexBuffer).
    D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
    geomDesc.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
    geomDesc.Triangles.IndexFormat                = DXGI_FORMAT_R32_UINT;
    geomDesc.Triangles.IndexCount                 = obj.tradTriangleCount * 3;
    geomDesc.Triangles.IndexBuffer                = obj.tradIndexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexFormat               = DXGI_FORMAT_R32G32B32_FLOAT;
    geomDesc.Triangles.VertexCount                = obj.totalVertexCount;
    geomDesc.Triangles.VertexBuffer.StartAddress  = obj.perFrameVertexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(XMFLOAT3);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs       = 1;
    inputs.pGeometryDescs = &geomDesc;
    inputs.Flags          = BuildFlagModeDxr1();
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO pb = {};
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &pb);

    constexpr UINT64 kAlign = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;  // 256
    const UINT64 perBlasBytes = (pb.ResultDataMaxSizeInBytes + kAlign - 1) & ~(kAlign - 1);
    const UINT64 poolBytes    = perBlasBytes * (UINT64)N_pooled;
    const UINT64 scratchBytes = std::max<UINT64>(pb.ScratchDataSizeInBytes, 256ull);

    AllocateUAVBuffer(device, poolBytes, &m_animClonesTradBlasPool,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                      L"Animated clones trad BLAS pool");
    AllocateUAVBuffer(device, scratchBytes, &m_animClonesTradBlasScratch,
                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                      L"Animated clones trad BLAS scratch (shared)");

    const D3D12_GPU_VIRTUAL_ADDRESS poolBaseGVA = m_animClonesTradBlasPool->GetGPUVirtualAddress();
    for (UINT k = 0; k < N_pooled; ++k)
        m_animatedClones[k].blasGPUVA = poolBaseGVA + (UINT64)k * perBlasBytes;
    // Clones beyond the pool cap fall back to source's BLAS (0 GVA -> ternary
    // in BuildTlasClassic picks animBlasGVA).
    for (UINT k = N_pooled; k < N; ++k)
        m_animatedClones[k].blasGPUVA = 0;

    SampleLog::LogF(L"[anim-clones trad phase2] N=%u (pooled=%u, shared=%u), per-clone BLAS=%llu bytes, pool=%.2f MB\n",
                    N, N_pooled, N - N_pooled, (unsigned long long)perBlasBytes,
                    poolBytes / (1024.0 * 1024.0));
}

// =====================================================================================
// Per-frame trad-mode animated update.  Mirrors UpdateAnimatedObjectPerFrame's
// shape but builds a DXR1 BLAS from perFrameVertexBuffer + tradIndexBuffer
// instead of going through INSTANTIATE_CLUSTER_TEMPLATES + BLAS_FROM_CLAS.
//
// Two operating modes selected by m_traditionalAnimMode (toggled by [F]):
//   Rebuild: full BuildRaytracingAccelerationStructure each frame with
//            PREFER_FAST_TRACE.  Slower but produces the highest-quality
//            BVH each frame.
//   Refit:   BuildRaytracingAccelerationStructure with PERFORM_UPDATE flag,
//            reusing the existing BVH topology.  ~4-10x cheaper but BVH
//            quality drifts as vertices wobble away from the topology
//            cached on the source build -- works because the animation
//            stays within the rest-pose envelope (kEnvelopeScale).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::UpdateAnimatedTradPerFrame(UINT pfTimestampBase)
{
    if (!m_animatedObjectEnabled) return;
    if (m_geometryMode != GeometryMode::Traditional) return;

    auto& obj  = m_animatedObject;
    auto cl4_ts = m_dxrCommandList.Get();  // ID3D12GraphicsCommandList4 (has EndQuery)
    auto cl    = m_deviceResources->GetCommandList();

    // ------------------------------------------------------------------
    // 1) Compute pass: AnimateBall.cs deforms restPositionsBuffer -> perFrameVertexBuffer
    //    Shared with the cluster path -- same CS, same root sig, same
    //    state-cycle convention (UAV before dispatch, transition to
    //    NON_PIXEL_SHADER_RESOURCE for the BLAS build's vertex read).
    // ------------------------------------------------------------------
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 0);
    cl->SetComputeRootSignature(m_animComputeRS.Get());
    cl->SetPipelineState(m_animComputePSO.Get());
    // Anim params written into mapped CB by UpdateSceneConstantBuffer
    // each frame.  CB-via-CBV means GPU reads latest content at execute
    // time, so a frozen t propagates to all in-flight frames immediately
    // (matches the camera CB's behaviour -- the pre-pause-fix asymmetry
    // was that camera was mapped but CS used root constants).
    cl->SetComputeRootConstantBufferView (0, m_animParamsBuf->GetGPUVirtualAddress());
    cl->SetComputeRootShaderResourceView (1, obj.restPositionsBuffer ->GetGPUVirtualAddress());
    cl->SetComputeRootUnorderedAccessView(2, obj.perFrameVertexBuffer->GetGPUVirtualAddress());
    const UINT groups = (obj.totalVertexCount + 63u) / 64u;
    cl->Dispatch(groups, 1, 1);

    {
        D3D12_RESOURCE_BARRIER bars[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameVertexBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameVertexBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cl->ResourceBarrier(_countof(bars), bars);
    }
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 1);

    // ------------------------------------------------------------------
    // 2) BLAS build: rebuild or refit per m_traditionalAnimMode.
    // ------------------------------------------------------------------
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 2);
    D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
    geomDesc.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
    geomDesc.Triangles.IndexFormat                = DXGI_FORMAT_R32_UINT;
    geomDesc.Triangles.IndexCount                 = obj.tradTriangleCount * 3;
    geomDesc.Triangles.IndexBuffer                = obj.tradIndexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexFormat               = DXGI_FORMAT_R32G32B32_FLOAT;
    geomDesc.Triangles.VertexCount                = obj.totalVertexCount;
    geomDesc.Triangles.VertexBuffer.StartAddress  = obj.perFrameVertexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(XMFLOAT3);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs       = 1;
    inputs.pGeometryDescs = &geomDesc;
    inputs.Flags          = BuildFlagModeDxr1()
                          | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    const bool wantRefit = (m_traditionalAnimMode == TraditionalAnimMode::Refit)
                            && obj.tradBlasInitialized;
    if (wantRefit)
        inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs                             = inputs;
    buildDesc.DestAccelerationStructureData      = obj.tradBlasGPUVA;
    buildDesc.SourceAccelerationStructureData    = wantRefit ? obj.tradBlasGPUVA : 0;
    buildDesc.ScratchAccelerationStructureData   = obj.tradBlasScratchBuffer->GetGPUVirtualAddress();

    m_dxrCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
    obj.tradBlasInitialized = true;

    // ------------------------------------------------------------------
    // 2b) Trad-mode phase-2 anim clones: per-clone BLAS rebuild.  Each
    //     clone gets its own dest GVA in m_animClonesTradBlasPool (set
    //     by BuildAnimatedClonesTradSetup); same geom desc shape as the
    //     source above (same shared perFrameVertexBuffer +
    //     obj.tradIndexBuffer inputs), so geom desc bytes are identical
    //     except for dest.  Scratch is SHARED across all N builds and
    //     serialised by UAV barriers between calls -- DXR1 has no
    //     batched-build API so this is genuinely N driver calls per
    //     frame.  At [N]=10K that's 2000 calls (~tens of ms of CPU
    //     overhead alone, plus the per-build GPU work) -- exactly the
    //     overhead DXR2's batched ExecuteIndirectRTASOperations API
    //     beats by ~100x in the cluster path.
    // ------------------------------------------------------------------
    if (m_animClonesTradBlasPool && !m_animatedClones.empty())
    {
        auto scratchBar = CD3DX12_RESOURCE_BARRIER::UAV(m_animClonesTradBlasScratch.Get());
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS cInputs = {};
        cInputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        cInputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
        cInputs.NumDescs       = 1;
        cInputs.pGeometryDescs = &geomDesc;
        cInputs.Flags          = BuildFlagModeDxr1();
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC cBuildDesc = {};
        cBuildDesc.Inputs                          = cInputs;
        cBuildDesc.ScratchAccelerationStructureData = m_animClonesTradBlasScratch->GetGPUVirtualAddress();
        for (size_t k = 0; k < m_animatedClones.size(); ++k)
        {
            // Skip clones beyond the trad-pool cap (their blasGPUVA is 0,
            // they share source's tradBlasGPUVA via the TLAS fallback).
            if (m_animatedClones[k].blasGPUVA == 0) continue;
            cBuildDesc.DestAccelerationStructureData   = m_animatedClones[k].blasGPUVA;
            cBuildDesc.SourceAccelerationStructureData = 0;  // always rebuild
            m_dxrCommandList->BuildRaytracingAccelerationStructure(&cBuildDesc, 0, nullptr);
            if (k + 1 < m_animatedClones.size())
                cl->ResourceBarrier(1, &scratchBar);
        }
        // Single UAV barrier on the clone pool covers all clone BLAS writes.
        auto poolBar = CD3DX12_RESOURCE_BARRIER::UAV(m_animClonesTradBlasPool.Get());
        cl->ResourceBarrier(1, &poolBar);
    }


    {
        auto bar = CD3DX12_RESOURCE_BARRIER::UAV(obj.tradBlasStorage.Get());
        cl->ResourceBarrier(1, &bar);
    }
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 3);

    // 3) Restore perFrameVertexBuffer to UAV for next frame's compute.
    {
        auto bar = CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameVertexBuffer.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        cl->ResourceBarrier(1, &bar);
    }
}

void D3D12RaytracingClusteredGeometry::MeasureAnimatedClasBytesOneShot()
{
    if (!m_animatedObjectEnabled) return;
    auto& obj = m_animatedObject;
    auto device = m_deviceResources->GetD3DDevice();
    auto cmdList = m_deviceResources->GetCommandList();

    // BuildAnimatedObjectSetup left the cmd list OPEN and the perFrameVertexBuffer
    // in UNORDERED_ACCESS state (compute UAV).  The animated compute pass that
    // populates positions hasn't run yet (it runs every frame in
    // UpdateAnimatedObjectPerFrame) -- so the source VB still holds zeros from
    // its initial allocation.  That's fine for *sizing* purposes: INSTANTIATE
    // produces fixed-shape CLAS leaves regardless of vertex values, so the size
    // it reports for an all-zeros input matches the size for any other input.

    // Need to transition perFrameVertexBuffer UAV -> NON_PIXEL_SHADER_RESOURCE
    // before INSTANTIATE, then back to UAV after (matching the per-frame state
    // cycle in UpdateAnimatedObjectPerFrame).
    D3D12_RESOURCE_BARRIER toRead =
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameVertexBuffer.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &toRead);

    D3D12_RTAS_CLUSTER_LIMITS limits = {};
    limits.MaxArgCount                                   = obj.clusterCount;
    limits.MaxUniqueGeometryIndexAndFlagsCountPerCluster = 1;
    limits.MaxTriangleCountPerCluster                    = obj.maxTrisPerCluster;
    limits.MaxVertexCountPerCluster                      = obj.maxVertsPerCluster;
    limits.MaxTotalTriangleCount                         = obj.mesh.totalTriangles;
    limits.MaxTotalVertexCount                           = obj.totalVertexCount;

    D3D12_RTAS_INSTANTIATE_CLUSTER_TEMPLATE_INPUTS_DESC instDesc = {};
    instDesc.ClusterLimits      = limits;
    instDesc.Flags              = BuildFlagModeRtas();
    instDesc.Mode               = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;
    instDesc.VertexSourceFormat = D3D12_VERTEX_FORMAT_FLOAT32_3;

    D3D12_RTAS_OPERATION_INPUTS opInputsInst = {};
    opInputsInst.Type                            = D3D12_RTAS_OPERATION_TYPE_INSTANTIATE_CLUSTER_TEMPLATES;
    opInputsInst.pInstantiateClusterTemplateDesc = &instDesc;

    // Same as per-frame INSTANTIATE but with ResultSizeArray populated.
    D3D12_RTAS_BATCHED_OPERATION_DATA batchedInst = {};
    batchedInst.BatchResultData       = obj.perFrameClasResultBuffer->GetGPUVirtualAddress();
    batchedInst.BatchScratchData      = obj.perFrameClasScratchBuffer->GetGPUVirtualAddress();
    batchedInst.ResultAddressArray    = { obj.perFrameClasAddressArray->GetGPUVirtualAddress(),
                                          sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batchedInst.ResultSizeArray       = { obj.perFrameClasSizesBuffer->GetGPUVirtualAddress(),
                                          sizeof(UINT64) };
    batchedInst.IndirectArgumentArray = { obj.perFrameInstArgsBuffer->GetGPUVirtualAddress(),
                                          sizeof(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS) };

    D3D12_RTAS_OPERATION_DESC opDescInst = {};
    opDescInst.Inputs                = opInputsInst;
    opDescInst.pBatchedOperationData = &batchedInst;
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDescInst,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);

    D3D12_RESOURCE_BARRIER instUav[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameClasResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameClasSizesBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameClasAddressArray.Get()),
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameClasAddressArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
    };
    cmdList->ResourceBarrier(_countof(instUav), instUav);

    // Copy size UAV -> readback.
    D3D12_RESOURCE_BARRIER sizesToCopy =
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameClasSizesBuffer.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);
    cmdList->ResourceBarrier(1, &sizesToCopy);
    cmdList->CopyBufferRegion(obj.perFrameClasSizesReadback.Get(), 0,
                              obj.perFrameClasSizesBuffer.Get(), 0,
                              (UINT64)obj.clusterCount * sizeof(UINT64));
    D3D12_RESOURCE_BARRIER sizesBackToUav =
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameClasSizesBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &sizesBackToUav);

    // Restore perFrameVertexBuffer state so the next per-frame compute pass
    // finds it as UAV (matches what UpdateAnimatedObjectPerFrame expects).
    D3D12_RESOURCE_BARRIER backToUav =
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameVertexBuffer.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &backToUav);

    // Synchronous flush + wait so we can map the readback below.
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();
    auto allocator = m_deviceResources->GetCommandAllocator();
    ThrowIfFailed(allocator->Reset());
    ThrowIfFailed(cmdList->Reset(allocator, nullptr));

    // Map + sum + store.
    UINT64 sumActual = 0;
    void* mapped = nullptr;
    D3D12_RANGE rdRange = { 0, (size_t)obj.clusterCount * sizeof(UINT64) };
    ThrowIfFailed(obj.perFrameClasSizesReadback->Map(0, &rdRange, &mapped));
    const UINT64* sizes = reinterpret_cast<const UINT64*>(mapped);
    UINT64 minSz = UINT64_MAX, maxSz = 0;
    for (UINT c = 0; c < obj.clusterCount; ++c)
    {
        sumActual += sizes[c];
        minSz = std::min(minSz, sizes[c]);
        maxSz = std::max(maxSz, sizes[c]);
    }
    D3D12_RANGE noWrite = { 0, 0 };
    obj.perFrameClasSizesReadback->Unmap(0, &noWrite);

    m_overlayStats.animatedPerFrameClasActualBytes = sumActual;
    SampleLog::LogF(L"[animated CLAS measure] %u clusters: bytes min=%llu max=%llu mean=%llu total=%llu\n",
                    obj.clusterCount,
                    (unsigned long long)minSz, (unsigned long long)maxSz,
                    obj.clusterCount > 0 ? (unsigned long long)(sumActual / obj.clusterCount) : 0ull,
                    (unsigned long long)sumActual);
}


// =============================================================================
// Overlay UI methods moved to OverlayUI.cpp:
//   StashOverlayStatsAsPrev, RefreshOverlayStatsCurrent, CaptureOverlayStatsSnapshot,
//   CreateUIFont, RenderUI
// =============================================================================




// =====================================================================================
// Per-frame work for the animated object. Called from DoRender BEFORE the TLAS
// rebuild + DispatchRays. Total work: 1 memcpy (positions) + 2 batched
// ExecuteIndirectRTASOperations calls + 2 UAV barriers.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::UpdateAnimatedObjectPerFrame(UINT pfTimestampBase)
{
    if (!m_animatedObjectEnabled) return;
    auto& obj = m_animatedObject;

    auto cl = m_deviceResources->GetCommandList();

    // ------------------------------------------------------------------
    // 1) GPU compute pass: AnimateBall.cs reads obj.restPositionsBuffer
    //    (SRV) and writes the deformed positions into
    //    obj.perFrameVertexBuffer (UAV).  The compute root signature is
    //    intentionally tiny (root constants + raw SRV + raw UAV) so we
    //    don't need to touch the descriptor heap here -- the raytracing
    //    holds (and Dispatch() doesn't read from it).
    //
    //    Per-frame CPU work is now ~5 D3D12 method calls: set RS+PSO,
    //    push 2 dwords, 2 SetComputeRoot*View, 1 Dispatch, 1 barrier.
    // ------------------------------------------------------------------
    cl->SetComputeRootSignature(m_animComputeRS.Get());
    cl->SetPipelineState(m_animComputePSO.Get());
    // CBV (mapped) -- see comment at the other dispatch site above.
    cl->SetComputeRootConstantBufferView (0, m_animParamsBuf->GetGPUVirtualAddress());
    cl->SetComputeRootShaderResourceView (1, obj.restPositionsBuffer ->GetGPUVirtualAddress());
    cl->SetComputeRootUnorderedAccessView(2, obj.perFrameVertexBuffer->GetGPUVirtualAddress());
    const UINT kThreadsPerGroup = 64;
    const UINT groups = (obj.totalVertexCount + kThreadsPerGroup - 1) / kThreadsPerGroup;
    cl->Dispatch(groups, 1, 1);

    // DispatchRays and the preceding TLAS/BLAS/CLAS builds can still be in
    // flight when this command list reaches the next frame. A legacy NULL UAV
    // barrier is global: it orders every prior acceleration-structure read/write
    // and scratch UAV access before all overwrites below. The address array has
    // a real resource-state change as well because INSTANTIATE writes it.
    {
        D3D12_RESOURCE_BARRIER reuseBarriers[] = {
            CD3DX12_RESOURCE_BARRIER::UAV(nullptr),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameClasAddressArray.Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        };
        cl->ResourceBarrier(_countof(reuseBarriers), reuseBarriers);
    }

    // UAV barrier + transition to NON_PIXEL_SHADER_RESOURCE so INSTANTIATE
    // sees the just-written positions.  After INSTANTIATE we transition back
    // to UAV for the next frame's compute pass.
    {
        D3D12_RESOURCE_BARRIER toRead[2] = {
            CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameVertexBuffer.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameVertexBuffer.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        };
        cl->ResourceBarrier(_countof(toRead), toRead);
    }
    // ------------------------------------------------------------------
    // 2) INSTANTIATE_CLUSTER_TEMPLATES - one batched op, N args. The args
    //    were pre-filled at setup time (ClusterTemplate + VertexBuffer slice
    //    per cluster); only their *contents* (the positions) change per frame.
    // ------------------------------------------------------------------
    D3D12_RTAS_CLUSTER_LIMITS limits = {};
    limits.MaxArgCount                                   = obj.clusterCount;
    limits.MaxUniqueGeometryIndexAndFlagsCountPerCluster = 1;
    limits.MaxTriangleCountPerCluster                    = obj.maxTrisPerCluster;
    limits.MaxVertexCountPerCluster                      = obj.maxVertsPerCluster;
    limits.MaxTotalTriangleCount                         = obj.mesh.totalTriangles;
    limits.MaxTotalVertexCount                           = obj.totalVertexCount;

    D3D12_RTAS_INSTANTIATE_CLUSTER_TEMPLATE_INPUTS_DESC instDesc = {};
    instDesc.ClusterLimits      = limits;
    instDesc.Flags              = BuildFlagModeRtas();
    instDesc.Mode               = D3D12_RTAS_OPERATION_MODE_IMPLICIT_DESTINATIONS;
    instDesc.VertexSourceFormat = D3D12_VERTEX_FORMAT_FLOAT32_3;

    D3D12_RTAS_OPERATION_INPUTS opInputsInst = {};
    opInputsInst.Type                            = D3D12_RTAS_OPERATION_TYPE_INSTANTIATE_CLUSTER_TEMPLATES;
    opInputsInst.pInstantiateClusterTemplateDesc = &instDesc;

    D3D12_RTAS_BATCHED_OPERATION_DATA batchedInst = {};
    batchedInst.BatchResultData       = obj.perFrameClasResultBuffer->GetGPUVirtualAddress();
    batchedInst.BatchScratchData      = obj.perFrameClasScratchBuffer->GetGPUVirtualAddress();
    batchedInst.ResultAddressArray    = { obj.perFrameClasAddressArray->GetGPUVirtualAddress(),
                                          sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batchedInst.IndirectArgumentArray = { obj.perFrameInstArgsBuffer->GetGPUVirtualAddress(),
                                          sizeof(D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS) };

    D3D12_RTAS_OPERATION_DESC opDescInst = {};
    opDescInst.Inputs                = opInputsInst;
    opDescInst.pBatchedOperationData = &batchedInst;
    // Bracket JUST the INSTANTIATE op with a timestamp pair so the overlay
    // can attribute time changes to vertex format / precision (which only
    // affect INSTANTIATE, not BLAS-from-CLAS).  The barriers immediately
    // after are intentionally OUTSIDE the bracket -- those are zero-cost
    // pipeline ordering edges, not work.
    auto cl4_ts = m_dxrCommandList.Get();
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 0);
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDescInst,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 1);

    D3D12_RESOURCE_BARRIER instBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameClasResultBuffer.Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(obj.perFrameClasAddressArray.Get()),
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameClasAddressArray.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        // INSTANTIATE consumed perFrameVertexBuffer (NON_PIXEL_SHADER_RESOURCE);
        // flip back to UAV for next frame's compute pass.  Doing this RIGHT
        // after the INSTANTIATE op keeps the buffer ready for the next
        // dispatch without an extra mid-frame transition.
        CD3DX12_RESOURCE_BARRIER::Transition(obj.perFrameVertexBuffer.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
    };
    m_dxrCommandList->ResourceBarrier(_countof(instBarriers), instBarriers);

    // ------------------------------------------------------------------
    // 3) BUILD_BLAS_FROM_CLAS - per-frame BLAS rebuild.
    //    Phase-2 anim clones extend this to produce 1 + N_animClones
    //    BLASes in the same batched call (source + every clone reads
    // ------------------------------------------------------------------
    // 3) BUILD_BLAS_FROM_CLAS - per-frame source BLAS rebuild
    //    (+ optional per-clone BLASes when m_animClonesBlasPool exists,
    //    i.e. phase-2 anim clones enabled).  M = 1 + N_anim_clones in
    //    that case; M = 1 otherwise.  All args share source's
    //    perFrameClasAddressArray (the SAME animated CLAS).
    // ------------------------------------------------------------------
    D3D12_RTAS_CLAS_INPUTS_DESC blasDesc = {};
    blasDesc.Flags              = BuildFlagModeRtas();
    const UINT M_blasArgs = m_animClonesBlasPool
        ? (1u + (UINT)m_animatedClones.size())
        : 1u;
    blasDesc.MaxArgCount        = M_blasArgs;
    blasDesc.Mode               = D3D12_RTAS_OPERATION_MODE_EXPLICIT_DESTINATIONS;
    blasDesc.MaxTotalClasCount  = obj.clusterCount * M_blasArgs;
    blasDesc.MaxClasCountPerArg = obj.clusterCount;

    D3D12_RTAS_OPERATION_INPUTS opInputsBlas = {};
    opInputsBlas.Type      = D3D12_RTAS_OPERATION_TYPE_BUILD_BLAS_FROM_CLAS;
    opInputsBlas.pClasDesc = &blasDesc;

    D3D12_RTAS_BATCHED_OPERATION_DATA batchedBlas = {};
    // Phase 2 ON: read from the clone-pool args/dests/scratch buffers
    // (M = 1 source + N clones, all built in this one batched op).
    // Phase 2 OFF: original 1-entry path that built only source.
    const bool phase2 = (m_animClonesBlasPool != nullptr);
    batchedBlas.BatchScratchData      = (phase2 ? m_animClonesBlasScratchBuffer : obj.blasScratchBuffer)->GetGPUVirtualAddress();
    batchedBlas.ResultAddressArray    = { (phase2 ? m_animClonesBlasResultAddrBuffer : obj.blasResultAddrBuffer)->GetGPUVirtualAddress(),
                                          sizeof(D3D12_GPU_VIRTUAL_ADDRESS) };
    batchedBlas.IndirectArgumentArray = { (phase2 ? m_animClonesBlasArgsBuffer : obj.blasArgsBuffer)->GetGPUVirtualAddress(),
                                          sizeof(D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS) };

    D3D12_RTAS_OPERATION_DESC opDescBlas = {};
    opDescBlas.Inputs                = opInputsBlas;
    opDescBlas.pBatchedOperationData = &batchedBlas;
    // that shouldn't move much when the precision slider changes.
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 2);
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &opDescBlas,
        D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    if (pfTimestampBase != UINT_MAX)
        cl4_ts->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, pfTimestampBase + 3);

    auto blasBarrier = CD3DX12_RESOURCE_BARRIER::UAV(obj.blasStorage.Get());
    // Two barriers when phase-2 clone pool exists: one on source's
    // blasStorage (own committed resource) + one on the pool (separate
    // committed resource).  Single barrier suffices in phase-1
    // (no clone pool) -- m_animClonesBlasPool is null then.
    D3D12_RESOURCE_BARRIER blasBarriers[2] = { blasBarrier, blasBarrier };
    UINT nBlasBarriers = 1;
    if (m_animClonesBlasPool) {
        blasBarriers[1] = CD3DX12_RESOURCE_BARRIER::UAV(m_animClonesBlasPool.Get());
        nBlasBarriers = 2;
    }
    m_dxrCommandList->ResourceBarrier(nBlasBarriers, blasBarriers);
}

void D3D12RaytracingClusteredGeometry::BuildTlasClassic()
{
    auto device = m_deviceResources->GetD3DDevice();
    const UINT N_static    = (UINT)m_objects.size();
    const UINT N_anim      = m_animatedObjectEnabled ? 1u : 0u;
    const UINT N_animClone = (UINT)m_animatedClones.size();
    const UINT N_total     = N_static + N_anim + N_animClone;

    // Build the full instance-desc array. The animated object's BLAS lives at
    // a fixed GVA (EXPLICIT_DESTINATIONS), so this array is good for the life
    // of the sample - only the TLAS BVH itself needs per-frame rebuild to
    // pick up the animated BLAS's new root bounds.
    // Per-instance flags + hit-group pick.
    //
    // FORCE_OPAQUE: set when neither baseline material nor any per-cluster
    // checker override introduces translucency or refractivity - then the
    // any-hit dispatch is skipped at traversal time.
    //
    // HIT GROUP: opaque-only instances use OpaqueHitGroup (no any-hit
    // shader bound at all - cheapest possible closesthit path).
    // Anything that may refract or translucently-reject goes to GlassHit-
    // Group (closesthit handles full Fresnel + refraction; any-hit handles
    // stochastic translucency).
    //
    // The two decisions share the same predicate ("does this instance
    // EVER need refraction or translucency?") - if no, OpaqueHitGroup +
    // FORCE_OPAQUE; if yes, GlassHitGroup + NONE.
    auto needsGlass = [](const MaterialDesc& mat, const ClusterObject& obj) -> bool
    {
        if (mat.refractivity > 0.0f || mat.translucency > 0.0f)
            return true;
        if (obj.checker.enabled)
        {
            // Per-cluster checker override may introduce refractivity on
            // a subset of clusters (e.g. sphere2: matte baseline + glass
            // odd-parity tiles).  -1 sentinel = no override (keep baseline).
            if (obj.checker.evenParity.overrideRefr > 0.0f) return true;
            if (obj.checker.oddParity.overrideRefr  > 0.0f) return true;
        }
        return false;
    };
    // Matches the hit-group shader table layout in
    // CreateRaytracingPipelineAndShaderTables: opaque block at offset 0,
    // glass block at offset 2, mixed-glass (unified GlassHit for both
    // regions) block at offset 4 (2 records per block: primary + shadow,
    // mixed has TWO 2-record blocks back-to-back for region 0 & 1).
    constexpr UINT kHitGroupContribOpaque     = 0;
    constexpr UINT kHitGroupContribGlass      = 2;
    constexpr UINT kHitGroupContribMixedGlass = 4;

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances(N_total);
    UINT nOpaque = 0, nGlass = 0, nMixed = 0;
    for (UINT i = 0; i < N_static; ++i)
    {
        const auto& obj = m_objects[i];
        XMMATRIX rot = XMMatrixRotationRollPitchYaw(obj.worldRotEuler.x,
                                                    obj.worldRotEuler.y,
                                                    obj.worldRotEuler.z);
        XMMATRIX m = XMMatrixScaling(obj.worldScale, obj.worldScale, obj.worldScale)
                   * rot
                   * XMMatrixTranslation(obj.worldPos.x, obj.worldPos.y, obj.worldPos.z);
        XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instances[i].Transform), m);
        instances[i].InstanceID  = obj.instanceID;
        instances[i].InstanceMask= 0xFF;
        // Hit-group contribution.  Three cases:
        //   * SINGLE-region opaque-only        -> kHitGroupContribOpaque
        //   * SINGLE-region glass OR ALL-glass -> kHitGroupContribGlass
        //   * MULTI-region MIXED (some regions opaque, some glass)
        //                                      -> kHitGroupContribMixedGlass
        //
        // The "mixed" routing binds GlassHit to BOTH region hit groups.
        // GlassHit's Fresnel-Schlick degenerates cleanly on chrome
        // material (refr=0 -> Tw=0 -> no refraction trace), so the
        // chrome region still looks chrome and the glass region gets
        // full refraction.  Shader-table block selection is per TLAS instance,
        // so mixed-region objects use this unified GlassHit block even though
        // their material lookup itself uses the per-geometry index.
        UINT firstRegionMatSlot = obj.perRegionMaterialSlot.empty()
            ? obj.instanceID
            : obj.perRegionMaterialSlot[0];
        const bool firstIsGlass = needsGlass(m_materials[firstRegionMatSlot], obj);
        // Count glass regions to detect MIXED (some glass, some opaque).
        UINT glassRegionCount = firstIsGlass ? 1u : 0u;
        for (UINT r = 1; r < (UINT)obj.perRegionMaterialSlot.size(); ++r)
        {
            if (needsGlass(m_materials[obj.perRegionMaterialSlot[r]], obj))
                ++glassRegionCount;
        }
        const UINT totalRegions = std::max<UINT>(1u, (UINT)obj.perRegionMaterialSlot.size());
        const bool isMixed       = (glassRegionCount > 0 && glassRegionCount < totalRegions);
        const bool anyRegionIsGlass = (glassRegionCount > 0);

        if (isMixed)
            instances[i].InstanceContributionToHitGroupIndex = kHitGroupContribMixedGlass;
        else if (anyRegionIsGlass)
            instances[i].InstanceContributionToHitGroupIndex = kHitGroupContribGlass;
        else
            instances[i].InstanceContributionToHitGroupIndex = kHitGroupContribOpaque;

        instances[i].Flags = anyRegionIsGlass ? D3D12_RAYTRACING_INSTANCE_FLAG_NONE
                                              : D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
        const bool isGlass = anyRegionIsGlass;  // for stats below
        // Non-orientable / self-intersecting surfaces need double-sided
        // traversal: their triangle winding doesn't have a consistent
        // global "outward" (Klein bottle's classic problem), and culling
        // any "back-facing" triangles leaves visible holes along the
        // orientation seam.  This per-instance flag overrides the ray's
        // RAY_FLAG_CULL_BACK_FACING_TRIANGLES so only THIS instance pays
        // the cost; orientable meshes (sphere, torus, cube, slab) keep
        // their fast single-sided traversal.
        if (m_objects[i].nonOrientable)
            instances[i].Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
        // Per-mode BLAS GVA: cluster path uses obj.blasGPUVA (built via
        // BUILD_BLAS_FROM_CLAS); traditional path uses obj.tradBlasGPUVA
        // (built via classic BuildRaytracingAccelerationStructure).
        instances[i].AccelerationStructure =
            (m_geometryMode == GeometryMode::Clusters) ? obj.blasGPUVA : obj.tradBlasGPUVA;
        if (isMixed)        ++nMixed;
        else if (isGlass)   ++nGlass;
        else                ++nOpaque;
    }
    // Animated ball is included in both modes -- the cluster path
    // references obj.blasGPUVA (BLAS-from-CLAS) while the traditional
    // path references obj.tradBlasGPUVA (DXR1 BLAS rebuilt/refitted
    // per frame from perFrameVertexBuffer + tradIndexBuffer).
    const bool hasAnimInst = m_animatedObjectEnabled;
    if (hasAnimInst)
    {
        const auto& a = m_animatedObject;
        XMMATRIX m = XMMatrixScaling(a.worldScale, a.worldScale, a.worldScale)
                   * XMMatrixTranslation(a.worldPos.x, a.worldPos.y, a.worldPos.z);
        XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instances[N_static].Transform), m);
        instances[N_static].InstanceID  = a.instanceID;
        instances[N_static].InstanceMask= 0xFF;
        // Animated object has no ClusterObject scene config yet; treat by
        // material only (currently refractive glass -> GlassHitGroup).
        const auto& aMat = m_materials[a.instanceID];
        const bool isGlass = (aMat.refractivity > 0.0f) || (aMat.translucency > 0.0f);
        instances[N_static].InstanceContributionToHitGroupIndex =
            isGlass ? kHitGroupContribGlass : kHitGroupContribOpaque;
        instances[N_static].Flags = isGlass ? D3D12_RAYTRACING_INSTANCE_FLAG_NONE
                                            : D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
        // Per-mode BLAS GVA for the animated instance.
        instances[N_static].AccelerationStructure =
            (m_geometryMode == GeometryMode::Clusters) ? a.blasGPUVA : a.tradBlasGPUVA;
        (isGlass ? nGlass : nOpaque)++;
    }
    // [N] animated clones: each is a TLAS instance pointing at the
    // SAME shared animated BLAS GVA as the source instance above.
    // Per-frame work is therefore O(1) regardless of clone count
    // (phase 1; phase 2 will switch to per-clone CLAS+BLAS).
    if (N_animClone > 0)
    {
        const auto& a = m_animatedObject;
        const auto& aMat = m_materials[a.instanceID];
        const bool isGlass = (aMat.refractivity > 0.0f) || (aMat.translucency > 0.0f);
        const D3D12_GPU_VIRTUAL_ADDRESS animBlasGVA =
            (m_geometryMode == GeometryMode::Clusters) ? a.blasGPUVA : a.tradBlasGPUVA;
        const UINT contrib = isGlass ? kHitGroupContribGlass : kHitGroupContribOpaque;
        const D3D12_RAYTRACING_INSTANCE_FLAGS animFlags = isGlass
            ? D3D12_RAYTRACING_INSTANCE_FLAG_NONE
            : D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
        for (UINT k = 0; k < N_animClone; ++k)
        {
            const auto& ac = m_animatedClones[k];
            const UINT row = N_static + N_anim + k;
            XMMATRIX rot = XMMatrixRotationRollPitchYaw(ac.worldRotEuler.x,
                                                        ac.worldRotEuler.y,
                                                        ac.worldRotEuler.z);
            XMMATRIX m = XMMatrixScaling(ac.worldScale, ac.worldScale, ac.worldScale)
                       * rot
                       * XMMatrixTranslation(ac.worldPos.x, ac.worldPos.y, ac.worldPos.z);
            XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instances[row].Transform), m);
            // CRITICAL: InstanceID must be a VALID material slot (0..8),
            // not the sentinel 0xFFFFFFFFu material-override value.
            // GlassAnyHit reads g_materials[InstanceID()] -- a sentinel
            // InstanceID truncates to 0xFFFFFF (24-bit field) and OOBs
            // the 9-entry materials buffer, page-faulting the GPU and
            // triggering DXGI_ERROR_DEVICE_HUNG (TDR).  Use the source
            // animated ball's instanceID (= 7, animated glass material)
            // so any-hit reads a valid slot.
            //
            // The per-instance material OVERRIDE mechanism still works
            // via g_instanceMatOverride[InstanceIndex()] which carries
            // ac.materialOverrideSlot (separate from InstanceID).  When
            // it's sentinel 0xFFFFFFFFu the shader's closesthit no-ops
            // the override and falls back to ctx.meta.materialSlot from
            // the per-cluster table -- i.e., the source's chrome+
            // checker look applies naturally.
            instances[row].InstanceID  = a.instanceID;
            instances[row].InstanceMask = 0xFF;
            // Full glass material -- now safe because any-hit reads a
            // valid InstanceID.  Per-cluster overrides give clones the
            // source's chrome+checker look exactly.
            instances[row].InstanceContributionToHitGroupIndex = contrib;
            instances[row].Flags = animFlags;
            // Phase 2: each clone may have its own per-frame-rebuilt BLAS
            // in m_animClonesBlasPool (cluster mode) or
            // m_animClonesTradBlasPool (trad mode, capped at
            // kTradPerClonePoolMax clones via BuildAnimatedClonesTradSetup).
            // Fall back to source's animated BLAS GVA when this clone
            // didn't get its own pool slot (ac.blasGPUVA == 0).
            instances[row].AccelerationStructure =
                (ac.blasGPUVA != 0) ? ac.blasGPUVA : animBlasGVA;
            (isGlass ? nGlass : nOpaque)++;
        }
    }
    AllocateUploadBuffer(device, instances.data(),
                         instances.size() * sizeof(instances[0]),
                         &m_tlasInstanceDescs, L"TLAS instance descs");
    {
        std::vector<UINT> overrides(N_total, 0xFFFFFFFFu);
        for (UINT i = m_sourceObjectCount; i < N_static; ++i)
        {
            // For static clones the override slot lives in instanceID.
            overrides[i] = m_objects[i].instanceID;
        }
        // Animated clones come AFTER the source animated row.
        for (UINT k = 0; k < N_animClone; ++k)
        {
            overrides[N_static + N_anim + k] = m_animatedClones[k].materialOverrideSlot;
        }
        AllocateUploadBuffer(device, overrides.data(),
                             overrides.size() * sizeof(UINT),
                             &m_instanceMaterialOverrideBuffer,
                             L"Per-instance material overrides");
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
    tlasInputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.NumDescs       = N_total;
    tlasInputs.Flags          = BuildFlagModeDxr1();
    tlasInputs.InstanceDescs  = m_tlasInstanceDescs->GetGPUVirtualAddress();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &prebuild);
    SampleLog::LogF(L"[TLAS prebuild] %u instances (%u static + %u animated + %u animClone): result=%llu, scratch=%llu bytes\n",
                    N_total, N_static, N_anim, N_animClone,
                    (unsigned long long)prebuild.ResultDataMaxSizeInBytes,
                    (unsigned long long)prebuild.ScratchDataSizeInBytes);

    AllocateUAVBuffer(device, prebuild.ResultDataMaxSizeInBytes, &m_tlasBuffer,
                      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TLAS storage");
    AllocateUAVBuffer(device, std::max<UINT64>(prebuild.ScratchDataSizeInBytes, 256ull),
                      &m_tlasScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"TLAS scratch");

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs                             = tlasInputs;
    buildDesc.DestAccelerationStructureData      = m_tlasBuffer->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData   = m_tlasScratchBuffer->GetGPUVirtualAddress();
    m_dxrCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_tlasBuffer.Get());
    m_dxrCommandList->ResourceBarrier(1, &barrier);
}

// ---------------------------------------------------------------------------------
// TLAS rebuild for per-frame use. Same inputs (instance descs buffer) as the
// initial build - only the TLAS BVH itself needs refreshing because the
// animated BLAS contents have changed (its GVA is stable). Buffers are reused;
// nothing reallocated. Single batched command + UAV barrier.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::RebuildTlasPerFrame()
{
    if (!m_tlasBuffer || !m_tlasScratchBuffer || !m_tlasInstanceDescs) return;
    const UINT N_total = (UINT)m_objects.size()
        + (m_animatedObjectEnabled ? 1u : 0u)
        + (UINT)m_animatedClones.size();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
    tlasInputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.NumDescs       = N_total;
    tlasInputs.Flags          = BuildFlagModeDxr1();
    tlasInputs.InstanceDescs  = m_tlasInstanceDescs->GetGPUVirtualAddress();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs                             = tlasInputs;
    buildDesc.DestAccelerationStructureData      = m_tlasBuffer->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData   = m_tlasScratchBuffer->GetGPUVirtualAddress();
    // Order the preceding DispatchRays/TLAS read and scratch use before the
    // in-place TLAS rebuild. A NULL UAV barrier covers both AS and scratch.
    auto preOverwrite = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    m_dxrCommandList->ResourceBarrier(1, &preOverwrite);
    m_dxrCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_tlasBuffer.Get());
    m_dxrCommandList->ResourceBarrier(1, &barrier);
}

void D3D12RaytracingClusteredGeometry::DumpClusterStatsAsync()
{
    auto device       = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    const UINT N = m_totalClusterCount;

    auto rbHeap  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)N * sizeof(UINT64));
    ComPtr<ID3D12Resource> readback;
    ThrowIfFailed(device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &bufDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)));

    ComPtr<ID3D12CommandAllocator> alloc;
    ComPtr<ID3D12GraphicsCommandList> cl;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc)));
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.Get(), nullptr, IID_PPV_ARGS(&cl)));
    auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    cl->ResourceBarrier(1, &toCopy);
    cl->CopyBufferRegion(readback.Get(), 0, m_clasSizeArray.Get(), 0, (UINT64)N * sizeof(UINT64));
    auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_clasSizeArray.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cl->ResourceBarrier(1, &fromCopy);
    ThrowIfFailed(cl->Close());
    ID3D12CommandList* cls[] = { cl.Get() };
    commandQueue->ExecuteCommandLists(1, cls);
    m_deviceResources->WaitForGpu();

    void* p = nullptr;
    D3D12_RANGE rng = { 0, (SIZE_T)N * sizeof(UINT64) };
    ThrowIfFailed(readback->Map(0, &rng, &p));
    const UINT64* sizes = reinterpret_cast<const UINT64*>(p);
    UINT64 total = 0, mn = UINT64_MAX, mx = 0;
    for (UINT i = 0; i < N; ++i) { total += sizes[i]; mn = std::min(mn, sizes[i]); mx = std::max(mx, sizes[i]); }
    D3D12_RANGE noWrite = { 0, 0 };
    readback->Unmap(0, &noWrite);

    SampleLog::LogF(L"[CLAS stats] %u clusters: bytes min=%llu max=%llu mean=%llu total=%llu\n",
                    N, (unsigned long long)mn, (unsigned long long)mx,
                    (unsigned long long)(total / N), (unsigned long long)total);
}

// ---------------------------------------------------------------------------------
// Read back the AS build timestamps from m_buildQueryReadback and compute
// per-operation wall-clock times. Called once after init (post-WaitForGpu).
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::ReadBuildTimestamps()
{
    // Record CLAS storage total for the title bar.
    m_totalClasBytes = 0;
    if (m_clasResultBuffer) m_totalClasBytes = m_clasResultBuffer->GetDesc().Width;

    // Compute total BLAS allocation size.  Cluster path: pooled into
    // m_clusterBlasPoolBuffer.  Traditional static path: pooled into
    // m_tradBlasWorstcasePool (implicit alloc mode) OR
    // m_tradBlasCompactPool (compact alloc mode; worst-case pool is
    // dropped at end of BuildTraditionalStaticAS).  Animated trad
    // path is still per-object on m_animatedObject.tradBlasStorage.
    m_totalBlasBytes = 0;
    if (m_clusterBlasPoolBuffer)
        m_totalBlasBytes += m_clusterBlasPoolBuffer->GetDesc().Width;
    if (m_tradBlasWorstcasePool)
        m_totalBlasBytes += m_tradBlasWorstcasePool->GetDesc().Width;
    if (m_tradBlasCompactPool)
        m_totalBlasBytes += m_tradBlasCompactPool->GetDesc().Width;

    UINT64* ts = nullptr;
    D3D12_RANGE r = { 0, sizeof(UINT64) * kBuildTimestampCount };
    ThrowIfFailed(m_buildQueryReadback->Map(0, &r, reinterpret_cast<void**>(&ts)));
    const double inv = 1000.0 / (double)m_timestampFrequency;     // ticks -> ms
    m_clasBuildMs  = (ts[1] - ts[0]) * inv;
    m_blasBuildMs  = (ts[3] - ts[2]) * inv;
    m_tlasBuildMs  = (ts[5] - ts[4]) * inv;
    m_totalBuildMs = (ts[5] - ts[0]) * inv;
    D3D12_RANGE noWrite = { 0, 0 };
    m_buildQueryReadback->Unmap(0, &noWrite);

    SampleLog::LogF(L"[build wall-clock] CLAS=%.3f ms, BLAS=%.3f ms, TLAS=%.3f ms (total %.3f ms)\n",
                    m_clasBuildMs, m_blasBuildMs, m_tlasBuildMs, m_totalBuildMs);
    SampleLog::LogF(L"[storage] CLAS: %llu bytes  BLAS: %llu bytes\n",
                    (unsigned long long)m_totalClasBytes,
                    (unsigned long long)m_totalBlasBytes);

    // Per-mode CLAS memory stats. Use [CLAS mem] tag so the table is grep-able.
    // Notes:
    //  - sumActualBytes is the irreducible CLAS storage (= sum of per-cluster
    //    sizes the driver picked). It's the same for all three modes; the
    //    modes differ only in how much we *reserved* on top of that.
    //  - resultPrebuildMax is the worst-case alloc the driver would ask for
    //    in IMPLICIT mode (used as the reference for "wasted" bytes).
    //  - peakResidentBytes is the maximum live result-buffer memory at any
    //    instant during the build flow.
    const auto& s = m_clasMemStats;
    const double overheadVsActual = s.sumActualBytes
        ? 100.0 * (double)((double)s.resultFinalBytes - (double)s.sumActualBytes) / (double)s.sumActualBytes
        : 0.0;
    const double savingsVsImplicit = s.resultPrebuildMax
        ? 100.0 * (double)((double)s.resultPrebuildMax - (double)s.resultFinalBytes) / (double)s.resultPrebuildMax
        : 0.0;
    SampleLog::LogF(
        L"[CLAS mem mode=%s] prebuild_max=%llu  initial=%llu  final=%llu  peak=%llu  "
        L"sum_actual=%llu  scratch=(%llu, %llu)  cpu_phase_ms=(%.3f, %.3f)  "
        L"final_overhead_vs_actual=%+.1f%%  savings_vs_implicit=%+.1f%%\n",
        ClasAllocModeName(),
        (unsigned long long)s.resultPrebuildMax,
        (unsigned long long)s.resultInitialBytes,
        (unsigned long long)s.resultFinalBytes,
        (unsigned long long)s.peakResidentBytes,
        (unsigned long long)s.sumActualBytes,
        (unsigned long long)s.scratchBytesPhase1,
        (unsigned long long)s.scratchBytesPhase2,
        s.cpuWallMsPhase1, s.cpuWallMsPhase2,
        overheadVsActual, savingsVsImplicit);

    // Discoverability hint: if the user is on the simplest path (implicit)
    // AND the over-allocation is actually painful (>2x), point them at the
    // alternative modes. Most users won't read all of d3d12.h, so the sample
    // itself should help connect "you allocated 1.2 MB" to "you only used
    // 220 KB; here's how to fix it".
    if (m_clasAllocMode == ClasAllocMode::Implicit
        && s.sumActualBytes > 0
        && s.resultFinalBytes > s.sumActualBytes * 2)
    {
        const double ratio = (double)s.resultFinalBytes / (double)s.sumActualBytes;
        SampleLog::LogF(
            L"[CLAS mem hint] implicit mode allocated %.1fx the bytes actually "
            L"used (%llu / %llu). Re-run with --clas-alloc get-sizes for an "
            L"exact-fit alloc, or --clas-alloc compact for build-then-compact "
            L"semantics. See readme.md for the tradeoff table.\n",
            ratio,
            (unsigned long long)s.resultFinalBytes,
            (unsigned long long)s.sumActualBytes);
    }
}

// Title bar intentionally left as the default static "<app name>" set at
// window-creation time.  All per-frame stats live in the on-screen overlay
// (RenderUI()) which has no width limit and supports colour-coded keybinds.
// =====================================================================================
// Raytracing pipeline + shader tables (unchanged from milestone 2).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::CreateRaytracingPipelineAndShaderTables()
{
    auto device = m_deviceResources->GetD3DDevice();
    {
        CD3DX12_DESCRIPTOR_RANGE uavRange;
        uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        CD3DX12_ROOT_PARAMETER params[GlobalRootSig::Count];
        params[GlobalRootSig::OutputUAVSlot].InitAsDescriptorTable(1, &uavRange);
        params[GlobalRootSig::AccelerationStructureSlot].InitAsShaderResourceView(0);
        params[GlobalRootSig::SceneCBVSlot].InitAsConstantBufferView(0);
        // Per-instance materials live in a structured buffer at t1, indexed by
        // InstanceID() in the closesthit/anyhit shaders.
        params[GlobalRootSig::MaterialsSRVSlot].InitAsShaderResourceView(1);
        // Per-cluster shader-side buffers for smooth normals.  DXR2 cluster
        // geometry's CLAS only takes positions in its vertex buffer, so per-
        // vertex normals (and the index buffer, so we can find which 3
        // vertices a triangle hit references) travel through 3 separate
        // structured buffers indexed by ClusterID() + PrimitiveIndex().
        params[GlobalRootSig::ClusterNormalsSRVSlot].InitAsShaderResourceView(2);
        params[GlobalRootSig::ClusterIndicesSRVSlot].InitAsShaderResourceView(3);
        params[GlobalRootSig::ClusterOffsetsSRVSlot].InitAsShaderResourceView(4);
        // Per-cluster GENERIC metadata buffer.  Replaces ALL hardcoded
        // per-instance / per-cid-range branches in the closesthit.  See
        // RaytracingHlslCompat.h ClusterMeta.
        params[GlobalRootSig::ClusterMetaSRVSlot].InitAsShaderResourceView(5);
        // Tiny per-instance first-cluster-ID lookup (uint per TLAS
        // instance) -- read only by the traditional-BLAS path so it can
        // compute the same cid the cluster path's ClusterID() returns.
        params[GlobalRootSig::TradTriToCidSRVSlot].InitAsShaderResourceView(6);
        params[GlobalRootSig::TradGeomTriBaseSRVSlot].InitAsShaderResourceView(7);
        params[GlobalRootSig::PerInstGeomMaterialSRVSlot].InitAsShaderResourceView(8);
        // Per-instance material override (one UINT per TLAS instance);
        // see InstanceMaterialOverrideSRVSlot in the header for the
        // sentinel semantics.
        params[GlobalRootSig::InstanceMaterialOverrideSRVSlot].InitAsShaderResourceView(9);
        CD3DX12_ROOT_SIGNATURE_DESC desc(_countof(params), params);
        ComPtr<ID3DBlob> blob, err;
        ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err),
            err ? (wchar_t*)err->GetBufferPointer() : L"D3D12SerializeRootSignature failed\n");
        ThrowIfFailed(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
            IID_PPV_ARGS(&m_globalRootSignature)));
        m_globalRootSignature->SetName(L"Global root signature");
    }

    // Optional empty local root signature. The HLK IndirectBuild test creates
    // one of these alongside the global RS - leaving it in here for parity
    // even though our shaders don't take per-shader records. Harmless on
    // adapters that don't need it.
    SampleLog::Write(L"  >>> Create empty local root signature\n");
    {
        CD3DX12_ROOT_SIGNATURE_DESC localDesc(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
        ComPtr<ID3DBlob> blob, err;
        ThrowIfFailed(D3D12SerializeRootSignature(&localDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err),
            err ? (wchar_t*)err->GetBufferPointer() : L"D3D12SerializeRootSignature(local) failed\n");
        ThrowIfFailed(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
            IID_PPV_ARGS(&m_localRootSignature)));
        m_localRootSignature->SetName(L"Empty local root signature");
    }

    CD3DX12_STATE_OBJECT_DESC pipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    pipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>()
        ->SetRootSignature(m_localRootSignature.Get());

    auto lib = pipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    lib->DefineExport(c_raygenName);
    lib->DefineExport(c_opaqueClosestHitName);
    lib->DefineExport(c_glassClosestHitName);
    lib->DefineExport(c_glassAnyHitName);
    lib->DefineExport(c_missName);
    lib->DefineExport(c_shadowMissName);

    // ---- OPAQUE primary hit group --------------------------------------
    // Bound to instances whose material chain (baseline + every per-cluster
    // checker override) keeps refractivity = 0 and translucency = 0.
    // Currently: only sphere0 (chrome).
    //
    // NO any-hit shader binding - any-hit dispatch is skipped entirely
    // by the runtime for traversals on this hit group.  Combined with the
    // FORCE_OPAQUE TLAS instance flag this is the cheapest possible
    // closesthit path: every triangle hit goes straight to OpaqueHit.
    auto opaqueHG = pipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    opaqueHG->SetClosestHitShaderImport(c_opaqueClosestHitName);
    opaqueHG->SetHitGroupExport(c_opaqueHitGroupName);
    opaqueHG->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // ---- GLASS primary hit group ---------------------------------------
    // Bound to any instance whose material chain includes refraction OR
    // translucency at the baseline OR via per-cluster checker overrides.
    // Currently: spheres 1/2/3, torus, cube, floor, animated, Klein.
    //
    // any-hit = GlassAnyHit (stochastic translucency reject).  Closest-
    // hit = GlassHit (full Fresnel + refraction + reflection composition).
    auto glassHG = pipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    glassHG->SetClosestHitShaderImport(c_glassClosestHitName);
    glassHG->SetAnyHitShaderImport(c_glassAnyHitName);
    glassHG->SetHitGroupExport(c_glassHitGroupName);
    glassHG->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shadow hit group: empty (no closest-hit, no any-hit). Shadow rays use
    // SKIP_CLOSEST_HIT_SHADER + FORCE_OPAQUE so neither shader can run on a
    // hit. The hit group still has to exist for ray-contribution-index 1.
    auto shadowHitGroup = pipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    shadowHitGroup->SetHitGroupExport(c_shadowHitGroupName);
    shadowHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Payload: max(Payload(float4), ShadowPayload(bool)) -> 16 bytes is enough.
    auto shaderConfig = pipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    shaderConfig->Config(/*payload*/ 4 * sizeof(float) + 2 * sizeof(uint), /*attribs*/ 2 * sizeof(float));   // Payload = float4 color + uint depth + uint inGlass
    auto globalRS = pipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRS->SetRootSignature(m_globalRootSignature.Get());

    // ALLOW_CLUSTERED_GEOMETRY is the DXR2 opt-in for the shader to traverse a
    // BLAS built from CLAS. Without it, hits on a Cluster BLAS are undefined.
    auto pipelineConfig = pipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT>();
    // MaxRecursionDepth = 8: budget for cascading-refraction paths now that
    // EVERY object in the scene is a glass variant.  A primary ray that
    // pierces several glass volumes in sequence can chain refractions to
    // depth 4-5 before bottoming out, plus 1 shadow ray per surface hit.
    // Worst-case path:
    //   raygen TraceRay     -> recursion 1 (primary closesthit on glass A)
    //   refract-in glass A   -> recursion 2 (back-face of A)
    //   refract-out into air -> recursion 3 (front-face of glass B)
    //   refract-in glass B   -> recursion 4 (back-face of B)
    //   refract-out          -> recursion 5 (next surface)
    //   reflection bounce    -> recursion 6 (mirror-reflect off some glass)
    //   shadow ray           -> recursion 7 (LEAF; SKIP_CLOSEST_HIT)
    //   safety               -> recursion 8 (unused headroom)
    // The closesthit gates refraction at myDepth <= 4 to stay well under
    // this budget while still letting 4+ glass volumes compose visually.
    pipelineConfig->Config(16, D3D12_RAYTRACING_PIPELINE_FLAG_ALLOW_CLUSTERED_GEOMETRY);

    SampleLog::Write(L"  >>> CreateStateObject\n");
    HRESULT hrCSO = m_dxrDevice->CreateStateObject(pipeline, IID_PPV_ARGS(&m_dxrStateObject));
    SampleLog::LogF(L"  CreateStateObject -> hr=0x%08X\n", (unsigned)hrCSO);
    ThrowIfFailed(hrCSO, L"CreateStateObject failed\n");
    m_dxrStateObject->SetName(L"RT pipeline (cluster-aware)");

    SampleLog::Write(L"  >>> Build shader tables\n");
    ComPtr<ID3D12StateObjectProperties> props;
    ThrowIfFailed(m_dxrStateObject->QueryInterface(IID_PPV_ARGS(&props)));
    void* rgID            = props->GetShaderIdentifier(c_raygenName);
    void* missID          = props->GetShaderIdentifier(c_missName);
    void* shadowMissID    = props->GetShaderIdentifier(c_shadowMissName);
    void* opaqueHgID      = props->GetShaderIdentifier(c_opaqueHitGroupName);
    void* glassHgID       = props->GetShaderIdentifier(c_glassHitGroupName);
    void* shadowHgID      = props->GetShaderIdentifier(c_shadowHitGroupName);
    const UINT idSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    const UINT recordSize = Align(idSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

    // Single-record table (raygen).
    auto makeTable1 = [&](void* shaderID, ComPtr<ID3D12Resource>& outTable, const wchar_t* name)
    {
        std::vector<uint8_t> data(recordSize, 0);
        memcpy(data.data(), shaderID, idSize);
        AllocateUploadBuffer(device, data.data(), data.size(), &outTable, name);
    };
    // Two-record table (miss).
    auto makeTable2 = [&](void* shaderID0, void* shaderID1,
                          ComPtr<ID3D12Resource>& outTable, const wchar_t* name)
    {
        std::vector<uint8_t> data(recordSize * 2, 0);
        memcpy(data.data() + 0,          shaderID0, idSize);
        memcpy(data.data() + recordSize, shaderID1, idSize);
        AllocateUploadBuffer(device, data.data(), data.size(), &outTable, name);
    };
    // ---- HIT-GROUP shader table layout ----
    // kHitGroupRecordCount records: one opaque block, then enough
    // glass-capable records for every legal contribution + GeometryIndex
    // combination.  Each block is two contiguous records (primary + shadow):
    //
    //   [0] OpaqueHitGroup    <-- primary for OPAQUE instances (InstanceContrib=0)
    //   [1] ShadowHitGroup    <-- shadow  for OPAQUE instances (RayContrib=1)
    //   [2] GlassHitGroup     <-- primary for GLASS  instances (InstanceContrib=2)
    //   [3] ShadowHitGroup    <-- shadow  for GLASS  instances (RayContrib=1)
    //   [4] GlassHitGroup     <-- primary for MIXED  instances, region 0
    //                             (InstanceContrib=4 with Multiplier=2,
    //                              gi=0 -> 4, gi=1 -> 6)
    //   [5] ShadowHitGroup    <-- shadow  for MIXED  instances, region 0
    //   [6] GlassHitGroup     <-- primary for MIXED  instances, region 1
    //   [7] ShadowHitGroup    <-- shadow  for MIXED  instances, region 1
    //
    // Per-instance InstanceContributionToHitGroupIndex picks the block;
    // shadow rays add RayContributionToHitGroupIndex=1 within the block.
    // The shadow records share the same dummy hit-group identifier but
    // physically live at distinct table indices so the same RayContrib=1
    // offset works from any block start.
    //
    // The "mixed" block exists so MULTI-REGION instances with mixed
    // material kinds (e.g. mixed sphere = chrome upper + glass lower)
    // route both regions to GlassHit unconditionally.  GlassHit's
    // Fresnel-Schlick degenerates cleanly on the chrome region (refr=0
    // -> Tw=0, no refraction trace) so chrome still looks chrome, and
    // the glass region gets full refraction.  The hit-group table block is
    // selected per TLAS instance; this conservative GlassHit block keeps one
    // routing shape for mixed regions while material selection still uses the
    // canonical GeometryIndex lookup.  Cost:
    // one extra Fresnel calculation per chrome hit, negligible in practice.
    constexpr UINT kHighestInstanceContribution = 4u;
    constexpr UINT kHitGroupRecordCount = kHighestInstanceContribution
        + 2u * kMaxGeomsPerInstance;
    auto makeHitGroupTable = [&](ComPtr<ID3D12Resource>& outTable, const wchar_t* name)
    {
        std::vector<uint8_t> data(recordSize * kHitGroupRecordCount, 0);
        memcpy(data.data() + 0 * recordSize, opaqueHgID, idSize);
        memcpy(data.data() + 1 * recordSize, shadowHgID, idSize);
        for (UINT record = 2; record < kHitGroupRecordCount; record += 2)
        {
            memcpy(data.data() + record * recordSize, glassHgID, idSize);
            memcpy(data.data() + (record + 1) * recordSize, shadowHgID, idSize);
        }
        AllocateUploadBuffer(device, data.data(), data.size(), &outTable, name);
    };

    makeTable1(rgID,                    m_rayGenShaderTable,   L"raygen shader table");
    makeTable2(missID, shadowMissID,    m_missShaderTable,     L"miss shader table (primary + shadow)");
    makeHitGroupTable(m_hitGroupShaderTable,
                      L"hit-group shader table (opaque + all legal glass geometry indices)");
}

// ---------------------------------------------------------------------------------
// Compute pipeline for the per-frame GPU ball animation pass.
//
// Root signature is intentionally tiny: 2 dwords of inline constants
// (time + vertex count), one raw SRV (rest positions), one raw UAV
// (animated positions).  All bindings are root-direct -- no descriptor
// heap involvement -- so this pass is self-contained and doesn't need
// to share descriptor slots with the raytracing path.
//
// PSO is built from the dxc-compiled cs_6_6 bytecode embedded via
// AnimateBall.hlsl.h.  Called once at init from CreateDeviceDependentResources.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::CreateAnimationComputePipeline()
{
    auto device = m_deviceResources->GetD3DDevice();

    CD3DX12_ROOT_PARAMETER rootParams[3] = {};
    rootParams[0].InitAsConstantBufferView(/*shaderReg*/0);            // b0 = mapped CBV { float t, uint vertexCount }; CB lets the time
                                                                       // value propagate to ALL in-flight frames immediately when CPU
                                                                       // writes it (unlike root constants which bake-in per frame --
                                                                       // that asymmetry made wobble visibly lag the camera for ~3
                                                                       // frames after pause, since the camera CB was already mapped
                                                                       // and the CS time wasn't).
    rootParams[1].InitAsShaderResourceView(0);                         // t0 = ByteAddressBuffer rest positions
    rootParams[2].InitAsUnorderedAccessView(0);                        // u0 = RWByteAddressBuffer anim positions

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init(_countof(rootParams), rootParams, 0, nullptr,
                D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             &serialized, &error);
    if (FAILED(hr))
    {
        if (error) SampleLog::LogF(L"[anim-cs] root sig serialize failed: %hs\n",
                                   (const char*)error->GetBufferPointer());
        ThrowIfFailed(hr);
    }
    ThrowIfFailed(device->CreateRootSignature(0, serialized->GetBufferPointer(),
                                              serialized->GetBufferSize(),
                                              IID_PPV_ARGS(&m_animComputeRS)));
    m_animComputeRS->SetName(L"Animate ball compute root sig");

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_animComputeRS.Get();
    psoDesc.CS             = CD3DX12_SHADER_BYTECODE((void*)g_pAnimateBall, ARRAYSIZE(g_pAnimateBall));
    ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_animComputePSO)));
    m_animComputePSO->SetName(L"Animate ball compute PSO");

    SampleLog::LogF(L"[anim-cs] compute PSO built (%zu bytes of CS bytecode)\n",
                    (size_t)ARRAYSIZE(g_pAnimateBall));

    // Allocate the persistent-mapped CB that AnimateBall.hlsl reads its
    // (time, vertexCount) inputs from.  256-byte aligned per D3D12 CBV
    // alignment requirement.
    {
        auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufDesc    = CD3DX12_RESOURCE_DESC::Buffer(256);
        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeap, D3D12_HEAP_FLAG_NONE, &bufDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_animParamsBuf)));
        m_animParamsBuf->SetName(L"Anim params CB (t + vertexCount)");
        CD3DX12_RANGE noRead(0, 0);
        ThrowIfFailed(m_animParamsBuf->Map(0, &noRead, reinterpret_cast<void**>(&m_animParamsMapped)));
        *m_animParamsMapped = { 0.0f, 0u, 0u, 0u };
    }
}

// =====================================================================
// Build the compute pipeline that GPU-fills the per-cluster
// INSTANTIATE_CLUSTER_TEMPLATES_ARGS array for the animated object.
// See FillInstantiateArgs.hlsl for the shader contract; this just wires
// the root signature parameters in the same slot order the shader
// expects:
//   b0 = root constants (5 dwords: pfVbLo, pfVbHi, clusterCount,
//                                  clusterIdOffset, vertexStride)
//   u1 = templateGvas   (root UAV;  shader reads only)
//   t0 = vertexOffsets  (root SRV)
//   u0 = argsOut        (root UAV)
//
// 4 root parameters total = 5 + 2 + 2 + 2 = 11 dwords, well under the
// 64-DWORD root-sig budget.
// =====================================================================
void D3D12RaytracingClusteredGeometry::CreateFillInstantiateArgsPipeline()
{
    auto device = m_deviceResources->GetD3DDevice();

    CD3DX12_ROOT_PARAMETER rootParams[4] = {};
    rootParams[0].InitAsConstants(5, /*shaderReg*/0);                  // b0
    rootParams[1].InitAsUnorderedAccessView(/*shaderReg*/1);           // u1 = templateGvas
    rootParams[2].InitAsShaderResourceView(/*shaderReg*/0);            // t0 = vertexOffsets
    rootParams[3].InitAsUnorderedAccessView(/*shaderReg*/0);           // u0 = argsOut

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init(_countof(rootParams), rootParams, 0, nullptr,
                D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             &serialized, &error);
    if (FAILED(hr))
    {
        if (error) SampleLog::LogF(L"[fill-args-cs] root sig serialize failed: %hs\n",
                                   (const char*)error->GetBufferPointer());
        ThrowIfFailed(hr);
    }
    ThrowIfFailed(device->CreateRootSignature(0, serialized->GetBufferPointer(),
                                              serialized->GetBufferSize(),
                                              IID_PPV_ARGS(&m_fillInstArgsRS)));
    m_fillInstArgsRS->SetName(L"FillInstantiateArgs root sig");

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_fillInstArgsRS.Get();
    psoDesc.CS             = CD3DX12_SHADER_BYTECODE((void*)g_pFillInstantiateArgs, ARRAYSIZE(g_pFillInstantiateArgs));
    ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_fillInstArgsPSO)));
    m_fillInstArgsPSO->SetName(L"FillInstantiateArgs compute PSO");

    SampleLog::LogF(L"[fill-args-cs] compute PSO built (%zu bytes of CS bytecode)\n",
                    (size_t)ARRAYSIZE(g_pFillInstantiateArgs));
}


// =====================================================================
// Tiny helper used by the four args-fill pipeline builders below.
// Each pipeline has exactly the same C++ shape: a root sig built from
// a small N-element rootParams array, no static samplers, no flags,
// + a single CS PSO referencing the supplied bytecode.  Factoring this
// out keeps each Create*Pipeline body focused on its actual schema.
//
// DESIGN NOTE -- separate CSes vs ubershader:
//   We dispatch one CS per args-buffer type (FillClasFromTrianglesArgs,
//   FillClusterTemplateArgs, FillBlasFromClasArgs, FillMoveClusterArgs,
//   FillInstantiateArgs).  This is the simplest, most readable layout
//   for a sample but it's NOT the cheapest possible.  Alternatives a
//   production renderer might use:
//
//     (a) Ubershader / mega-kernel.  Combine all 6 fills into a single
//         CS that branches on SV_GroupID -- different thread groups
//         handle different tasks.  One Dispatch() instead of six.
//         Saves command-processor overhead (~10s of us across the suite)
//         and improves SM occupancy: our smallest fill (FillBlasArgs at
//         8 entries = 1 group) barely uses any of the GPU's 100+ SMs;
//         combined dispatches fit the GPU better.  Costs: one big root
//         sig binds ALL inputs/outputs (root-param-budget pressure),
//         shader source gets larger.
//
//     (b) Async-compute queue.  Submit the independent fills to a
//         separate compute queue so they overlap with the graphics+RTAS
//         queue's work.  Requires queue-fence sync but lets the GPU
//         schedule fills in unused SM headroom while ExecuteIndirect
//         RTAS ops are issuing.
//
//     (c) ExecuteIndirect with N dispatch records.  One submit, driver
//         may schedule records concurrently (vendor-dependent).
//
//   For this sample the args fills run only at init / on config-change,
//   total cost << 1 ms, so simplicity wins.  In a real LOD-driven
//   renderer that re-emits args every frame, (a) or (b) would matter.
// =====================================================================
static void BuildArgsFillPipeline(
    ID3D12Device*                       device,
    const D3D12_ROOT_PARAMETER*         rootParams,
    UINT                                rootParamCount,
    const void*                         csBytecode,
    SIZE_T                              csByteSize,
    const wchar_t*                      rsName,
    const wchar_t*                      psoName,
    ComPtr<ID3D12RootSignature>&        outRS,
    ComPtr<ID3D12PipelineState>&        outPSO)
{
    CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init(rootParamCount, rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             &serialized, &error);
    if (FAILED(hr))
    {
        if (error) SampleLog::LogF(L"[fill-args] root sig '%ls' serialize failed: %hs\n",
                                   rsName, (const char*)error->GetBufferPointer());
        ThrowIfFailed(hr);
    }
    ThrowIfFailed(device->CreateRootSignature(0, serialized->GetBufferPointer(),
                                              serialized->GetBufferSize(),
                                              IID_PPV_ARGS(&outRS)));
    outRS->SetName(rsName);

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = outRS.Get();
    psoDesc.CS             = CD3DX12_SHADER_BYTECODE(csBytecode, csByteSize);
    ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&outPSO)));
    outPSO->SetName(psoName);
}

// FillMoveClusterArgs: b0 + u1 (src GVAs) + u0 (out args).
void D3D12RaytracingClusteredGeometry::CreateFillMoveArgsPipeline()
{
    auto device = m_deviceResources->GetD3DDevice();
    CD3DX12_ROOT_PARAMETER rp[3] = {};
    rp[0].InitAsConstants(1, /*shaderReg*/0);                  // b0 = { clusterCount }
    rp[1].InitAsUnorderedAccessView(/*shaderReg*/1);           // u1 = srcAddrs
    rp[2].InitAsUnorderedAccessView(/*shaderReg*/0);           // u0 = argsOut
    BuildArgsFillPipeline(device, rp, _countof(rp),
        g_pFillMoveClusterArgs, ARRAYSIZE(g_pFillMoveClusterArgs),
        L"FillMoveClusterArgs root sig", L"FillMoveClusterArgs compute PSO",
        m_fillMoveArgsRS, m_fillMoveArgsPSO);
    SampleLog::LogF(L"[fill-args-cs] FillMoveClusterArgs PSO built (%zu bytes)\n",
                    (size_t)ARRAYSIZE(g_pFillMoveClusterArgs));
}

// FillBlasFromClasArgs: b0 + t0 (meta) + u0 (out args).
void D3D12RaytracingClusteredGeometry::CreateFillBlasArgsPipeline()
{
    auto device = m_deviceResources->GetD3DDevice();
    CD3DX12_ROOT_PARAMETER rp[3] = {};
    rp[0].InitAsConstants(2, /*shaderReg*/0);                  // b0 = { entryCount, addrStride }
    rp[1].InitAsShaderResourceView(/*shaderReg*/0);            // t0 = inputs
    rp[2].InitAsUnorderedAccessView(/*shaderReg*/0);           // u0 = argsOut
    BuildArgsFillPipeline(device, rp, _countof(rp),
        g_pFillBlasFromClasArgs, ARRAYSIZE(g_pFillBlasFromClasArgs),
        L"FillBlasFromClasArgs root sig", L"FillBlasFromClasArgs compute PSO",
        m_fillBlasArgsRS, m_fillBlasArgsPSO);
    SampleLog::LogF(L"[fill-args-cs] FillBlasFromClasArgs PSO built (%zu bytes)\n",
                    (size_t)ARRAYSIZE(g_pFillBlasFromClasArgs));
}

// FillClasFromTrianglesArgs: b0 (5 dwords) + t0 (meta) + u0 (out args).
void D3D12RaytracingClusteredGeometry::CreateFillClasTriArgsPipeline()
{
    auto device = m_deviceResources->GetD3DDevice();
    CD3DX12_ROOT_PARAMETER rp[3] = {};
    rp[0].InitAsConstants(5, /*shaderReg*/0);                  // b0
    rp[1].InitAsShaderResourceView(/*shaderReg*/0);            // t0 = meta
    rp[2].InitAsUnorderedAccessView(/*shaderReg*/0);           // u0 = argsOut
    BuildArgsFillPipeline(device, rp, _countof(rp),
        g_pFillClasFromTrianglesArgs, ARRAYSIZE(g_pFillClasFromTrianglesArgs),
        L"FillClasFromTrianglesArgs root sig", L"FillClasFromTrianglesArgs compute PSO",
        m_fillClasTriArgsRS, m_fillClasTriArgsPSO);
    SampleLog::LogF(L"[fill-args-cs] FillClasFromTrianglesArgs PSO built (%zu bytes)\n",
                    (size_t)ARRAYSIZE(g_pFillClasFromTrianglesArgs));
}

// FillClusterTemplateArgs: b0 (6 dwords) + t0 (meta) + u0 (out args).
void D3D12RaytracingClusteredGeometry::CreateFillTemplateArgsPipeline()
{
    auto device = m_deviceResources->GetD3DDevice();
    CD3DX12_ROOT_PARAMETER rp[3] = {};
    rp[0].InitAsConstants(6, /*shaderReg*/0);                  // b0
    rp[1].InitAsShaderResourceView(/*shaderReg*/0);            // t0 = meta
    rp[2].InitAsUnorderedAccessView(/*shaderReg*/0);           // u0 = argsOut
    BuildArgsFillPipeline(device, rp, _countof(rp),
        g_pFillClusterTemplateArgs, ARRAYSIZE(g_pFillClusterTemplateArgs),
        L"FillClusterTemplateArgs root sig", L"FillClusterTemplateArgs compute PSO",
        m_fillTemplateArgsRS, m_fillTemplateArgsPSO);
    SampleLog::LogF(L"[fill-args-cs] FillClusterTemplateArgs PSO built (%zu bytes)\n",
                    (size_t)ARRAYSIZE(g_pFillClusterTemplateArgs));
}






void D3D12RaytracingClusteredGeometry::CreateDescriptorHeapAndRaytracingOutput()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 8;    // slot 0 = RT-output UAV, slot 1 = font SRV (DirectXTK), spare = 6
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap)));
    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto bbDesc = m_deviceResources->GetRenderTarget()->GetDesc();
    auto outDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        m_deviceResources->GetBackBufferFormat(), bbDesc.Width, bbDesc.Height, 1, 1, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
        &outDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
    m_raytracingOutput->SetName(L"Raytracing output");

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    UINT idx = AllocateDescriptor(&cpuHandle);
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &uavDesc, cpuHandle);
    m_raytracingOutputUAV = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), idx, m_descriptorSize);

    auto upload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto cbufDesc = CD3DX12_RESOURCE_DESC::Buffer(Align((UINT)sizeof(SceneConstantBuffer), 256));
    ThrowIfFailed(device->CreateCommittedResource(&upload, D3D12_HEAP_FLAG_NONE,
        &cbufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_sceneCB)));
    m_sceneCB->SetName(L"Scene CB");
    CD3DX12_RANGE noRead(0, 0);
    ThrowIfFailed(m_sceneCB->Map(0, &noRead, reinterpret_cast<void**>(&m_sceneCBMapped)));
}

UINT D3D12RaytracingClusteredGeometry::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu)
{
    UINT idx = m_descriptorsAllocated++;
    *outCpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), idx, m_descriptorSize);
    return idx;
}

// =====================================================================================
// Time-based camera orbit. Uses m_cameraSeconds (accumulated wall-clock, decoupled
// from m_animSeconds since the pause-fix) so [Space] freezes camera independently
// of [M] freezing animation -- the bench script can lock camera for stable
// per-frame cost while still measuring the per-frame animated AS rebuilds.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::UpdateSceneConstantBuffer()
{
    if (!m_sceneCBMapped) return;

    // Slowly orbit around the scene center.  Two independent cycles so the
    // camera traces a Lissajous-style path that doesn't repeat for tens of
    // minutes:
    //   - YAW (pan):   2 pi / 30 s   per cycle  -- full revolution every 30 s.
    //   - DOLLY (z):   2 pi / 19 s   per cycle  -- radius pulses INWARD only.
    // 30 and 19 are coprime (19 is prime), so the eye never re-visits the
    // same (yaw, radius) tuple within a reasonable session.  The dolly uses a
    // (1 - cos)/2 envelope rather than a bare sin so the camera NEVER goes
    // farther back than the rest position -- it only swings closer to the
    // scene, framing interior cluster detail at the inner peak.  Range:
    // [kBaseRadius - kDollyAmp, kBaseRadius].
    const double t           = m_cameraSeconds;
    const float  angle       = float(t * (2.0 * M_PI / 30.0));
    constexpr float kBaseRadius  = 6.5f;     // outermost orbit distance (= rest position)
    constexpr float kDollyAmp    = 3.0f;     // inward swing magnitude; radius sweeps 3.5..6.5
    constexpr float kDollyPeriod = 19.0f;    // seconds; coprime with 30s pan period (full in-and-out every 19 s)
    const float  dollyPhase  = float(t * (2.0 * M_PI / kDollyPeriod));
    const float  radius      = kBaseRadius
                             - kDollyAmp * 0.5f * (1.0f - std::cos(dollyPhase));
    const float  height      = 1.5f;                                 // eye lifted (was 0.8) - higher vantage
    XMVECTOR eye = XMVectorSet(radius * std::sin(angle), height,
                               -radius * std::cos(angle), 1.0f);
    XMVECTOR at  = XMVectorSet(0.5f, 0.0f, 0.0f, 1.0f);   // look-at LOWERED (was 1.2) - camera now pitches DOWN noticeably -> floor reads as the dominant surface, sky shrinks to a band at the top, slab gets seen more from above
    XMVECTOR up  = XMVectorSet(0, 1, 0, 0);

    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMMATRIX viewToWorld = XMMatrixInverse(nullptr, view);

    SceneConstantBuffer cb = {};
    // NB: XMMATRIX is row-major, HLSL constant-buffer matrices default to
    // column-major packing. Storing the XM matrix directly (without transpose)
    // makes HLSL see XM_M^T, and `mul(M, v_col)` in HLSL then computes
    // XM_M^T * v_col, which is the correct view->world transform of a column
    // direction vector. Transposing here AND using `mul(M, v)` in HLSL would
    // double-flip the convention and put the camera basis in the wrong place
    // (cardinal-axis orbit positions end up pointing AWAY from the scene).
    cb.viewToWorld = viewToWorld;
    XMStoreFloat4(&cb.cameraPosition, eye);
    cb.miscParams.x = (float)m_width / (float)m_height;
    cb.miscParams.y = std::tan(60.0f * (XM_PI / 180.0f) * 0.5f);   // 60deg vertical FOV
    cb.miscParams.z = (float)m_aaSamplesPerPixel;                  // raygen sample count (1/2/4)
    cb.miscParams.w = m_clusterTint;                               // 0..1 cluster-rainbow tint blend

    // Runtime knobs the shader reads each TraceRay.  See SceneConstantBuffer
    // in RaytracingHlslCompat.h for the slot reservations.
    cb.runtimeParams.x = ReflectionBounces();                      // computed from m_bounceSlider
    cb.runtimeParams.y = RefractionBounces();                      // ditto (= refl or refl+2 with clamps)
    cb.runtimeParams.z = IsTraditional() ? 1u : 0u;                // 1 -> closest-hit uses per-instance lookups instead of ClusterID
    // Sun in upper-back-right. Direction TO the light, normalized. .w is the
    // ambient floor: even fully-shadowed pixels get this fraction of base
    // colour so the scene reads instead of going pitch black.
    XMVECTOR sunDir = XMVector3Normalize(XMVectorSet(0.45f, 0.75f, 0.50f, 0.0f));
    XMStoreFloat4(&cb.lightDir, sunDir);
    cb.lightDir.w = 0.40f;                                          // ambient floor
    memcpy(m_sceneCBMapped, &cb, sizeof(cb));

    // Anim params CB (mapped) -- written here every frame so the latest
    // (frozen-when-paused) t propagates to all in-flight CS dispatches
    // immediately, matching the camera CB's semantics.  Used by
    // UpdateAnimatedObjectPerFrame's CS dispatch via root CBV (NOT
    // 32-bit root constants which would bake in stale values).
    if (m_animParamsMapped)
    {
        m_animParamsMapped->t           = (float)m_animSeconds;
        m_animParamsMapped->vertexCount = m_animatedObjectEnabled ? m_animatedObject.totalVertexCount : 0u;
    }
}

// =====================================================================================
// Per-frame
// =====================================================================================
void D3D12RaytracingClusteredGeometry::OnUpdate()
{
    m_timer.Tick();
    if (!m_animPaused)
        m_animSeconds += m_timer.GetElapsedSeconds();
    if (!m_cameraPaused)
        m_cameraSeconds += m_timer.GetElapsedSeconds();
    UpdateSceneConstantBuffer();

    // Wall-clock frame-time measurement for the overlay's FPS / ms-per-frame
    // line.  Independent of StepTimer's 100 ms cap (see m_frameTimeRing
    // header comment).  Rolling average over m_frameTimeWindow samples so
    // the displayed value doesn't jitter frame-to-frame (HW) and doesn't
    // lag minutes behind state changes (WARP); window size auto-resolved
    // in OnInit per adapter.  First frame has no prior timestamp so we
    // just seed m_lastFrameWallTime; rolling buffer fills over the next
    // N frames.
    const auto now = std::chrono::steady_clock::now();
    if (m_lastFrameWallTime.time_since_epoch().count() != 0 && m_frameTimeWindow > 0)
    {
        const double dt = std::chrono::duration<double>(now - m_lastFrameWallTime).count();
        // Maintain rolling sum: subtract the slot we're about to overwrite,
        // add the new dt.  Until the ring fills, the overwritten slot is
        // 0 so we just accumulate.
        m_frameTimeRingSum -= m_frameTimeRing[m_frameTimeRingIdx];
        m_frameTimeRing[m_frameTimeRingIdx] = dt;
        m_frameTimeRingSum += dt;
        m_frameTimeRingIdx = (m_frameTimeRingIdx + 1) % m_frameTimeWindow;
        if (m_frameTimeRingCount < m_frameTimeWindow) ++m_frameTimeRingCount;
    }
    m_lastFrameWallTime = now;
}

void D3D12RaytracingClusteredGeometry::OnRender()
{
    if (!m_deviceResources->IsWindowVisible()) return;

    // For --screenshot-at <seconds>, advance the animation clock straight to
    // the requested timestamp on the first rendered frame (so the orbit camera
    // is at the right angle when we capture).
    if (m_screenshotAtSeconds >= 0 && !m_screenshotTaken && m_framesRendered == 0)
    {
        // --screenshot-at <seconds>: jump CAMERA orbit to <seconds> on the
        // first frame.  Doesn't touch m_animSeconds (the animated-ball wobble);
        // pre-decoupling this set m_animSeconds because they were the same
        // clock, but the orbit angle is the only thing screenshot users care
        // about so just set the camera clock.
        m_cameraSeconds = m_screenshotAtSeconds;
        UpdateSceneConstantBuffer();
    }

    DoRender();
    ++m_framesRendered;

    // ---- Headless measurement helpers (--log-pf-every / --exit-after-frames /
    // --at).  Driven off m_framesRendered, executed once per OnRender, all
    // logged via SampleLog::LogF so a wrapper script can scrape them.
    if (m_logPfEveryFrames > 0 && (m_framesRendered % m_logPfEveryFrames) == 0)
    {
        SampleLog::LogF(L"[pf-stats frame=%u alloc=%ls rebuild=%ls] "
                        L"inst=%.3fus animBlas=%.3fus tlas=%.3fus "
                        L"staticBlas=%.3fus staticClas=%.3fus\n",
                        m_framesRendered, ClasAllocModeName(), StaticRebuildModeName(),
                        m_pfInstantiateMs * 1000.0, m_pfBlasRebuildMs * 1000.0,
                        m_pfTlasRebuildMs   * 1000.0,
                        m_pfStaticBlasMs    * 1000.0,
                        m_pfStaticClasMs    * 1000.0);
    }
    for (size_t i = 0; i < m_scheduledActions.size(); )
    {
        const auto& a = m_scheduledActions[i];
        if (a.frame == m_framesRendered)
        {
            const wchar_t* act = a.action.c_str();
            SampleLog::LogF(L"[scheduled frame=%u] action=%ls\n", m_framesRendered, act);
            if      (_wcsicmp(act, L"alloc-implicit")    == 0)
            {   m_clasAllocMode = ClasAllocMode::Implicit;
                RebuildStaticAccelerationStructures(L"scheduled alloc-implicit"); }
            else if (_wcsicmp(act, L"alloc-getsizes")    == 0)
            {   m_clasAllocMode = ClasAllocMode::GetSizes;
                RebuildStaticAccelerationStructures(L"scheduled alloc-getsizes"); }
            else if (_wcsicmp(act, L"alloc-compact")     == 0)
            {   m_clasAllocMode = ClasAllocMode::Compact;
                RebuildStaticAccelerationStructures(L"scheduled alloc-compact"); }
            else if (_wcsicmp(act, L"rebuild-none")      == 0)
            {   m_staticRebuildMode = StaticRebuildMode::None;
                CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"rebuild-blas")      == 0)
            {   m_staticRebuildMode = StaticRebuildMode::BlasOnly;
                CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"rebuild-clas-blas") == 0)
            {   m_staticRebuildMode = StaticRebuildMode::ClasAndBlas;
                CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"geom-clusters")     == 0)
            {   if (m_clustersAndPtlasSupported) {
                    m_geometryMode = GeometryMode::Clusters;
                    RebuildStaticAccelerationStructures(L"scheduled geom-clusters"); } }
            else if (_wcsicmp(act, L"geom-traditional")  == 0)
            {   m_geometryMode = GeometryMode::Traditional;
                RebuildStaticAccelerationStructures(L"scheduled geom-traditional"); }
            else if (_wcsicmp(act, L"trad-implicit")     == 0)
            {   m_traditionalAllocMode = TraditionalAllocMode::Implicit;
                RebuildStaticAccelerationStructures(L"scheduled trad-implicit"); }
            else if (_wcsicmp(act, L"trad-compact")      == 0)
            {   m_traditionalAllocMode = TraditionalAllocMode::Compact;
                RebuildStaticAccelerationStructures(L"scheduled trad-compact"); }
            else if (_wcsicmp(act, L"anim-rebuild")      == 0)
            {   m_traditionalAnimMode = TraditionalAnimMode::Rebuild;
                CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"anim-refit")        == 0)
            {   m_traditionalAnimMode = TraditionalAnimMode::Refit;
                CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"extra-none")        == 0)
            {   m_extraInstancesMode = ExtraInstancesMode::None;
                RebuildStaticAccelerationStructures(L"scheduled extra-none"); }
            else if (_wcsicmp(act, L"extra-100")         == 0)
            {   m_extraInstancesMode = ExtraInstancesMode::Hundred;
                RebuildStaticAccelerationStructures(L"scheduled extra-100"); }
            else if (_wcsicmp(act, L"extra-1k")          == 0 ||
                     _wcsicmp(act, L"extra-1000")        == 0)
            {   m_extraInstancesMode = ExtraInstancesMode::Thousand;
                RebuildStaticAccelerationStructures(L"scheduled extra-1k"); }
            else if (_wcsicmp(act, L"extra-10k")         == 0 ||
                     _wcsicmp(act, L"extra-10000")       == 0)
            {   m_extraInstancesMode = ExtraInstancesMode::TenThousand;
                RebuildStaticAccelerationStructures(L"scheduled extra-10k"); }
            else if (_wcsicmp(act, L"flags-none")        == 0)
            {   m_buildFlagMode = BuildFlagMode::None;
                if (m_staticRebuildMode == StaticRebuildMode::None)
                    RebuildStaticAccelerationStructures(L"scheduled flags-none");
                else
                    CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"flags-fast-build")  == 0)
            {   m_buildFlagMode = BuildFlagMode::FastBuild;
                if (m_staticRebuildMode == StaticRebuildMode::None)
                    RebuildStaticAccelerationStructures(L"scheduled flags-fast-build");
                else
                    CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"flags-fast-trace")  == 0)
            {   m_buildFlagMode = BuildFlagMode::FastTrace;
                if (m_staticRebuildMode == StaticRebuildMode::None)
                    RebuildStaticAccelerationStructures(L"scheduled flags-fast-trace");
                else
                    CaptureOverlayStatsSnapshot(); }
            else if (_wcsicmp(act, L"log")               == 0)
            {   SampleLog::LogF(L"[scheduled-snap frame=%u alloc=%ls rebuild=%ls] "
                                L"inst=%.3fus animBlas=%.3fus tlas=%.3fus "
                                L"staticBlas=%.3fus staticClas=%.3fus\n",
                                m_framesRendered, ClasAllocModeName(), StaticRebuildModeName(),
                                m_pfInstantiateMs * 1000.0, m_pfBlasRebuildMs * 1000.0,
                                m_pfTlasRebuildMs   * 1000.0,
                                m_pfStaticBlasMs    * 1000.0,
                                m_pfStaticClasMs    * 1000.0); }
            else if (_wcsicmp(act, L"exit")              == 0)
            {   PostQuitMessage(0); return; }
            // Remove fired action so it doesn't refire.
            m_scheduledActions.erase(m_scheduledActions.begin() + i);
        }
        else
        {
            ++i;
        }
    }
    if (m_exitAfterFrames > 0 && m_framesRendered >= m_exitAfterFrames)
    {
        PostQuitMessage(0);
        return;
    }
    // Wall-clock benchmark mode: arm the start timestamp on first frame,
    // then once m_benchSeconds has elapsed, write the JSON snapshot and
    // quit.  Arming on frame 1 (not 0) skips the init-frame which can
    // include AS-build amortisation in the wall-clock delta -- by frame 1
    // OnRender has completed once and the steady-state path is active.
    if (m_benchSeconds > 0.0)
    {
        // Arm on the first rendered frame (skips the init frame which can
        // amortise AS-build work into the wall-clock delta).  Sentinel value
        // for "not yet armed" is time_point::min() -- see header initialiser.
        const bool armed = (m_benchStartWallTime != std::chrono::steady_clock::time_point::min());
        if (!armed && m_framesRendered >= 1)
        {
            m_benchStartWallTime = std::chrono::steady_clock::now();
            SampleLog::LogF(L"[bench] measurement window started: %.2f s wall-clock, out=%ls\n",
                            m_benchSeconds,
                            m_benchOutPath.empty() ? L"(no --bench-out; will skip JSON)" : m_benchOutPath.c_str());
        }
        else if (armed && !m_benchSnapshotWritten)
        {
            const double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - m_benchStartWallTime).count();
            if (elapsed >= m_benchSeconds)
            {
                if (!m_benchOutPath.empty())
                    WriteBenchmarkSnapshot();
                m_benchSnapshotWritten = true;
                PostQuitMessage(0);
                return;
            }
        }
    }
    // ----

    bool capture = false;
    if (m_screenshotFrame >= 0 && (UINT)m_screenshotFrame < m_framesRendered && !m_screenshotTaken)
        capture = true;
    // Wait a few frames for swap chain warm-up before time-based capture too.
    // Need at least kPerFrameRingSlots * 2 + 5 frames to also get stable
    // per-frame timestamp EMA reads in the log on shutdown (otherwise the
    // ring buffer hasn't filled yet).  Pump up to m_pfSnapTargetCount + a
    // bit so the rolling per-frame snap window has actually completed when
    // we capture -- so screenshots after a config-change toggle show real
    // post-toggle numbers, not the "recalculating..." placeholder we
    // display during the ~1 s settle.  Old behaviour was a hardcoded
    // frame 15 which exited too early on toggle-tests.
    if (m_screenshotAtSeconds >= 0 && m_framesRendered >= 80 && !m_screenshotTaken)
        capture = true;

    if (capture)
    {
        m_screenshotTaken = true;
        CaptureBackBufferToFile(m_screenshotPath);
        PostQuitMessage(0);
    }
}

void D3D12RaytracingClusteredGeometry::DoRender()
{
    m_deviceResources->Prepare();
    auto cl  = m_deviceResources->GetCommandList();
    auto cl4 = m_dxrCommandList.Get();

    // ---- Per-frame AS work + timestamp queries -----------------------------
    // Read back the slot that was written N-frames ago first (data is safely
    // past GPU completion - we read the slot we're about to overwrite). Skip
    // until we've captured at least kPerFrameRingSlots samples to avoid
    // reading uninitialised heap data.
    if (m_animatedObjectEnabled)
    {
        if (m_pfFramesCaptured >= kPerFrameRingSlots)
        {
            // The slot we're about to write was last written N frames ago and
            // resolved on the GPU before this frame was even submitted.
            UINT readSlot = m_pfWriteSlot;
            UINT64 ts[kPerFrameTsPerSlot] = {};
            CD3DX12_RANGE readRange(readSlot * kPerFrameTsPerSlot * sizeof(UINT64),
                                   (readSlot + 1) * kPerFrameTsPerSlot * sizeof(UINT64));
            void* mapped = nullptr;
            if (SUCCEEDED(m_pfQueryReadback->Map(0, &readRange, &mapped)))
            {
                memcpy(ts, (uint8_t*)mapped + readSlot * kPerFrameTsPerSlot * sizeof(UINT64),
                       sizeof(ts));
                D3D12_RANGE noWrite = { 0, 0 };
                m_pfQueryReadback->Unmap(0, &noWrite);

                const double freq = (double)m_timestampFrequency;
                // Slot pairs: (0,1)=anim INSTANTIATE, (2,3)=anim BLAS,
                // (4,5)=TLAS, (6,7)=static BLAS rebuild ([R]),
                // (8,9)=static CLAS rebuild ([R] mode 2 + Implicit).
                auto deltaMs = [&](UINT a, UINT b) {
                    return (ts[b] >= ts[a]) ? (double)(ts[b] - ts[a]) * 1000.0 / freq : 0.0;
                };
                const double instMs       = deltaMs(0, 1);
                const double blasMs       = deltaMs(2, 3);
                const double tlasMs       = deltaMs(4, 5);
                const double staticBlasMs = deltaMs(6, 7);
                const double staticClasMs = deltaMs(8, 9);
                // EMA, alpha=0.1 for a sub-second smoothing window.
                constexpr double a = 0.1;
                m_pfInstantiateMs = m_pfInstantiateMs * (1.0 - a) + instMs * a;
                m_pfBlasRebuildMs = m_pfBlasRebuildMs * (1.0 - a) + blasMs * a;
                m_pfTlasRebuildMs = m_pfTlasRebuildMs * (1.0 - a) + tlasMs * a;
                m_pfStaticBlasMs  = m_pfStaticBlasMs  * (1.0 - a) + staticBlasMs * a;
                m_pfStaticClasMs  = m_pfStaticClasMs  * (1.0 - a) + staticClasMs * a;

                // Headless raw-per-frame log (--log-raw): dump the unfiltered
                // timestamp deltas for this frame so we can see transients the
                // EMA would smooth away.  Same gating as --log-pf-every but
                // logs the raw values, not the EMA.  Also logs the current
                // EMA + the latest snapshot value the overlay would show, so
                // we can see if the snapshot diverges from the raw timing.
                if (m_logRawPfEveryFrames > 0 &&
                    (m_framesRendered % m_logRawPfEveryFrames) == 0)
                {
                    SampleLog::LogF(L"[pf-raw frame=%u alloc=%ls rebuild=%ls] "
                                    L"raw=%.3fus ema=%.3fus snap=%.3fus  "
                                    L"(anim raw=%.3f ema=%.3f, animBlas raw=%.3f, tlas raw=%.3f)\n",
                                    m_framesRendered, ClasAllocModeName(), StaticRebuildModeName(),
                                    staticBlasMs * 1000.0,
                                    m_pfStaticBlasMs * 1000.0,
                                    m_overlayStats.pfStaticBlasMs * 1000.0,
                                    instMs * 1000.0, m_pfInstantiateMs * 1000.0,
                                    blasMs * 1000.0, tlasMs * 1000.0);
                }

                // Rolling per-frame snapshot.  Accumulate samples into
                // m_pfSnapAccum; when we have m_pfSnapTargetCount, mean
                // them into m_overlayStats and start the next window.
                // m_pfSnapSkipFramesLeft != 0 means a recent toggle and we
                // need to flush stale ring-buffer entries first.  See the
                // header comment on m_pfSnapAccum for the full mechanism.
                if (m_pfSnapSkipFramesLeft > 0)
                {
                    --m_pfSnapSkipFramesLeft;
                }
                else
                {
                    m_pfSnapAccum.instMs       += instMs;
                    m_pfSnapAccum.blasMs       += blasMs;
                    m_pfSnapAccum.tlasMs       += tlasMs;
                    m_pfSnapAccum.staticBlasMs += staticBlasMs;
                    m_pfSnapAccum.staticClasMs += staticClasMs;
                    ++m_pfSnapSamplesCollected;
                    if (m_pfSnapSamplesCollected >= m_pfSnapTargetCount)
                    {
                        const double inv = 1.0 / m_pfSnapTargetCount;
                        m_overlayStats.pfInstantiateMs = m_pfSnapAccum.instMs       * inv;
                        m_overlayStats.pfBlasRebuildMs = m_pfSnapAccum.blasMs       * inv;
                        m_overlayStats.pfTlasRebuildMs = m_pfSnapAccum.tlasMs       * inv;
                        m_overlayStats.pfStaticBlasMs  = m_pfSnapAccum.staticBlasMs * inv;
                        m_overlayStats.pfStaticClasMs  = m_pfSnapAccum.staticClasMs * inv;
                        m_overlayStats.pfTimingValid   = true;
                        // First post-toggle window completed -- live timings
                        // now reflect the new mode, drop the "recalculating..."
                        // placeholder.
                        m_pfTimingSettlingAfterToggle  = false;
                        m_pfSnapAccum            = PfSnapAccum{};
                        m_pfSnapSamplesCollected = 0;
                    }
                }

            }
        }

        // Record this frame's per-frame work.  Timestamp slot layout:
        //   base+0..1  INSTANTIATE_CLUSTER_TEMPLATES (animated)
        //   base+2..3  BUILD_BLAS_FROM_CLAS  (animated)
        //   base+4..5  TLAS rebuild
        //   base+6..7  static BLAS rebuild  ([R] mode 1 or 2; else no-op)
        //   base+8..9  static CLAS rebuild  ([R] mode 2 + Implicit alloc; else no-op)
        // Static CLAS must run BEFORE static BLAS (BLAS reads CLAS); static
        // BLAS must run BEFORE TLAS (TLAS sees BLAS).  Timestamps emitted
        // unconditionally so the resolve range is always valid -- when the
        // mode says no work, the begin/end pair brackets nothing and the
        // delta is ~0 (just GPU query overhead).
        const UINT base = m_pfWriteSlot * kPerFrameTsPerSlot;
        // Per-frame animated update: cluster path uses the
        // INSTANTIATE_CLUSTER_TEMPLATES + BLAS_FROM_CLAS pipeline; trad
        // path uses BuildRaytracingAccelerationStructure (rebuild or
        // refit per [F]).  Both emit the same 4-timestamp layout
        // (base+0..1 = AnimateBall.cs, base+2..3 = AS build) so the
        // overlay split between "anim CS" and "anim build" works for
        // both modes uniformly.
        //
        // PAUSE BEHAVIOUR: this update runs every frame, paused or not.
        // The CS reads its time from a mapped CB (m_animParamsBuf,
        // populated in UpdateSceneConstantBuffer); when paused, the
        // CPU-side m_animSeconds freezes, the CB content freezes, and
        // every in-flight CS dispatch reads the FROZEN time at execute
        // time.  Vertex buffer therefore freezes the same instant the
        // camera does (camera is in m_sceneCB, same propagation
        // mechanism).
        if (m_geometryMode == GeometryMode::Clusters)
            UpdateAnimatedObjectPerFrame(base);
        else
            UpdateAnimatedTradPerFrame(base);

        cl4->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base + 8);
        if (m_staticRebuildMode == StaticRebuildMode::ClasAndBlas &&
            m_geometryMode == GeometryMode::Clusters)
            RebuildStaticClasPerFrame();
        cl4->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base + 9);

        cl4->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base + 6);
        if (m_staticRebuildMode != StaticRebuildMode::None &&
            m_geometryMode == GeometryMode::Clusters)
            RebuildStaticBlasPerFrame();
        cl4->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base + 7);

        cl4->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base + 4);
        RebuildTlasPerFrame();
        cl4->EndQuery(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base + 5);


        cl->ResolveQueryData(m_pfQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
            base, kPerFrameTsPerSlot, m_pfQueryReadback.Get(),
            base * sizeof(UINT64));

        m_pfWriteSlot = (m_pfWriteSlot + 1) % kPerFrameRingSlots;
        if (m_pfFramesCaptured < kPerFrameRingSlots * 2)
            ++m_pfFramesCaptured;
    }
    cl4->SetComputeRootSignature(m_globalRootSignature.Get());
    ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap.Get() };
    cl4->SetDescriptorHeaps(_countof(heaps), heaps);
    cl4->SetComputeRootDescriptorTable(GlobalRootSig::OutputUAVSlot, m_raytracingOutputUAV);
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::AccelerationStructureSlot,
        m_tlasBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootConstantBufferView(GlobalRootSig::SceneCBVSlot, m_sceneCB->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::MaterialsSRVSlot,
        m_materialsBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::ClusterNormalsSRVSlot,
        m_clusterNormalsBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::ClusterIndicesSRVSlot,
        m_clusterIndicesBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::ClusterOffsetsSRVSlot,
        m_clusterOffsetsBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::ClusterMetaSRVSlot,
        m_clusterMetaBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::TradTriToCidSRVSlot,
        m_tradTriToCidBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::TradGeomTriBaseSRVSlot,
        m_tradGeomTriBaseBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::PerInstGeomMaterialSRVSlot,
        m_perInstGeomMaterialBuffer->GetGPUVirtualAddress());
    cl4->SetComputeRootShaderResourceView(GlobalRootSig::InstanceMaterialOverrideSRVSlot,
        m_instanceMaterialOverrideBuffer->GetGPUVirtualAddress());
    cl4->SetPipelineState1(m_dxrStateObject.Get());

    auto bbDesc = m_deviceResources->GetRenderTarget()->GetDesc();
    D3D12_DISPATCH_RAYS_DESC drd = {};
    drd.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
    drd.RayGenerationShaderRecord.SizeInBytes  = m_rayGenShaderTable->GetDesc().Width;
    drd.MissShaderTable.StartAddress           = m_missShaderTable->GetGPUVirtualAddress();
    drd.MissShaderTable.SizeInBytes            = m_missShaderTable->GetDesc().Width;
    // Stride = single record size (32 B), not whole-table width. The miss
    // table now has TWO records (primary + shadow); each TraceRay's
    // MissShaderIndex selects which one by stepping `stride` bytes in.
    drd.MissShaderTable.StrideInBytes          = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    drd.HitGroupTable.StartAddress             = m_hitGroupShaderTable->GetGPUVirtualAddress();
    drd.HitGroupTable.SizeInBytes              = m_hitGroupShaderTable->GetDesc().Width;
    drd.HitGroupTable.StrideInBytes            = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    drd.Width  = (UINT)bbDesc.Width;
    drd.Height = (UINT)bbDesc.Height;
    drd.Depth  = 1;
    cl4->DispatchRays(&drd);

    D3D12_RESOURCE_BARRIER toCopy[2] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST),
    };
    cl->ResourceBarrier(_countof(toCopy), toCopy);
    cl->CopyResource(m_deviceResources->GetRenderTarget(), m_raytracingOutput.Get());
    D3D12_RESOURCE_BARRIER toRtAndPresent[2] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
    };
    cl->ResourceBarrier(_countof(toRtAndPresent), toRtAndPresent);

    // Paint on-screen overlay text (stats, key bindings).  The back buffer is
    // now in RENDER_TARGET; SpriteBatch binds it as an RTV, draws text, and
    // we hand off to Present() which transitions to PRESENT internally.
    RenderUI();

    m_deviceResources->Present();

    // DirectXTK: tag per-frame upload pages with the queue's current fence value
    // AFTER the cmd list has been executed (Present did ExecuteCommandList).  This
    // is the canonical placement -- calling Commit before Present would signal the
    // fence on a queue position before our text-draw commands and the ring
    // allocator could reclaim live upload pages.
    if (m_graphicsMemory) m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
}

// =====================================================================================
// Lifecycle
//
// The close-hang fix: the persistent map on m_sceneCB plus the swap chain in
// fullscreen state (occasionally) was making the implicit cleanup wait. Explicit
// Unmap + WaitForGpu BEFORE any ComPtr release is the safe order.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, minimized)) return;
    UpdateForSizeChange(width, height);

    // The DXR output UAV is sized to match the back buffer.  After
    // WindowSizeChanged resizes the swap chain, the UAV is the OLD size
    // and the per-frame `CopyResource(m_raytracingOutput -> back buffer)`
    // hangs (size mismatch).  Recreate the UAV at the new size and rebind
    // it into descriptor slot 0 (where CreateDescriptorHeapAndRaytracing-
    // Output originally placed it - first AllocateDescriptor call).
    auto device = m_deviceResources->GetD3DDevice();
    auto bbDesc = m_deviceResources->GetRenderTarget()->GetDesc();
    if (m_raytracingOutput &&
        (bbDesc.Width  != m_raytracingOutput->GetDesc().Width ||
         bbDesc.Height != m_raytracingOutput->GetDesc().Height))
    {
        m_raytracingOutput.Reset();
        auto outDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_deviceResources->GetBackBufferFormat(), bbDesc.Width, bbDesc.Height, 1, 1, 1, 0,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
            &outDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
        m_raytracingOutput->SetName(L"Raytracing output");

        // Overwrite descriptor heap slot 0 in place (don't AllocateDescriptor
        // again - that would leak the slot and bump m_descriptorsAllocated).
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, m_descriptorSize);
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &uavDesc, cpuHandle);
    }
}

void D3D12RaytracingClusteredGeometry::OnDestroy()
{
    SampleLog::Write(L"OnDestroy: starting shutdown\n");
    if (m_animatedObjectEnabled && m_pfFramesCaptured >= kPerFrameRingSlots)
    {
        SampleLog::LogF(L"[per-frame wall-clock, EMA] INSTANTIATE=%.4f ms  BLAS=%.4f ms  TLAS=%.4f ms  total=%.4f ms\n",
                        m_pfInstantiateMs, m_pfBlasRebuildMs, m_pfTlasRebuildMs,
                        m_pfInstantiateMs + m_pfBlasRebuildMs + m_pfTlasRebuildMs);
    }

    if (m_deviceResources)
    {
        // Make sure the GPU is idle before tearing down resources it still
        // references (BLASes, scratch, etc).
        m_deviceResources->WaitForGpu();
    }

    // Explicit unmap of the persistently-mapped CB. Letting the implicit
    // ComPtr destructor handle this CAN make swap chain / dxgi shutdown hang
    // intermittently on the experimental runtime.
    if (m_sceneCB && m_sceneCBMapped)
    {
        m_sceneCB->Unmap(0, nullptr);
        m_sceneCBMapped = nullptr;
    }
    if (m_animParamsBuf && m_animParamsMapped)
    {
        m_animParamsBuf->Unmap(0, nullptr);
        m_animParamsMapped = nullptr;
    }

    // Animated object: release all its GPU resources.  Args buffers are
    // now DEFAULT-heap UAVs (GPU-filled), no maps to release.
    {
        auto& a = m_animatedObject;
        a.templateInputBuffer.Reset();
        a.templateMetaBuffer.Reset();
        a.templateArgsBuffer.Reset();
        a.templateResultBuffer.Reset();
        a.templateScratchBuffer.Reset();
        a.templateAddressArray.Reset();
        a.restPositionsBuffer.Reset();
        a.perFrameVertexBuffer.Reset();
        a.perFrameInstArgsBuffer.Reset();
        a.vertexOffsetArray.Reset();
        a.perFrameClasResultBuffer.Reset();
        a.perFrameClasScratchBuffer.Reset();
        a.perFrameClasAddressArray.Reset();
        a.blasStorage.Reset();
        a.blasScratchBuffer.Reset();
        a.blasArgsBuffer.Reset();
        a.blasArgsMeta.Reset();
        a.blasResultAddrBuffer.Reset();
        m_animatedObjectEnabled = false;
    }

    // Drop our refs to D3D12 objects in dependency order so destruction is
    // deterministic. (Without this the order is the member-decl order, which
    // would release the device first - DXGI doesn't love that.)
    m_objects.clear();
    m_clusterInputBuffer.Reset();
    m_clasArgsMetaBuffer.Reset();
    m_clasArgsBuffer.Reset();
    m_clasResultBuffer.Reset();
    m_clasScratchBuffer.Reset();
    m_clasAddressArray.Reset();
    m_clasSizeArray.Reset();
    m_clasMoveArgsBuffer.Reset();
    m_blasScratchBuffer.Reset();
    m_blasArgsBuffer.Reset();
    m_blasArgsMeta.Reset();
    m_blasResultAddrBuffer.Reset();
    m_clusterBlasPoolBuffer.Reset();
    m_tradBlasCompactPool.Reset();
    m_tradBlasSharedScratch.Reset();
    // Trad VB+IB pools (introduced together with the per-clone BLAS pool).
    m_tradVertexPool.Reset();
    m_tradIndexPool.Reset();
    m_animClonesBlasPool.Reset();
    m_animClonesTradBlasPool.Reset();
    m_animClonesTradBlasScratch.Reset();
    m_tlasBuffer.Reset();
    m_tlasScratchBuffer.Reset();
    m_tlasInstanceDescs.Reset();
    m_dxrStateObject.Reset();
    m_globalRootSignature.Reset();
    m_rayGenShaderTable.Reset();
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();
    m_animComputePSO.Reset();
    m_animComputeRS.Reset();
    m_fillInstArgsPSO.Reset();
    m_fillInstArgsRS.Reset();
    m_fillMoveArgsPSO.Reset();
    m_fillMoveArgsRS.Reset();
    m_fillBlasArgsPSO.Reset();
    m_fillBlasArgsRS.Reset();
    m_fillClasTriArgsPSO.Reset();
    m_fillClasTriArgsRS.Reset();
    m_fillTemplateArgsPSO.Reset();
    m_fillTemplateArgsRS.Reset();
    m_descriptorHeap.Reset();
    m_raytracingOutput.Reset();
    m_sceneCB.Reset();
    // DirectXTK UI resources (must be torn down before the device that
    // owns their backing GPU resources is released).
    m_uiFont.reset();
    m_spriteBatch.reset();
    m_overlayPanelTexture.Reset();
    m_graphicsMemory.reset();
    m_dxr2CommandList.Reset();
    m_dxr2Device.Reset();
    m_dxrCommandList.Reset();
    m_dxrDevice.Reset();

    SampleLog::Write(L"OnDestroy: done\n");
}


void D3D12RaytracingClusteredGeometry::OnDeviceLost()
{
    // Dump DRED breadcrumbs + page-fault info to identify what GPU command hung.
    auto device = m_deviceResources->GetD3DDevice();
    if (device) {
        ComPtr<ID3D12DeviceRemovedExtendedData1> dred;
        HRESULT hrDred = device->QueryInterface(IID_PPV_ARGS(&dred));
        if (SUCCEEDED(hrDred) && dred) {
            SampleLog::Write(L"[DRED] === Device removed, extracting GPU breadcrumbs ===\n");
            // Auto-breadcrumbs: list of last GPU operations per command list
            D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 bcOut = {};
            HRESULT hrBC = dred->GetAutoBreadcrumbsOutput1(&bcOut);
            if (SUCCEEDED(hrBC) && bcOut.pHeadAutoBreadcrumbNode) {
                int nodeIdx = 0;
                for (auto* node = bcOut.pHeadAutoBreadcrumbNode; node; node = node->pNext, ++nodeIdx) {
                    UINT lastOp = node->pLastBreadcrumbValue ? *node->pLastBreadcrumbValue : 0;
                    UINT totalOps = node->BreadcrumbCount;
                    SampleLog::LogF(L"[DRED] node %d: list=%s, breadcrumbs %u/%u\n",
                        nodeIdx,
                        node->pCommandListDebugNameW ? node->pCommandListDebugNameW : L"(unnamed)",
                        lastOp, totalOps);
                    // Print operations near the last one (in case op N didn't finish, op N-1 was the previous one)
                    UINT from = (lastOp > 5) ? lastOp - 5 : 0;
                    UINT to   = std::min<UINT>(lastOp + 1, totalOps);
                    for (UINT i = from; i < to; ++i) {
                        SampleLog::LogF(L"[DRED]   op[%u] = %d %s\n",
                            i, (int)node->pCommandHistory[i],
                            (i == lastOp) ? L"<-- LAST EXECUTED" : L"");
                    }
                }
            } else {
                SampleLog::LogF(L"[DRED] no breadcrumbs (hr=0x%08X)\n", (unsigned)hrBC);
            }
            // Page fault info: VA + nearby allocations
            D3D12_DRED_PAGE_FAULT_OUTPUT pfOut = {};
            HRESULT hrPF = dred->GetPageFaultAllocationOutput(&pfOut);
            if (SUCCEEDED(hrPF)) {
                SampleLog::LogF(L"[DRED] PageFault VA=0x%llX\n",
                    (unsigned long long)pfOut.PageFaultVA);
                int allocIdx = 0;
                for (auto* a = pfOut.pHeadExistingAllocationNode; a; a = a->pNext, ++allocIdx) {
                    SampleLog::LogF(L"[DRED] existing alloc[%d]: name=%s type=%d\n",
                        allocIdx, a->ObjectNameW ? a->ObjectNameW : L"(unnamed)", (int)a->AllocationType);
                }
            }
        }
    }
    OnDestroy();
}

void D3D12RaytracingClusteredGeometry::OnDeviceRestored()
{
    CreateDeviceDependentResources();
}

// =====================================================================================
// Screenshot capture
// =====================================================================================
void D3D12RaytracingClusteredGeometry::CaptureBackBufferToFile(const std::wstring& path)
{
    auto device       = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();

    UINT capturedBackBufferIndex = m_deviceResources->GetPreviousFrameIndex();
    auto swapChain = m_deviceResources->GetSwapChain();
    ComPtr<ID3D12Resource> backBuffer;
    ThrowIfFailed(swapChain->GetBuffer(capturedBackBufferIndex, IID_PPV_ARGS(&backBuffer)));

    auto rtDesc = backBuffer->GetDesc();
    const UINT width  = (UINT)rtDesc.Width;
    const UINT height = (UINT)rtDesc.Height;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    UINT64 totalBytes = 0; UINT rowCount = 0; UINT64 rowSizeInBytes = 0;
    device->GetCopyableFootprints(&rtDesc, 0, 1, 0, &footprint, &rowCount, &rowSizeInBytes, &totalBytes);

    auto heapPropsRB = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    auto bufDesc     = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
    ComPtr<ID3D12Resource> readback;
    ThrowIfFailed(device->CreateCommittedResource(&heapPropsRB, D3D12_HEAP_FLAG_NONE,
        &bufDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)));

    ComPtr<ID3D12CommandAllocator> alloc;
    ComPtr<ID3D12GraphicsCommandList> cl;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc)));
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.Get(), nullptr, IID_PPV_ARGS(&cl)));
    {
        auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE);
        cl->ResourceBarrier(1, &toCopy);
        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = readback.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; dst.PlacedFootprint = footprint;
        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = backBuffer.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; src.SubresourceIndex = 0;
        cl->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        auto toPresent = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT);
        cl->ResourceBarrier(1, &toPresent);
    }
    ThrowIfFailed(cl->Close());
    ID3D12CommandList* cls[] = { cl.Get() };
    commandQueue->ExecuteCommandLists(1, cls);
    m_deviceResources->WaitForGpu();

    void* mappedRaw = nullptr;
    D3D12_RANGE readRange = { 0, (SIZE_T)totalBytes };
    ThrowIfFailed(readback->Map(0, &readRange, &mappedRaw));
    HRESULT hrSave = SaveBGRAToPng(path, width, height,
        (const uint8_t*)mappedRaw + footprint.Offset, footprint.Footprint.RowPitch);
    D3D12_RANGE writeRange = { 0, 0 };
    readback->Unmap(0, &writeRange);

    SampleLog::LogF(L"[screenshot] %s %ux%u -> %s\n",
        SUCCEEDED(hrSave) ? L"wrote" : L"FAILED to write", width, height, path.c_str());
}

HRESULT D3D12RaytracingClusteredGeometry::SaveBGRAToPng(const std::wstring& path,
                                                       UINT width, UINT height,
                                                       const uint8_t* data,
                                                       UINT rowPitchBytes)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool initedCom = SUCCEEDED(hr);
    auto cleanup = [&](HRESULT result) -> HRESULT { if (initedCom) CoUninitialize(); return result; };
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) return cleanup(hr);
    ComPtr<IWICStream> stream;
    if (FAILED(hr = factory->CreateStream(&stream))) return cleanup(hr);
    if (FAILED(hr = stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE))) return cleanup(hr);
    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder))) return cleanup(hr);
    if (FAILED(hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) return cleanup(hr);
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    if (FAILED(hr = encoder->CreateNewFrame(&frame, &props))) return cleanup(hr);
    if (FAILED(hr = frame->Initialize(props.Get()))) return cleanup(hr);
    if (FAILED(hr = frame->SetSize(width, height))) return cleanup(hr);
    WICPixelFormatGUID pf = GUID_WICPixelFormat32bppBGRA;
    if (FAILED(hr = frame->SetPixelFormat(&pf))) return cleanup(hr);
    const UINT tightStride = width * 4;
    if (rowPitchBytes == tightStride)
    {
        if (FAILED(hr = frame->WritePixels(height, tightStride, tightStride * height,
                                           const_cast<BYTE*>(data)))) return cleanup(hr);
    }
    else
    {
        std::vector<uint8_t> packed((size_t)tightStride * height);
        for (UINT y = 0; y < height; ++y)
            memcpy(packed.data() + (size_t)y * tightStride,
                   data + (size_t)y * rowPitchBytes, tightStride);
        if (FAILED(hr = frame->WritePixels(height, tightStride, tightStride * height,
                                           packed.data()))) return cleanup(hr);
    }
    if (FAILED(hr = frame->Commit())) return cleanup(hr);
    if (FAILED(hr = encoder->Commit())) return cleanup(hr);
    return cleanup(S_OK);
}



// =====================================================================================
// WriteBenchmarkSnapshot
//   Serialises the current overlay-snapshot state (adapter+driver+config+memory
//   +timings+FPS) to m_benchOutPath as UTF-8 JSON.  Called once when the
//   --bench-seconds window has elapsed, just before PostQuitMessage.
//
//   Design notes:
//   * Reads exclusively from m_overlayStats (memory + per-frame timings),
//     m_*BuildMs (static AS build times), and m_frameTimeRing (FPS).  No
//     separate measurement path -- guarantees the JSON matches what the
//     overlay would have shown at quit time, eliminating "the numbers in
//     the report don't match what I saw on screen" debugging.
//   * Mode fields use SHORT CLI-style tokens ("clusters", "fast-trace",
//     "compact") not the verbose human-readable *Name() strings, so the
//     JSON round-trips cleanly into config dicts the sweep driver script
//     understands.
//   * UTF-8 output -- wide strings converted via WideCharToMultiByte so
//     adapter names with non-ASCII characters (TM, em-dash, vendor branding)
//     survive serialisation.
//   * No external JSON library -- this is a sample, not a service.  Manual
//     std::ostringstream with hand-escaped strings is fine for ~50 well-
//     known scalar fields; quote() handles \", \\, control chars.
//   * Errors are logged via SampleLog and reported via exit code 1 from
//     the wrapper script when the JSON is missing; the sample never throws.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::WriteBenchmarkSnapshot()
{
    using std::chrono::system_clock;
    using std::chrono::steady_clock;
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    // Mode -> short CLI token.  Mirrors InputHandling.cpp's accepted args.
    auto geomTok = [&]() -> const wchar_t* {
        return m_geometryMode == GeometryMode::Clusters ? L"clusters" : L"traditional";
    };
    auto vtxTok = [&]() -> const wchar_t* {
        return m_vertexMode == VertexMode::Compressed1 ? L"compressed" : L"float";
    };
    auto clasAllocTok = [&]() -> const wchar_t* {
        switch (m_clasAllocMode) {
            case ClasAllocMode::Implicit: return L"implicit";
            case ClasAllocMode::GetSizes: return L"get-sizes";
            case ClasAllocMode::Compact:  return L"compact";
        }
        return L"?";
    };
    auto tradAllocTok = [&]() -> const wchar_t* {
        return m_traditionalAllocMode == TraditionalAllocMode::Compact ? L"compact" : L"implicit";
    };
    auto rebuildTok = [&]() -> const wchar_t* {
        switch (m_staticRebuildMode) {
            case StaticRebuildMode::None:        return L"none";
            case StaticRebuildMode::BlasOnly:    return L"blas";
            case StaticRebuildMode::ClasAndBlas: return L"clas-blas";
        }
        return L"?";
    };
    auto buildFlagTok = [&]() -> const wchar_t* {
        switch (m_buildFlagMode) {
            case BuildFlagMode::None:      return L"none";
            case BuildFlagMode::FastBuild: return L"fast-build";
            case BuildFlagMode::FastTrace: return L"fast-trace";
        }
        return L"?";
    };
    auto tradAnimTok = [&]() -> const wchar_t* {
        return m_traditionalAnimMode == TraditionalAnimMode::Refit ? L"refit" : L"rebuild";
    };
    auto extraTok = [&]() -> int {
        switch (m_extraInstancesMode) {
            case ExtraInstancesMode::None:        return 0;
            case ExtraInstancesMode::Hundred:     return 100;
            case ExtraInstancesMode::Thousand:    return 1000;
            case ExtraInstancesMode::TenThousand: return 10000;
        }
        return 0;
    };

    // wstring -> UTF-8 for JSON serialisation.
    auto w2u = [](const wchar_t* w) -> std::string {
        if (!w || !*w) return {};
        int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
        if (len <= 1) return {};
        std::string s(len - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), len, nullptr, nullptr);
        return s;
    };
    // JSON string-escape: ", \, control chars (<0x20).  Handles ASCII fine;
    // any non-ASCII byte from the UTF-8 input is passed through verbatim
    // (modern JSON parsers accept UTF-8 in strings).
    auto jq = [](const std::string& in) -> std::string {
        std::string out; out.reserve(in.size() + 2);
        out.push_back('"');
        for (char c : in) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\b': out += "\\b"; break;
                case '\f': out += "\\f"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if ((unsigned char)c < 0x20) {
                        char buf[8]; sprintf_s(buf, "\\u%04X", (unsigned)(unsigned char)c);
                        out += buf;
                    } else {
                        out.push_back(c);
                    }
            }
        }
        out.push_back('"');
        return out;
    };

    // ISO 8601 timestamp (UTC).
    auto tstamp = []() -> std::string {
        auto now = system_clock::now();
        auto t   = system_clock::to_time_t(now);
        std::tm utc = {};
        gmtime_s(&utc, &t);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
        return buf;
    };

    // FPS / ms-per-frame: same derivation the overlay uses.  When the ring
    // buffer hasn't filled (very-low-frame runs, or shutdown right after
    // init), we report NaN-equivalent (0.0) and a frames_measured field so
    // the caller can sanity-check.
    const double secPerFrame = (m_frameTimeRingCount > 0)
        ? (m_frameTimeRingSum / m_frameTimeRingCount) : 0.0;
    const double fps         = (secPerFrame > 0.0) ? (1.0 / secPerFrame) : 0.0;
    const double msPerFrame  = secPerFrame * 1000.0;

    const double benchElapsed = (m_benchStartWallTime != steady_clock::time_point::min())
        ? duration<double>(steady_clock::now() - m_benchStartWallTime).count()
        : 0.0;

    std::ostringstream o;
    o.precision(6);
    o << std::fixed;

    auto kvStr = [&](const char* key, const std::string& val, bool last=false) {
        o << "    \"" << key << "\": " << jq(val) << (last ? "\n" : ",\n");
    };
    auto kvD = [&](const char* key, double val, bool last=false) {
        o << "    \"" << key << "\": " << val << (last ? "\n" : ",\n");
    };
    auto kvU = [&](const char* key, unsigned long long val, bool last=false) {
        o << "    \"" << key << "\": " << val << (last ? "\n" : ",\n");
    };
    auto kvI = [&](const char* key, long long val, bool last=false) {
        o << "    \"" << key << "\": " << val << (last ? "\n" : ",\n");
    };
    auto kvB = [&](const char* key, bool val, bool last=false) {
        o << "    \"" << key << "\": " << (val ? "true" : "false") << (last ? "\n" : ",\n");
    };

    o << "{\n";
    o << "  \"schema_version\": 1,\n";
    o << "  \"timestamp\": " << jq(tstamp()) << ",\n";

    o << "  \"adapter\": {\n";
    kvStr("description",       w2u(m_deviceResources->GetAdapterDescription()));
    kvStr("driver_version",    w2u(m_deviceResources->GetDriverVersionString()));
    kvU  ("vendor_id",         m_deviceResources->GetAdapterVendorId());
    kvU  ("device_id",         m_deviceResources->GetAdapterDeviceId());
    kvU  ("dedicated_vram_bytes", m_deviceResources->GetDedicatedVideoMemoryBytes());
    kvB  ("clusters_supported", m_clustersAndPtlasSupported, /*last=*/true);
    o << "  },\n";

    o << "  \"config\": {\n";
    kvStr("geometry_mode",     w2u(geomTok()));
    kvStr("vertex_format",     w2u(vtxTok()));
    kvStr("clas_alloc",        w2u(clasAllocTok()));
    kvStr("trad_alloc",        w2u(tradAllocTok()));
    kvStr("static_rebuild",    w2u(rebuildTok()));
    kvStr("build_flags",       w2u(buildFlagTok()));
    kvStr("trad_anim_mode",    w2u(tradAnimTok()));
    kvI  ("extra_instances",   (long long)extraTok());
    kvU  ("position_truncate_bits",        m_positionTruncateBits);
    kvU  ("compressed_bits_per_component", m_compressedBitsPerComponent);
    kvU  ("aa_samples_per_pixel",          m_aaSamplesPerPixel);
    kvD  ("cluster_tint",                  (double)m_clusterTint);
    kvI  ("bounce_slider",                 (long long)m_bounceSlider);
    kvU  ("reflection_bounces",            ReflectionBounces());
    kvU  ("refraction_bounces",            RefractionBounces(), /*last=*/true);
    o << "  },\n";

    o << "  \"build_times_ms\": {\n";
    kvD("static_clas", m_clasBuildMs);
    kvD("static_blas", m_blasBuildMs);
    kvD("tlas",        m_tlasBuildMs);
    kvD("total",       m_totalBuildMs, /*last=*/true);
    o << "  },\n";

    o << "  \"frame_perf\": {\n";
    kvD("fps",                  fps);
    kvD("ms_per_frame",         msPerFrame);
    kvU("frames_in_avg_window", m_frameTimeRingCount);
    kvU("avg_window_size",      m_frameTimeWindow);
    // Per-frame timing: use the EMA (alpha=0.1 ~ 10-sample smoothing) instead
    // of m_overlayStats.pf*Ms.  The overlay-snap values require a full
    // pfSnapTargetCount=60 sample window to populate, which at low FPS
    // (e.g. n=1000 trad at 8 fps) takes 7.5 s -- longer than typical bench
    // windows, so the snap values would read zero.  EMA is current after
    // ~10 frames in any path.  pf_timing_valid still reflects whether the
    // snap window completed, for callers that want the strict mean.
    kvD("pf_instantiate_us",    m_pfInstantiateMs * 1000.0);
    kvD("pf_blas_us",           m_pfBlasRebuildMs * 1000.0);
    kvD("pf_tlas_us",           m_pfTlasRebuildMs * 1000.0);
    kvD("pf_static_blas_us",    m_pfStaticBlasMs  * 1000.0);
    kvD("pf_static_clas_us",    m_pfStaticClasMs  * 1000.0);
    // Strict snap mean (over kPfSnapTargetCount samples).  May be 0 in
    // short bench runs on slow scenes -- use the EMA values above for
    // headline numbers; reserve these for high-precision cross-checks.
    kvD("pf_instantiate_us_snap", m_overlayStats.pfInstantiateMs * 1000.0);
    kvD("pf_blas_us_snap",        m_overlayStats.pfBlasRebuildMs * 1000.0);
    kvD("pf_tlas_us_snap",        m_overlayStats.pfTlasRebuildMs * 1000.0);
    kvB("pf_timing_valid",      m_overlayStats.pfTimingValid, /*last=*/true);
    o << "  },\n";

    // Total AS memory: shared field for cross-mode comparisons.  In cluster
    // mode that's static CLAS actual + static BLAS + animated CLAS + animated
    // BLAS + TLAS + clone pool.  In trad mode it's static traditional BLAS
    // actual + animated trad BLAS + TLAS + clone pool.  Mode-specific fields
    // are also exposed below so a wrapper can drill down.
    UINT64 totalAsBytes = m_overlayStats.tlasBytes;
    if (m_overlayStats.geometryMode == (int)GeometryMode::Clusters) {
        totalAsBytes += m_overlayStats.staticClasActualBytes
                      + m_overlayStats.staticBlasTotalBytes
                      + m_overlayStats.animatedPerFrameClasActualBytes
                      + m_overlayStats.animatedBlasBytes
                      + m_overlayStats.animClonesBlasPoolBytes;
    } else {
        totalAsBytes += m_overlayStats.traditionalBlasActualBytes
                      + m_overlayStats.animatedTradBlasBytes
                      + m_overlayStats.animClonesTradBlasPoolBytes;
    }

    o << "  \"memory_bytes\": {\n";
    kvU("total_as",                       totalAsBytes);
    kvU("tlas",                           m_overlayStats.tlasBytes);
    // Cluster path
    kvU("static_clas_alloc",              m_overlayStats.staticClasAllocBytes);
    kvU("static_clas_actual",             m_overlayStats.staticClasActualBytes);
    kvU("static_clas_scratch",            m_overlayStats.staticClasScratchBytes);
    kvU("static_blas_total",              m_overlayStats.staticBlasTotalBytes);
    kvU("static_blas_scratch",            m_overlayStats.staticBlasScratchBytes);
    kvU("static_cluster_input",           m_overlayStats.staticClusterInputBytes);
    // Trad path
    kvU("traditional_blas_total",         m_overlayStats.traditionalBlasTotalBytes);
    kvU("traditional_blas_actual",        m_overlayStats.traditionalBlasActualBytes);
    kvU("traditional_blas_scratch",       m_overlayStats.traditionalBlasScratchBytes);
    kvU("traditional_vb",                 m_overlayStats.traditionalVbBytes);
    kvU("traditional_ib",                 m_overlayStats.traditionalIbBytes);
    // Animated (cluster)
    kvU("animated_template",              m_overlayStats.animatedTemplateBytes);
    kvU("animated_template_scratch",      m_overlayStats.animatedTemplateScratchBytes);
    kvU("animated_template_input",        m_overlayStats.animatedTemplateInputBytes);
    kvU("animated_rest_positions",        m_overlayStats.animatedRestPositionsBytes);
    kvU("animated_pf_clas_alloc",         m_overlayStats.animatedPerFrameClasAllocBytes);
    kvU("animated_pf_clas_actual",        m_overlayStats.animatedPerFrameClasActualBytes);
    kvU("animated_pf_clas_scratch",       m_overlayStats.animatedPerFrameClasScratchBytes);
    kvU("animated_blas",                  m_overlayStats.animatedBlasBytes);
    kvU("animated_blas_scratch",          m_overlayStats.animatedBlasScratchBytes);
    // Animated (trad)
    kvU("animated_trad_blas",             m_overlayStats.animatedTradBlasBytes);
    kvU("animated_trad_scratch",          m_overlayStats.animatedTradScratchBytes);
    kvU("animated_trad_ib",               m_overlayStats.animatedTradIbBytes);
    // Clone pools (animated clones get their own per-frame-rebuilt BLAS)
    kvU("anim_clones_blas_pool",          m_overlayStats.animClonesBlasPoolBytes);
    kvU("anim_clones_blas_scratch",       m_overlayStats.animClonesBlasScratchBytes);
    kvU("anim_clones_trad_blas_pool",     m_overlayStats.animClonesTradBlasPoolBytes);
    kvU("anim_clones_trad_blas_scratch",  m_overlayStats.animClonesTradBlasScratchBytes, /*last=*/true);
    o << "  },\n";

    o << "  \"scene\": {\n";
    kvU("total_clusters",       m_overlayStats.totalClusterCount);
    kvU("total_triangles",      m_overlayStats.totalTriangleCount);
    // Baseline-scene object count (set in BuildScene as m_sourceObjectCount).
    // The benchmark x-axis 'extra instances' is measured AGAINST this baseline,
    // so reports can label charts as 'Extra unique objects (vs N-object
    // baseline)' without hardcoding N.
    kvU("base_objects",         m_sourceObjectCount);
    // Animated mesh cluster + triangle counts (independent of the static scene
    // cluster count above).  Useful for bytes-per-cluster math when comparing
    // static vs animated CLAS sizes -- the animated sphere is meshed differently
    // (envelope-padded for animation) so direct CLAS-bytes comparisons need
    // normalisation by the cluster counts.
    kvU("animated_clusters",    m_animatedObjectEnabled ? m_animatedObject.clusterCount : 0u);
    kvU("animated_triangles",   m_animatedObjectEnabled ? m_animatedObject.mesh.totalTriangles : 0u);
    kvU("anim_clones_pooled",   m_overlayStats.animClonesPooledCount, /*last=*/true);
    o << "  },\n";

    o << "  \"bench\": {\n";
    kvD("requested_seconds",  m_benchSeconds);
    kvD("actual_seconds",     benchElapsed);
    kvU("frames_rendered",    m_framesRendered, /*last=*/true);
    o << "  }\n";
    o << "}\n";

    // Write UTF-8.  std::ofstream on Windows accepts a wide path through the
    // non-standard but universally available overload taking const wchar_t*.
    std::ofstream out(m_benchOutPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!out)
    {
        SampleLog::LogF(L"[bench] FAILED to open %ls for write\n", m_benchOutPath.c_str());
        return;
    }
    const std::string s = o.str();
    out.write(s.data(), (std::streamsize)s.size());
    out.close();
    SampleLog::LogF(L"[bench] snapshot written: %ls  (%zu bytes)\n",
                    m_benchOutPath.c_str(), s.size());
}
