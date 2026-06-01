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
#include "DXSample.h"
#include "MyDx12Utils.h"
#include "RenderPassGraph.h"
#include "SimpleDescriptorHeapAllocator.h"
#include "WorkMeter.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <initializer_list>
#include <optional>
#include <string>
#include <unordered_map>

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

struct GltfVertex;
// #include "GltfLoader.h"

using PipelineKey = Engine::PipelineKey;
using DescriptorKey = Engine::DescriptorKey;
using RtvKey = Engine::RtvKey;
using DsvKey = Engine::DsvKey;
using PassOperationKey = Engine::PassOperationKey;
using PassConstantsKey = Engine::PassConstantsKey;

class D3D12HelloTexture : public DXSample
{
public:
    D3D12HelloTexture(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void OnMouseDown(UINT8 button, int x, int y);
    virtual void OnMouseUp(UINT8 button, int x, int y);
    virtual void OnMouseMove(int x, int y);
    virtual void OnWindowSizeChanged(UINT width, UINT height);
    virtual void OnIdle();

private:
    static constexpr UINT kFrameCount = 2;
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

    static constexpr UINT kGBufferCount = 5;

    static constexpr UINT kHeapDescriptorCount = 100;
    // 0 - 99 : ImGui (new)

    static constexpr UINT kTextureCount = 1020;
    static constexpr UINT kTextureTypes = 1020; // Color Type : 0-9

    static constexpr DXGI_FORMAT kSwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
    static constexpr DXGI_FORMAT kBackBufferFormat = kSwapChainFormat;
    static constexpr DXGI_COLOR_SPACE_TYPE kHdr10ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
    static constexpr DXGI_COLOR_SPACE_TYPE kSdrColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    static constexpr UINT kDefaultToneMapOperator = 0; // 0: None, 1: Reinhard, 2: ACES
    static constexpr UINT kHdr10TransferFunction = 1;  // 0: Linear, 1: ST.2084 PQ, 2: HLG
    static constexpr UINT kSdrTransferFunction = 3;    // 0: Linear, 1: ST.2084 PQ, 2: HLG, 3: SDR Rec.709
    static constexpr float kDefaultExposure = 1.0f;
    static constexpr float kDefaultPaperWhiteNits = 300.0f;
    static constexpr float kDefaultMaxDisplayNits = 1000.0f;
    static constexpr UINT kHdr10MaxMasteringLuminance = 1000;
    static constexpr UINT kHdr10MinMasteringLuminance = 10; // 0.001 nits in 0.0001 nit units.
    static constexpr UINT16 kHdr10MaxContentLightLevel = 1000;
    static constexpr UINT16 kHdr10MaxFrameAverageLightLevel = 400;

    static constexpr UINT kInstanceBufferCount = kFrameCount;
    static constexpr UINT kMaterialBufferCount = 1;
    static constexpr UINT kConstantBufferCount = kFrameCount;
    static constexpr UINT kLightConstantBufferCount = kFrameCount;

    // Descriptor allocation order is tracked by DescriptorHeapHandle.
    // Current persistent descriptors: GBuffer SRVs, depth SRV, LightPass SRV, texture table, instance buffers,
    // material buffer, constant buffer, light constant buffer.
    static constexpr UINT kMainHeapDescriptorCount = kTextureCount + kInstanceBufferCount + kMaterialBufferCount +
                                                     kConstantBufferCount + kLightConstantBufferCount + kGBufferCount +
                                                     2;

    static constexpr float kTranslationSpeed = 0.005f;
    static constexpr float kPI = 3.141592f;
    static constexpr float kRotationSpeed = kPI / 180.f / 3.f;
    static constexpr float kOffsetBounds = 5.f;
    static constexpr float kCameraMoveSpeed = 0.01f;
    static constexpr float kMouseRotationSpeed = 0.01f;

    static constexpr UINT kMaxInstanceCount = 1000;
    static constexpr float kCubeScale = 0.2f;
    static constexpr UINT kMaterialCount = 256;

    static constexpr UINT kCubeVertexCount = (3 * 2 * 6);

    static constexpr int kGpuWorkMeterQueryCount = 100;

    static constexpr bool kGltfLoadingEnabled = true; // false: glTFモデルを読み込まずCubeを表示
    static constexpr bool kGltfMeshDisplay = true;    // true: glTFモデルを表示、false: Cubeを表示

    struct GridDim
    {
        GridDim(int x, int y, int z) : x(x), y(y), z(z) {}
        int x;
        int y;
        int z;
    };

    inline XMFLOAT3 instanceIdToXYZ(int instanceId, const GridDim& dim)
    {
        int x = instanceId % dim.x - (kMaxInstanceCount / dim.z / dim.y) / 2;
        int y = (instanceId / dim.x) % dim.y - (kMaxInstanceCount / dim.x / dim.y) / 2;
        int z = -instanceId / (dim.x * dim.y) + (kMaxInstanceCount / dim.x / dim.y) / 2;
        return XMFLOAT3((float)x, (float)y, (float)z);
    }
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

    struct alignas(16) InstanceData
    {
        XMFLOAT4X4 world;
        XMFLOAT4X4 prevWorld;
        UINT materialId;
    };

    struct InstanceDataForCPU
    {
        InstanceDataForCPU(XMFLOAT3 pos, XMFLOAT3 rot) : pos(pos), rot(rot) {}
        XMFLOAT3 pos;
        XMFLOAT3 rot;
    };

    struct CameraForCPU
    {
        CameraForCPU(XMFLOAT3 pos, XMFLOAT3 rot, float fov, float aspect, float nearZ, float farZ)
            : pos(pos), rot(rot), fov(fov), aspect(aspect), nearZ(nearZ), farZ(farZ)
        {

            updateAllMatrix();
        }

        void updateViewMatrix()
        {
            XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
            XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
            view = XMMatrixInverse(nullptr, rotMat * transMat);
        }
        void updateProjectionMatrix()
        {
            projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), // FOV
                                                  aspect,                  // aspect ratio
                                                  nearZ,                   // near
                                                  farZ                     // far
            );
        }
        void updateViewProjectionMatrix()
        {
            viewProjection = XMMatrixMultiply(view, projection);
        }
        void updateAllMatrix()
        {
            updateViewMatrix();
            updateProjectionMatrix();
            updateViewProjectionMatrix();
        }

        XMFLOAT3 pos;
        XMFLOAT3 rot;
        XMMATRIX view;

        float fov;
        float aspect;
        float nearZ;
        float farZ;
        XMMATRIX projection;

        XMMATRIX viewProjection;
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

    struct HdrOutputSettings
    {
        DXGI_COLOR_SPACE_TYPE currentSwapChainColorSpace = DXGI_COLOR_SPACE_CUSTOM;
        bool hdr10Enabled = false;

        DXGI_COLOR_SPACE_TYPE TargetColorSpace() const
        {
            return hdr10Enabled ? kHdr10ColorSpace : kSdrColorSpace;
        }
        UINT TransferFunction() const
        {
            return hdr10Enabled ? kHdr10TransferFunction : kSdrTransferFunction;
        }
    };

    struct HdrOutputPolicy
    {
        HdrOutputSettings settings;

        bool CheckSwapChainColorSpaceSupport(IDXGISwapChain3* swapChain, DXGI_COLOR_SPACE_TYPE colorSpace) const;
        bool CheckCurrentOutputHdr10Support(ComPtr<IDXGIFactory4>& dxgiFactory, HWND hwnd) const;
        void ApplySwapChainColorSpace(IDXGISwapChain3* swapChain, DXGI_COLOR_SPACE_TYPE colorSpace);
        void ApplyHdr10Metadata(IDXGISwapChain3* swapChain, bool enabled) const;
        void Update(ComPtr<IDXGIFactory4>& dxgiFactory, IDXGISwapChain3* swapChain, HWND hwnd);
        void ReapplyColorSpace(IDXGISwapChain3* swapChain);
    };

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

    struct GBuffer
    {
        static constexpr UINT kCount = kGBufferCount;

        enum Target : UINT
        {
            Albedo = 0,
            Normal = 1,
            Material = 2,
            MotionVector = 3,
            PBRParams = 4,
        };

        ComPtr<ID3D12Resource> resources[kCount];

        DXGI_FORMAT formats[kCount] = {
            DXGI_FORMAT_R8G8B8A8_UNORM,     // Albedo
            DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
            DXGI_FORMAT_R32_UINT,           // Material
            DXGI_FORMAT_R16G16_FLOAT,       // Motion Vector
            DXGI_FORMAT_R8G8B8A8_UNORM,     // PBR Params (Metallic, Roughness, Occlusion, Emissive)
        };

        D3D12_CLEAR_VALUE clearValues[kCount] = {
            {DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 1.0f}},
            {DXGI_FORMAT_R16G16B16A16_FLOAT, {0.5f, 0.5f, 1.0f, 1.0f}},
            {DXGI_FORMAT_R32_UINT, {0.0f, 0.0f, 0.0f, 0.0f}},
            {DXGI_FORMAT_R16G16_FLOAT, {0.0f, 0.0f, 0.0f, 0.0f}},
            {DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 0.0f}},
        };

        UINT rtvIndex[kCount] = {};
        DescriptorHeapHandle srvHandles[kCount];
    };
    static constexpr UINT kSwapChainRTVCount = kFrameCount;
    static constexpr UINT kGBufferRTVBaseIndex = kSwapChainRTVCount;
    static constexpr UINT kLightPassRTVIndex = kGBufferRTVBaseIndex + GBuffer::kCount;
    static constexpr UINT kRTVDescriptorCount = kFrameCount + GBuffer::kCount + 1;

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
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGIFactory4> m_dxgiFactory;
    GBuffer m_gbuffer;
    ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12Resource> m_lightPassRenderTarget;
    DescriptorHeapHandle m_depthStencilSrv;
    DescriptorHeapHandle m_lightPassColorSrv;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

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

    Engine::RenderPassKeyRegistry m_passKeyRegistry;
    Engine::RenderPassKeys m_passKeys;
    Engine::RenderPassBindingResolverRegistry m_passBindingResolvers;
    Engine::PipelineRegistry m_pipelineRegistry;

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
    std::vector<InstanceDataForCPU> m_instanceDataForCPU;
    std::vector<Material> m_materialData;
    LightingConstants m_lightingConstantsData;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    UINT m_gltfTextureCount = 0;

    std::vector<ComPtr<ID3D12Resource>> m_texture;

    UINT m_vertexCountPerInstance = kCubeVertexCount;
    UINT m_indexCountPerInstance = 0;

    int m_DisplayInstanceCount = static_cast<int>(kMaxInstanceCount);
    float m_meshScale = 0.5f;

    ComPtr<ID3D12Resource> m_materialBuffer;
    DescriptorHeapHandle m_materialBufferSrv;

    std::vector<CameraForCPU> m_camerasForCPU;
    ConstantBuffer m_constantBufferData;

    std::chrono::steady_clock::time_point m_prevTime;

    // Synchronization objects.
    UINT m_fremeIndexPrevious;
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;

    FrameResource m_frameResources[kFrameCount];

    bool m_pendingResize = false;
    UINT m_pendingResizeWidth = 0;
    UINT m_pendingResizeHeight = 0;

    bool m_isPlaying = false;
    bool m_isDraggingInstance = false;
    bool m_instanceTransformDirty = false;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
    XMFLOAT2 m_dragRotation = {0.0f, 0.0f};

    // CPU work meter
    MyDx12Util::WorkMeter m_workMeter;
    float m_cpuFrameTime = 0.f;

    // GPU work meter
    MyDx12Util::GpuWorkMeter m_gpuWorkMeter;

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

    enum class TransientResourceState
    {
        Uninitialized,   // Just instanced but not yet registered.
        Initialized,     // resource is not assigned and only set descriptor and clearValue, etc.
        Created,         // resource is assigned and being used by a pass
        PendingRelease1, // Waiting for retireFeceValue to be set, which indicates when the GPU will finish using this
                         // resource.
        PendingRelease2  // GPU has reached retireFenceValue, waiting for resource to be safe to destroy
    };

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

    using ResourceStateMap = std::unordered_map<std::string, D3D12_RESOURCE_STATES>;
    using ResourceLifetimeMap = Engine::ResourceLifetimeMap;
    using TransientResourceMap = std::unordered_map<std::string, TransientResource>;
    using ResourceUsage = Engine::ResourceUsage;
    using ResourceUsages = Engine::ResourceUsages;
    using PassDescriptorBinding = Engine::PassDescriptorBinding;
    using PassRenderTargetBinding = Engine::PassRenderTargetBinding;
    using PassConstantsBinding = Engine::PassConstantsBinding;
    using RenderPass = Engine::RenderPass;
    using RenderPassGraph = Engine::RenderPassGraph;

    struct ResourceRegistry
    {
        ResourceStateMap states;
        ResourceLifetimeMap lifetimes;
        TransientResourceMap transientResources;

        void AnalyzeLifetimes(const std::vector<RenderPass>& renderPasses);
        void ResetStates(std::initializer_list<ResourceUsage> usages);
        void RegisterTransientResource(TransientResource resource);
        void UnregisterTransientResource(const std::string& name);
        void MarkEndOfLifeResources(int passIndex, const char* backBufferName);
        void MarkPendingTransientResources(UINT64 fenceValue);
        std::vector<std::string> CollectGarbageTransientResources(UINT64 completedFenceValue);
        std::vector<std::string> GetResourcesStartingAtPass(int passIndex, const char* backBufferName) const;
        TransientResource* PrepareTransientResourceForCreate(const std::string& name);
        void MarkTransientResourceCreated(const std::string& name);
        ID3D12Resource* FindTransientD3DResource(const std::string& name) const;

        D3D12_RESOURCE_STATES GetState(const std::string& name) const
        {
            auto resourceState = states.find(name);
            return resourceState != states.end() ? resourceState->second : D3D12_RESOURCE_STATE_COMMON;
        }

        void SetState(const std::string& name, D3D12_RESOURCE_STATES state)
        {
            states[name] = state;
        }
    };

    RenderPassGraph m_renderPassGraph;
    ResourceRegistry m_resourceRegistry;
    using PassOperationHandler = void (D3D12HelloTexture::*)(const RenderPass& pass);
    using RenderPassAuthoringContext = Engine::RenderPassAuthoringContext<PassOperationHandler>;
    Engine::PassOperationRegistry<PassOperationHandler> m_passOperationRegistry;
    RenderPassAuthoringContext m_renderPassAuthoring;

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
    void InitImGui();
    void CreateConstantBuffer(ConstantBufferResource& constantBuffer, const void* initialData, UINT sizeInBytes);
    std::array<GltfVertex, kCubeVertexCount> CreateCubeVertices() const;

    void CreateDepthStencil(UINT width, UINT height);
    void RegisterDepthStencil(UINT width, UINT height);
    void RegisterLightPassRenderTarget(UINT width, UINT height);
    void CreateDepthStencilDescriptors();
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtv() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthDsv() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRTV(UINT index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetLightPassRTV() const;
    void RegisterPassBindingResolvers();

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

    void CreateGBufferResources();
    void CreateGBufferRTVs();
    void CreateGBufferSRVs();
    void CreateGBuffer();

    DescriptorHeapHandle CreateTextureFromRGBA8(const UINT8* pixels, UINT width, UINT height,
                                                ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& uploadHeap);

    void ReleaseResourcesAfterPass(int passIndex);
    void ResetResourceStates();

    void BindPassConstants(const RenderPass& pass);
    ID3D12PipelineState* GetPipelineState(PipelineKey pipeline) const;
    void TransitionPassResources(const RenderPass& pass);
    void TransitionResource(const ResourceUsage& usage);
    ID3D12Resource* ResolveResource(const std::string& name) const;

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
    void CreateDebugDumpReadback(ID3D12Resource* source, ComPtr<ID3D12Resource>& readback,
                                 D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout);
    void PrintDebugDump();

    void DrawInstanceWrapper(UINT instanceCount);
    void DrawFullscreenTriangle();

    void Resize(UINT width, UINT height);

    void WaitForGpu();
    UINT64 MoveToNextFrame();
    void FlushGpu();

    DescriptorHeapHandle AllocateTextureSRV(ID3D12Resource* texture);
};
