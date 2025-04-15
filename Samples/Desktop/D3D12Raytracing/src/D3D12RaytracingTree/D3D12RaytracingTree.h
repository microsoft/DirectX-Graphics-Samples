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


#define USE_OMMS

namespace GlobalRootSignatureParams {
    enum Value {
        FrameParams = 0,
        Count 
    };
}

namespace LocalRootSignatureParams {
    enum Value {
        CubeConstantSlot = 0,
        Count 
    };
}

class D3D12RaytracingTree : public DXSample
{
public:
    D3D12RaytracingTree(UINT width, UINT height, std::wstring name);

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
    TreeConstantBuffer m_treeCB;

    // Geometry
    struct D3DBuffer
    {
        ComPtr<ID3D12Resource> uploadResource, defaultResource;
        //D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
        //D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
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

    D3DBuffer m_ommDescBuffer, m_ommArrayBuffer, m_ommIndexBuffer;

    D3D12_RAYTRACING_OPACITY_MICROMAP_HISTOGRAM_ENTRY* m_ommHistogramBuffer;
    UINT m_numHistogramEntries;

    UINT m_numGeoms;
    UINT* m_indicesPerGeom;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;

    // Shader tables
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
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

    void UpdateCameraMatrices();
    void InitializeScene();
    void RecreateD3D();
    void DoRaytracing();
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
    void LoadModel(const char* modelPath, const char* ommPath);
    void BuildAccelerationStructures();
    void BuildShaderTables();
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CalculateFrameStats();
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
};
