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

#pragma once
#include "DXSampleHelper.h"
#include "GraphicsDevice.h"
#include "MyDx12Utils.h"
#include "Renderer/ClearPass.h"
#include "Renderer/BrdfLut.h"
#include "Renderer/DebugDumpCapture.h"
#include "Renderer/DeferredGpuReleaseQueue.h"
#include "Renderer/EnvironmentMap.h"
#include "Renderer/GBuffer.h"
#include "Renderer/HdrOutput.h"
#include "Renderer/LightingPass.h"
#include "Renderer/Material.h"
#include "Renderer/MaterialBuffer.h"
#include "Renderer/StagedDescriptorAllocator.h"
#include "Renderer/PipelineFactory.h"
#include "Renderer/AccelerationStructureResources.h"
#include "Renderer/RayQueryShadowPass.h"
#include "Renderer/RayQueryTlasDebugPass.h"
#include "Renderer/RayTracingSupport.h"
#include "Renderer/RenderPassExecution.h"
#include "Renderer/RenderPassGraph.h"
#include "Renderer/RenderPassResources.h"
#include "Renderer/RootSignatureLayout.h"
#include "Renderer/SceneGeometryPass.h"
#include "Renderer/SimpleDescriptorHeapAllocator.h"
#include "Renderer/ShadowMaskDebugPass.h"
#include "Renderer/ToneMap.h"
#include "Scene/Scene.h"
#include "WorkMeter.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <unordered_map>

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: Shutdown().
using Microsoft::WRL::ComPtr;

using PipelineKey = Engine::PipelineKey;
using DescriptorKey = Engine::DescriptorKey;
using RtvKey = Engine::RtvKey;
using DsvKey = Engine::DsvKey;
using PassOperationKey = Engine::PassOperationKey;
using PassConstantsKey = Engine::PassConstantsKey;
using ShaderBytecode = Engine::ShaderBytecode;
using GraphicsPipelineShaderSet = Engine::GraphicsPipelineShaderSet;
using InputLayoutDefinition = Engine::InputLayoutDefinition;
using ForwardPipelineDefinition = Engine::ForwardPipelineDefinition;
using GBufferPipelineDefinition = Engine::GBufferPipelineDefinition;
using DepthPrePassPipelineDefinition = Engine::DepthPrePassPipelineDefinition;
using FullscreenPipelineDefinition = Engine::FullscreenPipelineDefinition;
namespace RootSignatureLayout = Engine::RootSignatureLayout;

class HelloTextureEngine
{
public:
    enum class RenderViewMode
    {
        LightPass = 0,
        GBufferAlbedo,
        GBufferNormal,
        GBufferMaterial,
        GBufferMotionVector,
        GBufferPBRParams,
        Depth,
        ReflectionDirection,
        ViewDirection,
        WorldPosition,
        NdotV,
        IblEnvironment,
        IblDiffuseIrradiance,
        IblSpecularPrefilter,
        IblBrdfLut,
        ShadowMask,
        TlasDebug,
    };

    enum class RenderingPath
    {
        Forward = 0,
        Deferred,
    };

    struct LightingParams
    {
        XMFLOAT3 lightDirection = {0.4f, 0.7f, 0.6f};
        XMFLOAT3 lightColor = {1.0f, 1.0f, 1.0f};
        // HDR environment maps are bright, so the default IBL contribution is intentionally modest.
        float iblIntensity = 0.10f;
        float diffuseIntensity = 1.0f;
        bool skyboxEnabled = true;
        bool skyboxPreview = false;
        float skyboxPreviewExposure = 1.0f;
        bool directLightEnabled = true;
        bool diffuseIblEnabled = true;
        bool specularIblEnabled = true;
        bool emissiveEnabled = true;
        float iblDebugMip = 0.0f;
        float iblDebugExposure = 0.25f;
    };

    struct MaterialParams
    {
        float roughnessFactor = 1.0f;
        float metallicFactor = 1.0f;
        float ambientOcclusionFactor = 1.0f;
        float emissiveScale = 1.0f;
    };

    using Scene = Engine::Scene;
    using CameraState = Engine::CameraState;

    static constexpr UINT kMaxInstanceCount = 1000;
    static constexpr UINT kSpecularPrefilterMipCount = 6;

    using InstanceData = Engine::InstanceData;

    struct ToneMapParams
    {
        int operatorIndex = 0; // 0: None, 1: Reinhard, 2: ACES
        float exposure = 1.0f;
        float paperWhiteNits = 300.0f;
        float maxDisplayNits = 1000.0f;
    };

    struct ShadowSettings
    {
        bool enabled = true;
        float normalBias = 0.01f;
        float rayTMin = 0.001f;
        float rayTMax = 10000.0f;
        bool softShadowEnabled = false;
        int sampleCount = 1;
        float lightAngularRadius = 0.005f;
        float jitterStrength = 1.0f;
    };

    struct UiFrameContext
    {
        int frameIndex;
        float cpuFrameTime;
        bool rayTracingSupported;
        const wchar_t* rayTracingTierName;
        int rayTracingTierRaw;
        const std::vector<MyDx12Util::GpuWorkMeter::CheckPoint>& gpuCheckPoints;
    };

    using UiRenderHandler = std::function<void(ID3D12GraphicsCommandList*)>;
    using UpdateHandler = std::function<void()>;

    static constexpr UINT kSwapChainBufferCount = 2;
    static constexpr DXGI_FORMAT kSwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

    HelloTextureEngine(GraphicsDevice& graphicsDevice);

    void RequestResize(UINT width, UINT height);
    void Initialize(UINT width, UINT height);
    void RenderFrame(const UiRenderHandler& uiRenderHandler);
    void RunFrame(const UiRenderHandler& uiRenderHandler);
    void Shutdown();
    void SetScene(const Scene& scene);
    void ReloadSceneResources(const Scene& scene);
    void CloseSceneResources();
    UiFrameContext GetUiFrameContext() const;
    void SetUpdateHandler(UpdateHandler handler);
    void SetLightingParams(const LightingParams& params);
    void SetShadowSettings(const ShadowSettings& settings);
    const ShadowSettings& GetShadowSettings() const { return m_shadowSettings; }
    void SetMaterialParams(UINT materialIndex, const MaterialParams& params);
    void SetRenderingPath(RenderingPath renderingPath);
    void SetLightingPassDebugGradient(bool enabled);
    void SetBackBufferClearColor(const std::array<float, 4>& color);
    void SetDisplayInstanceCount(int count);
    void SetToneMapParams(const ToneMapParams& params);
    void SetRenderViewMode(RenderViewMode mode);
    void SetRequestHdrDump(bool request);
    void ReloadEnvironmentResources(const Engine::ProceduralEnvironmentSettings& settings);

private:
    static constexpr UINT kFrameCount = kSwapChainBufferCount;
    static constexpr UINT kTextureWidth = 256;
    static constexpr UINT kTextureHeight = 256;
    static constexpr UINT kTexturePixelSize = 4; // The number of bytes used to represent a pixel in the texture.
    static constexpr UINT kEnvironmentMapCubeSize = 128;
    static constexpr UINT kDiffuseIrradianceCubeSize = 32;
    static constexpr UINT kSpecularPrefilterCubeSize = 128;
    static constexpr UINT kBrdfLutSize = 256;

    struct PassKeyNames
    {
        // Pipeline keys identify GPU pipeline state selection. These names are registered with PSO creation and
        // later referenced by passes that need that bound state.
        struct Pipeline
        {
            static constexpr const char* Forward = "Forward";
            static constexpr const char* DepthPrePass = "DepthPrePass";
            static constexpr const char* GBuffer = "GBuffer";
            static constexpr const char* Lighting = "Lighting";
            static constexpr const char* LightingDebugGradient = "LightingDebugGradient";
            static constexpr const char* ToneMap = "ToneMap";
            static constexpr const char* GBufferDebug = "GBufferDebug";
            static constexpr const char* RayQueryShadow = "RayQueryShadow";
            static constexpr const char* RayQueryTlasDebug = "RayQueryTlasDebug";
            static constexpr const char* ShadowMaskDebug = "ShadowMaskDebug";
        };

        struct Descriptor
        {
            static constexpr const char* TextureTable = "TextureTable";
            static constexpr const char* InstanceBufferSrv = "InstanceBufferSrv";
            static constexpr const char* MaterialBufferSrv = "MaterialBufferSrv";
            static constexpr const char* EnvironmentMapSrv = "EnvironmentMapSrv";
            static constexpr const char* CameraCbv = "CameraCbv";
            static constexpr const char* LightCbv = "LightCbv";
            static constexpr const char* GBufferAlbedoSrv = "GBufferAlbedoSrv";
            static constexpr const char* ToneMapSceneColorSrv = "ToneMapSceneColorSrv";
            static constexpr const char* ShadowMaskSrv = "ShadowMaskSrv";
            static constexpr const char* ShadowMaskUav = "ShadowMaskUav";
            static constexpr const char* TlasDebugUav = "TlasDebugUav";
            static constexpr const char* AccelerationStructureSrv = "AccelerationStructureSrv";
        };

        struct Rtv
        {
            static constexpr const char* BackBuffer = "BackBuffer";
            static constexpr const char* GBufferAlbedo = "GBufferAlbedo";
            static constexpr const char* GBufferNormal = "GBufferNormal";
            static constexpr const char* GBufferMaterial = "GBufferMaterial";
            static constexpr const char* GBufferMotionVector = "GBufferMotionVector";
            static constexpr const char* GBufferPBRParams = "GBufferPBRParams";
            static constexpr const char* LightPass = "LightPass";
        };

        struct Dsv
        {
            static constexpr const char* Depth = "Depth";
        };

        // Operation keys identify command recording behavior. Keep them conceptually separate from Pipeline:
        // a pass may share an operation with another pass while using a different pipeline, or use no pipeline at all.
        struct Operation
        {
            static constexpr const char* Clear = "Clear";
            static constexpr const char* DepthPrePass = "DepthPrePass";
            static constexpr const char* GBuffer = "GBuffer";
            static constexpr const char* Forward = "Forward";
            static constexpr const char* Lighting = "Lighting";
            static constexpr const char* LightingDebugGradient = "LightingDebugGradient";
            static constexpr const char* ToneMap = "ToneMap";
            static constexpr const char* DebugDump = "DebugDump";
            static constexpr const char* GBufferDebug = "GBufferDebug";
            static constexpr const char* ShadowMaskDebug = "ShadowMaskDebug";
            static constexpr const char* RayQueryShadow = "RayQueryShadow";
            static constexpr const char* RayQueryTlasDebug = "RayQueryTlasDebug";
            static constexpr const char* ImGui = "ImGui";
        };

        struct Constants
        {
            static constexpr const char* ToneMap = "ToneMap";
            static constexpr const char* GBufferDebugTarget = "GBufferDebugTarget";
        };
    };

    using Pipe = PassKeyNames::Pipeline;
    using Desc = PassKeyNames::Descriptor;
    using RtvName = PassKeyNames::Rtv;
    using DsvName = PassKeyNames::Dsv;
    using Op = PassKeyNames::Operation;
    using ConstName = PassKeyNames::Constants;

    static constexpr UINT kTextureCount = 1020;
    static constexpr UINT kTextureTypes = 1020; // Color Type : 0-9
    static constexpr DXGI_FORMAT kBackBufferFormat = kSwapChainFormat;

    static constexpr UINT kInstanceBufferCount = kFrameCount;
    static constexpr UINT kMaterialBufferCount = 1;
    // Procedural environment reloads keep the previous descriptor table alive until its fence retires.
    // Each table is env / diffuse irradiance / specular prefilter / BRDF LUT, plus m_brdfLut owns one SRV.
    static constexpr UINT kEnvironmentDescriptorTableSize = 4;
    static constexpr UINT kEnvironmentDescriptorTableCapacity = 4;
    static constexpr UINT kEnvironmentMapDescriptorCount =
        kEnvironmentDescriptorTableSize * kEnvironmentDescriptorTableCapacity + 1;
    static constexpr UINT kConstantBufferCount = kFrameCount;
    static constexpr UINT kLightConstantBufferCount = kFrameCount;

    // Fixed SRV slots after the counted arrays (GBuffer SRVs, etc.).
    // Must be allocated in enum order, immediately after the GBuffer SRVs.
    enum PersistentSrvSlot : UINT
    {
        DepthStencilSrvSlot,
        LightPassColorSrvSlot,

        PersistentSrvSlotCount,
    };
    static constexpr UINT kShadowMaskDescriptorCount = 2; // SRV + UAV (dynamically allocated)
    static constexpr UINT kTlasDescriptorCount = 1;       // TLAS SRV

    // Descriptor allocation order is tracked by DescriptorHeapHandle.
    // Current persistent descriptors: GBuffer SRVs, depth SRV, LightPass SRV, TLAS SRV, environment map SRVs,
    // texture table, instance buffers, material buffer, constant buffer, light constant buffer.
    // ShadowMask descriptors live in a StagedDescriptorAllocator whose GPU
    // copies are staged into a reserved range of the main shader-visible heap.
    static constexpr UINT kMainHeapDescriptorCount = kTextureCount + kInstanceBufferCount + kMaterialBufferCount +
                                                      kEnvironmentMapDescriptorCount + kConstantBufferCount +
                                                      kLightConstantBufferCount + Engine::GBuffer::kCount +
                                                      PersistentSrvSlotCount + kTlasDescriptorCount;
    static constexpr UINT kStagedDescriptorReservedCount = 64;

    static constexpr int kGpuWorkMeterQueryCount = 100;

    struct alignas(256) ConstantBuffer
    {
        XMFLOAT4X4 viewProjection;
        XMFLOAT4X4 prevViewProjection;
        XMFLOAT4X4 invViewProjection;
        XMFLOAT3 cameraPosition = {0.0f, 0.0f, 0.0f};
        float padding = 0.0f;
    };

    struct alignas(256) LightingConstants
    {
        XMFLOAT3 lightDirection = {0.4f, 0.7f, 0.6f};
        float iblIntensity = 0.10f;
        XMFLOAT3 lightColor = {1.0f, 1.0f, 1.0f};
        float diffuseIntensity = 1.0f;
        XMFLOAT4 backgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
        float skyboxEnabled = 1.0f;
        float skyboxPreview = 0.0f;
        float skyboxPreviewExposure = 1.0f;
        float lightPassDebugViewMode = 0.0f;
        float directLightEnabled = 1.0f;
        float diffuseIblEnabled = 1.0f;
        float specularIblEnabled = 1.0f;
        float emissiveEnabled = 1.0f;
        float iblDebugMip = 0.0f;
        float iblDebugExposure = 0.25f;
        float rayTracingSupported = 0.0f;
        float shadowMaskBlurEnabled = 0.0f;
    };

    LightingConstants MakeLightingConstants() const;

    struct ConstantBufferResource
    {
        ComPtr<ID3D12Resource> buffer;
        DescriptorHeapHandle cbv;
        UINT8* mappedData = nullptr;
    };

    struct FrameResource
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        ComPtr<ID3D12Resource> instanceBuffer;
        DescriptorAllocation instanceBufferSrv;
        InstanceData* pSrvDataBegin = nullptr;
        ComPtr<ID3D12Resource> tlasInstanceBuffer;
        ConstantBufferResource cameraCB;
        ConstantBufferResource lightCB;
        UINT64 fenceValue = 0;
        std::vector<MyDx12Util::GpuWorkMeter::CheckPoint> gpuWorkMeterCheckPoints;
    };

    static constexpr UINT kSwapChainRTVCount = kFrameCount;
    static constexpr UINT kGBufferRTVBaseIndex = kSwapChainRTVCount;
    static constexpr UINT kLightPassRTVIndex = kGBufferRTVBaseIndex + Engine::GBuffer::kCount;
    static constexpr UINT kRTVDescriptorCount = kFrameCount + Engine::GBuffer::kCount + 1;

    struct DebugViewSettings
    {
        RenderViewMode renderViewMode = RenderViewMode::LightPass;
        bool requestHdrDump = false;
        bool hdrDumpPending = false;

        bool IsGBufferDebugView() const
        {
            return renderViewMode != RenderViewMode::LightPass &&
                   renderViewMode != RenderViewMode::ShadowMask &&
                   renderViewMode != RenderViewMode::TlasDebug &&
                   !IsLightPassDebugView();
        }
        bool IsLightPassDebugView() const
        {
            return renderViewMode == RenderViewMode::ReflectionDirection ||
                   renderViewMode == RenderViewMode::ViewDirection ||
                   renderViewMode == RenderViewMode::WorldPosition ||
                   renderViewMode == RenderViewMode::NdotV ||
                   renderViewMode == RenderViewMode::IblEnvironment ||
                   renderViewMode == RenderViewMode::IblDiffuseIrradiance ||
                   renderViewMode == RenderViewMode::IblSpecularPrefilter ||
                   renderViewMode == RenderViewMode::IblBrdfLut;
        }
        UINT GetGBufferDebugTarget() const
        {
            assert(IsGBufferDebugView());
            return static_cast<UINT>(renderViewMode) - static_cast<UINT>(RenderViewMode::GBufferAlbedo);
        }
    };

    // Pipeline objects.
    GraphicsDevice& m_graphicsDevice;
    UINT m_width = 0;
    UINT m_height = 0;
    float m_aspectRatio = 0.0f;
    std::wstring m_assetsPath;
    std::wstring m_shaderPath;
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    Engine::GBuffer m_gbuffer;
    ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12Resource> m_lightPassRenderTarget;
    ComPtr<ID3D12Resource> m_shadowMask;
    DescriptorHeapHandle m_depthStencilSrv;
    DescriptorHeapHandle m_lightPassColorSrv;
    StagedDescriptorRange m_shadowMaskRange;

    StagedDescriptorAllocator m_stageAllocator;

    Engine::AccelerationStructureResources m_accelerationStructures;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12RootSignature> m_proceduralEnvRootSignature;
    ComPtr<ID3D12RootSignature> m_rayQueryShadowRootSignature;
    ComPtr<ID3D12RootSignature> m_rayQueryTlasDebugRootSignature;
    ComPtr<ID3D12RootSignature> m_lightingRootSignature; // not used in this sample but created for future use

    ComPtr<ID3D12PipelineState> m_proceduralEnvPipeline;
    ComPtr<ID3D12PipelineState> m_rayQueryShadowPipeline;
    ComPtr<ID3D12PipelineState> m_rayQueryTlasDebugPipeline;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    ComPtr<ID3D12DescriptorHeap> m_heap;                     // CBV/SRV/UAV heap
    SimpleDescriptorHeapAllocator m_descriptorHeapAllocator; // Allocator for CBV/SRV/UAV heap
    ComPtr<ID3D12DescriptorHeap> m_proceduralEnvUavHeap;
    ComPtr<ID3D12Resource> m_proceduralEnvSettingsBuffer;

    RenderingPath m_renderingPath = RenderingPath::Deferred;
    bool m_lightingPassDebugGradientEnabled = false;
    Engine::RayTracingSupportInfo m_rayTracingSupport;
    Engine::ToneMapPass m_toneMapPass;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    DXGI_FORMAT m_backBufferFormat = kBackBufferFormat;
    HdrOutputPolicy m_hdrOutputPolicy;
    DebugViewSettings m_debugViewSettings;
    Engine::DebugDumpCapture m_debugDumpCapture;
    std::array<float, 4> m_backBufferClearColor = {0.0f, 0.2f, 0.4f, 1.0f};

    DescriptorHeapHandle m_textureTableStart;
    UINT m_texIndex[kTextureCount] = {};

    Engine::Scene m_scene;
    std::vector<Engine::Material> m_materialData;
    LightingParams m_lightingParams;
    ShadowSettings m_shadowSettings;
    Engine::ProceduralEnvironmentSettings m_environmentSettings;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    UINT m_sceneTextureCount = 0;

    std::vector<ComPtr<ID3D12Resource>> m_texture;
    std::vector<DescriptorAllocation> m_textureSrvs;

    struct EnvironmentDescriptorTable
    {
        DescriptorHeapHandle environment;
        DescriptorHeapHandle diffuseIrradiance;
        DescriptorHeapHandle specularPrefilter;
        DescriptorHeapHandle brdfLut;

        bool IsValid() const
        {
            return environment.IsValid();
        }
    };

    Engine::EnvironmentMap m_environmentMap;
    Engine::EnvironmentMap m_diffuseIrradianceMap;
    Engine::EnvironmentMap m_specularPrefilterMap;
    Engine::BrdfLut m_brdfLut;
    EnvironmentDescriptorTable m_environmentDescriptorTable;
    Engine::DeferredGpuReleaseQueue m_deferredGpuReleaseQueue;

    UINT m_vertexCountPerInstance = 0;
    UINT m_indexCountPerInstance = 0;
    bool m_usesIndexedDraw = false;
    bool m_sceneHasMaterials = false;
    bool m_sceneResourcesAvailable = false;

    int m_displayInstanceCount = static_cast<int>(kMaxInstanceCount);

    Engine::MaterialBuffer m_materialBuffer;

    static constexpr float kCameraNearZ = 0.1f;
    static constexpr float kCameraFarZ = 10000.0f;
    ConstantBuffer m_constantBufferData;

    std::chrono::steady_clock::time_point m_prevTime;

    // Synchronization objects.
    UINT m_previousFrameIndex;
    UINT m_currentFrameIndex;

    FrameResource m_frameResources[kFrameCount];

    bool m_pendingResize = false;
    UINT m_pendingResizeWidth = 0;
    UINT m_pendingResizeHeight = 0;

    // CPU work meter
    MyDx12Util::WorkMeter m_workMeter;
    float m_cpuFrameTime = 0.f;

    // GPU work meter
    MyDx12Util::GpuWorkMeter m_gpuWorkMeter;
    const UiRenderHandler* m_activeUiRenderHandler = nullptr;
    UpdateHandler m_updateHandler;
    static constexpr const char* kBackBufferResourceName = "BackBuffer";
    static constexpr const char* kDepthStencilResourceName = "DepthStencil";
    static constexpr const char* kLightPassRenderTargetResourceName = "LightPass.RenderTarget";
    static constexpr const char* kGBufferResourceNames[Engine::GBuffer::kCount] = {
        "GBuffer.Albedo", "GBuffer.Normal", "GBuffer.Material", "GBuffer.MotionVector", "GBuffer.PBRParams"};
    static constexpr const char* kShadowMaskResourceName = "ShadowMask";

    using TransientResourceState = Engine::TransientResourceState;

    struct TransientResource
    {
        TransientResourceState state = TransientResourceState::Uninitialized;

        std::string name;
        D3D12_RESOURCE_DESC desc = {};
        D3D12_CLEAR_VALUE clearValue = {};
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

        ComPtr<ID3D12Resource> resource;

        bool persistent = false;
        UINT64 retireFenceValue = 0;
        bool pendingRelease = false; // false : waiting for settign retireFenceValue
        bool retired = false;        // false : waiting for GPU fence
    };

    using ResourceRegistry = Engine::ResourceRegistry<TransientResource>;
    using ResourceUsage = Engine::ResourceUsage;
    using ResourceUsages = Engine::ResourceUsages;
    using PassDescriptorBinding = Engine::PassDescriptorBinding;
    using PassRenderTargetBinding = Engine::PassRenderTargetBinding;
    using PassConstantsBinding = Engine::PassConstantsBinding;
    using RenderPass = Engine::RenderPass;
    using RenderPassGraph = Engine::RenderPassGraph;

    ResourceRegistry m_resourceRegistry;
    std::vector<ResourceUsage> m_resourceDefaultStates;
    using PassOperationHandler = void (HelloTextureEngine::*)(const RenderPass& pass);
    using RenderGraphRuntime = Engine::RenderGraphRuntime<PassOperationHandler>;
    RenderGraphRuntime m_renderGraphRuntime;

    struct PipelineShaderBytecode
    {
        GraphicsPipelineShaderSet forward;
        GraphicsPipelineShaderSet depthPrePass;
        GraphicsPipelineShaderSet gbuffer;
        GraphicsPipelineShaderSet gbufferDebug;
        GraphicsPipelineShaderSet shadowMaskDebug;
        GraphicsPipelineShaderSet lighting;
        GraphicsPipelineShaderSet lightingDebugGradient;
        GraphicsPipelineShaderSet toneMap;
        ShaderBytecode proceduralEnv;
        ShaderBytecode rayQueryShadow;
        ShaderBytecode rayQueryTlasDebug;
    };

    void LoadPipeline();
    void LoadAssets();
    void CreateRootSignature();
    void CreateProceduralEnvRootSignature();
    void CreateRayQueryShadowRootSignature();
    void CreateRayQueryTlasDebugRootSignature();
    void CreatePipelineStates();
    ShaderBytecode LoadShaderBytecode(LPCWSTR assetName);
    PipelineShaderBytecode LoadPipelineShaderBytecode();
    void RegisterPipelineStates(const PipelineShaderBytecode& shaders);
    void CreateInitialCommandList();
    void CreateSceneGeometryBuffers();
    void CreateSceneTextureResources(std::vector<ComPtr<ID3D12Resource>>& textureUploadHeap);
    void CreateEnvironmentMapResources(ComPtr<ID3D12Resource>& environmentMapUploadHeap,
                                       ComPtr<ID3D12Resource>& diffuseIrradianceUploadHeap,
                                       ComPtr<ID3D12Resource>& specularPrefilterUploadHeap);
    void CreateEnvironmentMapResourcesGpu(ComPtr<ID3D12Resource>& diffuseIrradianceUploadHeap,
                                          ComPtr<ID3D12Resource>& specularPrefilterUploadHeap);
    void ReleaseEnvironmentMapResources();
    UINT64 SignalFenceForQueuedGpuWork();
    Engine::DeferredGpuRelease CreateActiveEnvironmentRelease(UINT64 retireFenceValue);
    void QueueActiveEnvironmentResourcesForRelease(UINT64 retireFenceValue);
    Engine::DeferredGpuRelease CreateProceduralEnvGenerationRelease(
        UINT64 retireFenceValue,
        ComPtr<ID3D12CommandAllocator> proceduralEnvCommandAllocator);
    void QueueProceduralEnvGenerationResourcesForRelease(
        UINT64 retireFenceValue,
        ComPtr<ID3D12CommandAllocator> proceduralEnvCommandAllocator);
    void CollectDeferredGpuReleases();
    std::vector<DescriptorHeapHandle> CreateEnvironmentDescriptorHandles(
        const EnvironmentDescriptorTable& table,
        DescriptorHeapHandle environmentSrv,
        DescriptorHeapHandle diffuseIrradianceSrv,
        DescriptorHeapHandle specularPrefilterSrv) const;
    EnvironmentDescriptorTable AllocateEnvironmentDescriptorTable();
    void FreeEnvironmentDescriptorTable(const EnvironmentDescriptorTable& table);
    void CreateEnvironmentDescriptorTableSrvs(ID3D12Resource* environmentMap,
                                              ID3D12Resource* diffuseIrradianceMap,
                                              ID3D12Resource* specularPrefilterMap,
                                              const EnvironmentDescriptorTable& table);
    void CreateEnvironmentBrdfLutSrv(const EnvironmentDescriptorTable& table);
    void ValidateEnvironmentMapDescriptorTable() const;
    void PrepareSceneInstanceData();
    void CreateSceneMaterialResources();
    void CreateInstanceBuffers();
    void BuildAccelerationStructures();
    void RebuildAccelerationStructures();
    void ReleaseSceneResources();
    void CreateFrameConstantBuffers();
    void ExecuteInitialGpuSetup();
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    std::wstring GetShaderFullPath(LPCWSTR shaderName);
    void InitializeFrameResources();
    void InitResourceDefaultStates();
    void UpdateFrame();
    void DestroyFrameResources();
    void RegisterFullscreenPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                    const FullscreenPipelineDefinition& definition);
    void RegisterFullscreenPipelines(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                     std::initializer_list<FullscreenPipelineDefinition> definitions);
    void RegisterForwardPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                 const ForwardPipelineDefinition& definition);
    void RegisterGBufferPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                 const GBufferPipelineDefinition& definition);
    void RegisterDepthPrePassPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                      const DepthPrePassPipelineDefinition& definition);
    void UpdateHdr10DisplayMode();
    void UpdateCameraConstantBuffer();
    void CreateConstantBuffer(ConstantBufferResource& constantBuffer, const void* initialData, UINT sizeInBytes);
    void CreateDepthStencil(UINT width, UINT height);
    void RegisterDepthStencil(UINT width, UINT height);
    void RegisterLightPassRenderTarget(UINT width, UINT height);
    void CreateDepthStencilDescriptors();
    void CreateShadowMask(UINT width, UINT height);
    void CreateShadowMaskDescriptors();
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtv() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthDsv() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRTV(UINT index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetLightPassRTV() const;
    void RegisterPassBindingResolvers();
    void RegisterPassConstantsHandlers();
    void RegisterResourceResolvers();

    std::vector<UINT8> GenerateCheckerboardTextureData();
    void PopulateCommandList();

    void AddPass(RenderPass pass);
    ResourceUsages MakeResourceUsages(std::initializer_list<ResourceUsage> usages) const;
    ResourceUsages MakeGBufferReadUsages() const;
    PipelineKey PipelineId(const std::string& name);
    DescriptorKey DescriptorId(const std::string& name);
    RenderPass MakeClearPass();
    RenderPass MakeDepthPrePass();
    RenderPass MakeGBufferPass();
    RenderPass MakeRayQueryShadowPass();
    RenderPass MakeRayQueryTlasDebugPass();
    RenderPass MakeForwardPass();
    RenderPass MakeLightingPass();
    RenderPass MakeLightingDebugGradientPass();
    RenderPass MakeToneMapPass();
    RenderPass MakeDebugDumpPass();
    RenderPass MakeGBufferDebugPass();
    RenderPass MakeShadowMaskDebugPass();
    RenderPass MakeImGuiPass();
    void BuildRenderPasses();
    void AddSceneRenderPasses();
    void AddDeferredSceneOutputPass();
    void ValidateRenderPassGraph() const;
    void AnalyzeResourceLifetimes();
    void DebugPrintLifetimes();
    void ExecutePasses();
    void ExecutePassOperation(const RenderPass& pass);
    void CreateResourcesForPass(int passIndex);
    void CreateCommittedTransientResource(TransientResource& resource);
    void BindCreatedTransientResource(const std::string& name, ID3D12Resource* resource);
    void CreateLightPassRenderTargetDescriptors();
    void CreateDsvHeap();

    void CreateGBuffer();

    DescriptorAllocation CreateTextureFromRGBA8(const UINT8* pixels,
                                                UINT width,
                                                UINT height,
                                                ComPtr<ID3D12Resource>& texture,
                                                ComPtr<ID3D12Resource>& uploadHeap);

    void ReleaseResourcesAfterPass(int passIndex);
    void ResetResourceStates();

    ID3D12PipelineState* GetPipelineState(PipelineKey pipeline) const;
    Engine::ResourceTransitionContext MakeResourceTransitionContext();
    void TransitionPassResources(const RenderPass& pass);
    void TransitionResource(const ResourceUsage& usage);
    ID3D12Resource* FindTransientD3DResource(const std::string& name) const;

    D3D12_RESOURCE_STATES GetResourceState(const std::string& name) const;
    void SetResourceState(const std::string& name, D3D12_RESOURCE_STATES state);
    void MarkPendingTransientResources(UINT64 fenceValue);
    void CollectGarbageTransientResources();

    void UpdatePerFrameRenderSettings();
    UINT GetVisibleCubeCount() const;

    void BeginFrame();
    void ExecuteClearPass(const RenderPass& pass);
    void ExecuteDepthPrePass(const RenderPass& pass);
    void ExecuteGBufferPass(const RenderPass& pass);
    void ExecuteRayQueryShadowPass(const RenderPass& pass);
    void ExecuteRayQueryTlasDebugPass(const RenderPass& pass);
    void ExecuteForwardPass(const RenderPass& pass);
    void ExecuteLightingPass(const RenderPass& pass);
    void ExecuteLightingDebugGradientPass(const RenderPass& pass);
    void ExecuteToneMapPass(const RenderPass& pass);
    void ExecuteDebugDumpPass(const RenderPass& pass);
    void ExecuteGBufferDebugPass(const RenderPass& pass);
    void ExecuteShadowMaskDebugPass(const RenderPass& pass);
    void ExecuteImGuiPass(const RenderPass& pass);
    void RecordDebugDumpPass();
    void RecordImGuiPass();
    void EndFrame();
    void PrintDebugDump();

    Engine::SceneGeometryDrawDesc MakeSceneGeometryDrawDesc() const;
    Engine::ResolvedRenderTargets ResolveRenderTargets(const PassRenderTargetBinding& renderTargets) const;

    void ApplyResize(UINT width, UINT height);

    void WaitForGpu();
    UINT64 MoveToNextFrame();
    void FlushGpu();

    DescriptorAllocation AllocateTextureSRV(ID3D12Resource* texture);
};
