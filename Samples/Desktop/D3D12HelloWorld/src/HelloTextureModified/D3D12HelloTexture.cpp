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

int ComputeIntersectionArea(const RECT& a, const RECT& b)
{
    return max(0L, min(a.right, b.right) - max(a.left, b.left)) * max(0L, min(a.bottom, b.bottom) - max(a.top, b.top));
}

UINT16 Hdr10Chromaticity(float value)
{
    return static_cast<UINT16>(value * 50000.0f + 0.5f);
}

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

static void GetHardwareAdapter(IDXGIFactory1* pFactory,
                               IDXGIAdapter1** ppAdapter,
                               bool requestHighPerformanceAdapter = false)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (UINT adapterIndex = 0;
             SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                 adapterIndex,
                 requestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                               : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                 IID_PPV_ARGS(&adapter)));
             ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 618;
}
extern "C"
{
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

void GraphicsDevice::Initialize(const GraphicsDeviceDesc& desc)
{
    hwnd = desc.hwnd;
    width = desc.width;
    height = desc.height;

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    if (desc.useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(dxgiFactory.Get(), &hardwareAdapter);
        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = desc.bufferCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = desc.swapChainFormat;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    ThrowIfFailed(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&swapChain));
}

bool GraphicsDevice::HasSwapChain() const
{
    return device.Get() != nullptr && swapChain.Get() != nullptr;
}

UINT GraphicsDevice::CurrentBackBufferIndex() const
{
    return swapChain->GetCurrentBackBufferIndex();
}

void GraphicsDevice::GetBackBuffer(UINT index, REFIID riid, void** resource) const
{
    ThrowIfFailed(swapChain->GetBuffer(index, riid, resource));
}

void GraphicsDevice::ExecuteCommandLists(UINT commandListCount, ID3D12CommandList* const* commandLists)
{
    commandQueue->ExecuteCommandLists(commandListCount, commandLists);
}

void GraphicsDevice::CreateFence(UINT64 initialValue)
{
    ThrowIfFailed(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void GraphicsDevice::SignalFence(UINT64 value)
{
    ThrowIfFailed(commandQueue->Signal(fence.Get(), value));
}

UINT64 GraphicsDevice::CompletedFenceValue() const
{
    return fence->GetCompletedValue();
}

void GraphicsDevice::WaitForFenceValue(UINT64 value)
{
    ThrowIfFailed(fence->SetEventOnCompletion(value, fenceEvent));
    WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
}

void GraphicsDevice::CloseFenceEvent()
{
    if (fenceEvent != nullptr)
    {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
}

void GraphicsDevice::Present(UINT syncInterval, UINT flags)
{
    ThrowIfFailed(swapChain->Present(syncInterval, flags));
}

void GraphicsDevice::ResizeSwapChain(UINT bufferCount, UINT newWidth, UINT newHeight, DXGI_FORMAT format, UINT flags)
{
    width = newWidth;
    height = newHeight;
    ThrowIfFailed(swapChain->ResizeBuffers(bufferCount, width, height, format, flags));
}

HelloTextureEngine::HelloTextureEngine(UINT width, UINT height, GraphicsDevice& graphicsDevice)
    : m_graphicsDevice(graphicsDevice), m_width(width), m_height(height),
      m_aspectRatio(static_cast<float>(width) / static_cast<float>(height)), m_previousFrameIndex(0),
      m_currentFrameIndex(0), m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
      m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)), m_rtvDescriptorSize(0),
      m_descriptorSize(0)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;

    RegisterPassBindingResolvers();
    RegisterPassConstantsHandlers();
    RegisterResourceResolvers();
}

auto HelloTextureEngine::ToneMapPass::MakeShaderConstants(const HdrOutputSettings& hdrOutputSettings) const
    -> ToneMapSettings::ShaderConstants
{
    return settings.MakeShaderConstants(hdrOutputSettings.TransferFunction());
}

void HelloTextureEngine::Initialize(const EngineInitDesc& desc)
{
    assert(m_graphicsDevice.device != nullptr);
    assert(m_graphicsDevice.HasSwapChain());

    m_width = desc.width;
    m_height = desc.height;
    m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    InitializeFrameResources();
}

void HelloTextureEngine::InitializeFrameResources()
{
    m_prevTime = std::chrono::steady_clock::now();
    LoadPipeline();
    LoadAssets();
    InitImGui();
}

auto HelloTextureEngine::MakeGraphicsDeviceContext() const -> GraphicsDeviceContext
{
    return {
        m_graphicsDevice.device.Get(),
        m_graphicsDevice.commandQueue.Get(),
        m_graphicsDevice.hwnd,
    };
}

std::wstring HelloTextureEngine::GetAssetFullPath(LPCWSTR assetName)
{
    return m_assetsPath + assetName;
}

void HelloTextureEngine::SetSceneMesh(const GltfMeshData* mesh)
{
    m_sceneMesh = mesh;
}

void HelloTextureEngine::SetDebugUiHandler(DebugUiHandler handler)
{
    m_debugUiHandler = std::move(handler);
}

void HelloTextureEngine::SetUpdateHandler(UpdateHandler handler)
{
    m_updateHandler = std::move(handler);
}

void HelloTextureEngine::SetLightingParams(const LightingParams& params)
{
    m_lightingParams = params;
}

auto HelloTextureEngine::MakeLightingConstants() const -> LightingConstants
{
    return {
        m_lightingParams.lightDirection,
        m_lightingParams.ambientIntensity,
        m_lightingParams.lightColor,
        m_lightingParams.diffuseIntensity,
        {m_backBufferClearColor[0], m_backBufferClearColor[1], m_backBufferClearColor[2], m_backBufferClearColor[3]},
    };
}

void HelloTextureEngine::SetRenderingPath(RenderingPath renderingPath)
{
    m_renderingPath = renderingPath;
}

void HelloTextureEngine::SetLightingPassDebugGradient(bool enabled)
{
    m_lightingPassDebugGradientEnabled = enabled;
}

void HelloTextureEngine::SetBackBufferClearColor(const std::array<float, 4>& color)
{
    m_backBufferClearColor = color;
}

void HelloTextureEngine::SetCameraState(const CameraState& camera)
{
    m_camera = camera;
}

void HelloTextureEngine::SetInstanceData(const std::vector<InstanceData>& instanceData)
{
    m_instanceData = instanceData;
}

void HelloTextureEngine::SetDisplayInstanceCount(int count)
{
    m_displayInstanceCount = std::clamp(count, 0, static_cast<int>(kMaxInstanceCount));
}

void HelloTextureEngine::SetToneMapParams(const ToneMapParams& params)
{
    m_toneMapPass.settings.operatorIndex = params.operatorIndex;
    m_toneMapPass.settings.exposure = params.exposure;
    m_toneMapPass.settings.paperWhiteNits = params.paperWhiteNits;
    m_toneMapPass.settings.maxDisplayNits = params.maxDisplayNits;
}

void HelloTextureEngine::SetRenderViewMode(RenderViewMode mode)
{
    m_debugViewSettings.renderViewMode = mode;
}

void HelloTextureEngine::SetRequestHdrDump(bool request)
{
    m_debugViewSettings.requestHdrDump = request;
}

void HelloTextureEngine::UpdateCameraConstantBuffer()
{
    const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    const XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_camera.rot.x, m_camera.rot.y, m_camera.rot.z);
    const XMMATRIX transMat = XMMatrixTranslation(m_camera.pos.x, m_camera.pos.y, m_camera.pos.z);
    const XMMATRIX view = XMMatrixInverse(nullptr, rotMat * transMat);
    const XMMATRIX projection =
        XMMatrixPerspectiveFovLH(XMConvertToRadians(m_camera.fov), aspect, kCameraNearZ, kCameraFarZ);
    const XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(viewProjection));
    XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                    XMMatrixTranspose(XMMatrixInverse(nullptr, viewProjection)));
    m_constantBufferData.cameraPosition = m_camera.pos;
}

// Load the rendering pipeline dependencies.
void HelloTextureEngine::LoadPipeline()
{
    assert(m_graphicsDevice.device != nullptr);
    assert(m_graphicsDevice.HasSwapChain());

    UpdateHdr10DisplayMode();
    m_currentFrameIndex = m_graphicsDevice.CurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = kRTVDescriptorCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_graphicsDevice.device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_graphicsDevice.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a heap for SRV/CBV/UAV
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = kMainHeapDescriptorCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_graphicsDevice.device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
        // Create a descriptor allocator to manage the descriptors in the heap.
        m_descriptorHeapAllocator.Init(m_graphicsDevice.device.Get(), m_heap.Get());

        D3D12_DESCRIPTOR_HEAP_DESC imguiHeapDesc = {};
        imguiHeapDesc.NumDescriptors = kHeapDescriptorCount;
        imguiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        imguiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_graphicsDevice.device->CreateDescriptorHeap(&imguiHeapDesc, IID_PPV_ARGS(&m_imguiHeap)));
        m_ImGuiDescriptorHeapAllocator.Init(m_graphicsDevice.device.Get(), m_imguiHeap.Get());

        m_descriptorSize = m_graphicsDevice.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            m_graphicsDevice.GetBackBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            m_graphicsDevice.device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
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
        ThrowIfFailed(m_graphicsDevice.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                       IID_PPV_ARGS(&m_frameResources[n].commandAllocator)));
    }

    //
    m_gpuWorkMeter.Init(m_graphicsDevice.device.Get(),
                        kGpuWorkMeterQueryCount); // Initialize GPU work meter with a maximum of 100 timestamp queries.
}

bool HelloTextureEngine::HdrOutputPolicy::CheckSwapChainColorSpaceSupport(IDXGISwapChain3* swapChain,
                                                                          DXGI_COLOR_SPACE_TYPE colorSpace) const
{
    UINT colorSpaceSupport = 0;
    return SUCCEEDED(swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
           (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT);
}

bool HelloTextureEngine::HdrOutputPolicy::CheckCurrentOutputHdr10Support(ComPtr<IDXGIFactory4>& dxgiFactory,
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

void HelloTextureEngine::HdrOutputPolicy::ApplySwapChainColorSpace(IDXGISwapChain3* swapChain,
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

void HelloTextureEngine::HdrOutputPolicy::ApplyHdr10Metadata(IDXGISwapChain3* swapChain, bool enabled) const
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

void HelloTextureEngine::HdrOutputPolicy::Update(ComPtr<IDXGIFactory4>& dxgiFactory,
                                                 IDXGISwapChain3* swapChain,
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

void HelloTextureEngine::HdrOutputPolicy::ReapplyColorSpace(IDXGISwapChain3* swapChain)
{
    ApplySwapChainColorSpace(swapChain, settings.TargetColorSpace());
}

void HelloTextureEngine::UpdateHdr10DisplayMode()
{
    m_hdrOutputPolicy.Update(m_graphicsDevice.dxgiFactory, m_graphicsDevice.swapChain.Get(), m_graphicsDevice.hwnd);
}

DescriptorHeapHandle HelloTextureEngine::CreateTextureFromRGBA8(
    const UINT8* pixels, UINT width, UINT height, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& uploadHeap)
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
    ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &textureDesc,
                                                    D3D12_RESOURCE_STATE_COPY_DEST,
                                                    nullptr,
                                                    IID_PPV_ARGS(&texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                    D3D12_RESOURCE_STATE_GENERIC_READ,
                                                    nullptr,
                                                    IID_PPV_ARGS(&uploadHeap)));

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = pixels;
    textureData.RowPitch = width * kTexturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * height;

    UpdateSubresources(m_commandList.Get(), texture.Get(), uploadHeap.Get(), 0, 0, 1, &textureData);

    m_commandList->ResourceBarrier(1,
                                   &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
                                                                         D3D12_RESOURCE_STATE_COPY_DEST,
                                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    return AllocateTextureSRV(texture.Get());
}

// Load the sample assets.
void HelloTextureEngine::LoadAssets()
{
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned
        // will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_graphicsDevice.device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // t0 - t(TextureCount-1) : Texture SRVs: space 0 : 0 - (kTextureCount-1)
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV[1];
        rangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                          kTextureCount,
                          0 /*base*/,
                          0 /*space*/,
                          D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        // t0 : SRV structured buffer: space1 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV2[1];
        rangesSRV2[0].Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 1 /*space*/, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        // t0 : SRV structured buffer: space2 : 0
        CD3DX12_DESCRIPTOR_RANGE1 rangesSRV3[1];
        rangesSRV3[0].Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 2 /*space*/, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        // t0 - t3 : GBuffer SRVs, t4 : depth SRV, space 3
        CD3DX12_DESCRIPTOR_RANGE1 rangesGBufferSRV[1];
        rangesGBufferSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                 GBuffer::kCount + 1,
                                 0 /*base*/,
                                 3 /*space*/,
                                 D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        // t0 : HDR scene color SRV, space 4
        CD3DX12_DESCRIPTOR_RANGE1 rangesToneMapSRV[1];
        rangesToneMapSRV[0].Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*base*/, 4 /*space*/, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        CD3DX12_DESCRIPTOR_RANGE1 rangesCVB[1];
        rangesCVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_DESCRIPTOR_RANGE1 rangesLightCBV[1];
        rangesLightCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[9];
        rootParameters[0].InitAsDescriptorTable(1, &rangesSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // Texture SRVs
        rootParameters[1].InitAsDescriptorTable(1,
                                                &rangesSRV2[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Instance data)
        rootParameters[2].InitAsDescriptorTable(1,
                                                &rangesSRV3[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Material data)
        rootParameters[3].InitAsDescriptorTable(1, &rangesCVB[0], D3D12_SHADER_VISIBILITY_ALL); // Camera constants
        rootParameters[4].InitAsDescriptorTable(1, &rangesGBufferSRV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer SRVs
        rootParameters[5].InitAsDescriptorTable(1,
                                                &rangesLightCBV[0],
                                                D3D12_SHADER_VISIBILITY_PIXEL);    // Light constants
        rootParameters[6].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer debug target
        rootParameters[7].InitAsDescriptorTable(1,
                                                &rangesToneMapSRV[0],
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
        rootSignatureDesc.Init_1_1(_countof(rootParameters),
                                   rootParameters,
                                   1,
                                   &sampler,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_graphicsDevice.device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        UINT8* pVertexShaderData = nullptr;
        UINT8* pPixelShaderData = nullptr;
        UINT vertexShaderDataLength = 0;
        UINT pixelShaderDataLength = 0;

        ThrowIfFailed(ReadDataFromFile(
            GetAssetFullPath(L"shaders_VSMain.cso").c_str(), &pVertexShaderData, &vertexShaderDataLength));
        ThrowIfFailed(ReadDataFromFile(
            GetAssetFullPath(L"shaders_PSMain.cso").c_str(), &pPixelShaderData, &pixelShaderDataLength));

        UINT8* pDepthVS = nullptr;
        UINT depthVSSize = 0;

        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_DepthOnlyVS_VSMain.cso").c_str(), &pDepthVS, &depthVSSize));

        UINT8* pGBufferVS = nullptr;
        UINT8* pGBufferPS = nullptr;
        UINT gbufferVSSize = 0;
        UINT gbufferPSSize = 0;
        UINT8* pGBufferDebugVS = nullptr;
        UINT8* pGBufferDebugPS = nullptr;
        UINT gbufferDebugVSSize = 0;
        UINT gbufferDebugPSSize = 0;

        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_GBuffer_VSMain.cso").c_str(), &pGBufferVS, &gbufferVSSize));
        ThrowIfFailed(
            ReadDataFromFile(GetAssetFullPath(L"shaders_GBuffer_PSMain.cso").c_str(), &pGBufferPS, &gbufferPSSize));
        ThrowIfFailed(ReadDataFromFile(
            GetAssetFullPath(L"shaders_GBufferDebug_VSMain.cso").c_str(), &pGBufferDebugVS, &gbufferDebugVSSize));
        ThrowIfFailed(ReadDataFromFile(
            GetAssetFullPath(L"shaders_GBufferDebug_PSMain.cso").c_str(), &pGBufferDebugPS, &gbufferDebugPSSize));

        UINT8* pLightPassVS = nullptr;
        UINT8* pLightPassPS = nullptr;
        UINT lightPassVSSize = 0;
        UINT lightPassPSSize = 0;
        UINT8* pLightPassDebugGradientVS = nullptr;
        UINT8* pLightPassDebugGradientPS = nullptr;
        UINT lightPassDebugGradientVSSize = 0;
        UINT lightPassDebugGradientPSSize = 0;
        UINT8* pToneMapVS = nullptr;
        UINT8* pToneMapPS = nullptr;
        UINT toneMapVSSize = 0;
        UINT toneMapPSSize = 0;

        ThrowIfFailed(ReadDataFromFile(
            GetAssetFullPath(L"shaders_LightPass_VSMain.cso").c_str(), &pLightPassVS, &lightPassVSSize));
        ThrowIfFailed(ReadDataFromFile(
            GetAssetFullPath(L"shaders_LightPass_PSMain.cso").c_str(), &pLightPassPS, &lightPassPSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPassDebugGradient_VSMain.cso").c_str(),
                                       &pLightPassDebugGradientVS,
                                       &lightPassDebugGradientVSSize));
        ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(L"shaders_LightPassDebugGradient_PSMain.cso").c_str(),
                                       &pLightPassDebugGradientPS,
                                       &lightPassDebugGradientPSSize));
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
        const InputLayoutDefinition meshInputLayout = {inputElementDescs, _countof(inputElementDescs)};
        const InputLayoutDefinition depthInputLayout = {depthLayout, _countof(depthLayout)};

        //
        // Main Pass PSO
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        RegisterMainPipeline(psoDesc,
                             {Pipe::Main,
                              meshInputLayout,
                              {{pVertexShaderData, vertexShaderDataLength}, {pPixelShaderData, pixelShaderDataLength}},
                              DXGI_FORMAT_R16G16B16A16_FLOAT,
                              DXGI_FORMAT_D32_FLOAT});

        //
        // GBuffer PSO
        //
        RegisterGBufferPipeline(
            psoDesc, {Pipe::GBuffer, meshInputLayout, {{pGBufferVS, gbufferVSSize}, {pGBufferPS, gbufferPSSize}}});

        // Light Pass PSO, ToneMap PSO, GBuffer Debug PSO
        RegisterFullscreenPipelines(
            psoDesc,
            {{Pipe::Lighting,
              {{pLightPassVS, lightPassVSSize}, {pLightPassPS, lightPassPSSize}},
              DXGI_FORMAT_R16G16B16A16_FLOAT},
             {Pipe::LightingDebugGradient,
              {{pLightPassDebugGradientVS, lightPassDebugGradientVSSize},
               {pLightPassDebugGradientPS, lightPassDebugGradientPSSize}},
              DXGI_FORMAT_R16G16B16A16_FLOAT},
             {Pipe::ToneMap, {{pToneMapVS, toneMapVSSize}, {pToneMapPS, toneMapPSSize}}, m_backBufferFormat},
             {Pipe::GBufferDebug,
              {{pGBufferDebugVS, gbufferDebugVSSize}, {pGBufferDebugPS, gbufferDebugPSSize}},
              DXGI_FORMAT_R16G16B16A16_FLOAT}});

        //
        // Depth PrePass PSO
        //
        RegisterDepthPrePassPipeline(psoDesc, {Pipe::DepthPrePass, depthInputLayout, {{pDepthVS, depthVSSize}, {}}});
    }

    //
    CreateGBuffer();

    // Create the command list.
    ThrowIfFailed(m_graphicsDevice.device->CreateCommandList(0,
                                              D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              m_frameResources[m_currentFrameIndex].commandAllocator.Get(),
                                              GetPipelineState(PipelineId(Pipe::Main)),
                                              IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    assert(m_sceneMesh != nullptr);
    const GltfMeshData& mesh = *m_sceneMesh;
    assert(!mesh.vertices.empty());

    m_indexCountPerInstance = static_cast<UINT>(mesh.indices.size());
    m_vertexCountPerInstance = static_cast<UINT>(mesh.vertices.size());
    m_usesIndexedDraw = m_indexCountPerInstance > 0;
    m_sceneHasMaterials = !mesh.materials.empty();
    const UINT vertexBufferSize = static_cast<UINT>(sizeof(GltfVertex) * mesh.vertices.size());

    // Note: using upload heaps to transfer static data like vert buffers is not
    // recommended. Every time the GPU needs it, the upload heap will be marshalled
    // over. Please read up on Default Heap usage. An upload heap is used here for
    // code simplicity and because there are very few verts to actually transfer.
    MyDx12Util::CreateUploadBuffer(m_graphicsDevice.device, vertexBufferSize, m_vertexBuffer);

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, mesh.vertices.data(), vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(GltfVertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    if (m_usesIndexedDraw)
    {
        const UINT indexBufferSize = static_cast<UINT>(mesh.indices.size() * sizeof(uint32_t));

        MyDx12Util::CreateUploadBuffer(m_graphicsDevice.device, indexBufferSize, m_indexBuffer);

        UINT8* pIndexDataBegin = nullptr;
        ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
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
    textureUploadHeap.resize(kTextureCount);

    m_texture.resize(kTextureCount);

    std::vector<std::vector<UINT8>> texture(kTextureTypes);

    m_sceneTextureCount = static_cast<UINT>(mesh.textures.size());

    DBG_PRINT("m_sceneTextureCount = %d\n", m_sceneTextureCount);

    for (size_t i = 0; i < kTextureCount; i++)
    {
        bool useSceneTex = i < mesh.textures.size();
        UINT8* pixels = nullptr;
        UINT width, height;

        if (useSceneTex)
        {
            const auto& tex = mesh.textures[i];
            pixels = (UINT8*)tex.pixels.data();
            width = tex.width;
            height = tex.height;
            DBG_PRINT("[%d] sceneTexture :width %d height %d\n", i, tex.width, tex.height);
        }
        else
        {
            texture[i] = GenerateCheckerboardTextureData();
            pixels = &texture[i % kTextureTypes][0];
            width = kTextureWidth;
            height = kTextureHeight;
            DBG_PRINT("[%d] CheckerBoardTexture :width %d height %d\n", i, kTextureWidth, kTextureHeight);
        }

        DescriptorHeapHandle srv = CreateTextureFromRGBA8(pixels, width, height, m_texture[i], textureUploadHeap[i]);
        if (i == 0)
        {
            m_textureTableStart = srv;
        }
        m_texIndex[i] = srv.Index - m_textureTableStart.Index;
        DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
    }

    // Instance data is provided by the application via SetInstanceData().
    // Pre-allocate the buffer for kMaxInstanceCount entries.
    if (m_instanceData.empty())
    {
        m_instanceData.resize(kMaxInstanceCount);
    }

    // Generate the material data.
    m_materialData.clear();
    const auto resolveTextureIndex = [this](int sceneTextureIndex, UINT fallbackIndex) -> UINT
    {
        if (sceneTextureIndex >= 0 && sceneTextureIndex < static_cast<int>(kTextureCount))
        {
            return m_texIndex[sceneTextureIndex];
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

        if (m_sceneHasMaterials)
        {
            if (i < static_cast<int>(mesh.materials.size()))
            {
                const auto& gltfMaterial = mesh.materials[i];
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

        MyDx12Util::CreateUploadBuffer(m_graphicsDevice.device, instanceBufferSize, m_frameResources[n].instanceBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaxInstanceCount;
        srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);

        m_frameResources[n].instanceBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_graphicsDevice.device->CreateShaderResourceView(
            m_frameResources[n].instanceBuffer.Get(), &srvDesc, m_frameResources[n].instanceBufferSrv.cpu);

        m_frameResources[n].instanceBuffer->Map(
            0, nullptr, reinterpret_cast<void**>(&m_frameResources[n].pSrvDataBegin));
        memcpy(m_frameResources[n].pSrvDataBegin, m_instanceData.data(), instanceBufferSize);
        m_frameResources[n].instanceBuffer->Unmap(0, nullptr);
    }

    // Create SRV for material buffer (StructuredBuffer)
    {
        const UINT materialBufferSize = sizeof(Material) * kMaterialCount;

        MyDx12Util::CreateUploadBuffer(m_graphicsDevice.device, materialBufferSize, m_materialBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaterialCount;
        srvDesc.Buffer.StructureByteStride = sizeof(Material);

        m_materialBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_graphicsDevice.device->CreateShaderResourceView(m_materialBuffer.Get(), &srvDesc, m_materialBufferSrv.cpu);
        Material* pMaterialDataBegin = nullptr;
        m_materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pMaterialDataBegin));
        memcpy(pMaterialDataBegin, m_materialData.data(), materialBufferSize);
        m_materialBuffer->Unmap(0, nullptr);
    }

    UpdateCameraConstantBuffer();
    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;

    // Create the per-frame constant buffers.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        CreateConstantBuffer(m_frameResources[n].cameraCB, &m_constantBufferData, sizeof(m_constantBufferData));
        const LightingConstants initialLightingConstants = MakeLightingConstants();
        CreateConstantBuffer(m_frameResources[n].lightCB, &initialLightingConstants, sizeof(initialLightingConstants));
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        m_graphicsDevice.CreateFence(0);
        m_frameResources[m_currentFrameIndex].fenceValue = 1;

        // Wait for the command list to execute; we are reusing the same command
        // list in our main loop but for now, we just want to wait for setup to
        // complete before continuing.
        WaitForGpu();
    }
}

static SimpleDescriptorHeapAllocator* g_allocator = nullptr;

void HelloTextureEngine::InitImGui()
{
#if IMGUI_IMPL > 0
    const GraphicsDeviceContext graphicsContext = MakeGraphicsDeviceContext();
    g_allocator = &m_ImGuiDescriptorHeapAllocator;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(graphicsContext.hwnd);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = graphicsContext.device;
    init_info.CommandQueue = graphicsContext.commandQueue;
    init_info.NumFramesInFlight = kFrameCount;
    init_info.RTVFormat = m_backBufferFormat;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate
    // more)
    init_info.SrvDescriptorHeap = m_imguiHeap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*,
                                        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    { g_allocator->Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    { g_allocator->Free(&cpu_handle, &gpu_handle); };
    ImGui_ImplDX12_Init(&init_info);
#endif
}

void HelloTextureEngine::CreateConstantBuffer(ConstantBufferResource& constantBuffer,
                                              const void* initialData,
                                              UINT sizeInBytes)
{
    assert(sizeInBytes % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0);

    ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
                                                    D3D12_RESOURCE_STATE_GENERIC_READ,
                                                    nullptr,
                                                    IID_PPV_ARGS(&constantBuffer.buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constantBuffer.buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeInBytes;

    constantBuffer.cbv = m_descriptorHeapAllocator.AllocWithHandle();
    m_graphicsDevice.device->CreateConstantBufferView(&cbvDesc, constantBuffer.cbv.cpu);

    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(constantBuffer.buffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBuffer.mappedData)));
    memcpy(constantBuffer.mappedData, initialData, sizeInBytes);
}

DescriptorHeapHandle HelloTextureEngine::AllocateTextureSRV(ID3D12Resource* texture)
{
    DescriptorHeapHandle handle = m_descriptorHeapAllocator.AllocWithHandle();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.device->CreateShaderResourceView(texture, &srvDesc, handle.cpu);

    return handle;
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> HelloTextureEngine::GenerateCheckerboardTextureData()
{
    const UINT rowPitch = kTextureWidth * kTexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;       // The width of a cell in the checkboard texture.
    const UINT cellHeight = kTextureWidth >> 3; // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * kTextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

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

void HelloTextureEngine::CreateDsvHeap()
{
    if (m_dsvHeap)
        return;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_graphicsDevice.device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void HelloTextureEngine::CreateGBuffer()
{
    CreateGBufferResources();
    CreateGBufferRTVs();
    CreateGBufferSRVs();
}

void HelloTextureEngine::CreateGBufferResources()
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

        ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                        D3D12_HEAP_FLAG_NONE,
                                                        &desc,
                                                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                        &m_gbuffer.clearValues[i],
                                                        IID_PPV_ARGS(&m_gbuffer.resources[i])));
    }
}

void HelloTextureEngine::CreateGBufferRTVs()
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        m_gbuffer.rtvIndex[i] = kGBufferRTVBaseIndex + i;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_gbuffer.rtvIndex[i], m_rtvDescriptorSize);
        m_graphicsDevice.device->CreateRenderTargetView(m_gbuffer.resources[i].Get(), nullptr, rtvHandle);
    }
}

void HelloTextureEngine::CreateGBufferSRVs()
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

        m_graphicsDevice.device->CreateShaderResourceView(m_gbuffer.resources[i].Get(), &srvDesc, m_gbuffer.srvHandles[i].cpu);
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

void HelloTextureEngine::RegisterDepthStencil(UINT width, UINT height)
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

void HelloTextureEngine::RegisterLightPassRenderTarget(UINT width, UINT height)
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

void HelloTextureEngine::CreateDepthStencilDescriptors()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    m_graphicsDevice.device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[GBuffer::Albedo].Index + GBuffer::kCount);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.device->CreateShaderResourceView(m_depthStencil.Get(), &srvDesc, m_depthStencilSrv.cpu);
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetBackBufferRtv() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrameIndex, m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetDepthDsv() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetGBufferRTV(UINT index) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(m_gbuffer.rtvIndex[index], m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetLightPassRTV() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(kLightPassRTVIndex, m_rtvDescriptorSize);
    return h;
}

void HelloTextureEngine::RegisterPassBindingResolvers()
{
    m_renderGraphRuntime.Bindings().Clear();

    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::BackBuffer),
                                                [this]() { return GetBackBufferRtv(); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferAlbedo),
                                                [this]() { return GetGBufferRTV(GBuffer::Albedo); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferNormal),
                                                [this]() { return GetGBufferRTV(GBuffer::Normal); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferMaterial),
                                                [this]() { return GetGBufferRTV(GBuffer::Material); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferMotionVector),
                                                [this]() { return GetGBufferRTV(GBuffer::MotionVector); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferPBRParams),
                                                [this]() { return GetGBufferRTV(GBuffer::PBRParams); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::LightPass),
                                                [this]() { return GetLightPassRTV(); });

    m_renderGraphRuntime.Bindings().RegisterDsv(m_renderGraphRuntime.RegisterDsv(DsvName::Depth),
                                                [this]() { return GetDepthDsv(); });

    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::TextureTable),
                                                       [this]() { return m_textureTableStart.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::InstanceBufferSrv),
        [this]() { return m_frameResources[m_currentFrameIndex].instanceBufferSrv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::MaterialBufferSrv),
                                                       [this]() { return m_materialBufferSrv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::CameraCbv),
                                                       [this]()
                                                       { return m_frameResources[m_currentFrameIndex].cameraCB.cbv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::LightCbv),
                                                       [this]()
                                                       { return m_frameResources[m_currentFrameIndex].lightCB.cbv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::GBufferAlbedoSrv),
                                                       [this]() { return m_gbuffer.srvHandles[GBuffer::Albedo].gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::ToneMapSceneColorSrv),
        [this]() { return m_lightPassColorSrv.gpu; });
}

void HelloTextureEngine::RegisterPassConstantsHandlers()
{
    m_renderGraphRuntime.Constants().Clear();
    m_renderGraphRuntime.Constants().Register(
        m_renderGraphRuntime.RegisterConstants(ConstName::ToneMap),
        [this](UINT rootParameterIndex)
        {
            const auto constants = m_toneMapPass.MakeShaderConstants(m_hdrOutputPolicy.settings);
            m_commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, 5, &constants, 0);
        });
    m_renderGraphRuntime.Constants().Register(m_renderGraphRuntime.RegisterConstants(ConstName::GBufferDebugTarget),
                                              [this](UINT rootParameterIndex)
                                              {
                                                  const UINT debugTarget = m_debugViewSettings.GetGBufferDebugTarget();
                                                  m_commandList->SetGraphicsRoot32BitConstants(
                                                      rootParameterIndex, 1, &debugTarget, 0);
                                              });
}

void HelloTextureEngine::RegisterResourceResolvers()
{
    m_renderGraphRuntime.Resources().Clear();
    m_renderGraphRuntime.Resources().RegisterResource(kBackBufferResourceName,
                                                      [this]() { return m_renderTargets[m_currentFrameIndex].Get(); });
    m_renderGraphRuntime.Resources().RegisterResource(kDepthStencilResourceName,
                                                      [this]() { return m_depthStencil.Get(); });
    m_renderGraphRuntime.Resources().RegisterResource(kLightPassRenderTargetResourceName,
                                                      [this]() { return m_lightPassRenderTarget.Get(); });
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        m_renderGraphRuntime.Resources().RegisterResource(kGBufferResourceNames[i],
                                                          [this, i]() { return m_gbuffer.resources[i].Get(); });
    }
    m_renderGraphRuntime.Resources().SetFallbackResolver([this](const std::string& name)
                                                         { return FindTransientD3DResource(name); });
}

void HelloTextureEngine::CreateDepthStencil(UINT width, UINT height)
{
    // Release if DS exist
    m_depthStencil.Reset();

    // Create Depth Resource
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_graphicsDevice.device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

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

    ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &depthDesc,
                                                    D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                    &clearValue,
                                                    IID_PPV_ARGS(&m_depthStencil)));

    // Create DSV
    CreateDepthStencilDescriptors();
}

// Update frame-based values.
void HelloTextureEngine::UpdateFrame()
{
    PIXBeginEvent(0, L"UpdateFrame");

    if (m_updateHandler)
    {
        m_updateHandler();
    }

    m_frameResources[m_currentFrameIndex].instanceBuffer->Map(
        0, nullptr, reinterpret_cast<void**>(&m_frameResources[m_currentFrameIndex].pSrvDataBegin));
    memcpy(
        m_frameResources[m_currentFrameIndex].pSrvDataBegin, m_instanceData.data(), sizeof(InstanceData) * kMaxInstanceCount);
    m_frameResources[m_currentFrameIndex].instanceBuffer->Unmap(0, nullptr);

    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
    UpdateCameraConstantBuffer();
    memcpy(m_frameResources[m_currentFrameIndex].cameraCB.mappedData, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

void HelloTextureEngine::UpdateImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    if (m_debugUiHandler)
    {
        DebugUiContext context{static_cast<int>(m_currentFrameIndex),
                               m_cpuFrameTime,
                               m_frameResources[m_previousFrameIndex].gpuWorkMeterCheckPoints};
        m_debugUiHandler(context);
    }

    m_toneMapPass.settings.Normalize();
    const LightingConstants lightingConstants = MakeLightingConstants();
    memcpy(m_frameResources[m_currentFrameIndex].lightCB.mappedData, &lightingConstants, sizeof(lightingConstants));

    ImGui::Render();
}

UINT HelloTextureEngine::GetVisibleCubeCount() const
{
    return static_cast<UINT>(m_displayInstanceCount);
}

// Render the scene.
void HelloTextureEngine::RenderFrame()
{
    PIXBeginEvent(0, L"RenderFrame");

    // ImGui frame update
#if IMGUI_IMPL > 0
    UpdateImGui();
#endif

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    if (m_debugViewSettings.hdrDumpPending)
    {
        WaitForGpu();
        PrintDebugDump();
        m_debugViewSettings.requestHdrDump = false;
        m_debugViewSettings.hdrDumpPending = false;
    }

    // Present the frame.
    m_graphicsDevice.Present(1, 0);

    UINT64 submittedFenceValue = MoveToNextFrame();

    // submittedFenceValue marks completion of the command list submitted for this frame.
    MarkPendingTransientResources(submittedFenceValue);

    CollectGarbageTransientResources();

    m_gpuWorkMeter.ReadbackData(m_graphicsDevice.commandQueue.Get());

    PIXEndEvent();
}

void HelloTextureEngine::OnWindowSizeChanged(UINT width, UINT height)
{
    m_pendingResize = true;
    m_pendingResizeWidth = width;
    m_pendingResizeHeight = height;
}

void HelloTextureEngine::RunFrame()
{
    if (m_pendingResize)
    {
        Resize(m_pendingResizeWidth, m_pendingResizeHeight);
        m_pendingResize = false;
    }

    UpdateHdr10DisplayMode();

    m_workMeter.Start();
    UpdateFrame();
    RenderFrame();
    m_workMeter.End();
    m_cpuFrameTime = m_workMeter.GetCpuFrameTimeMs();
}

void HelloTextureEngine::OnMouseDown(UINT8, int, int) {}

void HelloTextureEngine::OnMouseUp(UINT8, int, int) {}

void HelloTextureEngine::OnMouseMove(int, int) {}

void HelloTextureEngine::Resize(UINT width, UINT height)
{
    DBG_PRINT("HelloTextureEngine::OnWindowSizeChanged() %d %d\n", width, height);
    m_width = width;
    m_height = height;

    if (width == 0 || height == 0)
    {
        return;
    }

    if (!m_graphicsDevice.HasSwapChain())
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
    m_graphicsDevice.ResizeSwapChain(kFrameCount, m_width, m_height, m_backBufferFormat, 0);
    m_hdrOutputPolicy.ReapplyColorSpace(m_graphicsDevice.swapChain.Get());

    // Preserve the previous frame index before taking the resized swap chain index.
    m_previousFrameIndex = m_currentFrameIndex;
    m_currentFrameIndex = m_graphicsDevice.CurrentBackBufferIndex();

    // Re-create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            m_graphicsDevice.GetBackBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            m_graphicsDevice.device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
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
    UpdateCameraConstantBuffer();

    // Screen
    m_viewport = CD3DX12_VIEWPORT(
        0.0f, 0.0f, static_cast<FLOAT>(m_width), static_cast<FLOAT>(m_height), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    // Imgui
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
}

void HelloTextureEngine::Shutdown()
{
    DestroyFrameResources();
}

void HelloTextureEngine::DestroyFrameResources()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    m_graphicsDevice.CloseFenceEvent();
}

void HelloTextureEngine::RegisterFullscreenPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                    const FullscreenPipelineDefinition& definition)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = MyDx12Util::CreateFullscreenPassPSODesc(baseDesc,
                                                                                      definition.shaders.vertex.data,
                                                                                      definition.shaders.vertex.size,
                                                                                      definition.shaders.pixel.data,
                                                                                      definition.shaders.pixel.size,
                                                                                      definition.renderTargetFormat);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.device.Get(), key, desc));
}

void HelloTextureEngine::RegisterFullscreenPipelines(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                     std::initializer_list<FullscreenPipelineDefinition> definitions)
{
    for (const FullscreenPipelineDefinition& definition : definitions)
    {
        RegisterFullscreenPipeline(baseDesc, definition);
    }
}

void HelloTextureEngine::RegisterMainPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                              const MainPipelineDefinition& definition)
{
    baseDesc.InputLayout = {definition.inputLayout.elements, definition.inputLayout.count};
    baseDesc.pRootSignature = m_rootSignature.Get();
    baseDesc.VS = CD3DX12_SHADER_BYTECODE(definition.shaders.vertex.data, definition.shaders.vertex.size);
    baseDesc.PS = CD3DX12_SHADER_BYTECODE(definition.shaders.pixel.data, definition.shaders.pixel.size);
    baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    baseDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    baseDesc.SampleMask = UINT_MAX;
    baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    baseDesc.NumRenderTargets = 1;
    baseDesc.RTVFormats[0] = definition.renderTargetFormat;
    baseDesc.DSVFormat = definition.depthStencilFormat;
    baseDesc.SampleDesc.Count = 1;
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.device.Get(), key, baseDesc));
}

void HelloTextureEngine::RegisterGBufferPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                 const GBufferPipelineDefinition& definition)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferBaseDesc = baseDesc;
    gbufferBaseDesc.InputLayout = {definition.inputLayout.elements, definition.inputLayout.count};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = MyDx12Util::CreateGBufferPSODesc(gbufferBaseDesc,
                                                                               definition.shaders.vertex.data,
                                                                               definition.shaders.vertex.size,
                                                                               definition.shaders.pixel.data,
                                                                               definition.shaders.pixel.size,
                                                                               m_gbuffer.formats,
                                                                               GBuffer::kCount);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.device.Get(), key, desc));
}

void HelloTextureEngine::RegisterDepthPrePassPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                      const DepthPrePassPipelineDefinition& definition)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;
    desc.InputLayout = {definition.inputLayout.elements, definition.inputLayout.count};
    desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    desc.VS = CD3DX12_SHADER_BYTECODE(definition.shaders.vertex.data, definition.shaders.vertex.size);
    desc.PS = {};
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
    desc.NumRenderTargets = 0;
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.device.Get(), key, desc));
}

void HelloTextureEngine::PopulateCommandList()
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

void HelloTextureEngine::AnalyzeResourceLifetimes()
{
    m_resourceRegistry.AnalyzeLifetimes(m_renderGraphRuntime.Graph().Passes());
}

void HelloTextureEngine::DebugPrintLifetimes()
{
    DBG_PRINT("Resource Lifetimes:\n");
    for (auto& [name, lt] : m_resourceRegistry.lifetimes)
    {
        DBG_PRINT("Resource %s: [%d - %d]\n", name.c_str(), lt.firstPass, lt.lastPass);
    }
}

ID3D12PipelineState* HelloTextureEngine::GetPipelineState(PipelineKey pipeline) const
{
    return m_renderGraphRuntime.Pipelines().Find(pipeline);
}

void HelloTextureEngine::ExecutePasses()
{
    Engine::ResourceTransitionContext resourceTransitions = MakeResourceTransitionContext();
    Engine::ExecuteRenderPassGraph(m_renderGraphRuntime.Graph(),
                                   {m_commandList.Get(),
                                    &m_renderGraphRuntime.Bindings(),
                                    &m_renderGraphRuntime.Pipelines(),
                                    &m_renderGraphRuntime.Constants(),
                                    &resourceTransitions,
                                    [this](int passIndex) { CreateResourcesForPass(passIndex); },
                                    [this](const RenderPass& pass) { ExecutePassOperation(pass); },
                                    [this](int passIndex) { ReleaseResourcesAfterPass(passIndex); }});
}

void HelloTextureEngine::ExecutePassOperation(const RenderPass& pass)
{
    const PassOperationHandler* handler = m_renderGraphRuntime.Operations().Find(pass.operation);
    assert(handler != nullptr && "Unsupported pass operation.");
    if (handler != nullptr)
    {
        const PassOperationHandler handlerFunc = *handler;
        (this->*handlerFunc)(pass);
    }
}

void HelloTextureEngine::CreateResourcesForPass(int passIndex)
{
    const std::vector<std::string> resourceNames =
        m_resourceRegistry.GetResourcesStartingAtPass(passIndex, kBackBufferResourceName);

    for (const std::string& name : resourceNames)
    {
        TransientResource* transientResource = m_resourceRegistry.PrepareTransientResourceForCreate(name);
        if (transientResource == nullptr)
            continue;
        auto& tr = *transientResource;

        CreateCommittedTransientResource(tr);
        m_resourceRegistry.MarkTransientResourceCreated(name);
        BindCreatedTransientResource(name, tr.resource.Get());

        DBG_PRINT("Resource %s created.\n", name.c_str());
    }
}

void HelloTextureEngine::CreateCommittedTransientResource(TransientResource& resource)
{
    ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &resource.desc,
                                                    resource.initialState,
                                                    &resource.clearValue,
                                                    IID_PPV_ARGS(&resource.resource)));
}

void HelloTextureEngine::BindCreatedTransientResource(const std::string& name, ID3D12Resource* resource)
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

void HelloTextureEngine::CreateLightPassRenderTargetDescriptors()
{
    m_graphicsDevice.device->CreateRenderTargetView(m_lightPassRenderTarget.Get(), nullptr, GetLightPassRTV());

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.device->CreateShaderResourceView(m_lightPassRenderTarget.Get(), &srvDesc, m_lightPassColorSrv.cpu);
}

void HelloTextureEngine::ReleaseResourcesAfterPass(int passIndex)
{
    m_resourceRegistry.MarkEndOfLifeResources(passIndex, kBackBufferResourceName);
}

void HelloTextureEngine::MarkPendingTransientResources(UINT64 fenceValue)
{
    m_resourceRegistry.MarkPendingTransientResources(fenceValue);
}

void HelloTextureEngine::CollectGarbageTransientResources()
{
    const UINT64 completed = m_graphicsDevice.CompletedFenceValue();
    const std::vector<std::string> releasedResources = m_resourceRegistry.CollectGarbageTransientResources(completed);

    for (const std::string& name : releasedResources)
    {
        auto transientResource = m_resourceRegistry.transientResources.find(name);
        if (transientResource != m_resourceRegistry.transientResources.end())
        {
            transientResource->second.resource.Reset();
            DBG_PRINT("Resource %s released.\n", name.c_str());
        }

        if (name == kDepthStencilResourceName)
        {
            m_depthStencil.Reset();
        }
    }
}

void HelloTextureEngine::ResetResourceStates()
{
    m_resourceRegistry.ResetStates({{kBackBufferResourceName, D3D12_RESOURCE_STATE_PRESENT},
                                    {kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE},
                                    {kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}});
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        SetResourceState(kGBufferResourceNames[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}

Engine::ResourceTransitionContext HelloTextureEngine::MakeResourceTransitionContext()
{
    return {m_commandList.Get(),
            [this](const std::string& name) { return m_renderGraphRuntime.Resources().Resolve(name); },
            [this](const std::string& name) { return GetResourceState(name); },
            [this](const std::string& name, D3D12_RESOURCE_STATES state) { SetResourceState(name, state); },
            [](const ResourceUsage& usage)
            { DBG_PRINT("Resource %s is null. Skip transition.\n", usage.name.c_str()); }};
}

void HelloTextureEngine::TransitionPassResources(const RenderPass& pass)
{
    Engine::TransitionPassResources(MakeResourceTransitionContext(), pass);
}

void HelloTextureEngine::TransitionResource(const ResourceUsage& usage)
{
    Engine::TransitionResource(MakeResourceTransitionContext(), usage);
}

ID3D12Resource* HelloTextureEngine::FindTransientD3DResource(const std::string& name) const
{
    auto transientResource = m_resourceRegistry.transientResources.find(name);
    if (transientResource == m_resourceRegistry.transientResources.end())
    {
        return nullptr;
    }

    return transientResource->second.resource.Get();
}

D3D12_RESOURCE_STATES HelloTextureEngine::GetResourceState(const std::string& name) const
{
    return m_resourceRegistry.GetState(name);
}

void HelloTextureEngine::SetResourceState(const std::string& name, D3D12_RESOURCE_STATES state)
{
    m_resourceRegistry.SetState(name, state);
}

void HelloTextureEngine::BeginFrame()
{

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_frameResources[m_currentFrameIndex].commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(),
                                       GetPipelineState(PipelineId(Pipe::Main))));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = {m_heap.Get()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    m_gpuWorkMeter.StartGpu(m_commandList.Get(), m_frameResources[m_currentFrameIndex].gpuWorkMeterCheckPoints);
}

void HelloTextureEngine::ExecuteClearPass(const RenderPass& pass)
{
    RecordClear(pass.renderTargets);
}

void HelloTextureEngine::ExecuteDepthPrePass(const RenderPass& pass)
{
    RecordDepthPrePass();
}

void HelloTextureEngine::ExecuteGBufferPass(const RenderPass& pass)
{
    RecordGBufferPass(pass.renderTargets);
}

void HelloTextureEngine::ExecuteMainPass(const RenderPass& pass)
{
    RecordMainPass(pass.renderTargets);
}

void HelloTextureEngine::ExecuteLightingPass(const RenderPass& pass)
{
    RecordLightPass();
}

void HelloTextureEngine::ExecuteLightingDebugGradientPass(const RenderPass& pass)
{
    RecordLightPassDebugGradient();
}

void HelloTextureEngine::ExecuteToneMapPass(const RenderPass& pass)
{
    RecordToneMapPass();
}

void HelloTextureEngine::ExecuteDebugDumpPass(const RenderPass& pass)
{
    RecordDebugDumpPass();
}

void HelloTextureEngine::ExecuteGBufferDebugPass(const RenderPass& pass)
{
    RecordGBufferDebugPass();
}

void HelloTextureEngine::ExecuteImGuiPass(const RenderPass& pass)
{
    RecordImGuiPass();
}

void HelloTextureEngine::RecordClear(const PassRenderTargetBinding& renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"ClearPrepass");
    assert(!renderTargets.rtvs.empty());
    assert(renderTargets.dsv.has_value());
    assert(renderTargets.clearColor.has_value());

    for (RtvKey rtv : renderTargets.rtvs)
    {
        m_commandList->ClearRenderTargetView(
            m_renderGraphRuntime.Bindings().ResolveRtv(rtv), renderTargets.clearColor->data(), 0, nullptr);
    }
    m_commandList->ClearDepthStencilView(m_renderGraphRuntime.Bindings().ResolveDsv(renderTargets.dsv.value()),
                                         D3D12_CLEAR_FLAG_DEPTH,
                                         1.0f,
                                         0,
                                         0,
                                         nullptr);

    PIXEndEvent(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Clear");
}

void HelloTextureEngine::DrawInstanceWrapper(UINT instanceCount)
{
    if (m_usesIndexedDraw)
    {
        m_commandList->IASetIndexBuffer(&m_indexBufferView);
        m_commandList->DrawIndexedInstanced(m_indexCountPerInstance, instanceCount, 0, 0, 0);
    }
    else
    {
        m_commandList->DrawInstanced(m_vertexCountPerInstance, instanceCount, 0, 0);
    }
}

void HelloTextureEngine::DrawFullscreenTriangle()
{
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawInstanced(3, 1, 0, 0);
}

//
// Depth Pre-pass
//
void HelloTextureEngine::RecordDepthPrePass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"DepthPrepass");
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Depth Prepass");
}

void HelloTextureEngine::RecordGBufferPass(const PassRenderTargetBinding& renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"GBufferPass");

    for (UINT i = 0; i < static_cast<UINT>(renderTargets.rtvs.size()); ++i)
    {
        m_commandList->ClearRenderTargetView(m_renderGraphRuntime.Bindings().ResolveRtv(renderTargets.rtvs[i]),
                                             m_gbuffer.clearValues[i].Color,
                                             0,
                                             nullptr);
    }

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Pass");
}

void HelloTextureEngine::RecordGBufferDebugPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"GBufferDebugPass");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Debug Pass");
}

void HelloTextureEngine::RecordLightPassDebugGradient()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"RecordLightPassDebugGradient");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "LightPassDebugGradient Pass");
}

void HelloTextureEngine::RecordLightPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"LightPass");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Lighting Pass");
}

void HelloTextureEngine::RecordToneMapPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"ToneMapPass");

    DrawFullscreenTriangle();

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ToneMap Pass");
}

void HelloTextureEngine::CreateDebugDumpReadback(ID3D12Resource* source,
                                                 ComPtr<ID3D12Resource>& readback,
                                                 D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout)
{
    D3D12_RESOURCE_DESC desc = source->GetDesc();
    UINT numRows = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes = 0;
    m_graphicsDevice.device->GetCopyableFootprints(&desc, 0, 1, 0, &layout, &numRows, &rowSizeInBytes, &totalBytes);

    ThrowIfFailed(m_graphicsDevice.device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &CD3DX12_RESOURCE_DESC::Buffer(totalBytes),
                                                    D3D12_RESOURCE_STATE_COPY_DEST,
                                                    nullptr,
                                                    IID_PPV_ARGS(&readback)));
}

void HelloTextureEngine::RecordDebugDumpPass()
{
    PIXBeginEvent(m_commandList.Get(), 0, L"DebugDump");

    CreateDebugDumpReadback(m_lightPassRenderTarget.Get(), m_lightPassDebugDumpReadback, m_lightPassDebugDumpLayout);
    CreateDebugDumpReadback(
        m_renderTargets[m_currentFrameIndex].Get(), m_backBufferDebugDumpReadback, m_backBufferDebugDumpLayout);

    CD3DX12_TEXTURE_COPY_LOCATION lightDst(m_lightPassDebugDumpReadback.Get(), m_lightPassDebugDumpLayout);
    CD3DX12_TEXTURE_COPY_LOCATION lightSrc(m_lightPassRenderTarget.Get(), 0);
    m_commandList->CopyTextureRegion(&lightDst, 0, 0, 0, &lightSrc, nullptr);

    CD3DX12_TEXTURE_COPY_LOCATION backBufferDst(m_backBufferDebugDumpReadback.Get(), m_backBufferDebugDumpLayout);
    CD3DX12_TEXTURE_COPY_LOCATION backBufferSrc(m_renderTargets[m_currentFrameIndex].Get(), 0);
    m_commandList->CopyTextureRegion(&backBufferDst, 0, 0, 0, &backBufferSrc, nullptr);

    m_debugViewSettings.hdrDumpPending = true;

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Debug Dump");
}

void HelloTextureEngine::PrintDebugDump()
{
    if (!m_lightPassDebugDumpReadback || !m_backBufferDebugDumpReadback)
    {
        return;
    }

    UINT8* lightData = nullptr;
    UINT8* backBufferData = nullptr;
    const D3D12_RANGE lightReadRange = {0, static_cast<SIZE_T>(m_lightPassDebugDumpReadback->GetDesc().Width)};
    const D3D12_RANGE backBufferReadRange = {0, static_cast<SIZE_T>(m_backBufferDebugDumpReadback->GetDesc().Width)};
    ThrowIfFailed(m_lightPassDebugDumpReadback->Map(0, &lightReadRange, reinterpret_cast<void**>(&lightData)));
    ThrowIfFailed(
        m_backBufferDebugDumpReadback->Map(0, &backBufferReadRange, reinterpret_cast<void**>(&backBufferData)));

    const UINT lightWidth = static_cast<UINT>(m_lightPassDebugDumpLayout.Footprint.Width);
    const UINT lightHeight = m_lightPassDebugDumpLayout.Footprint.Height;
    const UINT backBufferWidth = static_cast<UINT>(m_backBufferDebugDumpLayout.Footprint.Width);
    const UINT backBufferHeight = m_backBufferDebugDumpLayout.Footprint.Height;
    const UINT sampleYs[] = {lightHeight > 0 ? lightHeight / 4 : 0, lightHeight > 0 ? (lightHeight * 3) / 4 : 0};
    const char* bandNames[] = {"SDR[0,1]", "HDR[0,9]"};
    const UINT bandCount = m_lightingPassDebugGradientEnabled ? 2 : 1;
    const UINT sampleXs[] = {0, lightWidth / 4, lightWidth / 2, lightWidth > 0 ? lightWidth - 1 : 0};
    const char* sampleNames[] = {"left", "25%", "50%", "right"};

    DebugPrint("HDR DebugDump: LightPass=%ux%u BackBuffer=%ux%u hdr10=%d gradient=%d toneMap=%d exposure=%.3f "
               "paperWhite=%.1f maxDisplay=%.1f\n",
               lightWidth,
               lightHeight,
               backBufferWidth,
               backBufferHeight,
               m_hdrOutputPolicy.settings.hdr10Enabled ? 1 : 0,
               m_lightingPassDebugGradientEnabled ? 1 : 0,
               m_toneMapPass.settings.operatorIndex,
               m_toneMapPass.settings.exposure,
               m_toneMapPass.settings.paperWhiteNits,
               m_toneMapPass.settings.maxDisplayNits);
    if (m_lightingPassDebugGradientEnabled)
    {
        const float displayMaxSceneLinear =
            m_toneMapPass.settings.maxDisplayNits / (std::max)(m_toneMapPass.settings.paperWhiteNits, 1.0f);
        const float displayMaxMarkerX = std::pow((std::clamp)(displayMaxSceneLinear / 9.0f, 0.0f, 1.0f), 1.0f / 2.2f);
        DebugPrint("  HDR[0,9] display-max marker: sceneLinear=%.4f nits=%.1f x=%.4f%s\n",
                   displayMaxSceneLinear,
                   m_toneMapPass.settings.maxDisplayNits,
                   displayMaxMarkerX,
                   displayMaxSceneLinear >= 9.0f ? " (outside ramp)" : "");
    }

    for (UINT band = 0; band < bandCount; ++band)
    {
        const UINT sampleY =
            m_lightingPassDebugGradientEnabled ? sampleYs[band] : (lightHeight > 0 ? lightHeight / 2 : 0);
        const char* bandName = m_lightingPassDebugGradientEnabled ? bandNames[band] : "Scene";

        for (UINT i = 0; i < _countof(sampleXs); ++i)
        {
            const UINT lightX = (std::min)(sampleXs[i], lightWidth > 0 ? lightWidth - 1 : 0);
            const UINT backBufferX = backBufferWidth > 0 ? (std::min)(lightX, backBufferWidth - 1) : 0;
            const UINT backBufferY = backBufferHeight > 0 ? (std::min)(sampleY, backBufferHeight - 1) : 0;

            const UINT8* lightRow = lightData + m_lightPassDebugDumpLayout.Offset +
                                    static_cast<size_t>(sampleY) * m_lightPassDebugDumpLayout.Footprint.RowPitch;
            const UINT16* lightHalf = reinterpret_cast<const UINT16*>(lightRow + static_cast<size_t>(lightX) * 8);
            const float lightR = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[0]);
            const float lightG = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[1]);
            const float lightB = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[2]);
            const float lightA = DirectX::PackedVector::XMConvertHalfToFloat(lightHalf[3]);

            const UINT8* backBufferRow =
                backBufferData + m_backBufferDebugDumpLayout.Offset +
                static_cast<size_t>(backBufferY) * m_backBufferDebugDumpLayout.Footprint.RowPitch;
            const UINT backBufferRaw =
                *reinterpret_cast<const UINT*>(backBufferRow + static_cast<size_t>(backBufferX) * 4);
            const float outR = static_cast<float>(backBufferRaw & 0x3ff) / 1023.0f;
            const float outG = static_cast<float>((backBufferRaw >> 10) & 0x3ff) / 1023.0f;
            const float outB = static_cast<float>((backBufferRaw >> 20) & 0x3ff) / 1023.0f;
            const float outA = static_cast<float>((backBufferRaw >> 30) & 0x3) / 3.0f;

            const float expectedU = lightWidth > 0 ? (static_cast<float>(lightX) + 0.5f) / lightWidth : 0.0f;
            const float expectedV = lightHeight > 0 ? (static_cast<float>(sampleY) + 0.5f) / lightHeight : 0.0f;
            DebugPrint("  %s %s x=%u y=%u LightPass RGBA=(%.4f, %.4f, %.4f, %.4f)\n",
                       bandName,
                       sampleNames[i],
                       lightX,
                       sampleY,
                       lightR,
                       lightG,
                       lightB,
                       lightA);

            if (m_lightingPassDebugGradientEnabled)
            {
                const float expectedRampInput = (std::clamp)(expectedU, 0.0f, 1.0f);
                const float expectedMaxLinear = expectedV < 0.5f ? 1.0f : 9.0f;
                const float expectedPerceptualMax = std::pow(expectedMaxLinear, 1.0f / 2.2f);
                const float expectedPerceptualValue = expectedRampInput * expectedPerceptualMax;
                const float expectedSceneLinear = std::pow(expectedPerceptualValue, 2.2f);
                DebugPrint("  %s %s expected uv=(%.4f, %.4f) perceptual=(%.4f/%.4f) sceneLinear=%.4f "
                           "nits=%.1f\n",
                           bandName,
                           sampleNames[i],
                           expectedU,
                           expectedV,
                           expectedPerceptualValue,
                           expectedPerceptualMax,
                           expectedSceneLinear,
                           expectedSceneLinear * m_toneMapPass.settings.paperWhiteNits);
            }

            if (m_hdrOutputPolicy.settings.hdr10Enabled)
            {
                DebugPrint("  %s %s x=%u y=%u BackBuffer R10G10B10A2=(%.4f, %.4f, %.4f, %.4f) raw=0x%08x "
                           "PQ-nits=(%.1f, %.1f, %.1f)\n",
                           bandName,
                           sampleNames[i],
                           backBufferX,
                           backBufferY,
                           outR,
                           outG,
                           outB,
                           outA,
                           backBufferRaw,
                           St2084PqToNits(outR),
                           St2084PqToNits(outG),
                           St2084PqToNits(outB));
            }
            else
            {
                DebugPrint("  %s %s x=%u y=%u BackBuffer R10G10B10A2=(%.4f, %.4f, %.4f, %.4f) raw=0x%08x "
                           "SDR-linear=(%.4f, %.4f, %.4f) SDR-nits=(%.1f, %.1f, %.1f)\n",
                           bandName,
                           sampleNames[i],
                           backBufferX,
                           backBufferY,
                           outR,
                           outG,
                           outB,
                           outA,
                           backBufferRaw,
                           SrgbToLinear(outR),
                           SrgbToLinear(outG),
                           SrgbToLinear(outB),
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
void HelloTextureEngine::RecordMainPass(const PassRenderTargetBinding& renderTargets)
{
    PIXBeginEvent(m_commandList.Get(), 0, L"MainPass");
    if (renderTargets.clearColor)
    {
        for (RtvKey rtv : renderTargets.rtvs)
        {
            m_commandList->ClearRenderTargetView(
                m_renderGraphRuntime.Bindings().ResolveRtv(rtv), renderTargets.clearColor->data(), 0, nullptr);
        }
    }

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DrawInstanceWrapper(GetVisibleCubeCount());

    PIXEndEvent(m_commandList.Get());

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Main Pass");
}

void HelloTextureEngine::RecordImGuiPass()
{
#if IMGUI_IMPL > 0
    {
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        ID3D12DescriptorHeap* imguiHeaps[] = {m_imguiHeap.Get()};

        m_commandList->SetDescriptorHeaps(1, imguiHeaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
    }
#endif
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ImGUI");
}

void HelloTextureEngine::EndFrame()
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
void HelloTextureEngine::WaitForGpu()
{
    PIXBeginEvent(3, L"WaitForGpu");

    // Schedule a Signal command in the queue.
    m_graphicsDevice.SignalFence(m_frameResources[m_currentFrameIndex].fenceValue);

    // Wait until the fence has been processed.
    m_graphicsDevice.WaitForFenceValue(m_frameResources[m_currentFrameIndex].fenceValue);

    // Increment the fence value for the current frame.
    m_frameResources[m_currentFrameIndex].fenceValue++;

    PIXEndEvent();
}

void HelloTextureEngine::FlushGpu()
{
    for (UINT n = 0; n < kFrameCount; n++)
    {
        const UINT64 fenceValue = ++m_frameResources[n].fenceValue;

        m_graphicsDevice.SignalFence(fenceValue);
        m_graphicsDevice.WaitForFenceValue(fenceValue);
    }
}

// Prepare to render the next frame.
UINT64 HelloTextureEngine::MoveToNextFrame()
{
    PIXBeginEvent(2, L"MoveToNextFrame");

    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
    m_graphicsDevice.SignalFence(currentFenceValue);

    // Update the frame index.
    m_previousFrameIndex = m_currentFrameIndex;
    m_currentFrameIndex = m_graphicsDevice.CurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_graphicsDevice.CompletedFenceValue() < m_frameResources[m_currentFrameIndex].fenceValue)
    {
        PIXBeginEvent(4, L"WaitForSingleObjectEx");
        m_graphicsDevice.WaitForFenceValue(m_frameResources[m_currentFrameIndex].fenceValue);
        PIXEndEvent();
    }

    // Set the fence value for the next frame.
    m_frameResources[m_currentFrameIndex].fenceValue = currentFenceValue + 1;

    PIXEndEvent();

    return currentFenceValue;
}

