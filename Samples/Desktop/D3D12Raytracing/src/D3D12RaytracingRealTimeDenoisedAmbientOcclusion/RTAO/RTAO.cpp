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

#include "stdafx.h"
#include "RTAO.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "EngineProfiling.h"
#include "GpuTimeManager.h"
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "CompiledShaders\RTAO.hlsl.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;

namespace GlobalRootSignature {
    namespace Slot {
        enum Enum {
            AccelerationStructure = 0,
            RayOriginPosition,
            RayOriginSurfaceNormalDepth,
            AOAmbientCoefficient,
            AORayHitDistance,
            AORayDirectionOriginDepthHitSRV,
            AOSortedToSourceRayIndex,
            AOSurfaceAlbedo,
            ConstantBuffer,
            SampleBuffers,
            Count
        };
    }
}
  
// Shader entry points.
const wchar_t* RTAO::c_rayGenShaderNames[] = { L"RayGenShader", L"RayGenShader_sortedRays" };
const wchar_t* RTAO::c_closestHitShaderName = L"ClosestHitShader";
const wchar_t* RTAO::c_missShaderName = L"MissShader";

// Hit groups.
const wchar_t* RTAO::c_hitGroupName = L"HitGroup_Triangle";


#define RPP_SAMPLSETDISTRIBUTIONACROSSPIXELS1D 8
#define GROUND_TRUTH_RPP 256

// Turn A into a string literal without expanding macro definitions
// (however, if invoked from a macro, macro arguments are expanded).
#define STRINGIZE_X(A) #A

//Turn A into a string literal after macro-expanding it.
#define STRINGIZE(A) STRINGIZE_X(A)
       
namespace RTAO_Args
{
    void OnSppSampleSetChange(void*)
    {
        RTAO_Args::Spp_useGroundTruthSpp.SetValue(false, false);

        if (RTAO_Args::Spp > 1)
        {
            // TODO GetRandomRayDirection() supports sample set distribution only for 1 spp.
            RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels.SetValue(1, false);

            // Only non-ray sorted path supports > 1spp.
            RTAO_Args::RaySorting_Enabled.SetValue(false, false);
        }

        Sample::instance().RTAOComponent().RequestRecreateAOSamples();
    }
    
    void OnToggleSppGroundTruth(void*)
    {
        if (RTAO_Args::Spp_useGroundTruthSpp)
        {
            RTAO_Args::Spp.SetValue(GROUND_TRUTH_RPP, false);
            RTAO_Args::Spp_doCheckerboard.SetValue(false, false);
            RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels.SetValue(1, false);
            Composition_Args::CompositionMode.SetValue(CompositionType::AmbientOcclusionOnly_RawOneFrame, false);
        }
        else
        {
            RTAO_Args::Spp.SetValue(1, false);
            RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels.SetValue(RPP_SAMPLSETDISTRIBUTIONACROSSPIXELS1D, false);
            Composition_Args::CompositionMode.SetValue(CompositionType::AmbientOcclusionOnly_Denoised, false);
        }

        Sample::instance().RTAOComponent().RequestRecreateAOSamples();
    }

    void OnToggleSppCheckerboard(void*)
    {
        if (RTAO_Args::Spp_doCheckerboard)
        {
            RTAO_Args::Spp.SetValue(1, false);
            RTAO_Args::Spp_useGroundTruthSpp.SetValue(false, false);
            RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels.SetValue(RPP_SAMPLSETDISTRIBUTIONACROSSPIXELS1D, false);
        }
        
        Sample::instance().RTAOComponent().RequestRecreateAOSamples();
    }

    BoolVar RaySorting_Enabled(L"Render/AO/RTAO/Ray Sorting/Enabled", false);
    NumVar RaySorting_DepthBinSizeMultiplier(L"Render/AO/RTAO/Ray Sorting/Ray bin depth size (multiplier of MaxRayHitTime)", 0.1f, 0.01f, 10.f, 0.01f);

    IntVar Spp(L"Render/AO/RTAO/Sampling/Rays per pixel", 1, 1, 1024, 1, OnSppSampleSetChange);
    IntVar Spp_AOSampleSetDistributedAcrossPixels(L"Render/AO/RTAO/Sampling/Sample set distribution across NxN pixels ", RPP_SAMPLSETDISTRIBUTIONACROSSPIXELS1D, 1, 8, 1, OnSppSampleSetChange);
    BoolVar Spp_doCheckerboard(L"Render/AO/RTAO/Sampling/Overrides/Do checkerboard 0.5 spp", false, OnToggleSppCheckerboard);
    BoolVar Spp_useGroundTruthSpp(L"Render/AO/RTAO/Sampling/Overrides/Do ground truth spp (no denoising): " STRINGIZE(GROUND_TRUTH_RPP), false, OnToggleSppGroundTruth);

    BoolVar RayGen_RandomFrameSeed(L"Render/AO/RTAO/Random per-frame seed", true);

    const WCHAR* FloatingPointFormatsR[TextureResourceFormatR::Count] = { L"R32_FLOAT", L"R16_FLOAT", L"R8_SNORM" };
    EnumVar AmbientCoefficientResourceFormat(L"Render/Texture Formats/AO/RTAO/Ambient Coefficient", TextureResourceFormatR::R16_FLOAT, TextureResourceFormatR::Count, FloatingPointFormatsR, Sample::OnRecreateRaytracingResources);


    NumVar MaxRayHitTime(L"Render/AO/RTAO/Max ray hit time", 22, 0.0f, 100.0f, 0.2f);
    NumVar  MinimumAmbientIllumination(L"Render/AO/RTAO/Minimum Ambient Illumination", 0.07f, 0.0f, 1.0f, 0.01f);

    BoolVar ApproximateInterreflections_Enabled(L"Render/AO/RTAO/Approximate Interreflections/Enabled", true);
    NumVar ApproximateInterreflections_DiffuseReflectanceScale(L"Render/AO/RTAO/Approximate Interreflections/Diffuse Reflectance Scale", 0.5f, 0.0f, 1.0f, 0.1f);
    
    BoolVar ExponentialFalloff_Enabled(L"Render/AO/RTAO/Exponential Falloff/Enabled", true);
    NumVar ExponentialFalloff_DecayConstant(L"Render/AO/RTAO/Exponential Falloff/Decay Constant", 2.f, 0.0f, 20.f, 0.25f);
    NumVar ExponentialFalloff_MinOcclusionCutoff(L"Render/AO/RTAO/Exponential Falloff/Min Occlusion Cutoff", 0.4f, 0.0f, 1.f, 0.05f);
    
    BoolVar QuarterResAO(L"Render/AO/Quarter resolution", false, Sample::OnRecreateRaytracingResources, nullptr);
}


DXGI_FORMAT RTAO::ResourceFormat(ResourceType resourceType)
{
    switch (resourceType)
    {
    case ResourceType::AOCoefficient: return  TextureResourceFormatR::ToDXGIFormat(RTAO_Args::AmbientCoefficientResourceFormat);
    case ResourceType::RayHitDistance: return DXGI_FORMAT_R16_FLOAT;
    }

    return DXGI_FORMAT_UNKNOWN;
}

float RTAO::MaxRayHitTime()
{
    return RTAO_Args::MaxRayHitTime;
}

void RTAO::SetMaxRayHitTime(float maxRayHitTime)
{
    return RTAO_Args::MaxRayHitTime.SetValue(maxRayHitTime);
}

RTAO::RTAO()
{
    for (auto& rayGenShaderTableRecordSizeInBytes : m_rayGenShaderTableRecordSizeInBytes)
    {
        rayGenShaderTableRecordSizeInBytes = UINT_MAX;
    }
    m_generatorURNG.seed(1729);
}

void RTAO::Setup(shared_ptr<DeviceResources> deviceResources, shared_ptr<DX::DescriptorHeap> descriptorHeap, Scene& scene)
{
    m_deviceResources = deviceResources;
    m_cbvSrvUavHeap = descriptorHeap;

    CreateDeviceDependentResources(scene);
}


// Create resources that depend on the device.
void RTAO::CreateDeviceDependentResources(Scene& scene)
{
    CreateAuxilaryDeviceResources();

    // Initialize raytracing pipeline.

    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();

    CreateConstantBuffers();

    BuildShaderTables(scene);
}

void RTAO::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandList = m_deviceResources->GetCommandList();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    m_rayGen.Initialize(device, FrameCount);
    m_raySorter.Initialize(device, FrameCount);

    // Random sample buffers
    {
        UINT maxPixelsInSampleSet1D = RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels.MaxValue();
        UINT maxSamplesPerSet = RTAO_Args::Spp.MaxValue() * maxPixelsInSampleSet1D * maxPixelsInSampleSet1D;

        m_samplesGPUBuffer.Create(device, maxSamplesPerSet * c_NumSampleSets, FrameCount, L"GPU buffer: Random unit square samples");
        m_hemisphereSamplesGPUBuffer.Create(device, maxSamplesPerSet * c_NumSampleSets, FrameCount, L"GPU buffer: Random hemisphere samples");
    }
}

// Create constant buffers.
void RTAO::CreateConstantBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    m_CB.Create(device, FrameCount, L"RTAO Constant Buffer");
}


void RTAO::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        using namespace GlobalRootSignature;

        CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count]; 
        ranges[Slot::AOAmbientCoefficient].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 10);
        ranges[Slot::AORayHitDistance].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 15);   
        ranges[Slot::RayOriginPosition].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);  
        ranges[Slot::RayOriginSurfaceNormalDepth].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8);  
        ranges[Slot::AORayDirectionOriginDepthHitSRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 22);
        ranges[Slot::AOSortedToSourceRayIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 23);  
        ranges[Slot::AOSurfaceAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 24); 

        CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
        rootParameters[Slot::RayOriginPosition].InitAsDescriptorTable(1, &ranges[Slot::RayOriginPosition]);
        rootParameters[Slot::RayOriginSurfaceNormalDepth].InitAsDescriptorTable(1, &ranges[Slot::RayOriginSurfaceNormalDepth]);
        rootParameters[Slot::AOAmbientCoefficient].InitAsDescriptorTable(1, &ranges[Slot::AOAmbientCoefficient]);
        rootParameters[Slot::AORayHitDistance].InitAsDescriptorTable(1, &ranges[Slot::AORayHitDistance]);
        rootParameters[Slot::AORayDirectionOriginDepthHitSRV].InitAsDescriptorTable(1, &ranges[Slot::AORayDirectionOriginDepthHitSRV]);
        rootParameters[Slot::AOSortedToSourceRayIndex].InitAsDescriptorTable(1, &ranges[Slot::AOSortedToSourceRayIndex]);
        rootParameters[Slot::AOSurfaceAlbedo].InitAsDescriptorTable(1, &ranges[Slot::AOSurfaceAlbedo]);

        rootParameters[Slot::AccelerationStructure].InitAsShaderResourceView(0);
        rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);
        rootParameters[Slot::SampleBuffers].InitAsShaderResourceView(4);

        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRootSignature(device, globalRootSignatureDesc, &m_raytracingGlobalRootSignature, L"RTAO Global root signature");
    }
}


// DXIL library
// This contains the shaders and their entrypoints for the state object.
// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
void RTAO::CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRTAO, ARRAYSIZE(g_pRTAO));
    lib->SetDXILLibrary(&libdxil);
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
}

// Hit groups
// A hit group specifies closest hit, any hit and intersection shaders 
// to be executed when a ray intersects the geometry.
void RTAO::CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Triangle geometry hit groups
    {
        auto hitGroup = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();

        hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
        hitGroup->SetHitGroupExport(c_hitGroupName);
        hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local root signatures and other state.
void RTAO::CreateRaytracingPipelineStateObject()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Ambient Occlusion state object.
    {
        CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

        // DXIL library
        CreateDxilLibrarySubobject(&raytracingPipeline);

        // Hit groups
        CreateHitGroupSubobjects(&raytracingPipeline);

        // Shader config
        // Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
        auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
        UINT payloadSize = static_cast<UINT>(sizeof(ShadowRayPayload));	

        UINT attributeSize = sizeof(XMFLOAT2);  // float2 barycentrics
        shaderConfig->Config(payloadSize, attributeSize);

        // Global root signature
        // This is a root signature that is shared across all RTAO shaders invoked during a DispatchRays() call.
        auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

        // Pipeline config
        // Defines the maximum TraceRay() recursion depth.
        auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
        // PERFOMANCE TIP: Set max recursion depth as low as needed
        // as drivers may apply optimization strategies for low recursion depths.
        UINT maxRecursionDepth = 1;
        pipelineConfig->Config(maxRecursionDepth);

        PrintStateObjectDesc(raytracingPipeline);

        // Create the state object.
        ThrowIfFailed(device->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
    }
}

void RTAO::CreateTextureResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    CreateRenderTargetResource(device,  ResourceFormat(ResourceType::AOCoefficient), m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_AOResources[AOResource::AmbientCoefficient], initialResourceState, L"Render/AO Coefficient");
    CreateRenderTargetResource(device, ResourceFormat(ResourceType::RayHitDistance), m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_AOResources[AOResource::RayHitDistance], initialResourceState, L"Render/AO Hit Distance");

    CreateRenderTargetResource(device, DXGI_FORMAT_R8G8_UINT, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_sortedToSourceRayIndexOffset, initialResourceState, L"Sorted To Source Ray Index");
    CreateRenderTargetResource(device, COMPACT_NORMAL_DEPTH_DXGI_FORMAT, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_AORayDirectionOriginDepth, initialResourceState, L"AO Rays Direction, Origin Depth and Hit");
}


// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
// For AO, the shaders are simple with only one shader type per shader table.
void RTAO::BuildShaderTables(Scene& scene)
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderIDs[RTAORayGenShaderType::Count];
    void* missShaderID;
    void* hitGroupShaderID;

    // A shader name look-up table for shader table debug print out.
    unordered_map<void*, wstring> shaderIdToStringMap;

    auto GetShaderIDs = [&](auto* stateObjectProperties)
    {
        for (UINT i = 0; i < RTAORayGenShaderType::Count; i++)
        {
            rayGenShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_rayGenShaderNames[i]);
            shaderIdToStringMap[rayGenShaderIDs[i]] = c_rayGenShaderNames[i];
        }

        missShaderID = stateObjectProperties->GetShaderIdentifier(c_missShaderName);
        shaderIdToStringMap[missShaderID] = c_missShaderName;

        hitGroupShaderID = stateObjectProperties->GetShaderIdentifier(c_hitGroupName);
        shaderIdToStringMap[hitGroupShaderID] = c_hitGroupName;
    };

    // Get shader identifiers.
    UINT shaderIDSize;
    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
    GetShaderIDs(stateObjectProperties.Get());
    shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    // RayGen shader tables.
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize; // No root arguments

        for (UINT i = 0; i < RTAORayGenShaderType::Count; i++)
        {
            ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RTAO RayGenShaderTable");
            rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIDs[i], shaderIDSize, nullptr, 0));
            rayGenShaderTable.DebugPrint(shaderIdToStringMap);
            m_rayGenShaderTables[i] = rayGenShaderTable.GetResource();
        }
    }

    // Miss shader table.
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize; // No root arguments

        ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"RTAO MissShaderTable");
        missShaderTable.push_back(ShaderRecord(missShaderID, shaderIDSize, nullptr, 0));

        missShaderTable.DebugPrint(shaderIdToStringMap);
        m_missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
        m_missShaderTable = missShaderTable.GetResource();
    }


    // maxInstanceContributionToHitGroupIndex - since BLAS instances in this sample specify non-zero InstanceContributionToHitGroupIndex for Pathtracer raytracing pass, 
    //  the AO raytracing needs to add as many shader records to all hit group shader tables so that DXR shader addressing lands on a valid shader record for all BLASes.
    UINT maxInstanceContributionToHitGroupIndex = scene.AccelerationStructure()->GetMaxInstanceContributionToHitGroupIndex();

    // Hit group shader table.
    {
        // Duplicate the shader records because the TLAS has BLAS instances with non-zero InstanceContributionToHitGroupIndex.
        // For the last offset we need only one more shader record, because RTAO TraceRay always indexes the first shader record
        // of each BLAS instance shader record range due to RTAOTraceRayParameters::HitGroup::GeometryStride of 0.
        UINT numShaderRecords = maxInstanceContributionToHitGroupIndex + 1;
        UINT shaderRecordSize = shaderIDSize; // No root arguments

        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"RTAO HitGroupShaderTable");

        for (UINT i = 0; i < numShaderRecords; i++)
        {
            hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, nullptr, 0));
        }
        hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
        m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

void RTAO::CreateSamplesRNG()
{
    UINT pixelsInSampleSet1D = RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels;
    UINT samplesPerSet = RTAO_Args::Spp * pixelsInSampleSet1D * pixelsInSampleSet1D;
    m_randomSampler.Reset(samplesPerSet, c_NumSampleSets, Samplers::HemisphereDistribution::Cosine);

    UINT numSamples = m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets();
    for (UINT i = 0; i < numSamples; i++)
    {
        XMFLOAT3 p = m_randomSampler.GetHemisphereSample3D();
        // Convert [-1,1] to [0,1].
        m_samplesGPUBuffer[i].value = XMFLOAT2(p.x * 0.5f + 0.5f, p.y * 0.5f + 0.5f);
        m_hemisphereSamplesGPUBuffer[i].value = p;
    }
}

void RTAO::GetRayGenParameters(bool* isCheckerboardSamplingEnabled, bool* checkerboardLoadEvenPixels)
{
    *isCheckerboardSamplingEnabled = RTAO_Args::Spp_doCheckerboard;
    *checkerboardLoadEvenPixels = m_checkerboardGenerateRaysForEvenPixels;
}

void RTAO::DispatchRays(ID3D12Resource* rayGenShaderTable, UINT width, UINT height)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    ScopedTimer _prof(L"DispatchRays", commandList);

    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
    dispatchDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
    dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
    dispatchDesc.MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
    dispatchDesc.MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
    dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
    dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetDesc().Width;
    dispatchDesc.Width = width != 0 ? width : m_raytracingWidth;
    dispatchDesc.Height = height != 0 ? height : m_raytracingHeight;
    dispatchDesc.Depth = 1;
    commandList->SetPipelineState1(m_dxrStateObject.Get());

    resourceStateTracker->FlushResourceBarriers();
    commandList->DispatchRays(&dispatchDesc);
}

void RTAO::UpdateConstantBuffer(UINT frameIndex)
{
    uniform_int_distribution<UINT> seedDistribution(0, UINT_MAX);

    m_CB->seed = RTAO_Args::RayGen_RandomFrameSeed ? seedDistribution(m_generatorURNG) : 1879;
    m_CB->numSamplesPerSet = m_randomSampler.NumSamples();
    m_CB->numSampleSets = m_randomSampler.NumSampleSets();
    m_CB->numPixelsPerDimPerSet = RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels;

    m_CB->useSortedRays = RTAO_Args::RaySorting_Enabled;

    bool doCheckerboardRayGeneration = RTAO_Args::Spp_doCheckerboard;
    m_checkerboardGenerateRaysForEvenPixels = !m_checkerboardGenerateRaysForEvenPixels;
    m_CB->doCheckerboardSampling = doCheckerboardRayGeneration;
    m_CB->areEvenPixelsActive = m_checkerboardGenerateRaysForEvenPixels;
    UINT pixelStepX = doCheckerboardRayGeneration ? 2 : 1;
    m_CB->raytracingDim = XMUINT2(CeilDivide(m_raytracingWidth, pixelStepX), m_raytracingHeight);
    m_CB->spp = RTAO_Args::Spp;

    m_CB->approximateInterreflections = RTAO_Args::ApproximateInterreflections_Enabled;
    m_CB->diffuseReflectanceScale = RTAO_Args::ApproximateInterreflections_DiffuseReflectanceScale;
    m_CB->minimumAmbientIllumination = RTAO_Args::MinimumAmbientIllumination;
    m_CB->applyExponentialFalloff = RTAO_Args::ExponentialFalloff_Enabled;
    m_CB->exponentialFalloffDecayConstant = RTAO_Args::ExponentialFalloff_DecayConstant;

    // Calculate a theoretical max ray distance to be used in occlusion factor computation.
    // Occlusion factor of a ray hit is computed based of its ray hit time, falloff exponent and a max ray hit time.
    // By specifying a min occlusion factor of a ray, we can skip tracing rays that would have an occlusion 
    // factor less than the cutoff to save a bit of performance (generally 1-10% perf win without visible AO result impact).
    // Therefore the sample discerns between true maxRayHitTime, used in TraceRay, 
    // and a theoretical one used in calculating the occlusion factor on a hit.
    {
        float occclusionCutoff = RTAO_Args::ExponentialFalloff_MinOcclusionCutoff;
        float lambda = RTAO_Args::ExponentialFalloff_DecayConstant;

        // Invert occlusionFactor = exp(-lambda * t * t), where t is tHit/tMax of a ray.
        float t = sqrt(logf(occclusionCutoff) / -lambda);

        m_CB->maxAORayHitTime = t * RTAO_Args::MaxRayHitTime;
        m_CB->maxTheoreticalAORayHitTime = RTAO_Args::MaxRayHitTime;
    }

    m_CB.CopyStagingToGpu(frameIndex);
}

void RTAO::Run(
    D3D12_GPU_VIRTUAL_ADDRESS accelerationStructure,
    D3D12_GPU_DESCRIPTOR_HANDLE rayOriginSurfaceHitPositionResource,
    D3D12_GPU_DESCRIPTOR_HANDLE rayOriginSurfaceNormalDepthResource,
    D3D12_GPU_DESCRIPTOR_HANDLE rayOriginSurfaceAlbedoResource)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    ScopedTimer _prof(L"CalculateAmbientOcclusion_Root", commandList);

    if (m_isRecreateAOSamplesRequested)
    {
        CreateSamplesRNG();
        m_isRecreateAOSamplesRequested = false;
    }

    // Copy dynamic buffers to GPU.
    {
        UpdateConstantBuffer(frameIndex);
        m_hemisphereSamplesGPUBuffer.CopyStagingToGpu(frameIndex);
    }

    resourceStateTracker->TransitionResource(&m_AOResources[AOResource::AmbientCoefficient], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    resourceStateTracker->TransitionResource(&m_AOResources[AOResource::RayHitDistance], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    
    if (RTAO_Args::RaySorting_Enabled)
    {
        resourceStateTracker->TransitionResource(&m_sortedToSourceRayIndexOffset, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&Sample::g_debugOutput[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_AORayDirectionOriginDepth, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        bool doCheckerboardRayGeneration = RTAO_Args::Spp_doCheckerboard;

        UINT activeRaytracingWidth =
            doCheckerboardRayGeneration
            ? CeilDivide(m_raytracingWidth, 2)
            : m_raytracingWidth;

        resourceStateTracker->FlushResourceBarriers();
        m_rayGen.Run(
            commandList,
            activeRaytracingWidth,
            m_raytracingHeight,
            m_CB->seed,
            m_randomSampler.NumSamples(),
            m_randomSampler.NumSampleSets(),
            RTAO_Args::Spp_AOSampleSetDistributedAcrossPixels,
            doCheckerboardRayGeneration,
            m_checkerboardGenerateRaysForEvenPixels,
            m_cbvSrvUavHeap->GetHeap(),
            rayOriginSurfaceNormalDepthResource,
            rayOriginSurfaceHitPositionResource,
            m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex),
            m_AORayDirectionOriginDepth.gpuDescriptorWriteAccess);

        resourceStateTracker->TransitionResource(&m_AORayDirectionOriginDepth, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->InsertUAVBarrier(&m_AORayDirectionOriginDepth);

        float rayBinDepthSize = RTAO_Args::RaySorting_DepthBinSizeMultiplier * RTAO_Args::MaxRayHitTime;

        resourceStateTracker->FlushResourceBarriers();
        m_raySorter.Run(
            commandList,
            rayBinDepthSize,
            activeRaytracingWidth,
            m_raytracingHeight,
            true,
            m_cbvSrvUavHeap->GetHeap(),
            m_AORayDirectionOriginDepth.gpuDescriptorReadAccess,
            m_sortedToSourceRayIndexOffset.gpuDescriptorWriteAccess);

        resourceStateTracker->TransitionResource(&m_sortedToSourceRayIndexOffset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&Sample::g_debugOutput[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->InsertUAVBarrier(&m_sortedToSourceRayIndexOffset);
    }

    {
        ScopedTimer _prof(L"AO DispatchRays", commandList);

        commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

        // Bind inputs.
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, accelerationStructure);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::RayOriginPosition, rayOriginSurfaceHitPositionResource);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::RayOriginSurfaceNormalDepth, rayOriginSurfaceNormalDepthResource);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::SampleBuffers, m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex));
        commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::ConstantBuffer, m_CB.GpuVirtualAddress(frameIndex));
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AOSurfaceAlbedo, rayOriginSurfaceAlbedoResource);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AORayDirectionOriginDepthHitSRV, m_AORayDirectionOriginDepth.gpuDescriptorReadAccess);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AOSortedToSourceRayIndex, m_sortedToSourceRayIndexOffset.gpuDescriptorReadAccess);

        // Bind output RT.
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AOAmbientCoefficient, m_AOResources[AOResource::AmbientCoefficient].gpuDescriptorWriteAccess);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AORayHitDistance, m_AOResources[AOResource::RayHitDistance].gpuDescriptorWriteAccess);


        if (RTAO_Args::RaySorting_Enabled)
        {
            bool doCheckerboardRayGeneration = RTAO_Args::Spp_doCheckerboard;
            UINT activeRaytracingWidth =
                doCheckerboardRayGeneration
                ? CeilDivide(m_raytracingWidth, 2)
                : m_raytracingWidth;

            UINT NumRays = activeRaytracingWidth * m_raytracingHeight;
            DispatchRays(m_rayGenShaderTables[RTAORayGenShaderType::AOSortedRays].Get(), NumRays, 1);
        }
        else
        {
            DispatchRays(m_rayGenShaderTables[RTAORayGenShaderType::AOFullRes].Get());
        }
    }

    resourceStateTracker->TransitionResource(&m_AOResources[AOResource::AmbientCoefficient], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    resourceStateTracker->TransitionResource(&m_AOResources[AOResource::RayHitDistance], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    resourceStateTracker->InsertUAVBarrier(&m_AOResources[AOResource::AmbientCoefficient]);
    resourceStateTracker->InsertUAVBarrier(&m_AOResources[AOResource::RayHitDistance]);
}

void RTAO::CreateResolutionDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    CreateTextureResources();
}

void RTAO::SetResolution(UINT width, UINT height)
{
    m_raytracingWidth = width;
    m_raytracingHeight = height;

    CreateResolutionDependentResources();
}
