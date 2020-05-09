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
#include "RTAOGpuKernels.h"
#include "EngineTuning.h"
#include "Composition.h"
#include "Scene.h"

namespace RTAORayGenShaderType {
    enum Enum {
        AOFullRes = 0,
        AOSortedRays,
        Count
    };
}

namespace RTAO_Args
{
    extern BoolVar QuarterResAO;
    extern IntVar Spp;
    extern IntVar Spp_AOSampleSetDistributedAcrossPixels;
    extern BoolVar Spp_doCheckerboard;
    extern BoolVar Spp_useGroundTruthSpp;
    extern BoolVar RaySorting_Enabled;
    extern NumVar MaxRayHitTime;
}

class RTAO
{
public:
    enum class ResourceType {
        AOCoefficient = 0,
        RayHitDistance
    };

    // Ctors.
    RTAO();

    // Public methods.
    void Setup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DX::DescriptorHeap> descriptorHeap, Scene& scene);
    void Run(D3D12_GPU_VIRTUAL_ADDRESS accelerationStructure, D3D12_GPU_DESCRIPTOR_HANDLE rayOriginSurfaceHitPositionResource, D3D12_GPU_DESCRIPTOR_HANDLE rayOriginSurfaceNormalDepthResource, D3D12_GPU_DESCRIPTOR_HANDLE rayOriginSurfaceAlbedoResource);
    void SetResolution(UINT width, UINT height); 

    // Getters & setters.
    GpuResource(&AOResources())[AOResource::Count]{ return m_AOResources; }
    static DXGI_FORMAT ResourceFormat(ResourceType resourceType);
    float MaxRayHitTime();
    void SetMaxRayHitTime(float maxRayHitTime);
    void GetRayGenParameters(bool* isCheckerboardSamplingEnabled, bool* checkerboardLoadEvenPixels);

    UINT RaytracingWidth() { return m_raytracingWidth; }
    UINT RaytracingHeight() { return m_raytracingHeight; }
    void RequestRecreateAOSamples() { m_isRecreateAOSamplesRequested = true; }

private:
    void UpdateConstantBuffer(UINT frameIndex);
    void CreateDeviceDependentResources(Scene& scene);
    void CreateConstantBuffers();
    void CreateAuxilaryDeviceResources();
    void CreateRootSignatures();
    void CreateRaytracingPipelineStateObject();
    void CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateTextureResources();

    void CreateSamplesRNG();
    void CreateResolutionDependentResources();
    void BuildShaderTables(Scene& scene);
    void DispatchRays(ID3D12Resource* rayGenShaderTable, UINT width = 0, UINT height = 0);

    UINT m_raytracingWidth = 0;
    UINT m_raytracingHeight = 0;

    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::shared_ptr<DX::DescriptorHeap> m_cbvSrvUavHeap;
    std::mt19937 m_generatorURNG;

    // Raytracing shaders.
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_rayGenShaderNames[RTAORayGenShaderType::Count];
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_missShaderName;

    // Raytracing shader resources.
    GpuResource   m_AOResources[AOResource::Count];
    GpuResource   m_AORayDirectionOriginDepth;
    GpuResource   m_sortedToSourceRayIndexOffset;   // Index of a ray in the source array given a sorted index.
    
    ConstantBuffer<RTAOConstantBuffer> m_CB;
    UINT c_NumSampleSets = 83;
    Samplers::MultiJittered m_randomSampler;
    StructuredBuffer<AlignedUnitSquareSample2D> m_samplesGPUBuffer;
    StructuredBuffer<AlignedHemisphereSample3D> m_hemisphereSamplesGPUBuffer;
    BOOL m_isRecreateAOSamplesRequested = true;

    UINT		    m_numAORayGeometryHits;
    bool            m_checkerboardGenerateRaysForEvenPixels = false;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12StateObject>   m_dxrStateObject;

    // Shader tables
    ComPtr<ID3D12Resource> m_rayGenShaderTables[RTAORayGenShaderType::Count];
    UINT m_rayGenShaderTableRecordSizeInBytes[RTAORayGenShaderType::Count];
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    UINT m_hitGroupShaderTableStrideInBytes = UINT_MAX;
    ComPtr<ID3D12Resource> m_missShaderTable;
    UINT m_missShaderTableStrideInBytes = UINT_MAX;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

    // Compute shader & resources.
    RTAOGpuKernels::AORayGenerator  m_rayGen;
    RTAOGpuKernels::SortRays        m_raySorter;


    friend class Composition;
};
