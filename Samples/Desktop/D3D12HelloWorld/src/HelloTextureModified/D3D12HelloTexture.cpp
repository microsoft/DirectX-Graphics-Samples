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

#include "D3D12HelloTexture.h"

#include <algorithm>
#include <windows.h>

#include "MyDx12Utils.h"
#include "Renderer/DebugDumpReport.h"
#include "Renderer/RootSignatureFactory.h"
// Forward declaration for the staged allocator smoke test.


#include <random>
#include <combaseapi.h>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <wrl\client.h>
#include <array>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>
#include "DXSampleHelper.h"
#include "GraphicsDevice.h"
#include "Renderer\ClearPass.h"
#include "Renderer\DebugDumpCapture.h"
#include "Renderer\GBuffer.h"
#include "Renderer\LightingPass.h"
#include "Renderer\Material.h"
#include "Renderer\PipelineFactory.h"
#include "Renderer\RayQueryShadowPass.h"
#include "Renderer\RayQueryTlasDebugPass.h"
#include "Renderer\RayTracingSupport.h"
#include "Renderer\RenderPassExecution.h"
#include "Renderer\RenderPassResources.h"
#include "Renderer\ResolvedRenderTargets.h"
#include "Renderer\SceneGeometryPass.h"
#include "Renderer\SimpleDescriptorHeapAllocator.h"
#include "Renderer\ToneMap.h"
#include "Scene\Scene.h"
#include <d3d12.h>
#include <d3dx12_barriers.h>
#include <d3dx12_core.h>
#include <d3dx12_resource_helpers.h>
#include <d3dx12_root_signature.h>
#include <dxgiformat.h>
#include <d3dcompiler.h>
#include <pix3.h>

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 618;
}
extern "C"
{
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace
{

const wchar_t* EnvironmentSourceName(Engine::EnvironmentSource source)
{
    switch (source)
    {
        case Engine::EnvironmentSource::AssetHdr:
            return L"AssetHdr";
        case Engine::EnvironmentSource::ProceduralStudio:
            return L"ProceduralStudio";
        case Engine::EnvironmentSource::ProceduralSun:
            return L"ProceduralSun";
        case Engine::EnvironmentSource::ProceduralColorPanels:
            return L"ProceduralColorPanels";
        case Engine::EnvironmentSource::ProceduralHorizon:
            return L"ProceduralHorizon";
        default:
            return L"Unknown";
    }
}

} // namespace

HelloTextureEngine::HelloTextureEngine(GraphicsDevice& graphicsDevice)
    : m_graphicsDevice(graphicsDevice), m_width(0), m_height(0), m_aspectRatio(1.0f), m_previousFrameIndex(0),
      m_currentFrameIndex(0), m_rtvDescriptorSize(0)
{
    m_assetsPath = L"./Assets\\";
    m_shaderPath = L"./bin\\x64\\Debug\\";

    RegisterPassBindingResolvers();
    RegisterPassConstantsHandlers();
    RegisterResourceResolvers();
}

void HelloTextureEngine::Initialize(UINT width, UINT height)
{
    assert(m_graphicsDevice.Device() != nullptr);
    assert(m_graphicsDevice.HasSwapChain());

    m_width = width;
    m_height = height;
    m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    InitializeFrameResources();
}

void HelloTextureEngine::InitializeFrameResources()
{
    m_rayTracingSupport = Engine::RayTracingSupportInfo::Create(m_graphicsDevice.Device());
    wchar_t debugMessage[128] = {};
    swprintf_s(debugMessage,
               L"Ray tracing support: supported=%s tier=%s raw=%d\n",
               m_rayTracingSupport.IsSupported() ? L"true" : L"false",
               m_rayTracingSupport.TierName(),
               static_cast<int>(m_rayTracingSupport.Tier()));
    OutputDebugStringW(debugMessage);

    m_prevTime = std::chrono::steady_clock::now();
    LoadPipeline();
    LoadAssets();
    InitResourceDefaultStates();
}

void HelloTextureEngine::InitResourceDefaultStates()
{
    m_resourceDefaultStates.clear();
    m_resourceDefaultStates.push_back({kBackBufferResourceName, D3D12_RESOURCE_STATE_PRESENT});
    m_resourceDefaultStates.push_back({kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE});
    m_resourceDefaultStates.push_back({kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET});
    m_resourceDefaultStates.push_back({kShadowMaskResourceName, D3D12_RESOURCE_STATE_UNORDERED_ACCESS});
    for (UINT i = 0; i < Engine::GBuffer::kCount; ++i)
    {
        m_resourceDefaultStates.push_back({kGBufferResourceNames[i], D3D12_RESOURCE_STATE_RENDER_TARGET});
    }
}

std::wstring HelloTextureEngine::GetAssetFullPath(LPCWSTR assetName)
{
    return m_assetsPath + assetName;
}

std::wstring HelloTextureEngine::GetShaderFullPath(LPCWSTR shaderName)
{
    return m_shaderPath + shaderName;
}

HelloTextureEngine::UiFrameContext HelloTextureEngine::GetUiFrameContext() const
{
    return {static_cast<int>(m_currentFrameIndex),
            m_cpuFrameTime,
            m_rayTracingSupport.IsSupported(),
            m_rayTracingSupport.TierName(),
            static_cast<int>(m_rayTracingSupport.Tier()),
            m_frameResources[m_previousFrameIndex].gpuWorkMeterCheckPoints};
}

void HelloTextureEngine::SetUpdateHandler(UpdateHandler handler)
{
    m_updateHandler = std::move(handler);
}

void HelloTextureEngine::SetLightingParams(const LightingParams& params)
{
    m_lightingParams = params;
}

void HelloTextureEngine::SetShadowSettings(const ShadowSettings& settings)
{
    m_shadowSettings = settings;
}

void HelloTextureEngine::SetMaterialParams(UINT materialIndex, const MaterialParams& params)
{
    if (materialIndex >= m_materialData.size())
    {
        return;
    }

    Engine::Material& material = m_materialData[materialIndex];
    material.roughnessFactor = params.roughnessFactor;
    material.metallicFactor = params.metallicFactor;
    material.ambientOcclusionFactor = params.ambientOcclusionFactor;
    material.emissiveScale = params.emissiveScale;
    m_materialBuffer.Update(m_materialData);
}

auto HelloTextureEngine::MakeLightingConstants() const -> LightingConstants
{
    return {
        m_lightingParams.lightDirection,
        m_lightingParams.iblIntensity,
        m_lightingParams.lightColor,
        m_lightingParams.diffuseIntensity,
        {m_backBufferClearColor[0], m_backBufferClearColor[1], m_backBufferClearColor[2], m_backBufferClearColor[3]},
        m_lightingParams.skyboxEnabled ? 1.0f : 0.0f,
        m_lightingParams.skyboxPreview ? 1.0f : 0.0f,
        m_lightingParams.skyboxPreviewExposure,
        m_debugViewSettings.IsLightPassDebugView()
            ? static_cast<float>(static_cast<int>(m_debugViewSettings.renderViewMode) -
                                 static_cast<int>(RenderViewMode::ReflectionDirection) + 1)
            : 0.0f,
        m_lightingParams.directLightEnabled ? 1.0f : 0.0f,
        m_lightingParams.diffuseIblEnabled ? 1.0f : 0.0f,
        m_lightingParams.specularIblEnabled ? 1.0f : 0.0f,
        m_lightingParams.emissiveEnabled ? 1.0f : 0.0f,
        m_lightingParams.iblDebugMip,
        m_lightingParams.iblDebugExposure,
        m_rayTracingSupport.IsSupported() ? 1.0f : 0.0f,
        m_shadowSettings.softShadowEnabled ? 1.0f : 0.0f,
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

void HelloTextureEngine::SetScene(const Scene& scene)
{
    m_scene = scene;
}

void HelloTextureEngine::ReloadSceneResources(const Scene& scene)
{
    WaitForGpu();
    ReleaseSceneResources();
    SetScene(scene);

    ThrowIfFailed(m_frameResources[m_currentFrameIndex].commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(),
                                       GetPipelineState(PipelineId(Pipe::Forward))));

    CreateSceneGeometryBuffers();

    std::vector<ComPtr<ID3D12Resource>> textureUploadHeap;
    CreateSceneTextureResources(textureUploadHeap);
    PrepareSceneInstanceData();
    CreateSceneMaterialResources();
    CreateInstanceBuffers();

    if (m_rayTracingSupport.IsSupported())
    {
        BuildAccelerationStructures();
    }

    UpdateCameraConstantBuffer();
    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
    for (FrameResource& frameResource : m_frameResources)
    {
        memcpy(frameResource.cameraCB.mappedData, &m_constantBufferData, sizeof(m_constantBufferData));
    }

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    WaitForGpu();
}

void HelloTextureEngine::CloseSceneResources()
{
    WaitForGpu();
    ReleaseSceneResources();
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

void HelloTextureEngine::ReloadEnvironmentResources(const Engine::ProceduralEnvironmentSettings& settings)
{
    MyDx12Util::ScopedTimer _reloadTimer("ReloadEnvironmentResources total");

    m_environmentSettings = settings;

    WCHAR debugMessage[160] = {};
    swprintf_s(debugMessage, L"ReloadEnvironmentResources source=%s\n", EnvironmentSourceName(settings.source));
    OutputDebugStringW(debugMessage);

    if constexpr (Engine::kUseGpuProceduralEnvMap)
    {
        if (m_environmentSettings.source != Engine::EnvironmentSource::AssetHdr)
        {
            CollectDeferredGpuReleases();
            if (m_environmentMap.Resource() != nullptr)
            {
                const UINT64 retireFenceValue = SignalFenceForQueuedGpuWork();
                QueueActiveEnvironmentResourcesForRelease(retireFenceValue);
            }

            ComPtr<ID3D12CommandAllocator> proceduralEnvCommandAllocator;
            ThrowIfFailed(m_graphicsDevice.Device()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&proceduralEnvCommandAllocator)));
            ThrowIfFailed(m_commandList->Reset(proceduralEnvCommandAllocator.Get(), nullptr));

            ComPtr<ID3D12Resource> diffuseIrradianceUploadHeap;
            ComPtr<ID3D12Resource> specularPrefilterUploadHeap;
            CreateEnvironmentMapResourcesGpu(diffuseIrradianceUploadHeap,
                                             specularPrefilterUploadHeap);

            ThrowIfFailed(m_commandList->Close());
            ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
            m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
            const UINT64 generationFenceValue = SignalFenceForQueuedGpuWork();
            QueueProceduralEnvGenerationResourcesForRelease(generationFenceValue, proceduralEnvCommandAllocator);
            return;
        }
    }

    {
        MyDx12Util::ScopedTimer _waitGpu("  WaitForGpu (before reload)");
        WaitForGpu();
    }
    CollectDeferredGpuReleases();
    ThrowIfFailed(m_frameResources[m_currentFrameIndex].commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(), nullptr));

    ReleaseEnvironmentMapResources();

    ComPtr<ID3D12Resource> environmentMapUploadHeap;
    ComPtr<ID3D12Resource> diffuseIrradianceUploadHeap;
    ComPtr<ID3D12Resource> specularPrefilterUploadHeap;
    {
        MyDx12Util::ScopedTimer _cpuPath("  CreateEnvironmentMapResources (CPU path)");
        CreateEnvironmentMapResources(environmentMapUploadHeap, diffuseIrradianceUploadHeap, specularPrefilterUploadHeap);
    }

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    {
        MyDx12Util::ScopedTimer _waitGpu("  WaitForGpu (CPU path)");
        WaitForGpu();
    }
}

void HelloTextureEngine::UpdateCameraConstantBuffer()
{
    m_scene.camera.fov = std::clamp(m_scene.camera.fov, 0.1f, 179.0f);
    const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    const XMVECTOR eye = XMLoadFloat3(&m_scene.camera.pos);
    const XMVECTOR at = XMLoadFloat3(&m_scene.camera.gazePoint);
    const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    const XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    const XMMATRIX projection =
        XMMatrixPerspectiveFovLH(XMConvertToRadians(m_scene.camera.fov), aspect, kCameraNearZ, kCameraFarZ);
    const XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(viewProjection));
    XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                    XMMatrixTranspose(XMMatrixInverse(nullptr, viewProjection)));
    m_constantBufferData.cameraPosition = m_scene.camera.pos;
}

// Load the rendering pipeline dependencies.
void HelloTextureEngine::LoadPipeline()
{
    assert(m_graphicsDevice.Device() != nullptr);
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
        ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize =
            m_graphicsDevice.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a single shader-visible heap for CBV/SRV/UAV.
        // The heap is sized to hold both the regular descriptors (managed by
        // SimpleDescriptorHeapAllocator) and a reserved tail region for staging
        // copies (managed by StagedDescriptorAllocator).
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        // Total heap = regular descriptors + per-frame staged chunks.
        heapDesc.NumDescriptors = kMainHeapDescriptorCount + kStagedDescriptorReservedCount * kFrameCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));

        // Create a descriptor allocator limited to the regular (non-staged) region.
        m_descriptorHeapAllocator.Init(m_graphicsDevice.Device(), m_heap.Get(), kMainHeapDescriptorCount);

        // Initialize staged descriptor allocator for ShadowMask (and future transient descriptors).
        // The allocator owns a CPU heap and copies into a reserved range of the main GPU heap.
        {
            D3D12_CPU_DESCRIPTOR_HANDLE mainCpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
            D3D12_GPU_DESCRIPTOR_HANDLE mainGpuStart = m_heap->GetGPUDescriptorHandleForHeapStart();
            m_stageAllocator.Init(m_graphicsDevice.Device(),
                                  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                  4,
                                  mainCpuStart,
                                  mainGpuStart,
                                  kMainHeapDescriptorCount,
                                  kStagedDescriptorReservedCount);
        }
    }

    // create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            m_graphicsDevice.GetBackBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            m_graphicsDevice.Device()->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
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
        ThrowIfFailed(m_graphicsDevice.Device()->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[n].commandAllocator)));
    }

    //
    m_gpuWorkMeter.Init(m_graphicsDevice.Device(),
                        kGpuWorkMeterQueryCount); // Initialize GPU work meter with a maximum of 100 timestamp queries.

}

void HelloTextureEngine::UpdateHdr10DisplayMode()
{
    m_graphicsDevice.EnsureCurrentDxgiFactory();
    m_hdrOutputPolicy.Update(m_graphicsDevice.DxgiFactory(), m_graphicsDevice.SwapChain(), m_graphicsDevice.Hwnd());
}

DescriptorAllocation HelloTextureEngine::CreateTextureFromRGBA8(
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
    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &textureDesc,
                                                                     D3D12_RESOURCE_STATE_COPY_DEST,
                                                                     nullptr,
                                                                     IID_PPV_ARGS(&texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
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

void HelloTextureEngine::CreateEnvironmentMapResources(ComPtr<ID3D12Resource>& environmentMapUploadHeap,
                                                       ComPtr<ID3D12Resource>& diffuseIrradianceUploadHeap,
                                                       ComPtr<ID3D12Resource>& specularPrefilterUploadHeap)
{
    if (m_environmentSettings.source == Engine::EnvironmentSource::AssetHdr)
    {
        const std::wstring hdrPath = GetAssetFullPath(L"Environment\\default_environment.hdr");
        Engine::HdrImage hdrImage = {};
        if (Engine::TryLoadHdrImage(hdrPath.c_str(), hdrImage))
        {
            WCHAR debugMessage[256] = {};
            swprintf_s(debugMessage, L"Loaded HDRI: %s (%u x %u)\n", hdrPath.c_str(), hdrImage.width, hdrImage.height);
            OutputDebugStringW(debugMessage);

            const bool environmentMapCreated =
                m_environmentMap.TryCreateFromHdrEquirectangular(m_graphicsDevice.Device(),
                                                                 m_commandList.Get(),
                                                                 m_descriptorHeapAllocator,
                                                                 hdrImage,
                                                                 kEnvironmentMapCubeSize,
                                                                 false,
                                                                 environmentMapUploadHeap);
            const bool diffuseIrradianceCreated =
                m_diffuseIrradianceMap.TryCreateFromHdrEquirectangular(m_graphicsDevice.Device(),
                                                                       m_commandList.Get(),
                                                                       m_descriptorHeapAllocator,
                                                                       hdrImage,
                                                                       kDiffuseIrradianceCubeSize,
                                                                       true,
                                                                       diffuseIrradianceUploadHeap);
            const bool specularPrefilterCreated =
                m_specularPrefilterMap.TryCreateSpecularPrefilterFromHdrEquirectangular(m_graphicsDevice.Device(),
                                                                                        m_commandList.Get(),
                                                                                        m_descriptorHeapAllocator,
                                                                                        hdrImage,
                                                                                        kSpecularPrefilterCubeSize,
                                                                                        kSpecularPrefilterMipCount,
                                                                                        specularPrefilterUploadHeap);
            if (environmentMapCreated && diffuseIrradianceCreated && specularPrefilterCreated)
            {
                return;
            }
        }

        WCHAR fallbackMessage[256] = {};
        swprintf_s(fallbackMessage, L"Falling back to procedural envmap: %s\n", hdrPath.c_str());
        OutputDebugStringW(fallbackMessage);
    }

    Engine::ProceduralEnvironmentSettings fallbackSettings = m_environmentSettings;
    if (fallbackSettings.source == Engine::EnvironmentSource::AssetHdr)
    {
        fallbackSettings.source = Engine::EnvironmentSource::ProceduralSun;
    }

    WCHAR debugMessage[160] = {};
    swprintf_s(
        debugMessage, L"Creating procedural environment source=%s\n", EnvironmentSourceName(fallbackSettings.source));
    OutputDebugStringW(debugMessage);

    m_environmentMap.CreateProcedural(m_graphicsDevice.Device(),
                                      m_commandList.Get(),
                                      m_descriptorHeapAllocator,
                                      fallbackSettings,
                                      kEnvironmentMapCubeSize,
                                      false,
                                      environmentMapUploadHeap);
    m_diffuseIrradianceMap.CreateProcedural(m_graphicsDevice.Device(),
                                            m_commandList.Get(),
                                            m_descriptorHeapAllocator,
                                            fallbackSettings,
                                            kDiffuseIrradianceCubeSize,
                                            true,
                                            diffuseIrradianceUploadHeap);
    m_specularPrefilterMap.CreateSpecularPrefilterProcedural(m_graphicsDevice.Device(),
                                                             m_commandList.Get(),
                                                             m_descriptorHeapAllocator,
                                                             fallbackSettings,
                                                             kSpecularPrefilterCubeSize,
                                                             kSpecularPrefilterMipCount,
                                                             specularPrefilterUploadHeap);
}

void HelloTextureEngine::ReleaseEnvironmentMapResources()
{
    if (m_environmentDescriptorTable.IsValid())
    {
        ComPtr<ID3D12Resource> resource;
        DescriptorHeapHandle srv;
        m_specularPrefilterMap.Detach(resource, srv);
        m_diffuseIrradianceMap.Detach(resource, srv);
        m_environmentMap.Detach(resource, srv);
        FreeEnvironmentDescriptorTable(m_environmentDescriptorTable);
        m_environmentDescriptorTable = {};
        return;
    }

    m_specularPrefilterMap.Release(m_descriptorHeapAllocator);
    m_diffuseIrradianceMap.Release(m_descriptorHeapAllocator);
    m_environmentMap.Release(m_descriptorHeapAllocator);
}

UINT64 HelloTextureEngine::SignalFenceForQueuedGpuWork()
{
    const UINT64 fenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
    m_graphicsDevice.SignalFence(fenceValue);
    m_frameResources[m_currentFrameIndex].fenceValue++;
    return fenceValue;
}

Engine::DeferredGpuRelease HelloTextureEngine::CreateActiveEnvironmentRelease(UINT64 retireFenceValue)
{
    Engine::DeferredGpuRelease release = {};
    release.retireFenceValue = retireFenceValue;

    ComPtr<ID3D12Resource> environmentMap;
    ComPtr<ID3D12Resource> diffuseIrradianceMap;
    ComPtr<ID3D12Resource> specularPrefilterMap;
    DescriptorHeapHandle environmentSrv;
    DescriptorHeapHandle diffuseIrradianceSrv;
    DescriptorHeapHandle specularPrefilterSrv;

    m_environmentMap.Detach(environmentMap, environmentSrv);
    m_diffuseIrradianceMap.Detach(diffuseIrradianceMap, diffuseIrradianceSrv);
    m_specularPrefilterMap.Detach(specularPrefilterMap, specularPrefilterSrv);

    release.resources.push_back(std::move(environmentMap));
    release.resources.push_back(std::move(diffuseIrradianceMap));
    release.resources.push_back(std::move(specularPrefilterMap));

    release.descriptorHandles = CreateEnvironmentDescriptorHandles(m_environmentDescriptorTable,
                                                                   environmentSrv,
                                                                   diffuseIrradianceSrv,
                                                                   specularPrefilterSrv);

    return release;
}

void HelloTextureEngine::QueueActiveEnvironmentResourcesForRelease(UINT64 retireFenceValue)
{
    Engine::DeferredGpuRelease release = CreateActiveEnvironmentRelease(retireFenceValue);
    m_environmentDescriptorTable = {};

    m_deferredGpuReleaseQueue.Retire(std::move(release));
}

Engine::DeferredGpuRelease HelloTextureEngine::CreateProceduralEnvGenerationRelease(
    UINT64 retireFenceValue,
    ComPtr<ID3D12CommandAllocator> proceduralEnvCommandAllocator)
{
    Engine::DeferredGpuRelease release = {};
    release.retireFenceValue = retireFenceValue;
    release.commandAllocators.push_back(std::move(proceduralEnvCommandAllocator));
    release.descriptorHeaps.push_back(std::move(m_proceduralEnvUavHeap));
    release.resources.push_back(std::move(m_proceduralEnvSettingsBuffer));

    return release;
}

void HelloTextureEngine::QueueProceduralEnvGenerationResourcesForRelease(
    UINT64 retireFenceValue,
    ComPtr<ID3D12CommandAllocator> proceduralEnvCommandAllocator)
{
    Engine::DeferredGpuRelease release =
        CreateProceduralEnvGenerationRelease(retireFenceValue, std::move(proceduralEnvCommandAllocator));
    m_deferredGpuReleaseQueue.Retire(std::move(release));
}

void HelloTextureEngine::CollectDeferredGpuReleases()
{
    const UINT64 completedFenceValue = m_graphicsDevice.CompletedFenceValue();
    m_deferredGpuReleaseQueue.Collect(completedFenceValue, m_descriptorHeapAllocator);
}

std::vector<DescriptorHeapHandle> HelloTextureEngine::CreateEnvironmentDescriptorHandles(
    const EnvironmentDescriptorTable& table,
    DescriptorHeapHandle environmentSrv,
    DescriptorHeapHandle diffuseIrradianceSrv,
    DescriptorHeapHandle specularPrefilterSrv) const
{
    if (table.IsValid())
    {
        return {table.environment, table.diffuseIrradiance, table.specularPrefilter, table.brdfLut};
    }

    return {environmentSrv, diffuseIrradianceSrv, specularPrefilterSrv};
}

auto HelloTextureEngine::AllocateEnvironmentDescriptorTable() -> EnvironmentDescriptorTable
{
    EnvironmentDescriptorTable table = {};
    table.environment = m_descriptorHeapAllocator.AllocContiguous(kEnvironmentDescriptorTableSize);
    table.diffuseIrradiance = m_descriptorHeapAllocator.HandleFromIndex(table.environment.Index + 1);
    table.specularPrefilter = m_descriptorHeapAllocator.HandleFromIndex(table.environment.Index + 2);
    table.brdfLut = m_descriptorHeapAllocator.HandleFromIndex(table.environment.Index + 3);
    return table;
}

void HelloTextureEngine::FreeEnvironmentDescriptorTable(const EnvironmentDescriptorTable& table)
{
    if (!table.IsValid())
    {
        return;
    }

    m_descriptorHeapAllocator.Free(table.brdfLut);
    m_descriptorHeapAllocator.Free(table.specularPrefilter);
    m_descriptorHeapAllocator.Free(table.diffuseIrradiance);
    m_descriptorHeapAllocator.Free(table.environment);
}

void HelloTextureEngine::CreateEnvironmentDescriptorTableSrvs(ID3D12Resource* environmentMap,
                                                              ID3D12Resource* diffuseIrradianceMap,
                                                              ID3D12Resource* specularPrefilterMap,
                                                              const EnvironmentDescriptorTable& table)
{
    auto* device = m_graphicsDevice.Device();

    D3D12_SHADER_RESOURCE_VIEW_DESC cubeSrvDesc = {};
    cubeSrvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    cubeSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    cubeSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    cubeSrvDesc.TextureCube.MostDetailedMip = 0;

    cubeSrvDesc.TextureCube.MipLevels = 1;
    device->CreateShaderResourceView(environmentMap, &cubeSrvDesc, table.environment.cpu);
    device->CreateShaderResourceView(diffuseIrradianceMap, &cubeSrvDesc, table.diffuseIrradiance.cpu);

    cubeSrvDesc.TextureCube.MipLevels = kSpecularPrefilterMipCount;
    device->CreateShaderResourceView(specularPrefilterMap, &cubeSrvDesc, table.specularPrefilter.cpu);

    CreateEnvironmentBrdfLutSrv(table);
}

void HelloTextureEngine::CreateEnvironmentBrdfLutSrv(const EnvironmentDescriptorTable& table)
{
    if (!table.IsValid() || m_brdfLut.Resource() == nullptr)
    {
        return;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC brdfSrvDesc = {};
    brdfSrvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
    brdfSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    brdfSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    brdfSrvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(m_brdfLut.Resource(), &brdfSrvDesc, table.brdfLut.cpu);
}

void HelloTextureEngine::CreateEnvironmentMapResourcesGpu(ComPtr<ID3D12Resource>& diffuseIrradianceUploadHeap,
                                                          ComPtr<ID3D12Resource>& specularPrefilterUploadHeap)
{
    MyDx12Util::ScopedTimer _total("  CreateEnvironmentMapResourcesGpu total");
    (void)diffuseIrradianceUploadHeap;
    (void)specularPrefilterUploadHeap;

    auto* device = m_graphicsDevice.Device();
    auto* commandList = m_commandList.Get();

    Engine::ProceduralEnvironmentSettings settings = m_environmentSettings;
    if (settings.source == Engine::EnvironmentSource::AssetHdr)
    {
        settings.source = Engine::EnvironmentSource::ProceduralSun;
    }

    static constexpr UINT kEnvironmentMapFaceCount = 6;
    static constexpr int kGenerateEnvironment = 0;
    static constexpr int kGenerateDiffuseIrradiance = 1;
    static constexpr int kGenerateSpecularPrefilter = 2;

    ComPtr<ID3D12Resource> environmentMap;
    ComPtr<ID3D12Resource> diffuseIrradianceMap;
    ComPtr<ID3D12Resource> specularPrefilterMap;
    {
        MyDx12Util::ScopedTimer _createRes("    Create texture resources");

        auto createUavCubeResource = [device](UINT size, UINT mipCount, ComPtr<ID3D12Resource>& resource)
        {
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.MipLevels = static_cast<UINT16>(mipCount);
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            textureDesc.Width = size;
            textureDesc.Height = size;
            textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            textureDesc.DepthOrArraySize = kEnvironmentMapFaceCount;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                          D3D12_HEAP_FLAG_NONE,
                                                          &textureDesc,
                                                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                          nullptr,
                                                          IID_PPV_ARGS(&resource)));
        };

        createUavCubeResource(kEnvironmentMapCubeSize, 1, environmentMap);
        createUavCubeResource(kDiffuseIrradianceCubeSize, 1, diffuseIrradianceMap);
        createUavCubeResource(kSpecularPrefilterCubeSize, kSpecularPrefilterMipCount, specularPrefilterMap);
    }

    struct alignas(16) GpuSettings
    {
        int source;
        int pad0[3];
        DirectX::XMFLOAT4 skyColor;
        DirectX::XMFLOAT4 groundColor;
        DirectX::XMFLOAT4 lightColor;
        DirectX::XMFLOAT4 lightDirection;
        float backgroundIntensity;
        float lightIntensity;
        float lightSize;
        float fillIntensity;
        float colorPanelIntensity;
        float horizonSharpness;
        int outputSize;
        int generationMode;
        float roughness;
        int pad2;
    };

    static_assert(sizeof(GpuSettings) % 16 == 0);
    const UINT gpuSettingsStride = (sizeof(GpuSettings) + 255u) & ~255u;
    const UINT dispatchCount = 2 + kSpecularPrefilterMipCount;
    std::vector<GpuSettings> gpuSettings(dispatchCount);

    auto makeGpuSettings = [&settings](int generationMode, UINT outputSize, float roughness)
    {
        GpuSettings result = {};
        result.source = static_cast<int>(settings.source);
        result.skyColor = DirectX::XMFLOAT4(settings.skyColor.x, settings.skyColor.y, settings.skyColor.z, 0.0f);
        result.groundColor =
            DirectX::XMFLOAT4(settings.groundColor.x, settings.groundColor.y, settings.groundColor.z, 0.0f);
        result.lightColor =
            DirectX::XMFLOAT4(settings.lightColor.x, settings.lightColor.y, settings.lightColor.z, 0.0f);
        result.lightDirection =
            DirectX::XMFLOAT4(settings.lightDirection.x, settings.lightDirection.y, settings.lightDirection.z, 0.0f);
        result.backgroundIntensity = settings.backgroundIntensity;
        result.lightIntensity = settings.lightIntensity;
        result.lightSize = settings.lightSize;
        result.fillIntensity = settings.fillIntensity;
        result.colorPanelIntensity = settings.colorPanelIntensity;
        result.horizonSharpness = settings.horizonSharpness;
        result.outputSize = static_cast<int>(outputSize);
        result.generationMode = generationMode;
        result.roughness = roughness;
        return result;
    };

    gpuSettings[0] = makeGpuSettings(kGenerateEnvironment, kEnvironmentMapCubeSize, 0.0f);
    gpuSettings[1] = makeGpuSettings(kGenerateDiffuseIrradiance, kDiffuseIrradianceCubeSize, 0.0f);
    for (UINT mip = 0; mip < kSpecularPrefilterMipCount; ++mip)
    {
        const UINT mipSize = (std::max)(1u, kSpecularPrefilterCubeSize >> mip);
        const float roughness = kSpecularPrefilterMipCount > 1
                                    ? static_cast<float>(mip) / static_cast<float>(kSpecularPrefilterMipCount - 1)
                                    : 0.0f;
        gpuSettings[2 + mip] = makeGpuSettings(kGenerateSpecularPrefilter, mipSize, roughness);
    }

    {
        MyDx12Util::ScopedTimer _createCb("    Create cbuffer");

        const UINT cbSize = gpuSettingsStride * dispatchCount;
        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                      D3D12_HEAP_FLAG_NONE,
                                                      &CD3DX12_RESOURCE_DESC::Buffer(cbSize),
                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                      nullptr,
                                                      IID_PPV_ARGS(&m_proceduralEnvSettingsBuffer)));

        void* mappedData = nullptr;
        ThrowIfFailed(m_proceduralEnvSettingsBuffer->Map(0, nullptr, &mappedData));
        auto* mappedBytes = static_cast<UINT8*>(mappedData);
        for (UINT i = 0; i < dispatchCount; ++i)
        {
            memcpy(mappedBytes + gpuSettingsStride * i, &gpuSettings[i], sizeof(GpuSettings));
        }
        m_proceduralEnvSettingsBuffer->Unmap(0, nullptr);
    }

    D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
    uavHeapDesc.NumDescriptors = dispatchCount;
    uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&m_proceduralEnvUavHeap)));

    const UINT uavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const CD3DX12_CPU_DESCRIPTOR_HANDLE uavCpuStart(m_proceduralEnvUavHeap->GetCPUDescriptorHandleForHeapStart());
    const CD3DX12_GPU_DESCRIPTOR_HANDLE uavGpuStart(m_proceduralEnvUavHeap->GetGPUDescriptorHandleForHeapStart());

    auto createTextureCubeUav =
        [device, uavCpuStart, uavDescriptorSize](ID3D12Resource* resource, UINT mip, UINT descriptorIndex)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice = mip;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.ArraySize = kEnvironmentMapFaceCount;
        device->CreateUnorderedAccessView(
            resource,
            nullptr,
            &uavDesc,
            CD3DX12_CPU_DESCRIPTOR_HANDLE(uavCpuStart, descriptorIndex, uavDescriptorSize));
    };

    createTextureCubeUav(environmentMap.Get(), 0, 0);
    createTextureCubeUav(diffuseIrradianceMap.Get(), 0, 1);
    for (UINT mip = 0; mip < kSpecularPrefilterMipCount; ++mip)
    {
        createTextureCubeUav(specularPrefilterMap.Get(), mip, 2 + mip);
    }

    ID3D12DescriptorHeap* const descriptorHeaps[] = {m_proceduralEnvUavHeap.Get()};
    commandList->SetComputeRootSignature(m_proceduralEnvRootSignature.Get());
    commandList->SetPipelineState(m_proceduralEnvPipeline.Get());
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    {
        MyDx12Util::ScopedTimer _dispatch("    Dispatch environment + irradiance + prefilter");

        auto dispatchProceduralEnv = [commandList,
                                      uavGpuStart,
                                      uavDescriptorSize,
                                      settingsBuffer = m_proceduralEnvSettingsBuffer.Get(),
                                      gpuSettingsStride](UINT descriptorIndex, UINT settingsIndex, UINT outputSize)
        {
            const UINT dispatchX = (outputSize + 7) / 8;
            const UINT dispatchY = (outputSize + 7) / 8;
            commandList->SetComputeRootDescriptorTable(
                0, CD3DX12_GPU_DESCRIPTOR_HANDLE(uavGpuStart, descriptorIndex, uavDescriptorSize));
            commandList->SetComputeRootConstantBufferView(
                1, settingsBuffer->GetGPUVirtualAddress() + gpuSettingsStride * settingsIndex);
            commandList->Dispatch(dispatchX, dispatchY, kEnvironmentMapFaceCount);
        };

        dispatchProceduralEnv(0, 0, kEnvironmentMapCubeSize);
        dispatchProceduralEnv(1, 1, kDiffuseIrradianceCubeSize);
        for (UINT mip = 0; mip < kSpecularPrefilterMipCount; ++mip)
        {
            const UINT mipSize = (std::max)(1u, kSpecularPrefilterCubeSize >> mip);
            dispatchProceduralEnv(2 + mip, 2 + mip, mipSize);
        }

        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(environmentMap.Get(),
                                                 D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(diffuseIrradianceMap.Get(),
                                                 D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(specularPrefilterMap.Get(),
                                                 D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
        };
        commandList->ResourceBarrier(_countof(barriers), barriers);
    }

    m_environmentDescriptorTable = AllocateEnvironmentDescriptorTable();
    CreateEnvironmentDescriptorTableSrvs(environmentMap.Get(),
                                         diffuseIrradianceMap.Get(),
                                         specularPrefilterMap.Get(),
                                         m_environmentDescriptorTable);

    m_environmentMap.Attach(std::move(environmentMap), m_environmentDescriptorTable.environment);
    m_diffuseIrradianceMap.Attach(std::move(diffuseIrradianceMap), m_environmentDescriptorTable.diffuseIrradiance);
    m_specularPrefilterMap.Attach(std::move(specularPrefilterMap), m_environmentDescriptorTable.specularPrefilter);
}

void HelloTextureEngine::ValidateEnvironmentMapDescriptorTable() const
{
    const UINT environmentMapIndex = m_environmentMap.Srv().Index;
    const UINT diffuseIrradianceMapIndex = m_diffuseIrradianceMap.Srv().Index;
    const UINT specularPrefilterMapIndex = m_specularPrefilterMap.Srv().Index;
    const UINT brdfLutIndex =
        m_environmentDescriptorTable.IsValid() ? m_environmentDescriptorTable.brdfLut.Index : m_brdfLut.Srv().Index;
    const bool descriptorsAreContiguous = diffuseIrradianceMapIndex == environmentMapIndex + 1 &&
                                          specularPrefilterMapIndex == diffuseIrradianceMapIndex + 1 &&
                                          brdfLutIndex == specularPrefilterMapIndex + 1;

    WCHAR message[160] = {};
    swprintf_s(message,
               L"Environment descriptor table: env=%u diffuse=%u specular=%u brdf=%u contiguous=%s\n",
               environmentMapIndex,
               diffuseIrradianceMapIndex,
               specularPrefilterMapIndex,
               brdfLutIndex,
               descriptorsAreContiguous ? L"true" : L"false");
    OutputDebugStringW(message);

    assert(descriptorsAreContiguous &&
           "Environment, diffuse irradiance, specular prefilter, and BRDF LUT descriptors must be contiguous.");
}

// Load the sample assets.
void HelloTextureEngine::LoadAssets()
{
    CreateRootSignature();
    CreateProceduralEnvRootSignature();
    CreateRayQueryShadowRootSignature();
    CreateRayQueryTlasDebugRootSignature();
    CreatePipelineStates();
    CreateGBuffer();
    CreateShadowMask(m_width, m_height);
    CreateInitialCommandList();

    // Close the initial command list so ReloadEnvironmentResources can reset it.
    ThrowIfFailed(m_commandList->Close());

    // Create fence early so WaitForGpu() can be called during resource creation.
    m_graphicsDevice.CreateFence(0);
    m_frameResources[m_currentFrameIndex].fenceValue = 1;

    ReloadEnvironmentResources(m_environmentSettings);

    // ReloadEnvironmentResources closes the command list after execution.
    // Reopen for remaining setup commands.
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(), nullptr));

    ComPtr<ID3D12Resource> brdfLutUploadHeap;
    m_brdfLut.Create(
        m_graphicsDevice.Device(), m_commandList.Get(), m_descriptorHeapAllocator, kBrdfLutSize, brdfLutUploadHeap);
    CreateEnvironmentBrdfLutSrv(m_environmentDescriptorTable);
    ValidateEnvironmentMapDescriptorTable();
    CreateFrameConstantBuffers();
    ExecuteInitialGpuSetup();
}

void HelloTextureEngine::CreateRootSignature()
{
    Engine::CreateRootSignature(
        m_graphicsDevice.Device(), kTextureDescriptorCapacity, Engine::GBuffer::kCount + 1, m_rootSignature);
}

void HelloTextureEngine::CreateProceduralEnvRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE1 uavRange = {};
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
    rootParameters[0].InitAsDescriptorTable(1, &uavRange); // g_output (RWTexture2DArray)
    rootParameters[1].InitAsConstantBufferView(0);         // g_settings (b0)

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
    ThrowIfFailed(m_graphicsDevice.Device()->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_proceduralEnvRootSignature)));
}

void HelloTextureEngine::CreateRayQueryShadowRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE1 uavRange = {};
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE1 tlasSrvRange = {};
    tlasSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE1 depthSrvRange = {};
    depthSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE1 normalSrvRange = {};
    normalSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

    CD3DX12_DESCRIPTOR_RANGE1 cameraCbvRange = {};
    cameraCbvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);

    CD3DX12_ROOT_PARAMETER1 rootParameters[6] = {};
    rootParameters[0].InitAsDescriptorTable(1, &uavRange);       // g_shadowMask (u0)
    rootParameters[1].InitAsDescriptorTable(1, &tlasSrvRange);   // g_tlas (t0)
    rootParameters[2].InitAsDescriptorTable(1, &depthSrvRange);  // g_depth (t1)
    rootParameters[3].InitAsDescriptorTable(1, &normalSrvRange); // g_normal (t2)
    rootParameters[4].InitAsDescriptorTable(1, &cameraCbvRange); // CameraCB (b0)
    rootParameters[5].InitAsConstants(11, 1, 0);                 // ShadowConstants (b1): lightDirection, normalBias, rayTMin, rayTMax, enabled,
                                                                 //                      softShadowEnabled, sampleCount, lightAngularRadius, jitterStrength

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_graphicsDevice.Device()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    const D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion = featureData.HighestVersion;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, rootSignatureVersion, &signature, &error));
    ThrowIfFailed(m_graphicsDevice.Device()->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rayQueryShadowRootSignature)));
}

void HelloTextureEngine::CreateRayQueryTlasDebugRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE1 uavRange = {};
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE1 tlasSrvRange = {};
    tlasSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE1 depthSrvRange = {};
    depthSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE1 normalSrvRange = {};
    normalSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

    CD3DX12_DESCRIPTOR_RANGE1 cameraCbvRange = {};
    cameraCbvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);

    CD3DX12_ROOT_PARAMETER1 rootParameters[6] = {};
    rootParameters[0].InitAsDescriptorTable(1, &uavRange);       // g_tlasDebug (u0)
    rootParameters[1].InitAsDescriptorTable(1, &tlasSrvRange);   // g_tlas (t0)
    rootParameters[2].InitAsDescriptorTable(1, &depthSrvRange);  // g_depth (t1)
    rootParameters[3].InitAsDescriptorTable(1, &normalSrvRange); // g_normal (t2)
    rootParameters[4].InitAsDescriptorTable(1, &cameraCbvRange); // CameraCB (b0)
    rootParameters[5].InitAsConstants(3, 1, 0);                  // ShadowConstants lightDirection (b1, 3 floats)

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_graphicsDevice.Device()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    const D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion = featureData.HighestVersion;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, rootSignatureVersion, &signature, &error));
    ThrowIfFailed(m_graphicsDevice.Device()->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rayQueryTlasDebugRootSignature)));
}

auto HelloTextureEngine::LoadShaderBytecode(LPCWSTR assetName) -> ShaderBytecode
{
    UINT8* data = nullptr;
    UINT size = 0;
    ThrowIfFailed(ReadDataFromFile(GetShaderFullPath(assetName).c_str(), &data, &size));
    return {data, size};
}

auto HelloTextureEngine::LoadPipelineShaderBytecode() -> PipelineShaderBytecode
{
    PipelineShaderBytecode shaders = {};
    shaders.forward = {LoadShaderBytecode(L"shaders_VSMain.cso"), LoadShaderBytecode(L"shaders_PSMain.cso")};
    shaders.depthPrePass = {LoadShaderBytecode(L"shaders_DepthOnlyVS_VSMain.cso"), {}};
    shaders.gbuffer = {LoadShaderBytecode(L"shaders_GBuffer_VSMain.cso"),
                       LoadShaderBytecode(L"shaders_GBuffer_PSMain.cso")};
    shaders.gbufferDebug = {LoadShaderBytecode(L"shaders_GBufferDebug_VSMain.cso"),
                            LoadShaderBytecode(L"shaders_GBufferDebug_PSMain.cso")};
    shaders.shadowMaskDebug = {LoadShaderBytecode(L"shaders_ShadowMaskDebug_VSMain.cso"),
                               LoadShaderBytecode(L"shaders_ShadowMaskDebug_PSMain.cso")};
    shaders.lighting = {LoadShaderBytecode(L"shaders_LightPass_VSMain.cso"),
                        LoadShaderBytecode(L"shaders_LightPass_PSMain.cso")};
    shaders.lightingDebugGradient = {LoadShaderBytecode(L"shaders_LightPassDebugGradient_VSMain.cso"),
                                     LoadShaderBytecode(L"shaders_LightPassDebugGradient_PSMain.cso")};
    shaders.toneMap = {LoadShaderBytecode(L"shaders_ToneMap_VSMain.cso"),
                       LoadShaderBytecode(L"shaders_ToneMap_PSMain.cso")};
    shaders.proceduralEnv = LoadShaderBytecode(L"shaders_ProceduralEnvMap_CSMain.cso");
    shaders.rayQueryShadow = LoadShaderBytecode(L"shaders_RayQueryShadow_CSMain.cso");
    shaders.rayQueryTlasDebug = LoadShaderBytecode(L"shaders_RayQueryTlasDebug_CSMain.cso");
    return shaders;
}

void HelloTextureEngine::CreatePipelineStates()
{
    const PipelineShaderBytecode shaders = LoadPipelineShaderBytecode();
    RegisterPipelineStates(shaders);

    D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
    computeDesc.pRootSignature = m_proceduralEnvRootSignature.Get();
    computeDesc.CS = CD3DX12_SHADER_BYTECODE(shaders.proceduralEnv.data, shaders.proceduralEnv.size);
    ThrowIfFailed(
        m_graphicsDevice.Device()->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&m_proceduralEnvPipeline)));

    D3D12_COMPUTE_PIPELINE_STATE_DESC rayQueryShadowDesc = {};
    rayQueryShadowDesc.pRootSignature = m_rayQueryShadowRootSignature.Get();
    rayQueryShadowDesc.CS = CD3DX12_SHADER_BYTECODE(shaders.rayQueryShadow.data, shaders.rayQueryShadow.size);
    ThrowIfFailed(m_graphicsDevice.Device()->CreateComputePipelineState(&rayQueryShadowDesc,
                                                                         IID_PPV_ARGS(&m_rayQueryShadowPipeline)));

    D3D12_COMPUTE_PIPELINE_STATE_DESC rayQueryTlasDebugDesc = {};
    rayQueryTlasDebugDesc.pRootSignature = m_rayQueryTlasDebugRootSignature.Get();
    rayQueryTlasDebugDesc.CS = CD3DX12_SHADER_BYTECODE(shaders.rayQueryTlasDebug.data, shaders.rayQueryTlasDebug.size);
    ThrowIfFailed(m_graphicsDevice.Device()->CreateComputePipelineState(&rayQueryTlasDebugDesc,
                                                                         IID_PPV_ARGS(&m_rayQueryTlasDebugPipeline)));
}

void HelloTextureEngine::RegisterPipelineStates(const PipelineShaderBytecode& shaders)
{
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"MATERIALID", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    D3D12_INPUT_ELEMENT_DESC depthLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
    const InputLayoutDefinition meshInputLayout = {inputElementDescs, _countof(inputElementDescs)};
    const InputLayoutDefinition depthInputLayout = {depthLayout, _countof(depthLayout)};

    //
    // Forward Pass PSO
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    RegisterForwardPipeline(
        psoDesc,
        {Pipe::Forward, meshInputLayout, shaders.forward, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT});

    //
    // GBuffer PSO
    //
    RegisterGBufferPipeline(psoDesc, {Pipe::GBuffer, meshInputLayout, shaders.gbuffer});

    // Light Pass PSO, ToneMap PSO, GBuffer Debug PSO
    RegisterFullscreenPipelines(
        psoDesc,
        {{Pipe::Lighting, shaders.lighting, DXGI_FORMAT_R16G16B16A16_FLOAT},
         {Pipe::LightingDebugGradient, shaders.lightingDebugGradient, DXGI_FORMAT_R16G16B16A16_FLOAT},
         {Pipe::ToneMap, shaders.toneMap, m_backBufferFormat},
         {Pipe::GBufferDebug, shaders.gbufferDebug, DXGI_FORMAT_R16G16B16A16_FLOAT},
         {Pipe::ShadowMaskDebug, shaders.shadowMaskDebug, DXGI_FORMAT_R16G16B16A16_FLOAT}});

    //
    // Depth PrePass PSO
    //
    RegisterDepthPrePassPipeline(psoDesc, {Pipe::DepthPrePass, depthInputLayout, shaders.depthPrePass});
}

void HelloTextureEngine::CreateInitialCommandList()
{
    ThrowIfFailed(
        m_graphicsDevice.Device()->CreateCommandList(0,
                                                     D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                     m_frameResources[m_currentFrameIndex].commandAllocator.Get(),
                                                     GetPipelineState(PipelineId(Pipe::Forward)),
                                                     IID_PPV_ARGS(&m_commandList)));
}

void HelloTextureEngine::CreateSceneGeometryBuffers()
{
    assert(m_scene.mesh != nullptr);
    const Engine::SceneMesh& mesh = *m_scene.mesh;
    assert(!mesh.vertices.empty());

    m_indexCountPerInstance = static_cast<UINT>(mesh.indices.size());
    m_vertexCountPerInstance = static_cast<UINT>(mesh.vertices.size());
    m_usesIndexedDraw = m_indexCountPerInstance > 0;
    m_sceneHasMaterials = !mesh.materials.empty();
    const UINT vertexBufferSize = static_cast<UINT>(sizeof(Engine::SceneVertex) * mesh.vertices.size());

    // Note: using upload heaps to transfer static data like vert buffers is not
    // recommended. Every time the GPU needs it, the upload heap will be marshalled
    // over. Please read up on Default Heap usage. An upload heap is used here for
    // code simplicity and because there are very few verts to actually transfer.
    MyDx12Util::CreateUploadBuffer(m_graphicsDevice.Device(), vertexBufferSize, m_vertexBuffer);

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, mesh.vertices.data(), vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Engine::SceneVertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    if (m_usesIndexedDraw)
    {
        const UINT indexBufferSize = static_cast<UINT>(mesh.indices.size() * sizeof(uint32_t));

        MyDx12Util::CreateUploadBuffer(m_graphicsDevice.Device(), indexBufferSize, m_indexBuffer);

        UINT8* pIndexDataBegin = nullptr;
        ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, mesh.indices.data(), indexBufferSize);
        m_indexBuffer->Unmap(0, nullptr);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }
}

void HelloTextureEngine::CreateSceneTextureResources(std::vector<ComPtr<ID3D12Resource>>& textureUploadHeap)
{
    assert(m_scene.mesh != nullptr);
    const Engine::SceneMesh& mesh = *m_scene.mesh;

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.

    m_sceneTextureCount = static_cast<UINT>(mesh.textures.size());
    const UINT semanticFallbackBaseIndex = m_sceneTextureCount;
    const UINT textureResourceCount = m_sceneTextureCount + Engine::kTextureSemanticCount;
    assert(textureResourceCount <= kTextureDescriptorCapacity);

    // 3つの領域を定義
    // [0, m_sceneTextureCount)              : Scene textures (from glTF)
    // [m_sceneTextureCount, textureResourceCount) : Semantic fallback (5 types)
    // [textureResourceCount, kTextureDescriptorCapacity) : Unused -> BaseColor fallback

    textureUploadHeap.resize(textureResourceCount);

    m_texture.resize(textureResourceCount);
    m_textureSrvs.resize(kTextureDescriptorCapacity);
    m_texIndex.resize(textureResourceCount);

    std::vector<std::vector<UINT8>> texture(textureResourceCount);

    DBG_PRINT("m_sceneTextureCount = %d\n", m_sceneTextureCount);
    DBG_PRINT("textureResourceCount = %d\n", textureResourceCount);

    // ------------------------------------------------------------
    // Region 1: Scene textures [0, m_sceneTextureCount)
    // ------------------------------------------------------------
    for (UINT i = 0; i < m_sceneTextureCount; ++i)
    {
        const auto& tex = mesh.textures[i];
        UINT8* pixels = (UINT8*)tex.pixels.data();
        UINT width = tex.width;
        UINT height = tex.height;

        DBG_PRINT("[%d] sceneTexture :width %d height %d\n", i, width, height);

        DescriptorAllocation srv = CreateTextureFromRGBA8(pixels, width, height,
                                                          m_texture[i], textureUploadHeap[i]);
        m_textureSrvs[i] = std::move(srv);
        if (i == 0)
        {
            m_textureTableStart = m_textureSrvs[i].Handle();
        }
        m_texIndex[i] = m_textureSrvs[i].Handle().Index - m_textureTableStart.Index;
        DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
    }

    // ------------------------------------------------------------
    // Region 2: Semantic fallback textures [m_sceneTextureCount, textureResourceCount)
    // ------------------------------------------------------------
    for (UINT i = 0; i < Engine::kTextureSemanticCount; ++i)
    {
        const UINT idx = semanticFallbackBaseIndex + i;
        const auto semantic = static_cast<Engine::TextureSemantic>(i);
        texture[idx] = GenerateSemanticFallbackTextureData(semantic);

        UINT8* pixels = texture[idx].data();
        UINT width = kTextureWidth;
        UINT height = kTextureHeight;

        DBG_PRINT("[%d] fallback texture (%s) :width %d height %d\n",
                  idx, GetSemanticName(semantic), width, height);

        DescriptorAllocation srv = CreateTextureFromRGBA8(pixels, width, height,
                                                          m_texture[idx], textureUploadHeap[idx]);
        m_textureSrvs[idx] = std::move(srv);
        m_texIndex[idx] = m_textureSrvs[idx].Handle().Index - m_textureTableStart.Index;
        DBG_PRINT("Texture %d SRV index: %d\n", idx, m_texIndex[idx]);

        m_semanticFallbackTexIndex[i] = m_texIndex[idx];
    }

    // ------------------------------------------------------------
    // Region 3: Unused slots [textureResourceCount, kTextureDescriptorCapacity)
    // Fill with BaseColor fallback (white)
    // ------------------------------------------------------------
    const UINT baseColorFallbackIndex = semanticFallbackBaseIndex + static_cast<UINT>(Engine::TextureSemantic::BaseColor);
    assert(baseColorFallbackIndex < textureResourceCount);

    for (UINT i = textureResourceCount; i < kTextureDescriptorCapacity; ++i)
    {
        m_textureSrvs[i] = AllocateTextureSRV(m_texture[baseColorFallbackIndex].Get());
    }
}

void HelloTextureEngine::PrepareSceneInstanceData()
{
    // Instance data is provided by the application via SetScene().
    // Pre-allocate the buffer for kMaxInstanceCount entries.
    if (m_scene.instances.empty())
    {
        m_scene.instances.resize(kMaxInstanceCount);
    }
}

void HelloTextureEngine::CreateSceneMaterialResources()
{
    assert(m_scene.mesh != nullptr);
    const Engine::SceneMesh& mesh = *m_scene.mesh;

    // Generate the material data.
    m_materialData.clear();
    const auto resolveTextureIndex = [this](int sceneTextureIndex, Engine::TextureSemantic fallbackSemantic) -> UINT
    {
        if (sceneTextureIndex >= 0 && sceneTextureIndex < static_cast<int>(m_sceneTextureCount))
        {
            return m_texIndex[sceneTextureIndex];
        }
        return m_semanticFallbackTexIndex[static_cast<UINT>(fallbackSemantic)];
    };

    for (int i = 0; i < Engine::kMaterialCount; i++)
    {
        Engine::Material m = {};
        m.albedoTexIndex = resolveTextureIndex(-1, Engine::TextureSemantic::BaseColor);
        m.metallicRoughnessTexIndex = resolveTextureIndex(-1, Engine::TextureSemantic::MetallicRoughness);
        m.emissiveTexIndex = resolveTextureIndex(-1, Engine::TextureSemantic::Emissive);
        m.occlusionTexIndex = resolveTextureIndex(-1, Engine::TextureSemantic::Occlusion);
        m.normalTexIndex = resolveTextureIndex(-1, Engine::TextureSemantic::Normal);
        m.roughnessFactor = 0.2f + 0.6f * static_cast<float>(i % 16) / 15.0f;
        m.metallicFactor = (i % 8 == 0) ? 1.0f : 0.0f;
        m.occlusionStrength = 1.0f;
        m.ambientOcclusionFactor = 1.0f;
        m.emissiveScale = 1.0f;
        m.flags = 0;

        if (m_sceneHasMaterials)
        {
            if (i < static_cast<int>(mesh.materials.size()))
            {
                const auto& gltfMaterial = mesh.materials[i];
                m.albedoTexIndex =
                    resolveTextureIndex(gltfMaterial.albedoTexIndex, Engine::TextureSemantic::BaseColor);
                m.metallicRoughnessTexIndex =
                    resolveTextureIndex(gltfMaterial.metallicRoughnessTexIndex,
                                        Engine::TextureSemantic::MetallicRoughness);
                m.emissiveTexIndex =
                    resolveTextureIndex(gltfMaterial.emissiveTexIndex, Engine::TextureSemantic::Emissive);
                m.occlusionTexIndex =
                    resolveTextureIndex(gltfMaterial.occlusionTexIndex, Engine::TextureSemantic::Occlusion);
                m.normalTexIndex = resolveTextureIndex(gltfMaterial.normalTexIndex, Engine::TextureSemantic::Normal);
                if (gltfMaterial.normalTexIndex >= 0)
                {
                    m.flags |= Engine::kMaterialFlagHasNormalTexture;
                }
                m.roughnessFactor = gltfMaterial.roughnessFactor;
                m.metallicFactor = gltfMaterial.metallicFactor;
                m.occlusionStrength = gltfMaterial.occlusionStrength;
                m.ambientOcclusionFactor = gltfMaterial.ambientOcclusionFactor;
                m.emissiveScale = gltfMaterial.emissiveTexIndex >= 0 ? gltfMaterial.emissiveScale : 0.0f;
            }
        }

        m_materialData.push_back(m);
    }

    m_materialBuffer.Create(m_graphicsDevice.Device(), m_descriptorHeapAllocator, m_materialData);
}

void HelloTextureEngine::CreateInstanceBuffers()
{
    // Create the instance buffer.
    for (int n = 0; n < kFrameCount; n++)
    {
        const UINT instanceBufferSize = sizeof(InstanceData) * kMaxInstanceCount;

        MyDx12Util::CreateUploadBuffer(
            m_graphicsDevice.Device(), instanceBufferSize, m_frameResources[n].instanceBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaxInstanceCount;
        srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);

        m_frameResources[n].instanceBufferSrv = m_descriptorHeapAllocator.Allocate();
        m_graphicsDevice.Device()->CreateShaderResourceView(
            m_frameResources[n].instanceBuffer.Get(), &srvDesc, m_frameResources[n].instanceBufferSrv.Cpu());

        m_frameResources[n].instanceBuffer->Map(
            0, nullptr, reinterpret_cast<void**>(&m_frameResources[n].pSrvDataBegin));
        memset(m_frameResources[n].pSrvDataBegin, 0, instanceBufferSize);
        const UINT sceneInstanceCount = (std::min)(static_cast<UINT>(m_scene.instances.size()), kMaxInstanceCount);
        if (sceneInstanceCount > 0)
        {
            memcpy(m_frameResources[n].pSrvDataBegin,
                   m_scene.instances.data(),
                   sizeof(InstanceData) * sceneInstanceCount);
        }
        m_frameResources[n].instanceBuffer->Unmap(0, nullptr);

        // Create per-frame TLAS instance upload buffer.
        {
            const UINT tlasInstanceBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * kMaxInstanceCount;
            MyDx12Util::CreateUploadBuffer(
                m_graphicsDevice.Device(), tlasInstanceBufferSize, m_frameResources[n].tlasInstanceBuffer);
        }
    }

    m_sceneResourcesAvailable = true;
}

void HelloTextureEngine::BuildAccelerationStructures()
{
    const UINT instanceCount = (std::min)(
        static_cast<UINT>(m_scene.instances.size()),
        kMaxInstanceCount);

    m_accelerationStructures.Build(
        m_graphicsDevice.Device(),
        m_commandList.Get(),
        m_vertexBuffer.Get(),
        m_indexBuffer.Get(),
        m_vertexCountPerInstance,
        m_indexCountPerInstance,
        m_usesIndexedDraw,
        m_scene.instances.data(),
        instanceCount,
        m_frameResources[m_currentFrameIndex].tlasInstanceBuffer.Get(),
        m_descriptorHeapAllocator);
}

void HelloTextureEngine::RebuildAccelerationStructures()
{
    if (!m_sceneResourcesAvailable || !m_rayTracingSupport.IsSupported())
    {
        return;
    }

    const UINT instanceCount = (std::min)(
        GetVisibleCubeCount(),
        static_cast<UINT>(m_scene.instances.size()));

    m_accelerationStructures.RebuildTlas(
        m_graphicsDevice.Device(),
        m_commandList.Get(),
        m_scene.instances.data(),
        instanceCount,
        m_frameResources[m_currentFrameIndex].tlasInstanceBuffer.Get());
}

void HelloTextureEngine::ReleaseSceneResources()
{
    m_displayInstanceCount = 0;
    m_sceneResourcesAvailable = false;

    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_vertexBufferView = {};
    m_indexBufferView = {};

    m_vertexCountPerInstance = 0;
    m_indexCountPerInstance = 0;
    m_usesIndexedDraw = false;
    m_sceneHasMaterials = false;
    m_sceneTextureCount = 0;

    m_materialBuffer.Reset();
    for (FrameResource& frameResource : m_frameResources)
    {
        frameResource.instanceBufferSrv.Reset();
    }

    for (auto it = m_textureSrvs.rbegin(); it != m_textureSrvs.rend(); ++it)
    {
        it->Reset();
    }

    m_texture.clear();
    m_textureSrvs.clear();
    m_materialData.clear();
    m_textureTableStart = {};
    m_texIndex.clear();
    m_semanticFallbackTexIndex.fill(0);

    for (FrameResource& frameResource : m_frameResources)
    {
        frameResource.instanceBuffer.Reset();
        frameResource.pSrvDataBegin = nullptr;
        frameResource.tlasInstanceBuffer.Reset();
    }
}

void HelloTextureEngine::CreateFrameConstantBuffers()
{
    UpdateCameraConstantBuffer();
    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;

    // Create the per-frame constant buffers.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        CreateConstantBuffer(m_frameResources[n].cameraCB, &m_constantBufferData, sizeof(m_constantBufferData));
        const LightingConstants initialLightingConstants = MakeLightingConstants();
        CreateConstantBuffer(m_frameResources[n].lightCB, &initialLightingConstants, sizeof(initialLightingConstants));
    }
}

void HelloTextureEngine::ExecuteInitialGpuSetup()
{
    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    WaitForGpu();
}

void HelloTextureEngine::CreateConstantBuffer(ConstantBufferResource& constantBuffer,
                                              const void* initialData,
                                              UINT sizeInBytes)
{
    assert(sizeInBytes % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0);

    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
                                                                     D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                     nullptr,
                                                                     IID_PPV_ARGS(&constantBuffer.buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constantBuffer.buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeInBytes;

    constantBuffer.cbv = m_descriptorHeapAllocator.AllocWithHandle();
    m_graphicsDevice.Device()->CreateConstantBufferView(&cbvDesc, constantBuffer.cbv.cpu);

    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(constantBuffer.buffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBuffer.mappedData)));
    memcpy(constantBuffer.mappedData, initialData, sizeInBytes);
}

DescriptorAllocation HelloTextureEngine::AllocateTextureSRV(ID3D12Resource* texture)
{
    DescriptorAllocation handle = m_descriptorHeapAllocator.Allocate();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(texture, &srvDesc, handle.Cpu());

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

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 0xFF);
    UINT8 R = static_cast<UINT8>(dist(gen));
    UINT8 G = static_cast<UINT8>(dist(gen));
    UINT8 B = static_cast<UINT8>(dist(gen));

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

std::vector<UINT8> HelloTextureEngine::GenerateSolidTextureData(UINT8 r, UINT8 g, UINT8 b, UINT8 a)
{
    const UINT textureSize = kTextureWidth * kTextureHeight * kTexturePixelSize;
    std::vector<UINT8> data(textureSize);

    for (UINT n = 0; n < textureSize; n += kTexturePixelSize)
    {
        data[n + 0] = r;
        data[n + 1] = g;
        data[n + 2] = b;
        data[n + 3] = a;
    }

    return data;
}

std::vector<UINT8> HelloTextureEngine::GenerateSemanticFallbackTextureData(Engine::TextureSemantic semantic)
{
    switch (semantic)
    {
    case Engine::TextureSemantic::BaseColor:
        return GenerateSolidTextureData(255, 255, 255, 255);
    case Engine::TextureSemantic::MetallicRoughness:
        return GenerateSolidTextureData(255, 255, 0, 255);
    case Engine::TextureSemantic::Normal:
        return GenerateSolidTextureData(128, 128, 255, 255);
    case Engine::TextureSemantic::Occlusion:
        return GenerateSolidTextureData(255, 255, 255, 255);
    case Engine::TextureSemantic::Emissive:
        return GenerateSolidTextureData(0, 0, 0, 255);
    default:
        return GenerateCheckerboardTextureData();
    }
}

void HelloTextureEngine::CreateDsvHeap()
{
    if (m_dsvHeap)
        return;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void HelloTextureEngine::CreateGBuffer()
{
    m_gbuffer.CreateResources(m_graphicsDevice.Device(), m_width, m_height);
    m_gbuffer.CreateRTVs(m_graphicsDevice.Device(), m_rtvHeap.Get(), kGBufferRTVBaseIndex, m_rtvDescriptorSize);
    m_gbuffer.CreateSRVs(m_graphicsDevice.Device(), m_descriptorHeapAllocator);

    if (m_depthStencilSrv.Index == UINT_MAX)
    {
        m_depthStencilSrv = m_descriptorHeapAllocator.AllocWithHandle();
    }
    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[Engine::GBuffer::Albedo].Index + Engine::GBuffer::kCount);

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
    m_graphicsDevice.Device()->CreateDepthStencilView(
        m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[Engine::GBuffer::Albedo].Index + Engine::GBuffer::kCount);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(m_depthStencil.Get(), &srvDesc, m_depthStencilSrv.cpu);
}

void HelloTextureEngine::CreateShadowMask(UINT width, UINT height)
{
    m_shadowMask.Reset();

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &desc,
                                                                     D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                                     nullptr,
                                                                     IID_PPV_ARGS(&m_shadowMask)));
    m_shadowMask->SetName(L"ShadowMask");

    CreateShadowMaskDescriptors();
}

void HelloTextureEngine::CreateShadowMaskDescriptors()
{
    if (!m_shadowMaskRange.IsValid())
    {
        m_shadowMaskRange = m_stageAllocator.AllocContiguous(2);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(m_shadowMask.Get(), &srvDesc,
                                                        m_stageAllocator.CpuHandle(m_shadowMaskRange.Start));

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R8_UNORM;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    m_graphicsDevice.Device()->CreateUnorderedAccessView(m_shadowMask.Get(), nullptr, &uavDesc,
                                                         m_stageAllocator.CpuHandle(m_shadowMaskRange.Start + 1));
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetBackBufferRtv() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrameIndex, m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetDepthDsv() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetGBufferRTV(UINT index) const
{
    return m_gbuffer.GetRTV(m_rtvHeap.Get(), m_rtvDescriptorSize, index);
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
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Albedo); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferNormal),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Normal); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferMaterial),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Material); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferMotionVector),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::MotionVector); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferPBRParams),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::PBRParams); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferEmissive),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Emissive); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::LightPass),
                                                [this]() { return GetLightPassRTV(); });

    m_renderGraphRuntime.Bindings().RegisterDsv(m_renderGraphRuntime.RegisterDsv(DsvName::Depth),
                                                [this]() { return GetDepthDsv(); });

    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::TextureTable),
                                                       [this]() { return m_textureTableStart.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::InstanceBufferSrv),
        [this]() { return m_frameResources[m_currentFrameIndex].instanceBufferSrv.Gpu(); });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::MaterialBufferSrv),
                                                       [this]() { return m_materialBuffer.Srv().gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::EnvironmentMapSrv),
                                                       [this]() { return m_environmentMap.Srv().gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::CameraCbv),
        [this]() { return m_frameResources[m_currentFrameIndex].cameraCB.cbv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::LightCbv),
        [this]() { return m_frameResources[m_currentFrameIndex].lightCB.cbv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::GBufferAlbedoSrv),
                                                       [this]()
                                                       { return m_gbuffer.srvHandles[Engine::GBuffer::Albedo].gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::ToneMapSceneColorSrv),
        [this]() { return m_lightPassColorSrv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::ShadowMaskSrv),
                                                       [this]()
                                                       { return m_stageAllocator.GpuHandle(m_shadowMaskRange.Start); });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::ShadowMaskUav),
                                                       [this]()
                                                       {
                                                           return m_stageAllocator.GpuHandle(m_shadowMaskRange.Start + 1);
                                                       });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::TlasDebugUav),
                                                       [this]()
                                                       {
                                                           return m_stageAllocator.GpuHandle(m_shadowMaskRange.Start + 1);
                                                       });
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
    m_renderGraphRuntime.Resources().RegisterResource(kShadowMaskResourceName, [this]() { return m_shadowMask.Get(); });
    for (UINT i = 0; i < Engine::GBuffer::kCount; ++i)
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

    ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

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

    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
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

    if (m_sceneResourcesAvailable)
    {
        m_frameResources[m_currentFrameIndex].instanceBuffer->Map(
            0, nullptr, reinterpret_cast<void**>(&m_frameResources[m_currentFrameIndex].pSrvDataBegin));
        memset(m_frameResources[m_currentFrameIndex].pSrvDataBegin, 0, sizeof(InstanceData) * kMaxInstanceCount);
        const UINT sceneInstanceCount = (std::min)(static_cast<UINT>(m_scene.instances.size()), kMaxInstanceCount);
        if (sceneInstanceCount > 0)
        {
            memcpy(m_frameResources[m_currentFrameIndex].pSrvDataBegin,
                   m_scene.instances.data(),
                   sizeof(InstanceData) * sceneInstanceCount);
        }
        m_frameResources[m_currentFrameIndex].instanceBuffer->Unmap(0, nullptr);
    }

    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
    UpdateCameraConstantBuffer();
    memcpy(
        m_frameResources[m_currentFrameIndex].cameraCB.mappedData, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

void HelloTextureEngine::UpdatePerFrameRenderSettings()
{
    m_toneMapPass.settings.Normalize();
    const LightingConstants lightingConstants = MakeLightingConstants();
    memcpy(m_frameResources[m_currentFrameIndex].lightCB.mappedData, &lightingConstants, sizeof(lightingConstants));
}

UINT HelloTextureEngine::GetVisibleCubeCount() const
{
    if (!m_sceneResourcesAvailable)
    {
        return 0;
    }

    return static_cast<UINT>(m_displayInstanceCount);
}

// Render the scene.
void HelloTextureEngine::RenderFrame(const UiRenderHandler& uiRenderHandler)
{
    PIXBeginEvent(0, L"RenderFrame");

    // Select the per-frame chunk within the main heap's reserved range and
    // stage descriptors from the CPU heap into that chunk.
    // SetFrameIndex must match the current frame so that GpuHandle() (called
    // during command recording) points to the same chunk that Stage() writes.
    m_stageAllocator.SetFrameIndex(m_currentFrameIndex);
    m_stageAllocator.Stage();

    UpdatePerFrameRenderSettings();
    m_activeUiRenderHandler = &uiRenderHandler;

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();
    m_activeUiRenderHandler = nullptr;

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
    CollectDeferredGpuReleases();

    m_gpuWorkMeter.ReadbackData(m_graphicsDevice.CommandQueue());

    PIXEndEvent();
}

void HelloTextureEngine::RequestResize(UINT width, UINT height)
{
    m_pendingResize = true;
    m_pendingResizeWidth = width;
    m_pendingResizeHeight = height;
}

void HelloTextureEngine::RunFrame(const UiRenderHandler& uiRenderHandler)
{
    if (m_pendingResize)
    {
        ApplyResize(m_pendingResizeWidth, m_pendingResizeHeight);
        m_pendingResize = false;
    }

    UpdateHdr10DisplayMode();

    m_workMeter.Start();
    UpdateFrame();
    RenderFrame(uiRenderHandler);
    m_workMeter.End();
    m_cpuFrameTime = m_workMeter.GetCpuFrameTimeMs();
}

void HelloTextureEngine::ApplyResize(UINT width, UINT height)
{
    DBG_PRINT("HelloTextureEngine::ApplyResize() %d %d\n", width, height);
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

    // GraphicsDevice owns swap chain resizing; the engine rebuilds render resources that depend on its buffers.
    m_graphicsDevice.ResizeSwapChain(kFrameCount, m_width, m_height, m_backBufferFormat, 0);
    m_hdrOutputPolicy.ReapplyColorSpace(m_graphicsDevice.SwapChain());

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
            m_graphicsDevice.Device()->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    m_depthStencil.Reset();
    m_lightPassRenderTarget.Reset();
    m_shadowMask.Reset();
    m_resourceRegistry.UnregisterTransientResource(kDepthStencilResourceName);
    m_resourceRegistry.UnregisterTransientResource(kLightPassRenderTargetResourceName);
    RegisterDepthStencil(m_width, m_height);
    RegisterLightPassRenderTarget(m_width, m_height);
    CreateGBuffer();
    CreateShadowMask(m_width, m_height);

    // Camera
    UpdateCameraConstantBuffer();

    // Screen
    m_viewport = CD3DX12_VIEWPORT(
        0.0f, 0.0f, static_cast<FLOAT>(m_width), static_cast<FLOAT>(m_height), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
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
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Engine::CreateFullscreenPipelineDesc(baseDesc, definition);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, desc));
}

void HelloTextureEngine::RegisterFullscreenPipelines(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                     std::initializer_list<FullscreenPipelineDefinition> definitions)
{
    for (const FullscreenPipelineDefinition& definition : definitions)
    {
        RegisterFullscreenPipeline(baseDesc, definition);
    }
}

void HelloTextureEngine::RegisterForwardPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                 const ForwardPipelineDefinition& definition)
{
    baseDesc = Engine::CreateForwardPipelineDesc(baseDesc, m_rootSignature.Get(), definition);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, baseDesc));
}

void HelloTextureEngine::RegisterGBufferPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                 const GBufferPipelineDefinition& definition)
{
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc =
        Engine::CreateGBufferPipelineDesc(baseDesc, definition, m_gbuffer.formats, Engine::GBuffer::kCount);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, desc));
}

void HelloTextureEngine::RegisterDepthPrePassPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                      const DepthPrePassPipelineDefinition& definition)
{
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Engine::CreateDepthPrePassPipelineDesc(baseDesc, definition);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, desc));
}

void HelloTextureEngine::PopulateCommandList()
{
    PIXBeginEvent(1, L"PopulateCommandList");

    BeginFrame();
    ResetResourceStates();
    RebuildAccelerationStructures();
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
    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
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
    m_graphicsDevice.Device()->CreateRenderTargetView(m_lightPassRenderTarget.Get(), nullptr, GetLightPassRTV());

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(
        m_lightPassRenderTarget.Get(), &srvDesc, m_lightPassColorSrv.cpu);
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

        if (name == kLightPassRenderTargetResourceName)
        {
            m_lightPassRenderTarget.Reset();
        }
    }
}

void HelloTextureEngine::ResetResourceStates()
{
    m_resourceRegistry.states.clear();
    for (const auto& usage : m_resourceDefaultStates)
    {
        m_resourceRegistry.SetState(usage.name, usage.state);
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
                                       GetPipelineState(PipelineId(Pipe::Forward))));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    // The single shader-visible heap includes both regular descriptors and
    // the reserved staged region, so only one heap needs to be bound.
    ID3D12DescriptorHeap* ppHeaps[] = {m_heap.Get()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    m_gpuWorkMeter.StartGpu(m_commandList.Get(), m_frameResources[m_currentFrameIndex].gpuWorkMeterCheckPoints);
}

void HelloTextureEngine::ExecuteClearPass(const RenderPass& pass)
{
    Engine::RecordClearPass(m_commandList.Get(), ResolveRenderTargets(pass.renderTargets));
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Clear");
}

void HelloTextureEngine::ExecuteDepthPrePass(const RenderPass& pass)
{
    Engine::RecordDepthPrePass(m_commandList.Get(), MakeSceneGeometryDrawDesc());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Depth Prepass");
}

void HelloTextureEngine::ExecuteGBufferPass(const RenderPass& pass)
{
    assert(pass.renderTargets.rtvs.size() == Engine::GBuffer::kCount);

    Engine::GBufferPassDesc passDesc = {};
    passDesc.renderTargets = ResolveRenderTargets(pass.renderTargets);
    passDesc.clearValues = m_gbuffer.clearValues;
    passDesc.geometryDraw = MakeSceneGeometryDrawDesc();

    Engine::RecordGBufferPass(m_commandList.Get(), passDesc);
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Pass");
}

void HelloTextureEngine::ExecuteRayQueryShadowPass(const RenderPass& pass)
{
    UNREFERENCED_PARAMETER(pass);

    Engine::RayQueryShadowPassDesc passDesc = {};
    passDesc.rootSignature = m_rayQueryShadowRootSignature.Get();
    passDesc.pipelineState = m_rayQueryShadowPipeline.Get();
    passDesc.shadowMaskUav = m_stageAllocator.GpuHandle(m_shadowMaskRange.Start + 1);
    passDesc.tlasSrv = m_accelerationStructures.tlasSrv.Gpu();
    passDesc.depthSrv = m_depthStencilSrv.gpu;
    passDesc.normalSrv = m_gbuffer.srvHandles[Engine::GBuffer::Normal].gpu;
    passDesc.cameraCbv = m_frameResources[m_currentFrameIndex].cameraCB.cbv.gpu;
    passDesc.lightDirection = m_lightingParams.lightDirection;
    passDesc.normalBias = m_shadowSettings.normalBias;
    passDesc.rayTMin = m_shadowSettings.rayTMin;
    passDesc.rayTMax = m_shadowSettings.rayTMax;
    passDesc.enabled = m_shadowSettings.enabled ? 1 : 0;
    passDesc.softShadowEnabled = m_shadowSettings.softShadowEnabled ? 1 : 0;
    passDesc.sampleCount = static_cast<uint32_t>(m_shadowSettings.sampleCount);
    passDesc.lightAngularRadius = m_shadowSettings.lightAngularRadius;
    passDesc.jitterStrength = m_shadowSettings.jitterStrength;
    passDesc.width = m_width;
    passDesc.height = m_height;

    Engine::RecordRayQueryShadowPass(m_commandList.Get(), passDesc);
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "RayQuery Shadow Pass");
}

void HelloTextureEngine::ExecuteRayQueryTlasDebugPass(const RenderPass& pass)
{
    UNREFERENCED_PARAMETER(pass);

    Engine::RayQueryTlasDebugPassDesc passDesc = {};
    passDesc.rootSignature = m_rayQueryTlasDebugRootSignature.Get();
    passDesc.pipelineState = m_rayQueryTlasDebugPipeline.Get();
    passDesc.tlasDebugUav = m_stageAllocator.GpuHandle(m_shadowMaskRange.Start + 1);
    passDesc.tlasSrv = m_accelerationStructures.tlasSrv.Gpu();
    passDesc.depthSrv = m_depthStencilSrv.gpu;
    passDesc.normalSrv = m_gbuffer.srvHandles[Engine::GBuffer::Normal].gpu;
    passDesc.cameraCbv = m_frameResources[m_currentFrameIndex].cameraCB.cbv.gpu;
    passDesc.lightDirection = m_lightingParams.lightDirection;
    passDesc.width = m_width;
    passDesc.height = m_height;

    Engine::RecordRayQueryTlasDebugPass(m_commandList.Get(), passDesc);
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "RayQuery TlasDebug Pass");
}

void HelloTextureEngine::ExecuteForwardPass(const RenderPass& pass)
{
    Engine::ForwardPassDesc passDesc = {};
    passDesc.renderTargets = ResolveRenderTargets(pass.renderTargets);
    passDesc.geometryDraw = MakeSceneGeometryDrawDesc();

    Engine::RecordForwardPass(m_commandList.Get(), passDesc);
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Main Pass");
}

void HelloTextureEngine::ExecuteLightingPass(const RenderPass& pass)
{
    Engine::RecordLightingPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Lighting Pass");
}

void HelloTextureEngine::ExecuteLightingDebugGradientPass(const RenderPass& pass)
{
    Engine::RecordLightingDebugGradientPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "LightPassDebugGradient Pass");
}

void HelloTextureEngine::ExecuteToneMapPass(const RenderPass& pass)
{
    Engine::RecordToneMapPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ToneMap Pass");
}

void HelloTextureEngine::ExecuteDebugDumpPass(const RenderPass& pass)
{
    if (!m_lightingPassDebugGradientEnabled)
    {
        return;
    }

    RecordDebugDumpPass();
}

void HelloTextureEngine::ExecuteGBufferDebugPass(const RenderPass& pass)
{
    Engine::RecordGBufferDebugPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Debug Pass");
}

void HelloTextureEngine::ExecuteShadowMaskDebugPass(const RenderPass& pass)
{
    UNREFERENCED_PARAMETER(pass);

    Engine::RecordShadowMaskDebugPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ShadowMask Debug Pass");
}

void HelloTextureEngine::ExecuteImGuiPass(const RenderPass& pass)
{
    RecordImGuiPass();
}

void HelloTextureEngine::RecordDebugDumpPass()
{
    Engine::RecordDebugDumpCapture(m_commandList.Get(),
                                   m_graphicsDevice.Device(),
                                   m_lightPassRenderTarget.Get(),
                                   m_renderTargets[m_currentFrameIndex].Get(),
                                   m_debugDumpCapture);

    m_debugViewSettings.hdrDumpPending = true;

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Debug Dump");
}

void HelloTextureEngine::PrintDebugDump()
{
    if (!m_lightingPassDebugGradientEnabled)
    {
        return;
    }

    if (!m_debugDumpCapture.IsReady())
    {
        return;
    }

    Engine::DebugDumpMappedCapture mappedCapture = {};
    Engine::MapDebugDumpCapture(m_debugDumpCapture, mappedCapture);

    const Engine::DebugDumpReportDesc reportDesc = {
        mappedCapture,
        m_hdrOutputPolicy.settings,
        m_toneMapPass.settings,
    };
    Engine::PrintDebugDumpReport(reportDesc);

    Engine::UnmapDebugDumpCapture(m_debugDumpCapture);
}

auto HelloTextureEngine::MakeSceneGeometryDrawDesc() const -> Engine::SceneGeometryDrawDesc
{
    return {m_vertexBufferView,
            m_indexBufferView,
            m_usesIndexedDraw,
            m_vertexCountPerInstance,
            m_indexCountPerInstance,
            GetVisibleCubeCount()};
}

auto HelloTextureEngine::ResolveRenderTargets(const PassRenderTargetBinding& renderTargets) const
    -> Engine::ResolvedRenderTargets
{
    Engine::ResolvedRenderTargets resolvedRenderTargets = {};
    resolvedRenderTargets.rtvs.reserve(renderTargets.rtvs.size());

    for (RtvKey rtv : renderTargets.rtvs)
    {
        resolvedRenderTargets.rtvs.push_back(m_renderGraphRuntime.Bindings().ResolveRtv(rtv));
    }

    if (renderTargets.dsv)
    {
        resolvedRenderTargets.dsv = m_renderGraphRuntime.Bindings().ResolveDsv(renderTargets.dsv.value());
    }

    resolvedRenderTargets.clearColor = renderTargets.clearColor ? renderTargets.clearColor->data() : nullptr;
    return resolvedRenderTargets;
}

void HelloTextureEngine::RecordImGuiPass()
{
    if (m_activeUiRenderHandler != nullptr && *m_activeUiRenderHandler)
    {
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        (*m_activeUiRenderHandler)(m_commandList.Get());
    }
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ImGUI");
}

void HelloTextureEngine::EndFrame()
{
    m_gpuWorkMeter.EndGpu(m_commandList.Get());

    for (const auto& usage : m_resourceDefaultStates)
    {
        TransitionResource(usage);
    }

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
