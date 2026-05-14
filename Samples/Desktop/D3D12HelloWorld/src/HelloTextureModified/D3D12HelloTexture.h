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
#include <chrono>
#include <climits>
#include <functional>
#include <initializer_list>
#include <string>
#include <unordered_map>

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

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

    static constexpr UINT kHeapDescriptorCount = 1124;
    // 0 - 1019 : Texture
    // 1020, 1021 : instanceBuffer
    // 1022 : material buffer
    // 1023 : constant buffer
    // 1024 - 1123 : ImGui

    static constexpr UINT kTextureCount = 1020;
    static constexpr UINT kTextureTypes = 1020; // Color Type : 0-9

    static constexpr UINT kImGuiDescriptorStart = 1024;
    static constexpr UINT kImguiDescriptorNum = 100;

    static constexpr float kTranslationSpeed = 0.005f;
    static constexpr float kPI = 3.141592f;
    static constexpr float kRotationSpeed = kPI / 180.f / 3.f;
    static constexpr float kOffsetBounds = 5.f;
    static constexpr float kCameraMoveSpeed = 0.01f;

    static constexpr UINT kInstanceCount = 1000;
    static constexpr float kCubeScale = 0.2f;
    static constexpr UINT kMaterialCount = 256;

    static constexpr UINT kTriangleVertexCount = 10;
    static constexpr UINT kCubeVertexCount = (3 * 2 * 6);

    static constexpr int kGpuWorkMeterQueryCount = 100;

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

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct Material
    {
        UINT textureIndex;
        float padding[3]; // 16byte alignment
    };

    struct InstanceData
    {
        XMFLOAT4X4 world;
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
    };

    struct FrameResource
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        ComPtr<ID3D12Resource> instanceBuffer;
        InstanceData *pSrvDataBegin = nullptr;
        UINT64 fenceValue = 0;
        std::vector<MyDx12Util::GpuWorkMeter::CheckPoint> gpuWorkMeterCheckPoints;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_heap;                     // CBV/SRV/UAV heap
    SimpleDescriptorHeapAllocator m_descriptorHeapAllocator; // Allocator for CBV/SRV/UAV heap

    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12PipelineState> m_depthPrePassPSO;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_descriptorSize;
    UINT m_nextFrameIndex = 0;

    UINT m_texIndex[kTextureCount] = {};
    UINT m_nextFreeIndex = 0;

    std::vector<InstanceData> m_instanceData;
    std::vector<InstanceDataForCPU> m_instanceDataForCPU;
    std::vector<Material> m_materialData;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    std::vector<ComPtr<ID3D12Resource>> m_texture;

    UINT m_vertexCountPerInstance;

    ComPtr<ID3D12Resource> m_materialBuffer;
    Material *pMaterialDataBegin = nullptr;

    std::vector<CameraForCPU> m_camerasForCPU;
    ComPtr<ID3D12Resource> m_constantBuffer;
    UINT8 *m_pCbvDataBegin;
    ConstantBuffer m_constantBufferData;

    std::chrono::steady_clock::time_point m_prevTime;
    UINT m_texIndexId = 0;

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

    using ResourceUsageMap = std::unordered_map<std::string, ResourceUsage>;
    using ResourceStateMap = std::unordered_map<std::string, D3D12_RESOURCE_STATES>;
    using ResourceLifetimeMap = std::unordered_map<std::string, ResourceLifetime>;

    struct RenderPass
    {
        const wchar_t *name;
        ResourceUsageMap reads;
        ResourceUsageMap writes;
        std::function<void()> execute;
    };

    std::vector<RenderPass> m_renderPasses;
    ResourceStateMap m_resourceStates; // Current state per named resource.
    ResourceLifetimeMap m_resourceLifetimes;

    void LoadPipeline();
    void LoadAssets();
    void InitImGui();

    void CreateDepthStencil(UINT width, UINT height);

    std::vector<UINT8> GenerateTextureData();
    void PopulateCommandList();

    void AddPass(const wchar_t *name, ResourceUsageMap reads, ResourceUsageMap writes, std::function<void()> execute);
    ResourceUsageMap MakeResourceUsageMap(std::initializer_list<ResourceUsage> usages) const;
    void BuildRenderPasses();
    void AnalyzeResourceLifetimes();
    void DebugPrintLifetimes();
    void ExecutePasses();
    void ResetResourceStates();
    void TransitionPassResources(const RenderPass &pass);
    void TransitionResource(const ResourceUsage &usage);
    D3D12_RESOURCE_STATES GetResourceState(const std::string &name) const;
    void SetResourceState(const std::string &name, D3D12_RESOURCE_STATES state);

    void BeginFrame();
    void RecordClear();
    void RecordDepthPrePass();
    void RecordMainPass();
    void RecordImGuiPass();
    void EndFrame();

    void Resize(UINT width, UINT height);

    void WaitForGpu();
    void MoveToNextFrame();
    void FlushGpu();

    UINT AllocateTextureSRV(ID3D12Resource *texture);
};
