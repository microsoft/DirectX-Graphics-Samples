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

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include "D3D12HelloTexture.h"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <DirectXPackedVector.h>
#include <windows.h>

#include "GltfLoader.h"

#include "MyDx12Utils.h"

#include <pix3.h>

// ImGui
#define IMGUI_IMPL 1

#include <random>
int rand_0_255()
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 0xFF);
    return dist(gen);
}

int ComputeIntersectionArea(const RECT &a, const RECT &b)
{
    return max(0L, min(a.right, b.right) - max(a.left, b.left)) * max(0L, min(a.bottom, b.bottom) - max(a.top, b.top));
}

UINT16 Hdr10Chromaticity(float value) { return static_cast<UINT16>(value * 50000.0f + 0.5f); }

float St2084PqToNits(float pq)
{
    const float m1 = 2610.0f / 16384.0f;
    const float m2 = 2523.0f / 32.0f;
    const float c1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 128.0f;
    const float c3 = 2392.0f / 128.0f;

    pq = (std::max)(pq, 0.0f);
    const float n = std::pow(pq, 1.0f / m2);
    const float numerator = (std::max)(n - c1, 0.0f);
    const float denominator = (std::max)(c2 - c3 * n, 0.000001f);
    return 10000.0f * std::pow(numerator / denominator, 1.0f / m1);
}

float SrgbToLinear(float value)
{
    value = (std::clamp)(value, 0.0f, 1.0f);
    return value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
}

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 618;
}
extern "C"
{
    __declspec(dllexport) extern const char *D3D12SDKPath = ".\\D3D12\\";
}

D3D12HelloTexture::D3D12HelloTexture(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_frameIndex(0),
      m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
      m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)), m_rtvDescriptorSize(0),
      m_descriptorSize(0)
{
}

auto D3D12HelloTexture::ToneMapPass::MakeShaderConstants(const HdrOutputSettings &hdrOutputSettings) const
    -> ToneMapSettings::ShaderConstants
{
    return settings.MakeShaderConstants(hdrOutputSettings.TransferFunction());
}

void D3D12HelloTexture::PipelineRegistry::Create(ID3D12Device *device, PipelineKey key,
                                                 const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
{
    ComPtr<ID3D12PipelineState> pipelineState;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
    pipelines[key] = std::move(pipelineState);
}

ID3D12PipelineState *D3D12HelloTexture::PipelineRegistry::Find(PipelineKey key) const
{
    if (!key.IsValid())
    {
        return nullptr;
    }

    auto pipeline = pipelines.find(key);
    return pipeline != pipelines.end() ? pipeline->second.Get() : nullptr;
}

void D3D12HelloTexture::ResourceRegistry::AnalyzeLifetimes(const std::vector<RenderPass> &renderPasses)
{
    lifetimes = Engine::AnalyzeResourceLifetimes(renderPasses);
}

void D3D12HelloTexture::ResourceRegistry::ResetStates(std::initializer_list<ResourceUsage> usages)
{
    states.clear();
    for (const ResourceUsage &usage : usages)
    {
        SetState(usage.name, usage.state);
    }
}

void D3D12HelloTexture::ResourceRegistry::RegisterTransientResource(TransientResource resource)
{
    transientResources[resource.name] = std::move(resource);
}

void D3D12HelloTexture::ResourceRegistry::UnregisterTransientResource(const std::string &name)
{
    transientResources.erase(name);
}

void D3D12HelloTexture::ResourceRegistry::MarkEndOfLifeResources(int passIndex, const char *backBufferName)
{
    for (auto &[name, lt] : lifetimes)
    {
        if (lt.lastPass != passIndex)
            continue;

        if (name == backBufferName)
            continue;

        if (!transientResources.contains(name))
            continue;

        auto &tr = transientResources.at(name);

        if (tr.state == TransientResourceState::Uninitialized)
        {
            assert(false && "Transient resource must be registered before release.");
            DBG_PRINT("Resource %s is uninitialized.\n", name.c_str());
            continue;
        }

        if (tr.persistent)
            continue;

        if (tr.state != TransientResourceState::Created)
            continue;

        tr.state = TransientResourceState::PendingRelease1;

        DBG_PRINT("Resource %s endOfLife.\n", name.c_str());
    }
}

void D3D12HelloTexture::ResourceRegistry::MarkPendingTransientResources(UINT64 fenceValue)
{
    for (auto &[name, tr] : transientResources)
    {
        if (tr.state != TransientResourceState::PendingRelease1)
            continue;

        tr.retireFenceValue = fenceValue;
        tr.state = TransientResourceState::PendingRelease2;

        DBG_PRINT("Resource %s waitFenceValue.\n", name.c_str(), tr.retireFenceValue);
    }
}

std::vector<std::string>
D3D12HelloTexture::ResourceRegistry::CollectGarbageTransientResources(UINT64 completedFenceValue)
{
    std::vector<std::string> releasedResources;

    for (auto &[name, tr] : transientResources)
    {
        if (tr.state != TransientResourceState::PendingRelease2)
            continue;

        if (completedFenceValue < tr.retireFenceValue)
            continue;

        tr.resource.Reset();
        tr.retireFenceValue = 0;
        tr.state = TransientResourceState::Initialized;
        releasedResources.push_back(name);

        DBG_PRINT("Resource %s released.\n", name.c_str());
    }

    return releasedResources;
}

std::vector<std::string>
D3D12HelloTexture::ResourceRegistry::GetResourcesStartingAtPass(int passIndex, const char *backBufferName) const
{
    std::vector<std::string> resourceNames;

    for (const auto &[name, lt] : lifetimes)
    {
        if (lt.firstPass != passIndex)
            continue;

        if (name == backBufferName)
            continue;

        if (!transientResources.contains(name))
            continue;

        resourceNames.push_back(name);
    }

    return resourceNames;
}

auto D3D12HelloTexture::ResourceRegistry::PrepareTransientResourceForCreate(const std::string &name)
    -> TransientResource *
{
    auto transientResource = transientResources.find(name);
    if (transientResource == transientResources.end())
    {
        return nullptr;
    }

    auto &tr = transientResource->second;
    if (tr.state == TransientResourceState::Uninitialized)
    {
        assert(false && "Transient resource must be registered before use.");
        DBG_PRINT("Resource %s is uninitialized.\n", name.c_str());
        return nullptr;
    }

    if (tr.state == TransientResourceState::Created)
    {
        return nullptr;
    }

    if (tr.state == TransientResourceState::PendingRelease1 || tr.state == TransientResourceState::PendingRelease2)
    {
        tr.retireFenceValue = 0;
        tr.state = TransientResourceState::Created;
        DBG_PRINT("Resource %s reused before release.\n", name.c_str());
        return nullptr;
    }

    if (tr.state != TransientResourceState::Initialized)
    {
        return nullptr;
    }

    return &tr;
}

void D3D12HelloTexture::ResourceRegistry::MarkTransientResourceCreated(const std::string &name)
{
    auto transientResource = transientResources.find(name);
    if (transientResource == transientResources.end())
    {
        assert(false && "Transient resource must be registered before marking it created.");
        return;
    }

    transientResource->second.state = TransientResourceState::Created;
}

ID3D12Resource *D3D12HelloTexture::ResourceRegistry::FindTransientD3DResource(const std::string &name) const
{
    auto transientResource = transientResources.find(name);
    if (transientResource == transientResources.end())
    {
        return nullptr;
    }

    return transientResource->second.resource.Get();
}

void D3D12HelloTexture::OnInit()
{
    m_prevTime = std::chrono::steady_clock::now();
    LoadPipeline();
    LoadAssets();
    InitImGui();
}

// Load the rendering pipeline dependencies.
void D3D12HelloTexture::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
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

    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = kFrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = kSwapChainFormat;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    UpdateHdr10DisplayMode();
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = kRTVDescriptorCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a heap for SRV/CBV/UAV
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = kMainHeapDescriptorCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
        // Create a descriptor allocator to manage the descriptors in the heap.
        m_descriptorHeapAllocator.Init(m_device.Get(), m_heap.Get());

        D3D12_DESCRIPTOR_HEAP_DESC imguiHeapDesc = {};
        imguiHeapDesc.NumDescriptors = kHeapDescriptorCount;
        imguiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        imguiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&imguiHeapDesc, IID_PPV_ARGS(&m_imguiHeap)));
        m_ImGuiDescriptorHeapAllocator.Init(m_device.Get(), m_imguiHeap.Get());

        m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    // Create the depth stencil view.
    {
        CreateDsvHeap();
        RegisterDepthStencil(m_width, m_height);
        RegisterLightPassRenderTarget(m_width, m_height);
    }

    // create command allocators.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                       IID_PPV_ARGS(&m_frameResources[n].commandAllocator)));
    }

    //
    m_gpuWorkMeter.Init(m_device.Get(),
                        kGpuWorkMeterQueryCount); // Initialize GPU work meter with a maximum of 100 timestamp queries.
}

bool D3D12HelloTexture::HdrOutputPolicy::CheckSwapChainColorSpaceSupport(IDXGISwapChain3 *swapChain,
                                                                         DXGI_COLOR_SPACE_TYPE colorSpace) const
{
    UINT colorSpaceSupport = 0;
    return SUCCEEDED(swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
           (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT);
}

bool D3D12HelloTexture::HdrOutputPolicy::CheckCurrentOutputHdr10Support(ComPtr<IDXGIFactory4> &dxgiFactory,
                                                                        HWND hwnd) const
{
    if (!dxgiFactory->IsCurrent())
    {
        UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
    }

    RECT windowRect = {};
    GetWindowRect(hwnd, &windowRect);

    ComPtr<IDXGIAdapter1> adapter;
    ThrowIfFailed(dxgiFactory->EnumAdapters1(0, &adapter));

    ComPtr<IDXGIOutput> bestOutput;
    ComPtr<IDXGIOutput> output;
    int bestIntersectArea = -1;

    for (UINT outputIndex = 0; adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND; ++outputIndex)
    {
        DXGI_OUTPUT_DESC outputDesc = {};
        ThrowIfFailed(output->GetDesc(&outputDesc));

        const int intersectArea = ComputeIntersectionArea(windowRect, outputDesc.DesktopCoordinates);
        if (intersectArea > bestIntersectArea)
        {
            bestOutput = output;
            bestIntersectArea = intersectArea;
        }

        output.Reset();
    }

    if (!bestOutput)
    {
        return false;
    }

    ComPtr<IDXGIOutput6> output6;
    if (FAILED(bestOutput.As(&output6)))
    {
        return false;
    }

    DXGI_OUTPUT_DESC1 outputDesc = {};
    ThrowIfFailed(output6->GetDesc1(&outputDesc));
    return outputDesc.ColorSpace == kHdr10ColorSpace;
}

void D3D12HelloTexture::HdrOutputPolicy::ApplySwapChainColorSpace(IDXGISwapChain3 *swapChain,
                                                                  DXGI_COLOR_SPACE_TYPE colorSpace)
{
    if (settings.currentSwapChainColorSpace == colorSpace)
    {
        return;
    }

    if (CheckSwapChainColorSpaceSupport(swapChain, colorSpace))
    {
        ThrowIfFailed(swapChain->SetColorSpace1(colorSpace));
        settings.currentSwapChainColorSpace = colorSpace;
    }
}

void D3D12HelloTexture::HdrOutputPolicy::ApplyHdr10Metadata(IDXGISwapChain3 *swapChain, bool enabled) const
{
    ComPtr<IDXGISwapChain4> swapChain4;
    if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&swapChain4))))
    {
        return;
    }

    if (!enabled)
    {
        ThrowIfFailed(swapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_NONE, 0, nullptr));
        return;
    }

    DXGI_HDR_METADATA_HDR10 metadata = {};
    metadata.RedPrimary[0] = Hdr10Chromaticity(0.708f);
    metadata.RedPrimary[1] = Hdr10Chromaticity(0.292f);
    metadata.GreenPrimary[0] = Hdr10Chromaticity(0.170f);
    metadata.GreenPrimary[1] = Hdr10Chromaticity(0.797f);
    metadata.BluePrimary[0] = Hdr10Chromaticity(0.131f);
    metadata.BluePrimary[1] = Hdr10Chromaticity(0.046f);
    metadata.WhitePoint[0] = Hdr10Chromaticity(0.3127f);
    metadata.WhitePoint[1] = Hdr10Chromaticity(0.3290f);
    metadata.MaxMasteringLuminance = kHdr10MaxMasteringLuminance;
    metadata.MinMasteringLuminance = kHdr10MinMasteringLuminance;
    metadata.MaxContentLightLevel = kHdr10MaxContentLightLevel;
    metadata.MaxFrameAverageLightLevel = kHdr10MaxFrameAverageLightLevel;

    ThrowIfFailed(swapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(metadata), &metadata));
}

void D3D12HelloTexture::HdrOutputPolicy::Update(ComPtr<IDXGIFactory4> &dxgiFactory, IDXGISwapChain3 *swapChain,
                                                HWND hwnd)
{
    const bool hdr10Enabled = CheckCurrentOutputHdr10Support(dxgiFactory, hwnd) &&
                              CheckSwapChainColorSpaceSupport(swapChain, kHdr10ColorSpace);
    const bool hdr10StateChanged = settings.hdr10Enabled != hdr10Enabled;
    settings.hdr10Enabled = hdr10Enabled;

    ApplySwapChainColorSpace(swapChain, settings.TargetColorSpace());
    if (hdr10StateChanged)
    {
        ApplyHdr10Metadata(swapChain, hdr10Enabled);
    }
}

void D3D12HelloTexture::HdrOutputPolicy::ReapplyColorSpace(IDXGISwapChain3 *swapChain)
{
    ApplySwapChainColorSpace(swapChain, settings.TargetColorSpace());
}

void D3D12HelloTexture::UpdateHdr10DisplayMode()
{
    m_hdrOutputPolicy.Update(m_dxgiFactory, m_swapChain.Get(), Win32Application::GetHwnd());
}

std::array<GltfVertex, D3D12HelloTexture::kCubeVertexCount> D3D12HelloTexture::CreateCubeVertices() const
{

    constexpr float s = kCubeScale;
    constexpr float u = 1.f;

    return {{
        // front
        {{-s, -s, -s}, {0, u}, {0.0f, 0.0f, -1.0f}},
        {{-s, s, -s}, {0, 0}, {0.0f, 0.0f, -1.0f}},
        {{s, s, -s}, {u, 0}, {0.0f, 0.0f, -1.0f}},
        {{-s, -s, -s}, {0, u}, {0.0f, 0.0f, -1.0f}},
        {{s, s, -s}, {u, 0}, {0.0f, 0.0f, -1.0f}},
        {{s, -s, -s}, {u, u}, {0.0f, 0.0f, -1.0f}},

        // back
        {{-s, -s, s}, {u, u}, {0.0f, 0.0f, 1.0f}},
        {{s, s, s}, {0, 0}, {0.0f, 0.0f, 1.0f}},
        {{-s, s, s}, {u, 0}, {0.0f, 0.0f, 1.0f}},
        {{-s, -s, s}, {u, u}, {0.0f, 0.0f, 1.0f}},
        {{s, -s, s}, {0, u}, {0.0f, 0.0f, 1.0f}},
        {{s, s, s}, {0, 0}, {0.0f, 0.0f, 1.0f}},

        // left
        {{-s, -s, s}, {0, u}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, s}, {0, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, -s}, {u, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, -s, s}, {0, u}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, -s}, {u, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, -s, -s}, {u, u}, {-1.0f, 0.0f, 0.0f}},

        // right
        {{s, -s, -s}, {0, u}, {1.0f, 0.0f, 0.0f}},
        {{s, s, -s}, {0, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {1.0f, 0.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, -s, s}, {u, u}, {1.0f, 0.0f, 0.0f}},

        // top
        {{-s, s, -s}, {0, u}, {0.0f, 1.0f, 0.0f}},
        {{-s, s, s}, {0, 0}, {0.0f, 1.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {0.0f, 1.0f, 0.0f}},
        {{-s, s, -s}, {0, u}, {0.0f, 1.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {0.0f, 1.0f, 0.0f}},
        {{s, s, -s}, {u, u}, {0.0f, 1.0f, 0.0f}},

        // bottom
        {{-s, -s, s}, {u, 0}, {0.0f, -1.0f, 0.0f}},
        {{-s, -s, -s}, {u, u}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {0.0f, -1.0f, 0.0f}},
        {{-s, -s, s}, {u, 0}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, s}, {0, 0}, {0.0f, -1.0f, 0.0f}},
    }};
}

DescriptorHeapHandle D3D12HelloTexture::CreateTextureFromRGBA8(const UINT8 *pixels, UINT width, UINT height,
                                                               ComPtr<ID3D12Resource> &texture,
                                                               ComPtr<ID3D12Resource> &uploadHeap)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    // Create the GPU resource for the texture.
    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                    nullptr, IID_PPV_ARGS(&texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(
        m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadHeap)));

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = pixels;
    textureData.RowPitch = width * kTexturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * kTextureHeight;

    UpdateSubresources(m_commandList.Get(), texture.Get(), uploadHeap.Get(), 0, 0, 1, &textureData);

    m_commandList->ResourceBarrier(1,
                                   &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    return AllocateTextureSRV(texture.Get());
}

// Load the sample assets.
void D3D12HelloTexture::LoadAssets()
{
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned
        // will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // t0 - t(TextureCount-1) : Texture SRVs: space 0 : 0 - (kTextureCount-1)
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV[1];
        rangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, kTextureCount, 0 /*base*/, 0 /*space*/,
                          D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        // t0 : SRV structured buffer: space1 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV2[1];
        rangesSRV2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 1 /*space*/,
                           D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        // t0 : SRV structured buffer: space2 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV3[1];
        rangesSRV3[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 2 /*space*/,
                           D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        // t0 - t3 : GBuffer SRVs, t4 : depth SRV, space 3
        CD3DX12_DESCRIPTOR_RANGE1 rangesGBufferSRV[1];
        rangesGBufferSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBuffer::kCount + 1, 0 /*base*/, 3 /*space*/,
                                 D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        // t0 : HDR scene color SRV, space 4
        CD3DX12_DESCRIPTOR_RANGE1 rangesToneMapSRV[1];
        rangesToneMapSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 4 /*space*/,
                                 D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        CD3DX12_DESCRIPTOR_RANGE1 rangesCVB[1];
        rangesCVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_DESCRIPTOR_RANGE1 rangesLightCBV[1];
        rangesLightCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[9];
        rootParameters[0].InitAsDescriptorTable(1, &rangesSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // Texture SRVs
        rootParameters[1].InitAsDescriptorTable(1, &rangesSRV2[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Instance data)
        rootParameters[2].InitAsDescriptorTable(1, &rangesSRV3[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Material data)
        rootParameters[3].InitAsDescriptorTable(1, &rangesCVB[0], D3D12_SHADER_VISIBILITY_ALL); // Camera constants
        rootParameters[4].InitAsDescriptorTable(1, &rangesGBufferSRV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer SRVs
        rootParameters[5].InitAsDescriptorTable(1, &rangesLightCBV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL);    // Light constants
        rootParameters[6].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer debug target
        rootParameters[7].InitAsDescriptorTable(1, &rangesToneMapSRV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL);    // ToneMap HDR scene color
        rootParameters[8].InitAsConstants(5, 3, 0, D3D12_SHADER_VISIBILITY_PIXEL); // ToneMap constants

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
#if 1
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
#else
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
#endif
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                    IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        UINT8 *pVertexShaderData = nullptr;
        UINT8 *pPixelShaderData = nullptr;
        UINT vertexShaderDataLength = 0;
        UINT pixelShaderDataLength = 0;

        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_VSMain.cso").c_str(), &pVertexShaderData,
                                       &vertexShaderDataLength));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_PSMain.cso").c_str(), &pPixelShaderData,
                                       &pixelShaderDataLength));

        UINT8 *pDepthVS = nullptr;
        UINT depthVSSize = 0;

        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_DepthOnlyVS_VSMain.cso").c_str(), &pDepthVS, &depthVSSize));

        UINT8 *pGBufferVS = nullptr;
        UINT8 *pGBufferPS = nullptr;
        UINT gbufferVSSize = 0;
        UINT gbufferPSSize = 0;
        UINT8 *pGBufferDebugVS = nullptr;
        UINT8 *pGBufferDebugPS = nullptr;
        UINT gbufferDebugVSSize = 0;
        UINT gbufferDebugPSSize = 0;

        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_GBuffer_VSMain.cso").c_str(), &pGBufferVS, &gbufferVSSize));
        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_GBuffer_PSMain.cso").c_str(), &pGBufferPS, &gbufferPSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_GBufferDebug_VSMain.cso").c_str(), &pGBufferDebugVS,
                                       &gbufferDebugVSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_GBufferDebug_PSMain.cso").c_str(), &pGBufferDebugPS,
                                       &gbufferDebugPSSize));

        UINT8 *pLightPassVS = nullptr;
        UINT8 *pLightPassPS = nullptr;
        UINT lightPassVSSize = 0;
        UINT lightPassPSSize = 0;
        UINT8 *pLightPassDebugGradientVS = nullptr;
        UINT8 *pLightPassDebugGradientPS = nullptr;
        UINT lightPassDebugGradientVSSize = 0;
        UINT lightPassDebugGradientPSSize = 0;
        UINT8 *pToneMapVS = nullptr;
        UINT8 *pToneMapPS = nullptr;
        UINT toneMapVSSize = 0;
        UINT toneMapPSSize = 0;

        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPass_VSMain.cso").c_str(), &pLightPassVS,
                                       &lightPassVSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPass_PSMain.cso").c_str(), &pLightPassPS,
                                       &lightPassPSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPassDebugGradient_VSMain.cso").c_str(),
                                       &pLightPassDebugGradientVS, &lightPassDebugGradientVSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPassDebugGradient_PSMain.cso").c_str(),
                                       &pLightPassDebugGradientPS, &lightPassDebugGradientPSSize));
        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_ToneMap_VSMain.cso").c_str(), &pToneMapVS, &toneMapVSSize));
        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_ToneMap_PSMain.cso").c_str(), &pToneMapPS, &toneMapPSSize));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        D3D12_INPUT_ELEMENT_DESC depthLayout[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        //
        // Main Pass PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        RegisterMainPipeline(psoDesc, inputElementDescs, _countof(inputElementDescs), pVertexShaderData,
                             vertexShaderDataLength, pPixelShaderData, pixelShaderDataLength);

        //
        // GBuffer PSO
        //
        RegisterGBufferPipeline(psoDesc, pGBufferVS, gbufferVSSize, pGBufferPS, gbufferPSSize);

        //
        // LightPass PSO
        //
        RegisterFullscreenPipeline(Pipe::Lighting, psoDesc, pLightPassVS, lightPassVSSize, pLightPassPS, lightPassPSSize,
                                   DXGI_FORMAT_R16G16B16A16_FLOAT);

        RegisterFullscreenPipeline(Pipe::LightingDebugGradient, psoDesc, pLightPassDebugGradientVS,
                                   lightPassDebugGradientVSSize, pLightPassDebugGradientPS,
                                   lightPassDebugGradientPSSize, DXGI_FORMAT_R16G16B16A16_FLOAT);

        //
        // ToneMap PSO
        //
        RegisterFullscreenPipeline(Pipe::ToneMap, psoDesc, pToneMapVS, toneMapVSSize, pToneMapPS, toneMapPSSize,
                                   m_backBufferFormat);

        //
        // GBuffer Debug PSO
        //
        RegisterFullscreenPipeline(Pipe::GBufferDebug, psoDesc, pGBufferDebugVS, gbufferDebugVSSize,
                                   pGBufferDebugPS, gbufferDebugPSSize, DXGI_FORMAT_R16G16B16A16_FLOAT);

        //
        // Depth PrePass PSO
        //
        RegisterDepthPrePassPipeline(psoDesc, depthLayout, _countof(depthLayout), pDepthVS, depthVSSize);
    }

    //
    CreateGBuffer();

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              m_frameResources[m_frameIndex].commandAllocator.Get(),
                                              GetPipelineState(PipelineId(Pipe::Main)), IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    GltfMeshData mesh;

    std::vector<GltfVertex> vertices_;
    UINT vertexBufferSize_;

    if constexpr (kGltfLoadingEnabled)
    {
        bool loaded = LoadGltfMesh("Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", mesh);
        assert(loaded);
    }

    if constexpr (kGltfLoadingEnabled && kGltfMeshDisplay)
    {
        const UINT size = static_cast<UINT>(sizeof(GltfVertex) * mesh.vertices.size());
        m_indexCountPerInstance = static_cast<UINT>(mesh.indices.size());
        m_vertexCountPerInstance = static_cast<UINT>(mesh.vertices.size());
        vertices_ = mesh.vertices;
        vertexBufferSize_ = size;
    }
    else
    {
        auto a = CreateCubeVertices();
        vertices_ = std::vector<GltfVertex>(a.begin(), a.end());
        m_vertexCountPerInstance = static_cast<UINT>(vertices_.size());
        vertexBufferSize_ = static_cast<UINT>(sizeof(GltfVertex) * vertices_.size());
    }

    const std::vector<GltfVertex> vertices = vertices_;
    const UINT vertexBufferSize = vertexBufferSize_;

    // Note: using upload heaps to transfer static data like vert buffers is not
    // recommended. Every time the GPU needs it, the upload heap will be marshalled
    // over. Please read up on Default Heap usage. An upload heap is used here for
    // code simplicity and because there are very few verts to actually transfer.
    MyDx12Util::CreateUploadBuffer(m_device, vertexBufferSize, m_vertexBuffer);

    // Copy the triangle data to the vertex buffer.
    UINT8 *pVertexDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertices.data(), vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(GltfVertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    if constexpr (kGltfLoadingEnabled)
    {
        const UINT indexBufferSize = static_cast<UINT>(mesh.indices.size() * sizeof(uint32_t));

        MyDx12Util::CreateUploadBuffer(m_device, indexBufferSize, m_indexBuffer);

        UINT8 *pIndexDataBegin = nullptr;
        ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, mesh.indices.data(), indexBufferSize);
        m_indexBuffer->Unmap(0, nullptr);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.

    std::vector<ComPtr<ID3D12Resource>> textureUploadHeap;

    if constexpr (kGltfLoadingEnabled)
    {
        // randomな色のチェッカーボードテクスチャをkTextureCount毎生成
        textureUploadHeap.resize(kTextureCount);

        // glTFファイルからテクスチャを読み込む
        m_texture.resize(kTextureCount);

        std::vector<std::vector<UINT8>> texture(kTextureTypes);

        m_gltfTextureCount = static_cast<UINT>(mesh.textures.size());

        DBG_PRINT("m_gltfTextureCount = %d\n", m_gltfTextureCount);

        for (size_t i = 0; i < kTextureCount; i++)
        {
            bool useGltfTex = i < mesh.textures.size();
            UINT8 *pixels = nullptr;
            UINT width, height;

            if (useGltfTex)
            {
                // Gltfファイルのテクスチャがあれば使う。
                const auto &tex = mesh.textures[i];
                pixels = (UINT8 *)tex.pixels.data();
                width = tex.width;
                height = tex.height;
                DBG_PRINT("[%d] gltfTexture :width %d height %d\n", i, tex.width, tex.height);
            }
            else
            {
                // Gltfファイルのテクスチャが足りなければチェッカーボードテクスチャを使う。
                texture[i] = GenerateCheckerboardTextureData();
                pixels = &texture[i % kTextureTypes][0];
                width = kTextureWidth;
                height = kTextureHeight;
                DBG_PRINT("[%d] CheckerBoardTexture :width %d height %d\n", i, kTextureWidth, kTextureHeight);
            }

            DescriptorHeapHandle srv =
                CreateTextureFromRGBA8(pixels, width, height, m_texture[i], textureUploadHeap[i]);
            if (i == 0)
            {
                m_textureTableStart = srv;
            }
            m_texIndex[i] = srv.Index - m_textureTableStart.Index;
            DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
        }
    }
    else
    {
        // randomな色のチェッカーボードテクスチャをkTextureCount毎生成
        textureUploadHeap.resize(kTextureCount);

        m_texture.resize(kTextureCount);

        // CPUメモリ状にテクスチャを作る

        std::vector<std::vector<UINT8>> texture(kTextureTypes);
        for (int i = 0; i < kTextureTypes; i++)
        {
            texture[i] = GenerateCheckerboardTextureData();

            DescriptorHeapHandle srv = CreateTextureFromRGBA8(&texture[i % kTextureTypes][0], kTextureWidth,
                                                              kTextureHeight, m_texture[i], textureUploadHeap[i]);
            if (i == 0)
            {
                m_textureTableStart = srv;
            }
            m_texIndex[i] = srv.Index - m_textureTableStart.Index;
            DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
        }
    }

    // Generate the instance data.
    m_instanceData.clear();
    m_instanceDataForCPU.clear();
    for (int i = 0; i < kMaxInstanceCount; i++)
    {
        XMFLOAT3 pos = instanceIdToXYZ(i, GridDim(10, 10, 10));

        // CPU only
        m_instanceDataForCPU.emplace_back(pos, XMFLOAT3(0.0f, 0.0f, 0.0f));

        // CPU and GPU
        InstanceData d;

        if constexpr (kGltfLoadingEnabled)
        {
            // glTFのマテリアル数に応じてmaterialIdを割り当てる。足りない分は0番のマテリアルを使う。
            const UINT gltfMaterialCount = static_cast<UINT>(mesh.materials.size());
            d.materialId = gltfMaterialCount > 0 ? i % gltfMaterialCount : 0;
        }
        else
        {
            d.materialId = i % kMaterialCount;
        }

        XMMATRIX scaleMat = XMMatrixScaling(m_meshScale, m_meshScale, m_meshScale);
        XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMMATRIX worldMat = scaleMat * transMat;
        XMStoreFloat4x4(&d.world, XMMatrixTranspose(worldMat));
        d.prevWorld = d.world;
        m_instanceData.push_back(d);
    }

    // Generate the material data.
    m_materialData.clear();
    const auto resolveTextureIndex = [this](int gltfTextureIndex, UINT fallbackIndex) -> UINT
    {
        if (gltfTextureIndex >= 0 && gltfTextureIndex < static_cast<int>(kTextureCount))
        {
            return m_texIndex[gltfTextureIndex];
        }
        return fallbackIndex;
    };

    for (int i = 0; i < kMaterialCount; i++)
    {
        const UINT fallbackTexIndex = m_texIndex[i % kTextureCount];
        Material m = {};
        m.albedoTexIndex = fallbackTexIndex;
        m.metallicRoughnessTexIndex = fallbackTexIndex;
        m.emissiveTexIndex = fallbackTexIndex;
        m.occlusionTexIndex = fallbackTexIndex;
        m.normalTexIndex = fallbackTexIndex;
        m.roughnessFactor = 0.2f + 0.6f * static_cast<float>(i % 16) / 15.0f;
        m.metallicFactor = (i % 8 == 0) ? 1.0f : 0.0f;
        m.occlusionStrength = 1.0f;
        m.flags = 0;

        if constexpr (kGltfLoadingEnabled)
        {
            if (i < static_cast<int>(mesh.materials.size()))
            {
                const auto &gltfMaterial = mesh.materials[i];
                m.albedoTexIndex = resolveTextureIndex(gltfMaterial.albedoTexIndex, fallbackTexIndex);
                m.metallicRoughnessTexIndex =
                    resolveTextureIndex(gltfMaterial.metallicRoughnessTexIndex, fallbackTexIndex);
                m.emissiveTexIndex = resolveTextureIndex(gltfMaterial.emissiveTexIndex, fallbackTexIndex);
                m.occlusionTexIndex = resolveTextureIndex(gltfMaterial.occlusionTexIndex, fallbackTexIndex);
                m.normalTexIndex = resolveTextureIndex(gltfMaterial.normalTexIndex, fallbackTexIndex);
                m.roughnessFactor = gltfMaterial.roughnessFactor;
                m.metallicFactor = gltfMaterial.metallicFactor;
                m.occlusionStrength = gltfMaterial.occlusionStrength;
            }
        }

        m_materialData.push_back(m);
    }

    // Create the instance buffer.
    for (int n = 0; n < kFrameCount; n++)
    {
        const UINT instanceBufferSize = sizeof(InstanceData) * kMaxInstanceCount;

        MyDx12Util::CreateUploadBuffer(m_device, instanceBufferSize, m_frameResources[n].instanceBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaxInstanceCount;
        srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);

        m_frameResources[n].instanceBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_device->CreateShaderResourceView(m_frameResources[n].instanceBuffer.Get(), &srvDesc,
                                           m_frameResources[n].instanceBufferSrv.cpu);

        m_frameResources[n].instanceBuffer->Map(0, nullptr,
                                                reinterpret_cast<void **>(&m_frameResources[n].pSrvDataBegin));
        memcpy(m_frameResources[n].pSrvDataBegin, m_instanceData.data(), instanceBufferSize);
        m_frameResources[n].instanceBuffer->Unmap(0, nullptr);
    }

    // Create SRV for material buffer (StructuredBuffer)
    {
        const UINT materialBufferSize = sizeof(Material) * kMaterialCount;

        MyDx12Util::CreateUploadBuffer(m_device, materialBufferSize, m_materialBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaterialCount;
        srvDesc.Buffer.StructureByteStride = sizeof(Material);

        m_materialBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_device->CreateShaderResourceView(m_materialBuffer.Get(), &srvDesc, m_materialBufferSrv.cpu);
        Material *pMaterialDataBegin = nullptr;
        m_materialBuffer->Map(0, nullptr, reinterpret_cast<void **>(&pMaterialDataBegin));
        memcpy(pMaterialDataBegin, m_materialData.data(), materialBufferSize);
        m_materialBuffer->Unmap(0, nullptr);
    }

    m_camerasForCPU.clear();
    {
        m_camerasForCPU.emplace_back(XMFLOAT3(0.0f, 0.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 60.0f, m_aspectRatio,
                                     0.1f, 10000.0f);
        XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(m_camerasForCPU[0].viewProjection));
        m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
        XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                        XMMatrixTranspose(XMMatrixInverse(nullptr, m_camerasForCPU[0].viewProjection)));
        m_constantBufferData.cameraPosition = m_camerasForCPU[0].pos;
    }

    // Create the per-frame constant buffers.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        CreateConstantBuffer(m_frameResources[n].cameraCB, &m_constantBufferData, sizeof(m_constantBufferData));
        CreateConstantBuffer(m_frameResources[n].lightCB, &m_lightingConstantsData, sizeof(m_lightingConstantsData));
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList *ppCommandLists[] = {m_commandList.Get()};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_frameResources[m_frameIndex].fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command
        // list in our main loop but for now, we just want to wait for setup to
        // complete before continuing.
        WaitForGpu();
    }
}

static SimpleDescriptorHeapAllocator *g_allocator = nullptr;

void D3D12HelloTexture::InitImGui()
{
#if IMGUI_IMPL > 0
    g_allocator = &m_ImGuiDescriptorHeapAllocator;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(Win32Application::GetHwnd());

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = m_device.Get();
    init_info.CommandQueue = m_commandQueue.Get();
    init_info.NumFramesInFlight = kFrameCount;
    init_info.RTVFormat = m_backBufferFormat;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate
    // more)
    init_info.SrvDescriptorHeap = m_imguiHeap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle)
    { g_allocator->Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    { g_allocator->Free(&cpu_handle, &gpu_handle); };
    ImGui_ImplDX12_Init(&init_info);
#endif
}

void D3D12HelloTexture::CreateConstantBuffer(ConstantBufferResource &constantBuffer, const void *initialData,
                                             UINT sizeInBytes)
{
    assert(sizeInBytes % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0);

    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                    D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
                                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                    IID_PPV_ARGS(&constantBuffer.buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constantBuffer.buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeInBytes;

    constantBuffer.cbv = m_descriptorHeapAllocator.AllocWithHandle();
    m_device->CreateConstantBufferView(&cbvDesc, constantBuffer.cbv.cpu);

    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(constantBuffer.buffer->Map(0, &readRange, reinterpret_cast<void **>(&constantBuffer.mappedData)));
    memcpy(constantBuffer.mappedData, initialData, sizeInBytes);
}

DescriptorHeapHandle D3D12HelloTexture::AllocateTextureSRV(ID3D12Resource *texture)
{
    DescriptorHeapHandle handle = m_descriptorHeapAllocator.AllocWithHandle();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(texture, &srvDesc, handle.cpu);

    return handle;
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> D3D12HelloTexture::GenerateCheckerboardTextureData()
{
    const UINT rowPitch = kTextureWidth * kTexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;       // The width of a cell in the checkboard texture.
    const UINT cellHeight = kTextureWidth >> 3; // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * kTextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8 *pData = &data[0];

    UINT8 R = rand_0_255();
    UINT8 G = rand_0_255();
    UINT8 B = rand_0_255();

    // DBG_PRINT("R=%d G=%d B=%d\n", R, G, B);

    for (UINT n = 0; n < textureSize; n += kTexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n + 0] = 0x00; // R
            pData[n + 1] = 0x00; // G
            pData[n + 2] = 0x00; // B
            pData[n + 3] = 0xff; // A
        }
        else
        {
            pData[n + 0] = R;    // R
            pData[n + 1] = G;    // G
            pData[n + 2] = B;    // B
            pData[n + 3] = 0xff; // A
        }
    }

    return data;
}

void D3D12HelloTexture::CreateDsvHeap()
{
    if (m_dsvHeap)
        return;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void D3D12HelloTexture::CreateGBuffer()
{
    CreateGBufferResources();
    CreateGBufferRTVs();
    CreateGBufferSRVs();
}

void D3D12HelloTexture::CreateGBufferResources()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        m_gbuffer.resources[i].Reset();

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = m_width;
        desc.Height = m_height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = m_gbuffer.formats[i];
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET, &m_gbuffer.clearValues[i], IID_PPV_ARGS(&m_gbuffer.resources[i])));
    }
}

void D3D12HelloTexture::CreateGBufferRTVs()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        m_gbuffer.rtvIndex[i] = kGBufferRTVBaseIndex + i;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_gbuffer.rtvIndex[i],
                                                m_rtvDescriptorSize);
        m_device->CreateRenderTargetView(m_gbuffer.resources[i].Get(), nullptr, rtvHandle);
    }
}

void D3D12HelloTexture::CreateGBufferSRVs()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        if (m_gbuffer.srvHandles[i].Index == UINT_MAX)
        {
            m_gbuffer.srvHandles[i] = m_descriptorHeapAllocator.AllocWithHandle();
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = m_gbuffer.formats[i];
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;

        m_device->CreateShaderResourceView(m_gbuffer.resources[i].Get(), &srvDesc, m_gbuffer.srvHandles[i].cpu);
    }

    if (m_depthStencilSrv.Index == UINT_MAX)
    {
        m_depthStencilSrv = m_descriptorHeapAllocator.AllocWithHandle();
    }
    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[GBuffer::Albedo].Index + GBuffer::kCount);

    if (m_lightPassColorSrv.Index == UINT_MAX)
    {
        m_lightPassColorSrv = m_descriptorHeapAllocator.AllocWithHandle();
    }
    assert(m_lightPassColorSrv.Index == m_depthStencilSrv.Index + 1);
}

void D3D12HelloTexture::RegisterDepthStencil(UINT width, UINT height)
{
    TransientResource r;

    r.state = TransientResourceState::Initialized;
    r.name = kDepthStencilResourceName;
    r.persistent = true;

    r.desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    r.desc.Width = width;
    r.desc.Height = height;
    r.desc.DepthOrArraySize = 1;
    r.desc.MipLevels = 1;
    r.desc.Format = DXGI_FORMAT_R32_TYPELESS;
    r.desc.SampleDesc.Count = 1;
    r.desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    r.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    r.clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    r.clearValue.DepthStencil.Depth = 1.0f;
    r.clearValue.DepthStencil.Stencil = 0;

    r.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    m_resourceRegistry.RegisterTransientResource(std::move(r));
}

void D3D12HelloTexture::RegisterLightPassRenderTarget(UINT width, UINT height)
{
    TransientResource r;

    r.state = TransientResourceState::Initialized;
    r.name = kLightPassRenderTargetResourceName;
    r.persistent = true;

    r.desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    r.desc.Width = width;
    r.desc.Height = height;
    r.desc.DepthOrArraySize = 1;
    r.desc.MipLevels = 1;
    r.desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    r.desc.SampleDesc.Count = 1;
    r.desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    r.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    r.clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    r.clearValue.Color[0] = 0.0f;
    r.clearValue.Color[1] = 0.0f;
    r.clearValue.Color[2] = 0.0f;
    r.clearValue.Color[3] = 1.0f;

    r.initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_resourceRegistry.RegisterTransientResource(std::move(r));
}

void D3D12HelloTexture::CreateDepthStencilDescriptors()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[GBuffer::Albedo].Index + GBuffer::kCount);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_depthStencil.Get(), &srvDesc, m_depthStencilSrv.cpu);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetBackBufferRtv() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetDepthDsv() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetGBufferRTV(UINT index) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(m_gbuffer.rtvIndex[index], m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::GetLightPassRTV() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(kLightPassRTVIndex, m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::ResolveRtv(RtvKey key) const
{
    if (key == m_passKeys.RtvId(RtvName::BackBuffer))
    {
        return GetBackBufferRtv();
    }
    if (key == m_passKeys.RtvId(RtvName::GBufferAlbedo))
    {
        return GetGBufferRTV(GBuffer::Albedo);
    }
    if (key == m_passKeys.RtvId(RtvName::GBufferNormal))
    {
        return GetGBufferRTV(GBuffer::Normal);
    }
    if (key == m_passKeys.RtvId(RtvName::GBufferMaterial))
    {
        return GetGBufferRTV(GBuffer::Material);
    }
    if (key == m_passKeys.RtvId(RtvName::GBufferMotionVector))
    {
        return GetGBufferRTV(GBuffer::MotionVector);
    }
    if (key == m_passKeys.RtvId(RtvName::GBufferPBRParams))
    {
        return GetGBufferRTV(GBuffer::PBRParams);
    }
    if (key == m_passKeys.RtvId(RtvName::LightPass))
    {
        return GetLightPassRTV();
    }

    assert(false && "Unsupported RTV key.");
    return {};
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12HelloTexture::ResolveDsv(DsvKey key) const
{
    if (key == m_passKeys.DsvId(DsvName::Depth))
    {
        return GetDepthDsv();
    }

    assert(false && "Unsupported DSV key.");
    return {};
}

DescriptorHeapHandle D3D12HelloTexture::ResolveDescriptor(DescriptorKey key) const
{
    if (key == m_passKeys.DescriptorId(Desc::TextureTable))
    {
        return m_textureTableStart;
    }
    if (key == m_passKeys.DescriptorId(Desc::InstanceBufferSrv))
    {
        return m_frameResources[m_frameIndex].instanceBufferSrv;
    }
    if (key == m_passKeys.DescriptorId(Desc::MaterialBufferSrv))
    {
        return m_materialBufferSrv;
    }
    if (key == m_passKeys.DescriptorId(Desc::CameraCbv))
    {
        return m_frameResources[m_frameIndex].cameraCB.cbv;
    }
    if (key == m_passKeys.DescriptorId(Desc::LightCbv))
    {
        return m_frameResources[m_frameIndex].lightCB.cbv;
    }
    if (key == m_passKeys.DescriptorId(Desc::GBufferAlbedoSrv))
    {
        return m_gbuffer.srvHandles[GBuffer::Albedo];
    }
    if (key == m_passKeys.DescriptorId(Desc::ToneMapSceneColorSrv))
    {
        return m_lightPassColorSrv;
    }

    assert(false && "Unsupported descriptor key.");
    return {};
}

void D3D12HelloTexture::CreateDepthStencil(UINT width, UINT height)
{
    // Release if DS exist
    m_depthStencil.Reset();

    // Create Depth Resource
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                    &clearValue, IID_PPV_ARGS(&m_depthStencil)));

    // Create DSV
    CreateDepthStencilDescriptors();
}

// Update frame-based values.
void D3D12HelloTexture::OnUpdate()
{
    PIXBeginEvent(0, L"OnUpdate");

    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_prevTime).count();
    static float accumTime = 0.f;
    m_prevTime = now;

    const bool updateInstanceTransforms = m_isPlaying || m_instanceTransformDirty;
    if (updateInstanceTransforms)
    {

        // InstanceBufferのmaterialのtextureIdを1秒ごと切り替える
        if (m_isPlaying)
        {
            accumTime += deltaTime;
            if (accumTime > 1.0f)
            {
                for (int i = 0; i < kMaxInstanceCount; i++)
                {
                    if constexpr (kGltfLoadingEnabled)
                    {
                        m_instanceData[i].materialId = 0; // base texture only
                        //  glTFのテクスチャ数に合わせて切り替える
                        // m_instanceData[i].materialId = (m_instanceData[i].materialId + 1) % m_gltfTextureCount;
                    }
                    else
                    {
                        // チェッカーボードテクスチャ(kTextureCount)に合わせて切り替える
                        m_instanceData[i].materialId = (m_instanceData[i].materialId + 1) % kTextureCount;
                    }
                }
                accumTime = 0.f;
            }
        }

        // InstanceBufferのオフセットを毎フレーム更新する
        for (int i = 0; i < kMaxInstanceCount; i++)
        {
            m_instanceData[i].prevWorld = m_instanceData[i].world;
            bool resetMotionVector = false;

#if 1 // cube array auto-translation
            if (m_isPlaying)
            {
                m_instanceDataForCPU[i].pos.x += kTranslationSpeed;
            }
#endif
            if (m_instanceDataForCPU[i].pos.x > kOffsetBounds)
            {
                m_instanceDataForCPU[i].pos.x = -kOffsetBounds;
                resetMotionVector = true;
            }
            if (m_isPlaying)
            {
                m_instanceDataForCPU[i].rot.x += kRotationSpeed;
                if (m_instanceDataForCPU[i].rot.x >= 2.0 * kPI)
                {
                    m_instanceDataForCPU[i].rot.x = 0.f;
                }
                m_instanceDataForCPU[i].rot.y += kRotationSpeed;
                if (m_instanceDataForCPU[i].rot.y >= 2.0 * kPI)
                {
                    m_instanceDataForCPU[i].rot.y = 0.f;
                }
                m_instanceDataForCPU[i].rot.z += kRotationSpeed;
                if (m_instanceDataForCPU[i].rot.z >= 2.0 * kPI)
                {
                    m_instanceDataForCPU[i].rot.z = 0.f;
                }
            }

            XMMATRIX scaleMat = XMMatrixScaling(m_meshScale, m_meshScale, m_meshScale);

            XMMATRIX transMat = XMMatrixTranslation(m_instanceDataForCPU[i].pos.x, m_instanceDataForCPU[i].pos.y,
                                                    m_instanceDataForCPU[i].pos.z);

            XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_instanceDataForCPU[i].rot.x, m_instanceDataForCPU[i].rot.y,
                                                           m_instanceDataForCPU[i].rot.z);
            XMMATRIX dragRotMat = XMMatrixRotationRollPitchYaw(m_dragRotation.x, m_dragRotation.y, 0.0f);

            // XMMATRIX rotMat = XMMatrixRotationZ(m_instanceDataForCPU[i].rot.z);
            XMMATRIX worldMat = scaleMat * rotMat * dragRotMat * transMat;

            // XMStoreFloat4x4(&m_instanceData[i].world, worldMat);
            XMStoreFloat4x4(&m_instanceData[i].world, XMMatrixTranspose(worldMat));
            if (resetMotionVector)
            {
                m_instanceData[i].prevWorld = m_instanceData[i].world;
            }
        }
        m_instanceTransformDirty = false;
    }
    else
    {
        for (int i = 0; i < kMaxInstanceCount; i++)
        {
            m_instanceData[i].prevWorld = m_instanceData[i].world;
        }
    }

    m_frameResources[m_frameIndex].instanceBuffer->Map(
        0, nullptr, reinterpret_cast<void **>(&m_frameResources[m_frameIndex].pSrvDataBegin));
    memcpy(m_frameResources[m_frameIndex].pSrvDataBegin, m_instanceData.data(),
           sizeof(InstanceData) * kMaxInstanceCount);
    m_frameResources[m_frameIndex].instanceBuffer->Unmap(0, nullptr);

    if (GetForegroundWindow() == Win32Application::GetHwnd())
    {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            m_isPlaying = !m_isPlaying;
            Sleep(200); // スペースキーのトグルが速すぎるのを防止
        }

        if (GetAsyncKeyState('A') & 0x8000)
        {
            m_camerasForCPU[0].pos.x -= kCameraMoveSpeed;
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            m_camerasForCPU[0].pos.x += kCameraMoveSpeed;
        }

        if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.y -= kCameraMoveSpeed;
        }
        if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.y += kCameraMoveSpeed;
        }
        if ((GetAsyncKeyState('W') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.z += kCameraMoveSpeed;
        }
        if ((GetAsyncKeyState('S') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_camerasForCPU[0].pos.z -= kCameraMoveSpeed;
        }
    }

    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
    m_camerasForCPU[0].updateAllMatrix();
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(m_camerasForCPU[0].viewProjection));
    XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                    XMMatrixTranspose(XMMatrixInverse(nullptr, m_camerasForCPU[0].viewProjection)));
    m_constantBufferData.cameraPosition = m_camerasForCPU[0].pos;
    memcpy(m_frameResources[m_frameIndex].cameraCB.mappedData, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

void D3D12HelloTexture::UpdateImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug");

    ImGui::Text("Hello ImGui");
    ImGui::Text("FrameIndex: %d", m_frameIndex);
    ImGui::SliderInt("Display Instance Count", &m_DisplayInstanceCount, 0, static_cast<int>(kMaxInstanceCount));
    m_DisplayInstanceCount = std::clamp(m_DisplayInstanceCount, 0, static_cast<int>(kMaxInstanceCount));
    ImGui::SliderFloat("Mesh Scale", &m_meshScale, 0.1f, 2.0f);
    ImGui::SliderFloat("Camera FovH", &m_camerasForCPU[0].fov, 20.f, 150.f);
    ImGui::ColorEdit4("BackBuffer Clear", m_backBufferClearColor.data());
    ImGui::SliderFloat3("Light Direction", &m_lightingConstantsData.lightDirection.x, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", &m_lightingConstantsData.lightColor.x);
    ImGui::SliderFloat("Ambient", &m_lightingConstantsData.ambientIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &m_lightingConstantsData.diffuseIntensity, 0.0f, 4.0f);
    m_lightingConstantsData.backgroundColor = {m_backBufferClearColor[0], m_backBufferClearColor[1],
                                               m_backBufferClearColor[2], m_backBufferClearColor[3]};
    memcpy(m_frameResources[m_frameIndex].lightCB.mappedData, &m_lightingConstantsData,
           sizeof(m_lightingConstantsData));

    ImGui::Text("ToneMap");
    ImGui::RadioButton("None", &m_toneMapPass.settings.operatorIndex, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Reinhard", &m_toneMapPass.settings.operatorIndex, 1);
    ImGui::SameLine();
    ImGui::RadioButton("ACES", &m_toneMapPass.settings.operatorIndex, 2);
    ImGui::SliderFloat("Exposure", &m_toneMapPass.settings.exposure, 0.0f, 4.0f);
    ImGui::SliderFloat("Paper White", &m_toneMapPass.settings.paperWhiteNits, 80.0f, 500.0f, "%.0f nits");
    ImGui::SliderFloat("Display Max", &m_toneMapPass.settings.maxDisplayNits, 100.0f, 4000.0f, "%.0f nits");
    int renderingPath = static_cast<int>(m_renderingPath);
    ImGui::Text("Rendering Path");
    ImGui::RadioButton("Forward", &renderingPath, static_cast<int>(RenderingPath::Forward));
    ImGui::SameLine();
    ImGui::RadioButton("Deferred", &renderingPath, static_cast<int>(RenderingPath::Deferred));
    m_renderingPath = static_cast<RenderingPath>(renderingPath);

    const bool deferredRendering = m_renderingPath == RenderingPath::Deferred;
    if (!deferredRendering)
    {
        m_debugViewSettings.renderViewMode = RenderViewMode::LightPass;
    }

    if (ImGui::Button("Dump HDR Buffers"))
    {
        m_debugViewSettings.requestHdrDump = true;
    }
    m_toneMapPass.settings.Normalize();

    int renderViewMode = static_cast<int>(m_debugViewSettings.renderViewMode);
    ImGui::Text("Render View");
    ImGui::BeginDisabled(!deferredRendering);
    ImGui::RadioButton("LightPass", &renderViewMode, static_cast<int>(RenderViewMode::LightPass));
    ImGui::RadioButton("Albedo", &renderViewMode, static_cast<int>(RenderViewMode::GBufferAlbedo));
    ImGui::SameLine();
    ImGui::RadioButton("Normal", &renderViewMode, static_cast<int>(RenderViewMode::GBufferNormal));
    ImGui::SameLine();
    ImGui::RadioButton("Material", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMaterial));
    //    ImGui::SameLine();
    ImGui::RadioButton("MotionVector", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMotionVector));
    ImGui::SameLine();
    ImGui::RadioButton("PBRParams", &renderViewMode, static_cast<int>(RenderViewMode::GBufferPBRParams));
    ImGui::SameLine();
    ImGui::RadioButton("Depth", &renderViewMode, static_cast<int>(RenderViewMode::Depth));
    m_debugViewSettings.renderViewMode = static_cast<RenderViewMode>(renderViewMode);
    ImGui::EndDisabled();

    const bool lightPassView = deferredRendering && m_debugViewSettings.renderViewMode == RenderViewMode::LightPass;
    ImGui::BeginDisabled(!lightPassView);
    ImGui::Checkbox("Debug LightPass Gradient", &m_lightingPassDebugGradientEnabled);
    ImGui::EndDisabled();

    ImGui::Text("CPU Frame: %.2f ms (%.1f FPS)", m_cpuFrameTime, 1000.0f / m_cpuFrameTime);

    {
        auto &gpuCeckPoints = m_frameResources[m_fremeIndexPrevious].gpuWorkMeterCheckPoints;
        size_t gpuCheckPointCount = gpuCeckPoints.size();

        if (gpuCheckPointCount >= 2)
        {

            for (int i = 1; i < gpuCheckPointCount; i++)
            {
                auto &checkPoint = gpuCeckPoints[i];

                if (i < gpuCheckPointCount - 1)
                {
                    float timeFromPrevious = checkPoint.timeStamp - gpuCeckPoints[i - 1].timeStamp;
                    ImGui::Text("GPU[%d] %s: %f ms", i, checkPoint.name.c_str(), timeFromPrevious);
                }
                else
                {
                    ImGui::Text("GPU[%d] Total: %f ms", i, checkPoint.timeStamp);
                }
            }
        }
    }
    ImGui::End();
    ImGui::Render();
}

UINT D3D12HelloTexture::GetVisibleCubeCount() const { return static_cast<UINT>(m_DisplayInstanceCount); }

// Render the scene.
void D3D12HelloTexture::OnRender()
{
    PIXBeginEvent(0, L"OnRender");

    // ImGui frame update
#if IMGUI_IMPL > 0
    UpdateImGui();
#endif

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {m_commandList.Get()};
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    if (m_debugViewSettings.hdrDumpPending)
    {
        WaitForGpu();
        PrintDebugDump();
        m_debugViewSettings.requestHdrDump = false;
        m_debugViewSettings.hdrDumpPending = false;
    }

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    UINT64 submittedFenceValue = MoveToNextFrame();

    // submittedFenceValue = 今回SubmitしたCommandListが完了したことを示すFence値
    MarkPendingTransientResources(submittedFenceValue);

    CollectGarbageTransientResources();

    m_gpuWorkMeter.ReadbackData(m_commandQueue.Get());

    PIXEndEvent();
}

void D3D12HelloTexture::OnWindowSizeChanged(UINT width, UINT height)
{
    m_pendingResize = true;
    m_pendingResizeWidth = width;
    m_pendingResizeHeight = height;
}

void D3D12HelloTexture::OnIdle()
{
    if (m_pendingResize)
    {
        Resize(m_pendingResizeWidth, m_pendingResizeHeight);
        m_pendingResize = false;
    }

    UpdateHdr10DisplayMode();

    m_workMeter.Start();
    OnUpdate();
    OnRender();
    m_workMeter.End();
    m_cpuFrameTime = m_workMeter.GetCpuFrameTimeMs();
}

void D3D12HelloTexture::OnMouseDown(UINT8 button, int x, int y)
{
    if (button != VK_LBUTTON)
    {
        return;
    }

    m_isDraggingInstance = true;
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void D3D12HelloTexture::OnMouseUp(UINT8 button, int, int)
{
    if (button == VK_LBUTTON)
    {
        m_isDraggingInstance = false;
    }
}

void D3D12HelloTexture::OnMouseMove(int x, int y)
{
    if (!m_isDraggingInstance)
    {
        return;
    }

    const int dx = x - m_lastMouseX;
    const int dy = y - m_lastMouseY;
    m_lastMouseX = x;
    m_lastMouseY = y;

    m_dragRotation.y += static_cast<float>(dx) * kMouseRotationSpeed;
    m_dragRotation.x += static_cast<float>(dy) * kMouseRotationSpeed;
    m_instanceTransformDirty = true;
}

void D3D12HelloTexture::Resize(UINT width, UINT height)
{
    DBG_PRINT("D3D12HelloTexture::OnWindowSizeChanged() %d %d\n", width, height);
    m_width = width;
    m_height = height;

    if (width == 0 || height == 0)
    {
        return;
    }

    if (m_device.Get() == nullptr || m_swapChain.Get() == nullptr)
    {
        return;
    }

    FlushGpu();

    // Clear RTV
    for (UINT n = 0; n < kFrameCount; n++)
    {
        m_renderTargets[n].Reset();
    }

    // Resize SwapChain
    m_swapChain->ResizeBuffers(kFrameCount, m_width, m_height, m_backBufferFormat, 0);
    m_hdrOutputPolicy.ReapplyColorSpace(m_swapChain.Get());

    // ★重要
    m_fremeIndexPrevious = m_frameIndex;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Re-create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    m_depthStencil.Reset();
    m_lightPassRenderTarget.Reset();
    m_resourceRegistry.UnregisterTransientResource(kDepthStencilResourceName);
    m_resourceRegistry.UnregisterTransientResource(kLightPassRenderTargetResourceName);
    RegisterDepthStencil(m_width, m_height);
    RegisterLightPassRenderTarget(m_width, m_height);
    CreateGBuffer();

    // Camera
    m_camerasForCPU[0].aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    m_camerasForCPU[0].updateAllMatrix();

    // Screen
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(m_width), static_cast<FLOAT>(m_height),
                                  D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    // Imgui
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
}

void D3D12HelloTexture::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloTexture::RegisterFullscreenPipeline(const std::string &name,
                                                   const D3D12_GRAPHICS_PIPELINE_STATE_DESC &baseDesc,
                                                   const UINT8 *vertexShader, UINT vertexShaderSize,
                                                   const UINT8 *pixelShader, UINT pixelShaderSize,
                                                   DXGI_FORMAT renderTargetFormat)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = MyDx12Util::CreateFullscreenPassPSODesc(
        baseDesc, vertexShader, vertexShaderSize, pixelShader, pixelShaderSize, renderTargetFormat);
    const PipelineKey key = PipelineId(name);
    m_pipelineRegistry.Create(m_device.Get(), key, desc);
}

void D3D12HelloTexture::RegisterMainPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC &baseDesc,
                                             const D3D12_INPUT_ELEMENT_DESC *inputLayout, UINT inputLayoutCount,
                                             const UINT8 *vertexShader, UINT vertexShaderSize,
                                             const UINT8 *pixelShader, UINT pixelShaderSize)
{
    baseDesc.InputLayout = {inputLayout, inputLayoutCount};
    baseDesc.pRootSignature = m_rootSignature.Get();
    baseDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader, vertexShaderSize);
    baseDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader, pixelShaderSize);
    baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    baseDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    baseDesc.SampleMask = UINT_MAX;
    baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    baseDesc.NumRenderTargets = 1;
    baseDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    baseDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    baseDesc.SampleDesc.Count = 1;
    const PipelineKey key = PipelineId(Pipe::Main);
    m_pipelineRegistry.Create(m_device.Get(), key, baseDesc);
}

void D3D12HelloTexture::RegisterGBufferPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC &baseDesc,
                                                const UINT8 *vertexShader, UINT vertexShaderSize,
                                                const UINT8 *pixelShader, UINT pixelShaderSize)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = MyDx12Util::CreateGBufferPSODesc(
        baseDesc, vertexShader, vertexShaderSize, pixelShader, pixelShaderSize, m_gbuffer.formats, GBuffer::kCount);
    const PipelineKey key = PipelineId(Pipe::GBuffer);
    m_pipelineRegistry.Create(m_device.Get(), key, desc);
}

void D3D12HelloTexture::RegisterDepthPrePassPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC &baseDesc,
                                                     const D3D12_INPUT_ELEMENT_DESC *inputLayout, UINT inputLayoutCount,
                                                     const UINT8 *vertexShader, UINT vertexShaderSize)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;
    desc.InputLayout = {inputLayout, inputLayoutCount};
    desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    desc.VS = CD3DX12_SHADER_BYTECODE(vertexShader, vertexShaderSize);
    desc.PS = {};
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
    desc.NumRenderTargets = 0;
    const PipelineKey key = PipelineId(Pipe::DepthPrePass);
    m_pipelineRegistry.Create(m_device.Get(), key, desc);
}

void D3D12HelloTexture::PopulateCommandList()
{
    PIXBeginEvent(1, L"PopulateCommandList");

    BeginFrame();
    ResetResourceStates();
    BuildRenderPasses();
    ValidateRenderPassGraph();
    AnalyzeResourceLifetimes();
    // DebugPrintLifetimes();
    ExecutePasses();
    EndFrame();

    PIXEndEvent();
}

void D3D12HelloTexture::BuildRenderPasses()
{
    m_renderPassGraph.Clear();
    m_passOperationHandlers.clear();

    AddPass(MakeClearPass());
    AddPass(MakeDepthPrePass());
    AddSceneRenderPasses();
    AddPass(MakeToneMapPass());

    if (m_debugViewSettings.requestHdrDump)
    {
        AddPass(MakeDebugDumpPass());
    }

    AddPass(MakeImGuiPass());
}

void D3D12HelloTexture::AddSceneRenderPasses()
{
    if (m_renderingPath == RenderingPath::Forward)
    {
        AddPass(MakeMainPass());
    }
    else
    {
        AddPass(MakeGBufferPass());
        AddDeferredSceneOutputPass();
    }
}

void D3D12HelloTexture::AddDeferredSceneOutputPass()
{
    if (m_debugViewSettings.IsGBufferDebugView())
    {
        AddPass(MakeGBufferDebugPass());
    }
    else if (m_lightingPassDebugGradientEnabled)
    {
        AddPass(MakeLightingDebugGradientPass());
    }
    else
    {
        AddPass(MakeLightingPass());
    }
}

void D3D12HelloTexture::AddPass(RenderPass pass) { m_renderPassGraph.Add(std::move(pass)); }

void D3D12HelloTexture::ValidateRenderPassGraph() const
{
    for (const RenderPass &pass : m_renderPassGraph.Passes())
    {
        assert(pass.name != nullptr && "Render pass must have a name.");
        assert((!pass.pipeline.IsValid() || GetPipelineState(pass.pipeline) != nullptr) &&
               "Render pass references an unregistered pipeline.");
        assert(m_passOperationHandlers.find(pass.operation) != m_passOperationHandlers.end() &&
               "Render pass references an unregistered operation handler.");

        for (const ResourceUsage &usage : pass.reads)
        {
            assert(!usage.name.empty() && "Render pass read usage must have a resource name.");
        }

        for (const ResourceUsage &usage : pass.writes)
        {
            assert(!usage.name.empty() && "Render pass write usage must have a resource name.");
        }

        for (const PassDescriptorBinding &binding : pass.descriptorBindings)
        {
            (void)ResolveDescriptor(binding.descriptor);
        }

        for (RtvKey rtv : pass.renderTargets.rtvs)
        {
            (void)ResolveRtv(rtv);
        }

        if (pass.renderTargets.dsv)
        {
            (void)ResolveDsv(pass.renderTargets.dsv.value());
        }

        for (const PassConstantsBinding &binding : pass.constantsBindings)
        {
            bool supported = false;
            auto toneMapConstants = m_passKeys.constants.find(ConstName::ToneMap);
            supported = supported || (toneMapConstants != m_passKeys.constants.end() &&
                                      binding.constants == toneMapConstants->second);

            auto gbufferDebugTargetConstants = m_passKeys.constants.find(ConstName::GBufferDebugTarget);
            supported = supported || (gbufferDebugTargetConstants != m_passKeys.constants.end() &&
                                      binding.constants == gbufferDebugTargetConstants->second);

            assert(supported && "Render pass references unsupported constants binding.");
        }
    }
}

auto D3D12HelloTexture::MakeResourceUsages(std::initializer_list<ResourceUsage> usages) const -> ResourceUsages
{
    return ResourceUsages(usages);
}

auto D3D12HelloTexture::MakeGBufferReadUsages() const -> ResourceUsages
{
    return MakeResourceUsages(
        {{kGBufferResourceNames[GBuffer::Albedo], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::Normal], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::Material], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::MotionVector], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::PBRParams], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kDepthStencilResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}});
}

PipelineKey D3D12HelloTexture::PipelineId(const std::string &name)
{
    return m_passKeys.RegisterPipeline(name, m_passKeyRegistry);
}

DescriptorKey D3D12HelloTexture::DescriptorId(const std::string &name)
{
    return m_passKeys.RegisterDescriptor(name, m_passKeyRegistry);
}

RtvKey D3D12HelloTexture::RtvId(const std::string &name)
{
    return m_passKeys.RegisterRtv(name, m_passKeyRegistry);
}

DsvKey D3D12HelloTexture::DsvId(const std::string &name)
{
    return m_passKeys.RegisterDsv(name, m_passKeyRegistry);
}

PassOperationKey D3D12HelloTexture::OperationId(const std::string &name)
{
    return m_passKeys.RegisterOperation(name, m_passKeyRegistry);
}

PassConstantsKey D3D12HelloTexture::ConstantsId(const std::string &name)
{
    return m_passKeys.RegisterConstants(name, m_passKeyRegistry);
}

PassOperationKey D3D12HelloTexture::RegisterPassOperation(const std::string &name, PassOperationHandler handler)
{
    const PassOperationKey key = OperationId(name);
    auto [registered, inserted] = m_passOperationHandlers.emplace(key, handler);
    assert((inserted || registered->second == handler) && "Pass operation registered with a different handler.");
    registered->second = handler;
    return key;
}

auto D3D12HelloTexture::MakeGBufferSrvBindings() -> std::vector<PassDescriptorBinding>
{
    return {{RootParam_GBufferSrvBase, DescriptorId(Desc::GBufferAlbedoSrv)}};
}

auto D3D12HelloTexture::MakeClearPass() -> RenderPass
{
    return RenderPassBuilder(L"Clear")
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Rtv(RtvId(RtvName::BackBuffer))
        .Dsv(DsvId(DsvName::Depth))
        .ClearColor(m_backBufferClearColor)
        .Operation(RegisterPassOperation(Op::Clear, &D3D12HelloTexture::ExecuteClearPass))
        .Build();
}

auto D3D12HelloTexture::MakeDepthPrePass() -> RenderPass
{
    return RenderPassBuilder(L"Depth PrePass")
        .Pipeline(PipelineId(Pipe::DepthPrePass))
        .Writes({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Descriptor(RootParam_InstanceSrv, DescriptorId(Desc::InstanceBufferSrv))
        .Descriptor(RootParam_ConstantBuffer, DescriptorId(Desc::CameraCbv))
        .Dsv(DsvId(DsvName::Depth))
        .Operation(RegisterPassOperation(Op::DepthPrePass, &D3D12HelloTexture::ExecuteDepthPrePass))
        .Build();
}

auto D3D12HelloTexture::MakeGBufferPass() -> RenderPass
{
    return RenderPassBuilder(L"GBufferPass")
        .Pipeline(PipelineId(Pipe::GBuffer))
        .Reads({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Writes({{kGBufferResourceNames[GBuffer::Albedo], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::Normal], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::Material], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::MotionVector], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::PBRParams], D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_TextureTable, DescriptorId(Desc::TextureTable))
        .Descriptor(RootParam_InstanceSrv, DescriptorId(Desc::InstanceBufferSrv))
        .Descriptor(RootParam_MaterialSrv, DescriptorId(Desc::MaterialBufferSrv))
        .Descriptor(RootParam_ConstantBuffer, DescriptorId(Desc::CameraCbv))
        .Rtvs({RtvId(RtvName::GBufferAlbedo), RtvId(RtvName::GBufferNormal), RtvId(RtvName::GBufferMaterial),
               RtvId(RtvName::GBufferMotionVector), RtvId(RtvName::GBufferPBRParams)})
        .Dsv(DsvId(DsvName::Depth))
        .Operation(RegisterPassOperation(Op::GBuffer, &D3D12HelloTexture::ExecuteGBufferPass))
        .Build();
}

auto D3D12HelloTexture::MakeMainPass() -> RenderPass
{
    return RenderPassBuilder(L"MainPass")
        .Pipeline(PipelineId(Pipe::Main))
        .Reads({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_TextureTable, DescriptorId(Desc::TextureTable))
        .Descriptor(RootParam_InstanceSrv, DescriptorId(Desc::InstanceBufferSrv))
        .Descriptor(RootParam_MaterialSrv, DescriptorId(Desc::MaterialBufferSrv))
        .Descriptor(RootParam_ConstantBuffer, DescriptorId(Desc::CameraCbv))
        .Descriptor(RootParam_LightConstants, DescriptorId(Desc::LightCbv))
        .Rtv(RtvId(RtvName::LightPass))
        .Dsv(DsvId(DsvName::Depth))
        .ClearColor({0.0f, 0.0f, 0.0f, 1.0f})
        .Operation(RegisterPassOperation(Op::Main, &D3D12HelloTexture::ExecuteMainPass))
        .Build();
}

auto D3D12HelloTexture::MakeLightingPass() -> RenderPass
{
    return RenderPassBuilder(L"LightPass")
        .Pipeline(PipelineId(Pipe::Lighting))
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_GBufferSrvBase, DescriptorId(Desc::GBufferAlbedoSrv))
        .Descriptor(RootParam_MaterialSrv, DescriptorId(Desc::MaterialBufferSrv))
        .Descriptor(RootParam_ConstantBuffer, DescriptorId(Desc::CameraCbv))
        .Descriptor(RootParam_LightConstants, DescriptorId(Desc::LightCbv))
        .Rtv(RtvId(RtvName::LightPass))
        .Operation(RegisterPassOperation(Op::Lighting, &D3D12HelloTexture::ExecuteLightingPass))
        .Build();
}

auto D3D12HelloTexture::MakeLightingDebugGradientPass() -> RenderPass
{
    return RenderPassBuilder(L"LightPassDebugGradient")
        .Pipeline(PipelineId(Pipe::LightingDebugGradient))
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_GBufferSrvBase, DescriptorId(Desc::GBufferAlbedoSrv))
        .Descriptor(RootParam_MaterialSrv, DescriptorId(Desc::MaterialBufferSrv))
        .Descriptor(RootParam_ConstantBuffer, DescriptorId(Desc::CameraCbv))
        .Descriptor(RootParam_LightConstants, DescriptorId(Desc::LightCbv))
        .Rtv(RtvId(RtvName::LightPass))
        .Operation(RegisterPassOperation(Op::LightingDebugGradient,
                                         &D3D12HelloTexture::ExecuteLightingDebugGradientPass))
        .Constants(RootParam_ToneMapConstants, ConstantsId(ConstName::ToneMap))
        .Build();
}

auto D3D12HelloTexture::MakeToneMapPass() -> RenderPass
{
    return RenderPassBuilder(L"ToneMapPass")
        .Pipeline(PipelineId(Pipe::ToneMap))
        .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}})
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_ToneMapSceneColor, DescriptorId(Desc::ToneMapSceneColorSrv))
        .Rtv(RtvId(RtvName::BackBuffer))
        .Operation(RegisterPassOperation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass))
        .Constants(RootParam_ToneMapConstants, ConstantsId(ConstName::ToneMap))
        .Build();
}

auto D3D12HelloTexture::MakeDebugDumpPass() -> RenderPass
{
    return RenderPassBuilder(L"DebugDump")
        .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_COPY_SOURCE},
                {kBackBufferResourceName, D3D12_RESOURCE_STATE_COPY_SOURCE}})
        .Operation(RegisterPassOperation(Op::DebugDump, &D3D12HelloTexture::ExecuteDebugDumpPass))
        .Build();
}

auto D3D12HelloTexture::MakeGBufferDebugPass() -> RenderPass
{
    return RenderPassBuilder(L"GBufferDebugPass")
        .Pipeline(PipelineId(Pipe::GBufferDebug))
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptors(MakeGBufferSrvBindings())
        .Rtv(RtvId(RtvName::LightPass))
        .Operation(RegisterPassOperation(Op::GBufferDebug, &D3D12HelloTexture::ExecuteGBufferDebugPass))
        .Constants(RootParam_GBufferDebugConstants, ConstantsId(ConstName::GBufferDebugTarget))
        .Build();
}

auto D3D12HelloTexture::MakeImGuiPass() -> RenderPass
{
    return RenderPassBuilder(L"ImGui")
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Rtv(RtvId(RtvName::BackBuffer))
        .Operation(RegisterPassOperation(Op::ImGui, &D3D12HelloTexture::ExecuteImGuiPass))
        .Build();
}

void D3D12HelloTexture::AnalyzeResourceLifetimes()
{
    m_resourceRegistry.AnalyzeLifetimes(m_renderPassGraph.Passes());
}

void D3D12HelloTexture::DebugPrintLifetimes()
{
    DBG_PRINT("Resource Lifetimes:\n");
    for (auto &[name, lt] : m_resourceRegistry.lifetimes)
    {
        DBG_PRINT("Resource %s: [%d - %d]\n", name.c_str(), lt.firstPass, lt.lastPass);
    }
}

void D3D12HelloTexture::BindPassDescriptors(const RenderPass &pass)
{
    for (const auto &binding : pass.descriptorBindings)
    {
        m_commandList->SetGraphicsRootDescriptorTable(binding.rootParameterIndex,
                                                      ResolveDescriptor(binding.descriptor).gpu);
    }
}

void D3D12HelloTexture::BindPassRenderTargets(const RenderPass &pass)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
    rtvs.reserve(pass.renderTargets.rtvs.size());
    for (RtvKey rtv : pass.renderTargets.rtvs)
    {
        rtvs.push_back(ResolveRtv(rtv));
    }

    std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv;
    if (pass.renderTargets.dsv)
    {
        dsv = ResolveDsv(pass.renderTargets.dsv.value());
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE *rtvHandles = rtvs.empty() ? nullptr : rtvs.data();
    const D3D12_CPU_DESCRIPTOR_HANDLE *dsvHandle = dsv ? &dsv.value() : nullptr;

    m_commandList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvHandles, FALSE, dsvHandle);
}

ID3D12PipelineState *D3D12HelloTexture::GetPipelineState(PipelineKey pipeline) const
{
    return m_pipelineRegistry.Find(pipeline);
}

void D3D12HelloTexture::BindPassPipeline(const RenderPass &pass)
{
    ID3D12PipelineState *pipelineState = GetPipelineState(pass.pipeline);
    assert(!pass.pipeline.IsValid() || pipelineState != nullptr);
    if (pipelineState != nullptr)
    {
        m_commandList->SetPipelineState(pipelineState);
    }
}

void D3D12HelloTexture::BindPassConstants(const RenderPass &pass)
{
    for (const auto &binding : pass.constantsBindings)
    {
        if (binding.constants == m_passKeys.ConstantsId(ConstName::ToneMap))
        {
            const auto constants = m_toneMapPass.MakeShaderConstants(m_hdrOutputPolicy.settings);
            m_commandList->SetGraphicsRoot32BitConstants(binding.rootParameterIndex, 5, &constants, 0);
            continue;
        }
        if (binding.constants == m_passKeys.ConstantsId(ConstName::GBufferDebugTarget))
        {
            const UINT debugTarget = m_debugViewSettings.GetGBufferDebugTarget();
            m_commandList->SetGraphicsRoot32BitConstants(binding.rootParameterIndex, 1, &debugTarget, 0);
            continue;
        }

        assert(false && "Unsupported pass constants binding.");
    }
}

void D3D12HelloTexture::ExecutePasses()
{
    for (int passIndex = 0; passIndex < static_cast<int>(m_renderPassGraph.Size()); ++passIndex)
    {
        ExecutePass(passIndex);
    }
}

void D3D12HelloTexture::ExecutePass(int passIndex)
{
    CreateResourcesForPass(passIndex);

    const RenderPass &pass = m_renderPassGraph[passIndex];
    TransitionPassResources(pass);
    BindPassRenderTargets(pass);
    BindPassDescriptors(pass);
    BindPassPipeline(pass);
    BindPassConstants(pass);
    ExecutePassOperation(pass);

    ReleaseResourcesAfterPass(passIndex);
}

void D3D12HelloTexture::ExecutePassOperation(const RenderPass &pass)
{
    auto handler = m_passOperationHandlers.find(pass.operation);
    assert(handler != m_passOperationHandlers.end() && "Unsupported pass operation.");
    if (handler != m_passOperationHandlers.end())
    {
        (this->*handler->second)(pass);
    }
}

void D3D12HelloTexture::CreateResourcesForPass(int passIndex)
{
    const std::vector<std::string> resourceNames =
        m_resourceRegistry.GetResourcesStartingAtPass(passIndex, kBackBufferResourceName);

    for (const std::string &name : resourceNames)
    {
        TransientResource *transientResource = m_resourceRegistry.PrepareTransientResourceForCreate(name);
        if (transientResource == nullptr)
            continue;
        auto &tr = *transientResource;

        CreateCommittedTransientResource(tr);
        m_resourceRegistry.MarkTransientResourceCreated(name);
        BindCreatedTransientResource(name, tr.resource.Get());

        DBG_PRINT("Resource %s created.\n", name.c_str());
    }
}

void D3D12HelloTexture::CreateCommittedTransientResource(TransientResource &resource)
{
    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &resource.desc, resource.initialState,
                                                    &resource.clearValue, IID_PPV_ARGS(&resource.resource)));
}

void D3D12HelloTexture::BindCreatedTransientResource(const std::string &name, ID3D12Resource *resource)
{
    if (name == kDepthStencilResourceName)
    {
        m_depthStencil = resource;
        CreateDepthStencilDescriptors();
        return;
    }

    if (name == kLightPassRenderTargetResourceName)
    {
        m_lightPassRenderTarget = resource;
        CreateLightPassRenderTargetDescriptors();
        return;
    }

    assert(false && "Unsupported resource in BindCreatedTransientResource()");
}

void D3D12HelloTexture::CreateLightPassRenderTargetDescriptors()
{
    m_device->CreateRenderTargetView(m_lightPassRenderTarget.Get(), nullptr, GetLightPassRTV());

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_lightPassRenderTarget.Get(), &srvDesc, m_lightPassColorSrv.cpu);
}

void D3D12HelloTexture::ReleaseResourcesAfterPass(int passIndex)
{
    m_resourceRegistry.MarkEndOfLifeResources(passIndex, kBackBufferResourceName);
}

void D3D12HelloTexture::MarkPendingTransientResources(UINT64 fenceValue)
{
    m_resourceRegistry.MarkPendingTransientResources(fenceValue);
}

void D3D12HelloTexture::CollectGarbageTransientResources()
{
    const UINT64 completed = m_fence->GetCompletedValue();
    const std::vector<std::string> releasedResources = m_resourceRegistry.CollectGarbageTransientResources(completed);

    for (const std::string &name : releasedResources)
    {
        if (name == kDepthStencilResourceName)
        {
            m_depthStencil.Reset();
        }
    }
}

void D3D12HelloTexture::ResetResourceStates()
{
    m_resourceRegistry.ResetStates({{kBackBufferResourceName, D3D12_RESOURCE_STATE_PRESENT},
                                    {kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE},
                                    {kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}});
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        SetResourceState(kGBufferResourceNames[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}

void D3D12HelloTexture::TransitionPassResources(const RenderPass &pass)
{
    pass.ForEachResourceUsage([this](const ResourceUsage &usage) { TransitionResource(usage); });
}

void D3D12HelloTexture::TransitionResource(const ResourceUsage &usage)
{
    D3D12_RESOURCE_STATES currentState = GetResourceState(usage.name);
    if (currentState == usage.state)
    {
        return;
    }

    ID3D12Resource *resource = ResolveResource(usage.name);

    assert(resource != nullptr && "Cannot transition a null resource.");
    if (resource == nullptr)
    {
        DBG_PRINT("Resource %s is null. Skip transition.\n", usage.name.c_str());
        return;
    }

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, currentState, usage.state));
    SetResourceState(usage.name, usage.state);
}

ID3D12Resource *D3D12HelloTexture::ResolveResource(const std::string &name) const
{
    if (name == kBackBufferResourceName)
    {
        return m_renderTargets[m_frameIndex].Get();
    }

    if (name == kDepthStencilResourceName)
    {
        return m_depthStencil.Get();
    }

    if (name == kLightPassRenderTargetResourceName)
    {
        return m_lightPassRenderTarget.Get();
    }

    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        if (name == kGBufferResourceNames[i])
        {
            return m_gbuffer.resources[i].Get();
        }
    }

    return m_resourceRegistry.FindTransientD3DResource(name);
}

D3D12_RESOURCE_STATES D3D12HelloTexture::GetResourceState(const std::string &name) const
{
    return m_resourceRegistry.GetState(name);
}

void D3D12HelloTexture::SetResourceState(const std::string &name, D3D12_RESOURCE_STATES state)
{
    m_resourceRegistry.SetState(name, state);
}

void D3D12HelloTexture::BeginFrame()
{

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_frameResources[m_frameIndex].commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_frameIndex].commandAllocator.Get(),
                                       GetPipelineState(PipelineId(Pipe::Main))));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap *ppHeaps[] = {m_heap.Get()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    m_gpuWorkMeter.StartGpu(m_commandList.Get(), m_frameResources[m_frameIndex].gpuWorkMeterCheckPoints);
}

void D3D12HelloTexture::ExecuteClearPass(const RenderPass &pass) { RecordClear(pass.renderTargets); }

void D3D12HelloTexture::ExecuteDepthPrePass(const RenderPass &pass) { RecordDepthPrePass(); }

void D3D12HelloTexture::ExecuteGBufferPass(const RenderPass &pass) { RecordGBufferPass(pass.renderTargets); }

void D3D12HelloTexture::ExecuteMainPass(const RenderPass &pass) { RecordMainPass(pass.renderTargets); }

void D3D12HelloTexture::ExecuteLightingPass(const RenderPass &pass) { RecordLightPass(); }

void D3D12HelloTexture::ExecuteLightingDebugGradientPass(const RenderPass &pass) { RecordLightPassDebugGradient(); }

void D3D12HelloTexture::ExecuteToneMapPass(const RenderPass &pass) { RecordToneMapPass(); }

void D3D12HelloTexture::ExecuteDebugDumpPass(const RenderPass &pass) { RecordDebugDumpPass(); }

void D3D12HelloTexture::ExecuteGBufferDebugPass(const RenderPass &pass) { RecordGBufferDebugPass(); }

void D3D12HelloTexture::ExecuteImGuiPass(const RenderPass &pass) { RecordImGuiPass(); }

void D3D12HelloTexture::RecordClear(const PassRenderTargetBinding &renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"ClearPrepass");
    assert(!renderTargets.rtvs.empty());
    assert(renderTargets.dsv.has_value());
    assert(renderTargets.clearColor.has_value());

    for (RtvKey rtv : renderTargets.rtvs)
    {
        m_commandList->ClearRenderTargetView(ResolveRtv(rtv), renderTargets.clearColor->data(), 0, nullptr);
    }
    m_commandList->ClearDepthStencilView(ResolveDsv(renderTargets.dsv.value()), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0,
                                         nullptr);

    PIXEndEvent(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Clear");
}

void D3D12HelloTexture::DrawInstanceWrapper(UINT instanceCount)
{
    if (kGltfLoadingEnabled && kGltfMeshDisplay)
    {
        m_commandList->IASetIndexBuffer(&m_indexBufferView);
        m_commandList->DrawIndexedInstanced(m_indexCountPerInstance, instanceCount, 0, 0, 0);
    }
    else
    {
        m_commandList->DrawInstanced(m_vertexCountPerInstance, instanceCount, 0, 0);
    }
}

void D3D12HelloTexture::DrawFullscreenTriangle()
{
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawInstanced(3, 1, 0, 0);
}

//
// Depth Pre-pass
//
void D3D12HelloTexture::RecordDepthPrePass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"DepthPrepass");
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Depth Prepass");
}

void D3D12HelloTexture::RecordGBufferPass(const PassRenderTargetBinding &renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"GBufferPass");

    for (UINT i = 0; i < static_cast<UINT>(renderTargets.rtvs.size()); ++i)
    {
        m_commandList->ClearRenderTargetView(ResolveRtv(renderTargets.rtvs[i]), m_gbuffer.clearValues[i].Color, 0,
                                             nullptr);
    }

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Pass");
}

void D3D12HelloTexture::RecordGBufferDebugPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"GBufferDebugPass");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Debug Pass");
}

void D3D12HelloTexture::RecordLightPassDebugGradient()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"RecordLightPassDebugGradient");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "LightPassDebugGradient Pass");
}

void D3D12HelloTexture::RecordLightPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"LightPass");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Lighting Pass");
}

void D3D12HelloTexture::RecordToneMapPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"ToneMapPass");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ToneMap Pass");
}

void D3D12HelloTexture::CreateDebugDumpReadback(ID3D12Resource *source, ComPtr<ID3D12Resource> &readback,
                                                D3D12_PLACED_SUBRESOURCE_FOOTPRINT &layout)
{
    D3D12_RESOURCE_DESC desc = source->GetDesc();
    UINT numRows = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes = 0;
    m_device->GetCopyableFootprints(&desc, 0, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

    ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
                                                    D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(totalBytes),
                                                    D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)));
}

void D3D12HelloTexture::RecordDebugDumpPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"DebugDump");

    CreateDebugDumpReadback(m_lightPassRenderTarget.Get(), m_lightPassDebugDumpReadback, m_lightPassDebugDumpLayout);
    CreateDebugDumpReadback(m_renderTargets[m_frameIndex].Get(), m_backBufferDebugDumpReadback,
                            m_backBufferDebugDumpLayout);

    CD3DX12_TEXTURE_COPY_LOCATION lightDst(m_lightPassDebugDumpReadback.Get(), m_lightPassDebugDumpLayout);
    CD3DX12_TEXTURE_COPY_LOCATION lightSrc(m_lightPassRenderTarget.Get(), 0);
    m_commandList->CopyTextureRegion(&lightDst, 0, 0, 0, &lightSrc, nullptr);

    CD3DX12_TEXTURE_COPY_LOCATION backBufferDst(m_backBufferDebugDumpReadback.Get(), m_backBufferDebugDumpLayout);
    CD3DX12_TEXTURE_COPY_LOCATION backBufferSrc(m_renderTargets[m_frameIndex].Get(), 0);
    m_commandList->CopyTextureRegion(&backBufferDst, 0, 0, 0, &backBufferSrc, nullptr);

    m_debugViewSettings.hdrDumpPending = true;

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Debug Dump");
}

void D3D12HelloTexture::PrintDebugDump()
{
    if (!m_lightPassDebugDumpReadback || !m_backBufferDebugDumpReadback)
    {
        return;
    }

    UINT8 *lightData = nullptr;
    UINT8 *backBufferData = nullptr;
    const D3D12_RANGE lightReadRange = {0, static_cast<SIZE_T>(m_lightPassDebugDumpReadback->GetDesc().Width)};
    const D3D12_RANGE backBufferReadRange = {0, static_cast<SIZE_T>(m_backBufferDebugDumpReadback->GetDesc().Width)};
    ThrowIfFailed(m_lightPassDebugDumpReadback->Map(0, &lightReadRange, reinterpret_cast<void **>(&lightData)));
    ThrowIfFailed(
        m_backBufferDebugDumpReadback->Map(0, &backBufferReadRange, reinterpret_cast<void **>(&backBufferData)));

    const UINT lightWidth = static_cast<UINT>(m_lightPassDebugDumpLayout.Footprint.Width);
    const UINT lightHeight = m_lightPassDebugDumpLayout.Footprint.Height;
    const UINT backBufferWidth = static_cast<UINT>(m_backBufferDebugDumpLayout.Footprint.Width);
    const UINT backBufferHeight = m_backBufferDebugDumpLayout.Footprint.Height;
    const UINT sampleYs[] = {lightHeight > 0 ? lightHeight / 4 : 0, lightHeight > 0 ? (lightHeight * 3) / 4 : 0};
    const char *bandNames[] = {"SDR[0,1]", "HDR[0,9]"};
    const UINT bandCount = m_lightingPassDebugGradientEnabled ? 2 : 1;
    const UINT sampleXs[] = {0, lightWidth / 4, lightWidth / 2, lightWidth > 0 ? lightWidth - 1 : 0};
    const char *sampleNames[] = {"left", "25%", "50%", "right"};

    DebugPrint("HDR DebugDump: LightPass=%ux%u BackBuffer=%ux%u hdr10=%d gradient=%d toneMap=%d exposure=%.3f "
               "paperWhite=%.1f maxDisplay=%.1f\n",
               lightWidth, lightHeight, backBufferWidth, backBufferHeight,
               m_hdrOutputPolicy.settings.hdr10Enabled ? 1 : 0, m_lightingPassDebugGradientEnabled ? 1 : 0,
               m_toneMapPass.settings.operatorIndex, m_toneMapPass.settings.exposure,
               m_toneMapPass.settings.paperWhiteNits, m_toneMapPass.settings.maxDisplayNits);
    if (m_lightingPassDebugGradientEnabled)
    {
        const float displayMaxSceneLinear =
            m_toneMapPass.settings.maxDisplayNits / (std::max)(m_toneMapPass.settings.paperWhiteNits, 1.0f);
        const float displayMaxMarkerX = std::pow((std::clamp)(displayMaxSceneLinear / 9.0f, 0.0f, 1.0f), 1.0f / 2.2f);
        DebugPrint("  HDR[0,9] display-max marker: sceneLinear=%.4f nits=%.1f x=%.4f%s\n", displayMaxSceneLinear,
                   m_toneMapPass.settings.maxDisplayNits, displayMaxMarkerX,
                   displayMaxSceneLinear >= 9.0f ? " (outside ramp)" : "");
    }

    for (UINT band = 0; band < bandCount; ++band)
    {
        const UINT sampleY =
            m_lightingPassDebugGradientEnabled ? sampleYs[band] : (lightHeight > 0 ? lightHeight / 2 : 0);
        const char *bandName = m_lightingPassDebugGradientEnabled ? bandNames[band] : "Scene";

        for (UINT i = 0; i < _countof(sampleXs); ++i)
        {
            const UINT lightX = (std::min)(sampleXs[i], lightWidth > 0 ? lightWidth - 1 : 0);
            const UINT backBufferX = backBufferWidth > 0 ? (std::min)(lightX, backBufferWidth - 1) : 0;
            const UINT backBufferY = backBufferHeight > 0 ? (std::min)(sampleY, backBufferHeight - 1) : 0;

            const UINT8 *lightRow = lightData + m_lightPassDebugDumpLayout.Offset +
                                    static_cast<size_t>(sampleY) * m_lightPassDebugDumpLayout.Footprint.RowPitch;
            const UINT16 *lightHalf = reinterpret_cast<const UINT16 *>(lightRow + static_cast<size_t>(lightX) * 8);
            const float lightR = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[0]);
            const float lightG = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[1]);
            const float lightB = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[2]);
            const float lightA = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[3]);

            const UINT8 *backBufferRow =
                backBufferData + m_backBufferDebugDumpLayout.Offset +
                static_cast<size_t>(backBufferY) * m_backBufferDebugDumpLayout.Footprint.RowPitch;
            const UINT backBufferRaw =
                *reinterpret_cast<const UINT *>(backBufferRow + static_cast<size_t>(backBufferX) * 4);
            const float outR = static_cast<float>(backBufferRaw & 0x3ff) / 1023.0f;
            const float outG = static_cast<float>((backBufferRaw >> 10) & 0x3ff) / 1023.0f;
            const float outB = static_cast<float>((backBufferRaw >> 20) & 0x3ff) / 1023.0f;
            const float outA = static_cast<float>((backBufferRaw >> 30) & 0x3) / 3.0f;

            const float expectedU = lightWidth > 0 ? (static_cast<float>(lightX) + 0.5f) / lightWidth : 0.0f;
            const float expectedV = lightHeight > 0 ? (static_cast<float>(sampleY) + 0.5f) / lightHeight : 0.0f;
            DebugPrint("  %s %s x=%u y=%u LightPass RGBA=(%.4f, %.4f, %.4f, %.4f)\n", bandName, sampleNames[i], lightX,
                       sampleY, lightR, lightG, lightB, lightA);

            if (m_lightingPassDebugGradientEnabled)
            {
                const float expectedRampInput = (std::clamp)(expectedU, 0.0f, 1.0f);
                const float expectedMaxLinear = expectedV < 0.5f ? 1.0f : 9.0f;
                const float expectedPerceptualMax = std::pow(expectedMaxLinear, 1.0f / 2.2f);
                const float expectedPerceptualValue = expectedRampInput * expectedPerceptualMax;
                const float expectedSceneLinear = std::pow(expectedPerceptualValue, 2.2f);
                DebugPrint("  %s %s expected uv=(%.4f, %.4f) perceptual=(%.4f/%.4f) sceneLinear=%.4f "
                           "nits=%.1f\n",
                           bandName, sampleNames[i], expectedU, expectedV, expectedPerceptualValue,
                           expectedPerceptualMax, expectedSceneLinear,
                           expectedSceneLinear * m_toneMapPass.settings.paperWhiteNits);
            }

            if (m_hdrOutputPolicy.settings.hdr10Enabled)
            {
                DebugPrint("  %s %s x=%u y=%u BackBuffer R10G10B10A2=(%.4f, %.4f, %.4f, %.4f) raw=0x%08x "
                           "PQ-nits=(%.1f, %.1f, %.1f)\n",
                           bandName, sampleNames[i], backBufferX, backBufferY, outR, outG, outB, outA, backBufferRaw,
                           St2084PqToNits(outR), St2084PqToNits(outG), St2084PqToNits(outB));
            }
            else
            {
                DebugPrint("  %s %s x=%u y=%u BackBuffer R10G10B10A2=(%.4f, %.4f, %.4f, %.4f) raw=0x%08x "
                           "SDR-linear=(%.4f, %.4f, %.4f) SDR-nits=(%.1f, %.1f, %.1f)\n",
                           bandName, sampleNames[i], backBufferX, backBufferY, outR, outG, outB, outA, backBufferRaw,
                           SrgbToLinear(outR), SrgbToLinear(outG), SrgbToLinear(outB),
                           SrgbToLinear(outR) * m_toneMapPass.settings.paperWhiteNits,
                           SrgbToLinear(outG) * m_toneMapPass.settings.paperWhiteNits,
                           SrgbToLinear(outB) * m_toneMapPass.settings.paperWhiteNits);
            }
        }
    }

    const D3D12_RANGE writtenRange = {0, 0};
    m_lightPassDebugDumpReadback->Unmap(0, &writtenRange);
    m_backBufferDebugDumpReadback->Unmap(0, &writtenRange);
}

//
// Main Pass
//
void D3D12HelloTexture::RecordMainPass(const PassRenderTargetBinding &renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"MainPass");
    if (renderTargets.clearColor)
    {
        for (RtvKey rtv : renderTargets.rtvs)
        {
            m_commandList->ClearRenderTargetView(ResolveRtv(rtv), renderTargets.clearColor->data(), 0, nullptr);
        }
    }

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Main Pass");
}

void D3D12HelloTexture::RecordImGuiPass()
{
#if IMGUI_IMPL > 0
    {
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        ID3D12DescriptorHeap *imguiHeaps[] = {m_imguiHeap.Get()};

        m_commandList->SetDescriptorHeaps(1, imguiHeaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
    }
#endif
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ImGUI");
}

void D3D12HelloTexture::EndFrame()
{
    m_gpuWorkMeter.EndGpu(m_commandList.Get());

    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        TransitionResource({kGBufferResourceNames[i], D3D12_RESOURCE_STATE_RENDER_TARGET});
    }
    TransitionResource({kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE});

    // TODO: TransientResource refactor:
    // This manual restore keeps ResetResourceStates() consistent with the actual resource state.
    // Move next-frame/start-state handling into the transient resource or render graph metadata.
    TransitionResource({kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET});

    TransitionResource({kBackBufferResourceName, D3D12_RESOURCE_STATE_PRESENT});

    ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void D3D12HelloTexture::WaitForGpu()
{
    PIXBeginEvent(3, L"WaitForGpu");

    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_frameResources[m_frameIndex].fenceValue));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_frameResources[m_frameIndex].fenceValue, m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_frameResources[m_frameIndex].fenceValue++;

    PIXEndEvent();
}

void D3D12HelloTexture::FlushGpu()
{
    for (UINT n = 0; n < kFrameCount; n++)
    {
        const UINT64 fenceValue = ++m_frameResources[n].fenceValue;

        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValue));

        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));

        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

// Prepare to render the next frame.
UINT64 D3D12HelloTexture::MoveToNextFrame()
{
    PIXBeginEvent(2, L"MoveToNextFrame");

    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_frameResources[m_frameIndex].fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_fremeIndexPrevious = m_frameIndex;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_frameResources[m_frameIndex].fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_frameResources[m_frameIndex].fenceValue, m_fenceEvent));
        PIXBeginEvent(4, L"WaitForSingleObjectEx");
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
        PIXEndEvent();
    }

    // Set the fence value for the next frame.
    m_frameResources[m_frameIndex].fenceValue = currentFenceValue + 1;

    PIXEndEvent();

    return currentFenceValue;
}
