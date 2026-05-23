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
#include "SimpleDescriptorHeapAllocator.h"
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
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

// struct GltfVertex;
#include "GltfLoader.h"

class D3D12HelloTexture : public DXSample
{
  public:
    D3D12HelloTexture(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void OnWindowSizeChanged(UINT width, UINT height);
    virtual void OnIdle();

  private:
    static constexpr UINT kFrameCount = 2;
    static constexpr UINT kTextureWidth = 256;
    static constexpr UINT kTextureHeight = 256;
    static constexpr UINT kTexturePixelSize = 4; // The number of bytes used to represent a pixel in the texture.

    static constexpr UINT kGBufferCount = 4;

    static constexpr UINT kHeapDescriptorCount = 100;
    // 0 - 99 : ImGui (new)

    static constexpr UINT kTextureCount = 1020;
    static constexpr UINT kTextureTypes = 1020; // Color Type : 0-9

    static constexpr UINT kInstanceBufferCount = kFrameCount;
    static constexpr UINT kMaterialBufferCount = 1;
    static constexpr UINT kConstantBufferCount = kFrameCount;
    static constexpr UINT kLightConstantBufferCount = kFrameCount;

    // Descriptor allocation order is tracked by DescriptorHeapHandle.
    // Current persistent descriptors: GBuffer SRVs, depth SRV, texture table, instance buffers, material buffer,
    // constant buffer, light constant buffer.
    static constexpr UINT kMainHeapDescriptorCount = kTextureCount + kInstanceBufferCount + kMaterialBufferCount +
                                                     kConstantBufferCount + kLightConstantBufferCount + kGBufferCount +
                                                     1;

    static constexpr float kTranslationSpeed = 0.005f;
    static constexpr float kPI = 3.141592f;
    static constexpr float kRotationSpeed = kPI / 180.f / 3.f;
    static constexpr float kOffsetBounds = 5.f;
    static constexpr float kCameraMoveSpeed = 0.01f;

    static constexpr UINT kInstanceCount = 100;
    static constexpr float kCubeScale = 0.2f;
    static constexpr UINT kMaterialCount = 256;

    static constexpr UINT kCubeVertexCount = (3 * 2 * 6);

    static constexpr int kGpuWorkMeterQueryCount = 100;

    static constexpr bool kTlgffLoadingEnabled =
        true; // glTFメッシュとCubeを切り替える true: glTFモデルを読み込む、false: Cubeを描画する

    struct GridDim
    {
        GridDim(int x, int y, int z) : x(x), y(y), z(z) {}
        int x;
        int y;
        int z;
    };

    inline XMFLOAT3 instanceIdToXYZ(int instanceId, const GridDim &dim)
    {
        int x = instanceId % dim.x - (kInstanceCount / dim.z / dim.y) / 2;
        int y = (instanceId / dim.x) % dim.y - (kInstanceCount / dim.x / dim.y) / 2;
        int z = -instanceId / (dim.x * dim.y) + (kInstanceCount / dim.x / dim.y) / 2;
        return XMFLOAT3((float)x, (float)y, (float)z);
    }
    struct Material
    {
        UINT textureIndex;
        float roughness;
        float metallic;
        UINT flags;
    };

    struct InstanceData
    {
        XMFLOAT4X4 world;
        XMFLOAT4X4 prevWorld;
        UINT materialId;
        float padding[3]; // 16byte alignment
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
        void updateViewProjectionMatrix() { viewProjection = XMMatrixMultiply(view, projection); }
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
    };

    struct alignas(256) LightingConstants
    {
        XMFLOAT3 lightDirection = {0.4f, 0.7f, 0.6f};
        float ambientIntensity = 0.25f;
        XMFLOAT3 lightColor = {1.0f, 1.0f, 1.0f};
        float diffuseIntensity = 1.0f;
        XMFLOAT4 backgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
    };

    struct ConstantBufferResource
    {
        ComPtr<ID3D12Resource> buffer;
        DescriptorHeapHandle cbv;
        UINT8 *mappedData = nullptr;
    };

    struct FrameResource
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        ComPtr<ID3D12Resource> instanceBuffer;
        DescriptorHeapHandle instanceBufferSrv;
        InstanceData *pSrvDataBegin = nullptr;
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
        };

        ComPtr<ID3D12Resource> resources[kCount];

        DXGI_FORMAT formats[kCount] = {
            DXGI_FORMAT_R8G8B8A8_UNORM,     // Albedo
            DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
            DXGI_FORMAT_R32_UINT,           // Material
            DXGI_FORMAT_R16G16_FLOAT,       // Motion Vector
        };

        D3D12_CLEAR_VALUE clearValues[kCount] = {
            {DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 1.0f}},
            {DXGI_FORMAT_R16G16B16A16_FLOAT, {0.5f, 0.5f, 1.0f, 1.0f}},
            {DXGI_FORMAT_R32_UINT, {0.0f, 0.0f, 0.0f, 0.0f}},
            {DXGI_FORMAT_R16G16_FLOAT, {0.0f, 0.0f, 0.0f, 0.0f}},
        };

        UINT rtvIndex[kCount] = {};
        DescriptorHeapHandle srvHandles[kCount];
    };
    static constexpr UINT kSwapChainRTVCount = kFrameCount;
    static constexpr UINT kGBufferRTVBaseIndex = kSwapChainRTVCount;
    static constexpr UINT kRTVDescriptorCount = kFrameCount + GBuffer::kCount;

    enum class RenderViewMode
    {
        LightPass = 0,
        GBufferAlbedo,
        GBufferNormal,
        GBufferMaterial,
        GBufferMotionVector,
        Depth,
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    GBuffer m_gbuffer;
    ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    DescriptorHeapHandle m_depthStencilSrv;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12RootSignature> m_lightingRootSignature; // not used in this sample but created for future use

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    ComPtr<ID3D12DescriptorHeap> m_heap;                     // CBV/SRV/UAV heap
    SimpleDescriptorHeapAllocator m_descriptorHeapAllocator; // Allocator for CBV/SRV/UAV heap

    ComPtr<ID3D12DescriptorHeap> m_imguiHeap;
    SimpleDescriptorHeapAllocator m_ImGuiDescriptorHeapAllocator;

    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12PipelineState> m_depthPrePassPSO;
    ComPtr<ID3D12PipelineState> m_gbufferPSO;
    ComPtr<ID3D12PipelineState> m_gbufferDebugPSO;
    ComPtr<ID3D12PipelineState> m_lightPassPSO;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_descriptorSize;
    std::array<float, 4> m_backBufferClearColor = {0.0f, 0.2f, 0.4f, 1.0f};
    RenderViewMode m_renderViewMode = RenderViewMode::LightPass;

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

    std::vector<ComPtr<ID3D12Resource>> m_texture;

    UINT m_vertexCountPerInstance = kCubeVertexCount;
    UINT m_indexCountPerInstance = 0;

    int m_maxVisibleCubeCount = static_cast<int>(kInstanceCount);

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

    // CPU work meter
    MyDx12Util::WorkMeter m_workMeter;
    float m_cpuFrameTime = 0.f;

    // GPU work meter
    MyDx12Util::GpuWorkMeter m_gpuWorkMeter;

    static constexpr const char *kBackBufferResourceName = "BackBuffer";
    static constexpr const char *kDepthStencilResourceName = "DepthStencil";
    static constexpr const char *kGBufferResourceNames[GBuffer::kCount] = {"GBuffer.Albedo", "GBuffer.Normal",
                                                                           "GBuffer.Material", "GBuffer.MotionVector"};

    struct ResourceUsage
    {
        std::string name;
        ID3D12Resource *resource;
        D3D12_RESOURCE_STATES state;
    };

    struct ResourceLifetime
    {
        int firstPass = INT_MAX;
        int lastPass = -1;
    };

    enum RootParameterIndex
    {
        RootParam_TextureTable = 0,
        RootParam_InstanceSrv,
        RootParam_MaterialSrv,
        RootParam_ConstantBuffer,
        RootParam_GBufferSrvBase,
        RootParam_LightConstants,
        RootParam_GBufferDebugConstants
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

    using ResourceUsageMap = std::unordered_map<std::string, ResourceUsage>;
    using ResourceStateMap = std::unordered_map<std::string, D3D12_RESOURCE_STATES>;
    using ResourceLifetimeMap = std::unordered_map<std::string, ResourceLifetime>;
    using TransientResourceMap = std::unordered_map<std::string, TransientResource>;

    struct PassDescriptorBinding
    {
        UINT rootParameterIndex;
        DescriptorHeapHandle handle;
    };

    struct PassRenderTargetBinding
    {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
        std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv;
        std::optional<std::array<float, 4>> clearColor;
    };

    struct RenderPass
    {
        const wchar_t *name;
        ResourceUsageMap reads;
        ResourceUsageMap writes;
        std::vector<PassDescriptorBinding> descriptorBindings;
        PassRenderTargetBinding renderTargets;
        std::function<void(const RenderPass &)> execute;
    };

    std::vector<RenderPass> m_renderPasses;
    ResourceStateMap m_resourceStates; // Current state per named resource.
    ResourceLifetimeMap m_resourceLifetimes;
    TransientResourceMap m_transientResources;

    void LoadPipeline();
    void LoadAssets();
    void InitImGui();
    void CreateConstantBuffer(ConstantBufferResource &constantBuffer, const void *initialData, UINT sizeInBytes);
    std::array<GltfVertex, kCubeVertexCount> CreateCubeVertices() const;

    void CreateDepthStencil(UINT width, UINT height);
    void RegisterDepthStencil(UINT width, UINT height);
    void CreateDepthStencilDescriptors();
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtv() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthDsv() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRTV(UINT index) const;

    std::vector<UINT8> GenerateCheckerboardTextureData();
    void PopulateCommandList();

    void AddPass(const wchar_t *name, ResourceUsageMap reads, ResourceUsageMap writes,
                 std::vector<PassDescriptorBinding> descriptorBindings, PassRenderTargetBinding renderTargets,
                 std::function<void(const RenderPass &)> execute);
    ResourceUsageMap MakeResourceUsageMap(std::initializer_list<ResourceUsage> usages) const;
    ResourceUsageMap MakeGBufferReadUsageMap() const;
    std::vector<PassDescriptorBinding> MakeGBufferSrvBindings() const;
    bool IsGBufferDebugView() const;
    UINT GetGBufferDebugTarget() const;
    void BuildRenderPasses();
    void AnalyzeResourceLifetimes();
    void DebugPrintLifetimes();
    void ExecutePasses();
    void CreateResourcesForPass(int passIndex);
    void CreateDsvHeap();

    void CreateGBufferResources();
    void CreateGBufferRTVs();
    void CreateGBufferSRVs();
    void CreateGBuffer();

    void ReleaseResourcesAfterPass(int passIndex);
    void ResetResourceStates();

    void BindPassRenderTargets(const RenderPass &pass);
    void BindPassDescriptors(const RenderPass &pass);
    void TransitionPassResources(const RenderPass &pass);
    void TransitionResource(const ResourceUsage &usage);

    D3D12_RESOURCE_STATES GetResourceState(const std::string &name) const;
    void SetResourceState(const std::string &name, D3D12_RESOURCE_STATES state);
    void MarkPendingTransientResources(UINT64 fenceValue);
    void CollectGarbageTransientResources();

    void UpdateImGui();
    UINT GetVisibleCubeCount() const;

    void BeginFrame();
    void RecordClear(const PassRenderTargetBinding &renderTargets);
    void RecordDepthPrePass();
    void RecordGBufferPass(const PassRenderTargetBinding &renderTargets);
    void RecordGBufferDebugPass();
    void RecordLightPass();
    void RecordMainPass();
    void RecordImGuiPass();
    void EndFrame();

    void DrawInstanceWrapper(UINT vertexOrIndexCount, UINT instanceCount);

    void Resize(UINT width, UINT height);

    void WaitForGpu();
    UINT64 MoveToNextFrame();
    void FlushGpu();

    DescriptorHeapHandle AllocateTextureSRV(ID3D12Resource *texture);
};
