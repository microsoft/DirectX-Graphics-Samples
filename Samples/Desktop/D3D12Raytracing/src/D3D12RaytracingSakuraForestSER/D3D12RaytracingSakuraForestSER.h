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
#include "StepTimer.h"
#include "RaytracingHlslCompat.h"
#include "ObjModelLoader.h"

namespace GlobalRootSignatureParams {
    enum Value {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        SceneConstantSlot,
        VertexBuffersSlot,
        Count
    };
}

namespace LocalRootSignatureParams {
    enum Value {
        CubeConstantSlot = 0,
        Count
    };
}

class D3D12RaytracingSakuraForestSER : public DXSample
{
public:
    D3D12RaytracingSakuraForestSER(UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnDestroy();
    virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

private:
    static const UINT FrameCount = 3;
    UINT numTopLevelInstances = 13525;


    // We'll allocate space for several of these and they will need to be padded for alignment.
    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer constants;
        uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };
    AlignedSceneConstantBuffer* m_mappedConstantData;
    ComPtr<ID3D12Resource>       m_perFrameConstants;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;
    ComPtr<ID3D12StateObject> m_dxrStateObject;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated;
    UINT m_descriptorSize;

    // UI
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont> m_smallFont;

    std::unique_ptr<DirectX::Keyboard> m_keyboard;
    DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;

    // User toggle
    bool m_serEnabled;
    bool m_sortByHit;
    bool m_sortByMaterial;
    bool m_sortByBoth;
    bool m_rebuildASNextFrame;
    bool rotateCamera;

    // Texture resources
    ComPtr<ID3D12Resource> m_trunkTexture;
    ComPtr<ID3D12Resource> m_bushTexture;
    CD3DX12_GPU_DESCRIPTOR_HANDLE m_textureSrvGpuDescriptorTrunk;
    CD3DX12_GPU_DESCRIPTOR_HANDLE m_textureSrvGpuDescriptorBush;

    // Raytracing scene
    SceneConstantBuffer m_sceneCB[FrameCount];
    ObjectConstantBuffer m_objectCB;
    ObjectConstantBuffer m_cubeCB;
    ObjectConstantBuffer m_reflectiveCubeCB;
    ObjectConstantBuffer m_trunkCB;
    ObjectConstantBuffer m_leavesCB;
    ObjectConstantBuffer m_bushCB;

    // Asset loader
    ObjModelLoader m_ObjModelLoader;

    // Geometry
    struct D3DBuffer
    {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
    };
    D3DBuffer m_indexBuffer;
    D3DBuffer m_vertexBuffer;
    D3DBuffer m_trunkIndexBuffer;
    D3DBuffer m_trunkVertexBuffer;
    D3DBuffer m_leavesIndexBuffer;
    D3DBuffer m_leavesVertexBuffer;
    D3DBuffer m_bushIndexBuffer;
    D3DBuffer m_bushVertexBuffer;
    int m_totalTrunkVertexCount;
    int m_totalLeavesVertexCount;
    int m_totalBushVertexCount;
    
    // Acceleration structure
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructureCube;
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructureTrunk;
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructureLeaves;
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructureBushes;

    ComPtr<ID3D12Resource> m_topLevelAccelerationStructuretrunk;
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructureLeaves;
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    // Shader tables
    static const wchar_t* c_floorHitGroupName;
    static const wchar_t* c_trunkHitGroupName;
    static const wchar_t* c_leavesHitGroupName;
    static const wchar_t* c_bushHitGroupName;
    static const wchar_t* c_reflectiveCubeHitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_floorClosestHitShaderName;
    static const wchar_t* c_trunkClosestHitShaderName;
    static const wchar_t* c_leavesClosestHitShaderName;
    static const wchar_t* c_refCubeClosestHitShaderName;
    static const wchar_t* c_missShaderName;
    static const wchar_t* c_bushClosestHitShaderName;
    ComPtr<ID3D12Resource> m_missShaderTable;
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    ComPtr<ID3D12Resource> m_rayGenShaderTable;

    // Application state
    StepTimer m_timer;
    float m_curRotationAngleRad;
    XMVECTOR m_eye;
    XMVECTOR m_at;
    XMVECTOR m_up;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>    m_graphicsMemory;

    void UpdateCameraMatrices();
    void InitializeScene();
    void RecreateD3D();
    void DoRaytracing();
    void CreateConstantBuffers();
    void CreateDeviceDependentResources();
    void CreateUIFont();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    void CreateRootSignatures();
    void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
    void BuildCubeGeometry();
    void CreateTexture();
    void BuildTreeGeometry();
    void BuildAccelerationStructures();
    void BuildShaderTables();
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void RenderUI();
    void CopyRaytracingOutputToBackbuffer();
    void CalculateFrameStats();
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
};