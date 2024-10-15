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

#include "RaytracingSceneDefines.h"
#include "DirectXRaytracingHelper.h"
#include "RaytracingAccelerationStructure.h"
#include "CameraController.h"
#include "PerformanceTimers.h"
#include "Sampler.h"
#include "GpuKernels.h"
#include "EngineTuning.h"
#include "Scene.h"

namespace Pathtracer_Args
{
    extern NumVar DefaultAmbientIntensity;
}

class Pathtracer
{
public:
    // Ctors.
    Pathtracer();

    // Public methods.
    void Setup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DX::DescriptorHeap> descriptorHeap, Scene& scene);
    void Run(Scene& scene);
    void SetResolution(UINT GBufferWidth, UINT GBufferHeight, UINT RTAOWidth, UINT RTAOHeight);

    // Getters & setters.
    void SetCamera(const GameCore::Camera& camera);
    GpuResource(&GBufferResources(bool getQuarterResResources = false))[GBufferResource::Count];
    UINT Width() { return m_raytracingWidth; }
    UINT Height() { return m_raytracingHeight; }

    void RequestRecreateRaytracingResources() { m_isRecreateRaytracingResourcesRequested = true; }
private:
    void UpdateConstantBuffer(Scene& scene);
    void CreateDeviceDependentResources(Scene& scene);
    void CreateConstantBuffers();
    void CreateAuxilaryDeviceResources();
    void CreateRootSignatures();
    void CreateRaytracingPipelineStateObject();
    void CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateTextureResources();

    void CreateResolutionDependentResources();
    void BuildShaderTables(Scene& scene);
    void DispatchRays(ID3D12Resource* rayGenShaderTable, UINT width = 0, UINT height = 0);
    void DownsampleGBuffer();

    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::shared_ptr<DX::DescriptorHeap> m_cbvSrvUavHeap;

    UINT m_raytracingWidth; 
    UINT m_raytracingHeight;
    UINT m_quarterResWidth;
    UINT m_quarterResHeight;

    // Raytracing shaders.
    static const wchar_t* c_rayGenShaderNames[RayGenShaderType::Count];
    static const wchar_t* c_closestHitShaderNames[PathtracerRayType::Count];
    static const wchar_t* c_missShaderNames[PathtracerRayType::Count];
    static const wchar_t* c_hitGroupNames[PathtracerRayType::Count];

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12StateObject>   m_dxrStateObject;

    // Shader tables
    ComPtr<ID3D12Resource> m_rayGenShaderTables[RayGenShaderType::Count];
    UINT m_rayGenShaderTableRecordSizeInBytes[RayGenShaderType::Count];
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    UINT m_hitGroupShaderTableStrideInBytes = UINT_MAX;
    ComPtr<ID3D12Resource> m_missShaderTable;
    UINT m_missShaderTableStrideInBytes = UINT_MAX;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

    // Raytracing resources.
    ConstantBuffer<PathtracerConstantBuffer> m_CB;
    GpuResource m_GBufferResources[GBufferResource::Count];
    GpuResource m_GBufferQuarterResResources[GBufferResource::Count];

    D3D12_GPU_DESCRIPTOR_HANDLE m_nullVertexBufferGPUhandle;

    GpuKernels::CalculatePartialDerivatives  m_calculatePartialDerivativesKernel;
    GpuKernels::DownsampleGBufferDataBilateralFilter m_downsampleGBufferBilateralFilterKernel;

    bool m_isRecreateRaytracingResourcesRequested = false;
};