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
#include "GltfLoader.h"
#include "MyDx12Utils.h"
#include "Renderer/GBuffer.h"
#include "Renderer/HdrOutput.h"
#include "Renderer/RenderPassExecution.h"
#include "Renderer/RenderPassGraph.h"
#include "Renderer/RenderPassResources.h"
#include "Renderer/SimpleDescriptorHeapAllocator.h"
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

struct EngineInitDesc
{
    UINT initialWidth = 0;
    UINT initialHeight = 0;
};

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
        float ambientIntensity = 0.10f;
        float diffuseIntensity = 1.0f;
    };

    struct CameraState
    {
        XMFLOAT3 pos = {0.0f, 0.0f, -5.0f};
        XMFLOAT3 rot = {0.0f, 0.0f, 0.0f};
        float fov = 60.0f;
    };

    static constexpr UINT kMaxInstanceCount = 1000;

    struct alignas(16) InstanceData
    {
        XMFLOAT4X4 world;
        XMFLOAT4X4 prevWorld;
        UINT materialId;
    };

    struct ToneMapParams
    {
        int operatorIndex = 0; // 0: None, 1: Reinhard, 2: ACES
        float exposure = 1.0f;
        float paperWhiteNits = 300.0f;
        float maxDisplayNits = 1000.0f;
    };

    struct DebugUiContext
    {
        int frameIndex;
        float cpuFrameTime;
        const std::vector<MyDx12Util::GpuWorkMeter::CheckPoint>& gpuCheckPoints;
    };

    using DebugUiHandler = std::function<void(const DebugUiContext&)>;
    using UpdateHandler = std::function<void()>;

    static constexpr UINT kSwapChainBufferCount = 2;
    static constexpr DXGI_FORMAT kSwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

    HelloTextureEngine(UINT width, UINT height, GraphicsDevice& graphicsDevice);

    void HandleMouseDown(UINT8 button, int x, int y);
    void HandleMouseUp(UINT8 button, int x, int y);
    void HandleMouseMove(int x, int y);
    void RequestResize(UINT width, UINT height);
    void Initialize(const EngineInitDesc& desc);
    void RenderFrame();
    void RunFrame();
    void Shutdown();
    void SetSceneMesh(const GltfMeshData* mesh);
    void SetDebugUiHandler(DebugUiHandler handler);
    void SetUpdateHandler(UpdateHandler handler);
    void SetLightingParams(const LightingParams& params);
    void SetRenderingPath(RenderingPath renderingPath);
    void SetLightingPassDebugGradient(bool enabled);
    void SetBackBufferClearColor(const std::array<float, 4>& color);
    void SetCameraState(const CameraState& camera);
    void SetInstanceData(const std::vector<InstanceData>& instanceData);
    void SetDisplayInstanceCount(int count);
    void SetToneMapParams(const ToneMapParams& params);
    void SetRenderViewMode(RenderViewMode mode);
    void SetRequestHdrDump(bool request);

private:
    static constexpr UINT kFrameCount = kSwapChainBufferCount;
    static constexpr UINT kTextureWidth = 256;
    static constexpr UINT kTextureHeight = 256;
    static constexpr UINT kTexturePixelSize = 4; // The number of bytes used to represent a pixel in the texture.

    struct PassKeyNames
    {
        // Pipeline keys identify GPU pipeline state selection. These names are registered with PSO creation and
        // later referenced by passes that need that bound state.
        struct Pipeline
        {
            static constexpr const char* Main = "Main";
            static constexpr const char* DepthPrePass = "DepthPrePass";
            static constexpr const char* GBuffer = "GBuffer";
            static constexpr const char* Lighting = "Lighting";
            static constexpr const char* LightingDebugGradient = "LightingDebugGradient";
            static constexpr const char* ToneMap = "ToneMap";
            static constexpr const char* GBufferDebug = "GBufferDebug";
        };

        struct Descriptor
        {
            static constexpr const char* TextureTable = "TextureTable";
            static constexpr const char* InstanceBufferSrv = "InstanceBufferSrv";
            static constexpr const char* MaterialBufferSrv = "MaterialBufferSrv";
            static constexpr const char* CameraCbv = "CameraCbv";
            static constexpr const char* LightCbv = "LightCbv";
            static constexpr const char* GBufferAlbedoSrv = "GBufferAlbedoSrv";
            static constexpr const char* ToneMapSceneColorSrv = "ToneMapSceneColorSrv";
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
            static constexpr const char* Main = "Main";
            static constexpr const char* Lighting = "Lighting";
            static constexpr const char* LightingDebugGradient = "LightingDebugGradient";
            static constexpr const char* ToneMap = "ToneMap";
            static constexpr const char* DebugDump = "DebugDump";
            static constexpr const char* GBufferDebug = "GBufferDebug";
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

    static constexpr UINT kHeapDescriptorCount = 100;
    // 0 - 99 : ImGui (new)

    static constexpr UINT kTextureCount = 1020;
    static constexpr UINT kTextureTypes = 1020; // Color Type : 0-9

    static constexpr DXGI_FORMAT kBackBufferFormat = kSwapChainFormat;
    static constexpr UINT kDefaultToneMapOperator = 0; // 0: None, 1: Reinhard, 2: ACES
    static constexpr float kDefaultExposure = 1.0f;
    static constexpr float kDefaultPaperWhiteNits = 300.0f;
    static constexpr float kDefaultMaxDisplayNits = 1000.0f;

    static constexpr UINT kInstanceBufferCount = kFrameCount;
    static constexpr UINT kMaterialBufferCount = 1;
    static constexpr UINT kConstantBufferCount = kFrameCount;
    static constexpr UINT kLightConstantBufferCount = kFrameCount;

    // Descriptor allocation order is tracked by DescriptorHeapHandle.
    // Current persistent descriptors: GBuffer SRVs, depth SRV, LightPass SRV, texture table, instance buffers,
    // material buffer, constant buffer, light constant buffer.
    static constexpr UINT kMainHeapDescriptorCount = kTextureCount + kInstanceBufferCount + kMaterialBufferCount +
                                                     kConstantBufferCount + kLightConstantBufferCount + GBuffer::kCount +
                                                     2;

    static constexpr UINT kMaterialCount = 256;

    static constexpr int kGpuWorkMeterQueryCount = 100;

    struct Material
    {
        UINT albedoTexIndex;
        UINT metallicRoughnessTexIndex;
        UINT emissiveTexIndex;
        UINT occlusionTexIndex;
        UINT normalTexIndex;
        float roughnessFactor;
        float metallicFactor;
        float occlusionStrength;
        UINT flags;
    };

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
        float ambientIntensity = 0.10f;
        XMFLOAT3 lightColor = {1.0f, 1.0f, 1.0f};
        float diffuseIntensity = 1.0f;
        XMFLOAT4 backgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
    };

    LightingConstants MakeLightingConstants() const;

    struct ToneMapSettings
    {
        struct ShaderConstants
        {
            UINT toneMapOperator;
            UINT transferFunction;
            float exposure;
            float paperWhiteNits;
            float maxDisplayNits;
        };

        int operatorIndex = kDefaultToneMapOperator;
        float exposure = kDefaultExposure;
        float paperWhiteNits = kDefaultPaperWhiteNits;
        float maxDisplayNits = kDefaultMaxDisplayNits;

        void Normalize()
        {
            operatorIndex = std::clamp(operatorIndex, 0, 2);
            exposure = (std::max)(exposure, 0.0f);
            paperWhiteNits = (std::max)(paperWhiteNits, 1.0f);
            maxDisplayNits = (std::max)(maxDisplayNits, paperWhiteNits);
        }

        ShaderConstants MakeShaderConstants(UINT transferFunction) const
        {
            return {static_cast<UINT>(operatorIndex), transferFunction, exposure, paperWhiteNits, maxDisplayNits};
        }
    };

    struct ToneMapPass
    {
        ToneMapSettings settings;

        ToneMapSettings::ShaderConstants MakeShaderConstants(const HdrOutputSettings& hdrOutputSettings) const;
    };

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
        DescriptorHeapHandle instanceBufferSrv;
        InstanceData* pSrvDataBegin = nullptr;
        ConstantBufferResource cameraCB;
        ConstantBufferResource lightCB;
        UINT64 fenceValue = 0;
        std::vector<MyDx12Util::GpuWorkMeter::CheckPoint> gpuWorkMeterCheckPoints;
    };

    static constexpr UINT kSwapChainRTVCount = kFrameCount;
    static constexpr UINT kGBufferRTVBaseIndex = kSwapChainRTVCount;
    static constexpr UINT kLightPassRTVIndex = kGBufferRTVBaseIndex + GBuffer::kCount;
    static constexpr UINT kRTVDescriptorCount = kFrameCount + GBuffer::kCount + 1;

    struct DebugViewSettings
    {
        RenderViewMode renderViewMode = RenderViewMode::LightPass;
        bool requestHdrDump = false;
        bool hdrDumpPending = false;

        bool IsGBufferDebugView() const
        {
            return renderViewMode != RenderViewMode::LightPass;
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
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    GBuffer m_gbuffer;
    ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12Resource> m_lightPassRenderTarget;
    DescriptorHeapHandle m_depthStencilSrv;
    DescriptorHeapHandle m_lightPassColorSrv;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12RootSignature> m_lightingRootSignature; // not used in this sample but created for future use

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    ComPtr<ID3D12DescriptorHeap> m_heap;                     // CBV/SRV/UAV heap
    SimpleDescriptorHeapAllocator m_descriptorHeapAllocator; // Allocator for CBV/SRV/UAV heap

    ComPtr<ID3D12DescriptorHeap> m_imguiHeap;
    SimpleDescriptorHeapAllocator m_ImGuiDescriptorHeapAllocator;

    RenderingPath m_renderingPath = RenderingPath::Deferred;
    bool m_lightingPassDebugGradientEnabled = false;
    ToneMapPass m_toneMapPass;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_descriptorSize;
    DXGI_FORMAT m_backBufferFormat = kBackBufferFormat;
    HdrOutputPolicy m_hdrOutputPolicy;
    DebugViewSettings m_debugViewSettings;
    ComPtr<ID3D12Resource> m_lightPassDebugDumpReadback;
    ComPtr<ID3D12Resource> m_backBufferDebugDumpReadback;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_lightPassDebugDumpLayout = {};
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_backBufferDebugDumpLayout = {};
    std::array<float, 4> m_backBufferClearColor = {0.0f, 0.2f, 0.4f, 1.0f};

    DescriptorHeapHandle m_textureTableStart;
    UINT m_texIndex[kTextureCount] = {};

    std::vector<InstanceData> m_instanceData;
    std::vector<Material> m_materialData;
    LightingParams m_lightingParams;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    UINT m_sceneTextureCount = 0;

    std::vector<ComPtr<ID3D12Resource>> m_texture;

    UINT m_vertexCountPerInstance = 0;
    UINT m_indexCountPerInstance = 0;
    bool m_usesIndexedDraw = false;
    bool m_sceneHasMaterials = false;

    int m_displayInstanceCount = static_cast<int>(kMaxInstanceCount);

    ComPtr<ID3D12Resource> m_materialBuffer;
    DescriptorHeapHandle m_materialBufferSrv;

    CameraState m_camera;
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
    DebugUiHandler m_debugUiHandler;
    UpdateHandler m_updateHandler;
    const GltfMeshData* m_sceneMesh = nullptr;

    static constexpr const char* kBackBufferResourceName = "BackBuffer";
    static constexpr const char* kDepthStencilResourceName = "DepthStencil";
    static constexpr const char* kLightPassRenderTargetResourceName = "LightPass.RenderTarget";
    static constexpr const char* kGBufferResourceNames[GBuffer::kCount] = {
        "GBuffer.Albedo", "GBuffer.Normal", "GBuffer.Material", "GBuffer.MotionVector", "GBuffer.PBRParams"};

    enum RootParameterIndex
    {
        RootParam_TextureTable = 0,
        RootParam_InstanceSrv,
        RootParam_MaterialSrv,
        RootParam_ConstantBuffer,
        RootParam_GBufferSrvBase,
        RootParam_LightConstants,
        RootParam_GBufferDebugConstants,
        RootParam_ToneMapSceneColor,
        RootParam_ToneMapConstants
    };

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
    using PassOperationHandler = void (HelloTextureEngine::*)(const RenderPass& pass);
    using RenderGraphRuntime = Engine::RenderGraphRuntime<PassOperationHandler>;
    RenderGraphRuntime m_renderGraphRuntime;

    struct ShaderBytecode
    {
        const UINT8* data = nullptr;
        UINT size = 0;
    };

    struct GraphicsPipelineShaders
    {
        ShaderBytecode vertex;
        ShaderBytecode pixel;
    };

    struct InputLayoutDefinition
    {
        const D3D12_INPUT_ELEMENT_DESC* elements;
        UINT count;
    };

    struct MainPipelineDefinition
    {
        const char* name;
        InputLayoutDefinition inputLayout;
        GraphicsPipelineShaders shaders;
        DXGI_FORMAT renderTargetFormat;
        DXGI_FORMAT depthStencilFormat;
    };

    struct GBufferPipelineDefinition
    {
        const char* name;
        InputLayoutDefinition inputLayout;
        GraphicsPipelineShaders shaders;
    };

    struct DepthPrePassPipelineDefinition
    {
        const char* name;
        InputLayoutDefinition inputLayout;
        GraphicsPipelineShaders shaders;
    };

    struct FullscreenPipelineDefinition
    {
        const char* name;
        GraphicsPipelineShaders shaders;
        DXGI_FORMAT renderTargetFormat;
    };

    void LoadPipeline();
    void LoadAssets();
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    void InitializeFrameResources();
    void UpdateFrame();
    void DestroyFrameResources();
    void RegisterFullscreenPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                    const FullscreenPipelineDefinition& definition);
    void RegisterFullscreenPipelines(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                     std::initializer_list<FullscreenPipelineDefinition> definitions);
    void RegisterMainPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc, const MainPipelineDefinition& definition);
    void RegisterGBufferPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                 const GBufferPipelineDefinition& definition);
    void RegisterDepthPrePassPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                      const DepthPrePassPipelineDefinition& definition);
    void UpdateHdr10DisplayMode();
    void UpdateCameraConstantBuffer();
    void InitImGui();
    void CreateConstantBuffer(ConstantBufferResource& constantBuffer, const void* initialData, UINT sizeInBytes);
    void CreateDepthStencil(UINT width, UINT height);
    void RegisterDepthStencil(UINT width, UINT height);
    void RegisterLightPassRenderTarget(UINT width, UINT height);
    void CreateDepthStencilDescriptors();
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
    RenderPass MakeMainPass();
    RenderPass MakeLightingPass();
    RenderPass MakeLightingDebugGradientPass();
    RenderPass MakeToneMapPass();
    RenderPass MakeDebugDumpPass();
    RenderPass MakeGBufferDebugPass();
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

    DescriptorHeapHandle CreateTextureFromRGBA8(const UINT8* pixels,
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

    void UpdateImGui();
    UINT GetVisibleCubeCount() const;

    void BeginFrame();
    void ExecuteClearPass(const RenderPass& pass);
    void ExecuteDepthPrePass(const RenderPass& pass);
    void ExecuteGBufferPass(const RenderPass& pass);
    void ExecuteMainPass(const RenderPass& pass);
    void ExecuteLightingPass(const RenderPass& pass);
    void ExecuteLightingDebugGradientPass(const RenderPass& pass);
    void ExecuteToneMapPass(const RenderPass& pass);
    void ExecuteDebugDumpPass(const RenderPass& pass);
    void ExecuteGBufferDebugPass(const RenderPass& pass);
    void ExecuteImGuiPass(const RenderPass& pass);
    void RecordClear(const PassRenderTargetBinding& renderTargets);
    void RecordDepthPrePass();
    void RecordGBufferPass(const PassRenderTargetBinding& renderTargets);
    void RecordGBufferDebugPass();
    void RecordLightPass();
    void RecordLightPassDebugGradient();
    void RecordToneMapPass();
    void RecordDebugDumpPass();
    void RecordMainPass(const PassRenderTargetBinding& renderTargets);
    void RecordImGuiPass();
    void EndFrame();
    void CreateDebugDumpReadback(ID3D12Resource* source,
                                 ComPtr<ID3D12Resource>& readback,
                                 D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout);
    void PrintDebugDump();

    void DrawInstanceWrapper(UINT instanceCount);
    void DrawFullscreenTriangle();

    void ApplyResize(UINT width, UINT height);

    void WaitForGpu();
    UINT64 MoveToNextFrame();
    void FlushGpu();

    DescriptorHeapHandle AllocateTextureSRV(ID3D12Resource* texture);
};
