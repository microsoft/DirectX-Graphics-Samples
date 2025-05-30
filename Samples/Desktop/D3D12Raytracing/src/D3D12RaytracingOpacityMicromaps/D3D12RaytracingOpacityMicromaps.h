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

namespace GlobalRootSignatureParams {
    enum Value {
        FrameParams = 0,
        Count 
    };
}

class D3D12RaytracingOpacityMicromaps : public DXSample
{
public:
    D3D12RaytracingOpacityMicromaps(UINT width, UINT height, std::wstring name);

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
    static const UINT FrameCount = 2;

    // We'll allocate space for several of these and they will need to be padded for alignment.
    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer constants;
        uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };
    AlignedSceneConstantBuffer*  m_mappedConstantData;
    ComPtr<ID3D12Resource>       m_perFrameConstants;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;
    ComPtr<ID3D12StateObject> m_dxrStateObject;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorSize;
    
    // Raytracing scene
    SceneConstantBuffer m_sceneCB[FrameCount];

    // Geometry
    struct D3DBuffer
    {
        ComPtr<ID3D12Resource> uploadResource, defaultResource;
    };

    struct OMMSet
    {
        D3DBuffer descBuffer;
        D3DBuffer arrayBuffer;
        D3DBuffer indexBuffer;

        D3D12_RAYTRACING_OPACITY_MICROMAP_HISTOGRAM_ENTRY* histogramBuffer;
        UINT numHistogramEntries;
    };

    // Acceleration structure
    ComPtr<ID3D12Resource> m_scratchResource;
    ComPtr<ID3D12Resource> m_instanceDescs;
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;
    ComPtr<ID3D12Resource> m_ommAccelerationStructure;

    // Model-related resources
    D3DBuffer m_positionBuffer, m_normalBuffer, m_texCoordBuffer;
    D3DBuffer m_positionIndexBuffer, m_normalIndexBuffer, m_texCoordIndexBuffer;
    D3DBuffer m_indexBuffer;

    static const UINT NUM_TEXTURES = 4;

    ComPtr<ID3D12Resource> m_textureDefault[NUM_TEXTURES];
    ComPtr<ID3D12Resource> m_textureUpload[NUM_TEXTURES];

    ComPtr<ID3D12Resource> m_geometryOffsetBuffer;

    static const UINT MAX_SUBDIVISION_LEVELS = 12;
    OMMSet m_ommSets[MAX_SUBDIVISION_LEVELS][2];

    UINT m_numGeoms;
    UINT* m_indicesPerGeom;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;

    // Shader tables
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_anyHitShaderName;
    static const wchar_t* c_missShaderName;
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

    // UI
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont> m_smallFont;

    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;

    // User toggles
    bool m_ommEnabled;
    bool m_rotateCamera;
    bool m_use4State;
    UINT m_currentSubDLevel;
    float m_fov;

    bool m_haveLoadedTextures;
    bool m_rebuildASEveryFrame;
    bool m_rebuildASNextFrame;
    bool m_haveBuiltAccelerationStructuresOnce;

    UINT m_configFlags;
    UINT m_extraPrimaryRayFlags;
    UINT m_extraShadowRayFlags;

    float m_cameraRotationAngle;
    bool m_waitForGPUNextFrame;

    void UpdateCameraMatrices();
    void InitializeScene();
    void RecreateD3D();
    void DoRaytracing();
    void RenderUI();
    void CreateConstantBuffers();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    void CreateRootSignatures();
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
    void LoadAndBuildAccelerationStructures();
    void LoadTexture(const wchar_t* texturePath, ID3D12Resource** resource, ID3D12Resource** uploadResource);
    void LoadTextures();
    void LoadModel(const char* modelPath);
    void LoadOMM(const char* ommPath, OMMSet& buffers);
    void BuildAccelerationStructures(bool updateUploadBuffers);
    void BuildShaderTables();
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CalculateFrameStats();
    void CreateUIFont();
};
