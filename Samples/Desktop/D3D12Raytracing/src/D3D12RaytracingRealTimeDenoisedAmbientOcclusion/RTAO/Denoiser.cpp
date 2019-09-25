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
#include "Denoiser.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "EngineProfiling.h"
#include "GpuTimeManager.h"
#include "Denoiser.h"
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "Composition.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;


namespace Denoiser_Args
{
    // Temporal Cache.
    BoolVar TemporalSupersampling_Enabled(L"Render/AO/RTAO/Temporal Cache/Enabled", true);
    BoolVar TemporalSupersampling_CacheRawAOValue(L"Render/AO/RTAO/Temporal Cache/Cache Raw AO Value", true);
    NumVar TemporalSupersampling_MinSmoothingFactor(L"Render/AO/RTAO/Temporal Cache/Min Smoothing Factor", 0.03f, 0, 1.f, 0.01f);
    NumVar TemporalSupersampling_DepthTolerance(L"Render/AO/RTAO/Temporal Cache/Depth tolerance [%%]", 0.05f, 0, 1.f, 0.001f);
    BoolVar TemporalSupersampling_PerspectiveCorrectDepthInterpolation(L"Render/AO/RTAO/Temporal Cache/Depth testing/Use perspective correct depth interpolation", false);
    BoolVar TemporalSupersampling_UseDepthWeights(L"Render/AO/RTAO/Temporal Cache/Use depth weights", true);
    BoolVar TemporalSupersampling_UseNormalWeights(L"Render/AO/RTAO/Temporal Cache/Use normal weights", true);
    BoolVar TemporalSupersampling_ForceUseMinSmoothingFactor(L"Render/AO/RTAO/Temporal Cache/Force min smoothing factor", false);
    BoolVar TemporalSupersampling_CacheDenoisedOutput(L"Render/AO/RTAO/Temporal Cache/Cache denoised output", true);
    BoolVar TemporalSupersampling_ClampCachedValues_UseClamping(L"Render/AO/RTAO/Temporal Cache/Clamping/Enabled", true);

    // Std dev gamma - scales std dev on clamping. Larger values give more clamp tolerance, lower values give less tolerance (i.e. clamp quicker, better for motion).
    NumVar TemporalSupersampling_ClampCachedValues_StdDevGamma(L"Render/AO/RTAO/Temporal Cache/Clamping/Std.dev gamma", 0.6f, 0.1f, 10.f, 0.1f);

    // Minimum std.dev used in clamping
    // - higher values helps prevent clamping. 
    //   Especially on checkerboard 1spp sampling values of ~0.10 prevent random clamping.
    // - higher values limit clamping due to true change and increases ghosting.
    NumVar TemporalSupersampling_ClampCachedValues_MinStdDevTolerance(L"Render/AO/RTAO/Temporal Cache/Clamping/Minimum std.dev", 0.05f, 0.f, 1.f, 0.01f);
    NumVar TemporalSupersampling_ClampDifferenceToTsppScale(L"Render/AO/RTAO/Temporal Cache/Clamping/Tspp scale", 4.f, 0.f, 10.f, 0.05f);
    NumVar TemporalSupersampling_ClampCachedValues_AbsoluteDepthTolerance(L"Render/AO/RTAO/Temporal Cache/Depth threshold/Absolute depth tolerance", 1.0f, 0.0f, 100.f, 1.f);
    NumVar TemporalSupersampling_ClampCachedValues_DepthBasedDepthTolerance(L"Render/AO/RTAO/Temporal Cache/Depth threshold/Depth based depth tolerance", 1.0f, 0.0f, 100.f, 1.f);
    NumVar TemporalSupersampling_ClampCachedValues_DepthSigma(L"Render/AO/RTAO/Temporal Cache/Depth threshold/Depth sigma", 1.0f, 0.0f, 10.f, 0.01f);

    IntVar VarianceBilateralFilterKernelWidth(L"Render/RTAO/GpuKernels/CalculateVariance/Kernel width", 9, 3, 9, 2);

    // ToDoF test perf impact / visual quality gain at the end. Document.
    BoolVar PerspectiveCorrectDepthInterpolation(L"Render/AO/RTAO/Denoising/Pespective Correct Depth Interpolation", true);

    BoolVar UseAdaptiveKernelSize(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Enabled", true);
    BoolVar KernelRadius_RotateKernel_Enabled(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Rotate kernel radius/Enabled", true);
    IntVar KernelRadius_RotateKernel_NumCycles(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Rotate kernel radius/Num cycles", 3, 0, 10, 1);
    IntVar FilterMinKernelWidth(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Min kernel width", 3, 3, 101);
    NumVar FilterMaxKernelWidthPercentage(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Max kernel width [%% of screen width]", 1.5f, 0, 100, 0.1f);
    NumVar AdaptiveKernelSize_RayHitDistanceScaleFactor(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Hit distance scale factor", 0.02f, 0.001f, 0.1f, 0.001f);
    NumVar AdaptiveKernelSize_RayHitDistanceScaleExponent(L"Render/AO/RTAO/Denoising/AdaptiveKernelSize/Hit distance scale exponent", 2.0f, 1.0f, 5.0f, 0.1f);
    BoolVar Variance_UseDepthWeights(L"Render/AO/RTAO/Denoising/Variance/Use normal weights", true);
    BoolVar Variance_UseNormalWeights(L"Render/AO/RTAO/Denoising/Variance/Use normal weights", true);
    IntVar MinTsppToUseTemporalVariance(L"Render/AO/RTAO/Denoising/Min Temporal Variance Tspp", 4, 1, 40);
    NumVar MinVarianceToDenoise(L"Render/AO/RTAO/Denoising/Min Variance to denoise", 0.0f, 0.0f, 1.f, 0.01f);
    BoolVar UseSmoothedVariance(L"Render/AO/RTAO/Denoising/Use smoothed variance", false);

    BoolVar Checkerboard_LowerWeightForStaleSamples(L"Render/AO/RTAO/Denoising/Checkerboard support/Scale down stale samples weight", false);


    BoolVar LowTspp(L"Render/AO/RTAO/Denoising/Low tspp filter/enabled", true);
    IntVar LowTsppMaxTspp(L"Render/AO/RTAO/Denoising/Low tspp filter/Max tspp", 12, 0, 33);
    IntVar LowTspBlurPasses(L"Render/AO/RTAO/Denoising/Low tspp filter/Num blur passes", 3, 0, Denoiser::c_MaxNumDisocllusionBlurPasses);
    NumVar LowTsppDecayConstant(L"Render/AO/RTAO/Denoising/Low tspp filter/Decay constant", 1.0f, 0.1f, 32.f, 0.1f);
    BoolVar LowTsppFillMissingValues(L"Render/AO/RTAO/Denoising/Low tspp filter/Post-Temporal fill in missing values", true);
  
    const WCHAR* Modes[RTAOGpuKernels::AtrousWaveletTransformCrossBilateralFilter::FilterType::Count] = { L"EdgeStoppingGaussian3x3", L"EdgeStoppingGaussian5x5" };
    EnumVar Mode(L"Render/AO/RTAO/Denoising/Mode", RTAOGpuKernels::AtrousWaveletTransformCrossBilateralFilter::FilterType::EdgeStoppingGaussian3x3, RTAOGpuKernels::AtrousWaveletTransformCrossBilateralFilter::FilterType::Count, Modes);
    NumVar AODenoiseValueSigma(L"Render/AO/RTAO/Denoising/Value Sigma", 1.0f, 0.0f, 30.0f, 0.1f);

    NumVar AODenoiseDepthSigma(L"Render/AO/RTAO/Denoising/Depth Sigma", 1.0f, 0.0f, 10.0f, 0.02f); 
    NumVar AODenoiseDepthWeightCutoff(L"Render/AO/RTAO/Denoising/Depth Weight Cutoff", 0.2f, 0.0f, 2.0f, 0.01f);
    NumVar AODenoiseNormalSigma(L"Render/AO/RTAO/Denoising/Normal Sigma", 64, 0, 256, 4);
}


DXGI_FORMAT Denoiser::ResourceFormat(ResourceType resourceType)
{
    switch (resourceType)
    {
    case ResourceType::Variance: return DXGI_FORMAT_R16_FLOAT;
    case ResourceType::LocalMeanVariance: return DXGI_FORMAT_R16G16_FLOAT;
    }

    return DXGI_FORMAT_UNKNOWN;
}

void Denoiser::Setup(shared_ptr<DeviceResources> deviceResources, shared_ptr<DX::DescriptorHeap> descriptorHeap)
{
    m_deviceResources = deviceResources;
    m_cbvSrvUavHeap = descriptorHeap;

    CreateDeviceDependentResources();
}

// Create resources that depend on the device.
void Denoiser::CreateDeviceDependentResources()
{
    CreateAuxilaryDeviceResources();
}

void Denoiser::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    m_fillInCheckerboardKernel.Initialize(device, Sample::FrameCount);
    m_gaussianSmoothingKernel.Initialize(device, Sample::FrameCount);
    m_temporalCacheReverseReprojectKernel.Initialize(device, Sample::FrameCount);
    m_temporalCacheBlendWithCurrentFrameKernel.Initialize(device, Sample::FrameCount);
    m_atrousWaveletTransformFilter.Initialize(device, Sample::FrameCount);
    m_calculateMeanVarianceKernel.Initialize(device, Sample::FrameCount); 
    m_bilateralFilterKernel.Initialize(device, Sample::FrameCount, c_MaxNumDisocllusionBlurPasses);
}


// Run() can be optionally called in two explicit stages. This can
// be beneficial to retrieve temporally reprojected values 
// and configure current frame AO raytracing off of that 
// (such as vary spp based on average ray hit distance or tspp).
// Otherwise, all denoiser steps can be run via a single execute call.
void Denoiser::Run(Pathtracer& pathtracer, RTAO& rtao, DenoiseStage stage)
{
    auto commandList = m_deviceResources->GetCommandList();
    ScopedTimer _prof(L"Denoise", commandList);

    if (stage & Denoise_Stage1_TemporalSupersamplingReverseReproject)
    {
        TemporalSupersamplingReverseReproject(pathtracer);
    }

    if (stage & Denoise_Stage2_Denoise)
    {
        TemporalSupersamplingBlendWithCurrentFrame(rtao);
        ApplyAtrousWaveletTransformFilter(pathtracer, rtao);

        if (Denoiser_Args::LowTspp)
        {
            BlurDisocclusions(pathtracer);
        }
    }
}

void Denoiser::CreateResolutionDependentResources()
{
    CreateTextureResources();
}


void Denoiser::SetResolution(UINT width, UINT height)
{
    m_denoisingWidth = width;
    m_denoisingHeight = height;

    CreateResolutionDependentResources();
}
    

void Denoiser::CreateTextureResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    // Temporal cache resources.
    {
        for (UINT i = 0; i < 2; i++)
        {
            // Preallocate subsequent descriptor indices for both SRV and UAV groups.
            m_temporalCache[i][0].uavDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(TemporalSupersampling::Count);
            m_temporalCache[i][0].srvDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(TemporalSupersampling::Count);
            for (UINT j = 0; j < TemporalSupersampling::Count; j++)
            {
                m_temporalCache[i][j].uavDescriptorHeapIndex = m_temporalCache[i][0].uavDescriptorHeapIndex + j;
                m_temporalCache[i][j].srvDescriptorHeapIndex = m_temporalCache[i][0].srvDescriptorHeapIndex + j;
            }

            CreateRenderTargetResource(device, DXGI_FORMAT_R8_UINT, m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_temporalCache[i][TemporalSupersampling::Tspp], initialResourceState, L"Temporal Cache: Tspp");
            CreateRenderTargetResource(device, RTAO::ResourceFormat(RTAO::ResourceType::AOCoefficient), m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_temporalCache[i][TemporalSupersampling::CoefficientSquaredMean], initialResourceState, L"Temporal Cache: Coefficient Squared Mean");
            CreateRenderTargetResource(device, RTAO::ResourceFormat(RTAO::ResourceType::RayHitDistance), m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_temporalCache[i][TemporalSupersampling::RayHitDistance], initialResourceState, L"Temporal Cache: Ray Hit Distance");
            CreateRenderTargetResource(device, RTAO::ResourceFormat(RTAO::ResourceType::AOCoefficient), m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_temporalAOCoefficient[i], initialResourceState, L"Render/AO Temporally Supersampled Coefficient");
        }
    }

    CreateRenderTargetResource(device, DXGI_FORMAT_R16G16B16A16_UINT, m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_cachedTsppValueSquaredValueRayHitDistance, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Temporal Supersampling intermediate reprojected Tspp, Value, Squared Mean Value, Ray Hit Distance");

    // Variance resources
    {
        for (UINT i = 0; i < AOVarianceResource::Count; i++)
        {
            CreateRenderTargetResource(device, ResourceFormat(ResourceType::Variance), m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_varianceResources[i], initialResourceState, L"Post Temporal Reprojection Variance");
            CreateRenderTargetResource(device, ResourceFormat(ResourceType::LocalMeanVariance), m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_localMeanVarianceResources[i], initialResourceState, L"Local Mean Variance");
        }
    }

    CreateRenderTargetResource(device, DXGI_FORMAT_R8_UNORM, m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_disocclusionBlurStrength, initialResourceState, L"Disocclusion Denoising Blur Strength");
    CreateRenderTargetResource(device, COMPACT_NORMAL_DEPTH_DXGI_FORMAT, m_denoisingWidth, m_denoisingHeight, m_cbvSrvUavHeap.get(), &m_prevFrameGBufferNormalDepth, initialResourceState, L"Previous Frame GBuffer Normal Depth");
}


// Retrieves values from previous frame via reverse reprojection.
void Denoiser::TemporalSupersamplingReverseReproject(Pathtracer& pathtracer)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();

    ScopedTimer _prof(L"Temporal Supersampling p1 (Reverse Reprojection)", commandList);
        
    // Ping-pong input output indices across frames.
    UINT temporalCachePreviousFrameResourceIndex = m_temporalCacheCurrentFrameResourceIndex;
    m_temporalCacheCurrentFrameResourceIndex = (m_temporalCacheCurrentFrameResourceIndex + 1) % 2;

    UINT temporalCachePreviousFrameTemporalAOCoeficientResourceIndex = m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex;
    m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex = (m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex + 1) % 2;

    // Transition output resource to UAV state.        
    {
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_cachedTsppValueSquaredValueRayHitDistance, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    GpuResource (&GBufferResources)[GBufferResource::Count] = pathtracer.GBufferResources(RTAO_Args::QuarterResAO);

    UINT maxTspp = static_cast<UINT>(1 / Denoiser_Args::TemporalSupersampling_MinSmoothingFactor);
    resourceStateTracker->FlushResourceBarriers();

    m_temporalCacheReverseReprojectKernel.Run(
        commandList,
        m_denoisingWidth,
        m_denoisingHeight,
        m_cbvSrvUavHeap->GetHeap(),
        GBufferResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorReadAccess,
        GBufferResources[GBufferResource::PartialDepthDerivatives].gpuDescriptorReadAccess,
        GBufferResources[GBufferResource::ReprojectedNormalDepth].gpuDescriptorReadAccess,
        GBufferResources[GBufferResource::MotionVector].gpuDescriptorReadAccess,
        m_temporalAOCoefficient[temporalCachePreviousFrameTemporalAOCoeficientResourceIndex].gpuDescriptorReadAccess,
        m_prevFrameGBufferNormalDepth.gpuDescriptorReadAccess,
        m_temporalCache[temporalCachePreviousFrameResourceIndex][TemporalSupersampling::Tspp].gpuDescriptorReadAccess,
        m_temporalCache[temporalCachePreviousFrameResourceIndex][TemporalSupersampling::CoefficientSquaredMean].gpuDescriptorReadAccess,
        m_temporalCache[temporalCachePreviousFrameResourceIndex][TemporalSupersampling::RayHitDistance].gpuDescriptorReadAccess,
        m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp].gpuDescriptorWriteAccess,
        m_cachedTsppValueSquaredValueRayHitDistance.gpuDescriptorWriteAccess,
        RTAO_Args::QuarterResAO,
        Denoiser_Args::TemporalSupersampling_ClampCachedValues_DepthSigma);

    // Transition output resources to SRV state.
    // All the others are used as input/output UAVs in 2nd stage of Temporal Supersampling.
    {
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_cachedTsppValueSquaredValueRayHitDistance, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->InsertUAVBarrier(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp]);
    }

    // Cache the normal depth resource.
    {
        // TODO: replace copy with using a source resource directly.
        ScopedTimer _prof(L"Cache normal depth resource", commandList);
        CopyTextureRegion(
            commandList,
            GBufferResources[GBufferResource::SurfaceNormalDepth].GetResource(),
            m_prevFrameGBufferNormalDepth.GetResource(),
            &CD3DX12_BOX(0, 0, m_denoisingWidth, m_denoisingHeight),
            GBufferResources[GBufferResource::SurfaceNormalDepth].m_UsageState,
            m_prevFrameGBufferNormalDepth.m_UsageState);
    }
}

// Blends reprojected values with current frame values.
// Inactive pixels are filtered from active neighbors on checkerboard sampling
// before the blend operation.
void Denoiser::TemporalSupersamplingBlendWithCurrentFrame(RTAO& rtao)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();

    ScopedTimer _prof(L"TemporalSupersamplingBlendWithCurrentFrame", commandList);

    GpuResource* AOResources = rtao.AOResources();

    // Transition all output resources to UAV state.
    {
        resourceStateTracker->TransitionResource(&m_localMeanVarianceResources[AOVarianceResource::Raw], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->InsertUAVBarrier(&AOResources[AOResource::AmbientCoefficient]);
    }

    bool isCheckerboardSamplingEnabled;
    bool checkerboardLoadEvenPixels;
    rtao.GetRayGenParameters(&isCheckerboardSamplingEnabled, &checkerboardLoadEvenPixels);

    // Calculate local mean and variance for clamping during the blend operation.
    {
        ScopedTimer _prof(L"Calculate Mean and Variance", commandList);
        resourceStateTracker->FlushResourceBarriers();
        m_calculateMeanVarianceKernel.Run(
            commandList,
            m_cbvSrvUavHeap->GetHeap(),
            m_denoisingWidth,
            m_denoisingHeight,
            AOResources[AOResource::AmbientCoefficient].gpuDescriptorReadAccess,
            m_localMeanVarianceResources[AOVarianceResource::Raw].gpuDescriptorWriteAccess,
            Denoiser_Args::VarianceBilateralFilterKernelWidth,
            isCheckerboardSamplingEnabled,
            checkerboardLoadEvenPixels);

        // Interpolate the variance for the inactive cells from the valid checherkboard cells.
        if (isCheckerboardSamplingEnabled)
        {
            bool fillEvenPixels = !checkerboardLoadEvenPixels;
            resourceStateTracker->FlushResourceBarriers();
            m_fillInCheckerboardKernel.Run(
                commandList,
                m_cbvSrvUavHeap->GetHeap(),
                m_denoisingWidth,
                m_denoisingHeight,
                m_localMeanVarianceResources[AOVarianceResource::Raw].gpuDescriptorWriteAccess,
                fillEvenPixels);
        }

        resourceStateTracker->TransitionResource(&m_localMeanVarianceResources[AOVarianceResource::Raw], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->InsertUAVBarrier(&m_localMeanVarianceResources[AOVarianceResource::Raw]);
    }

    {
        resourceStateTracker->InsertUAVBarrier(&m_localMeanVarianceResources[AOVarianceResource::Smoothed]);
    }

    GpuResource* TemporalOutCoefficient = &m_temporalAOCoefficient[m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex];

    // Transition output resource to UAV state.      
    {
        resourceStateTracker->TransitionResource(TemporalOutCoefficient, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::CoefficientSquaredMean], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::RayHitDistance], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_varianceResources[AOVarianceResource::Raw], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_disocclusionBlurStrength, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->InsertUAVBarrier(&m_cachedTsppValueSquaredValueRayHitDistance);
    }

    resourceStateTracker->FlushResourceBarriers();
    m_temporalCacheBlendWithCurrentFrameKernel.Run(
        commandList,
        m_denoisingWidth,
        m_denoisingHeight,
        m_cbvSrvUavHeap->GetHeap(),
        AOResources[AOResource::AmbientCoefficient].gpuDescriptorReadAccess,
        m_localMeanVarianceResources[AOVarianceResource::Raw].gpuDescriptorReadAccess,
        AOResources[AOResource::RayHitDistance].gpuDescriptorReadAccess,
        TemporalOutCoefficient->gpuDescriptorWriteAccess,
        m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp].gpuDescriptorWriteAccess,
        m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::CoefficientSquaredMean].gpuDescriptorWriteAccess,
        m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::RayHitDistance].gpuDescriptorWriteAccess,
        m_cachedTsppValueSquaredValueRayHitDistance.gpuDescriptorReadAccess,
        m_varianceResources[AOVarianceResource::Raw].gpuDescriptorWriteAccess,
        m_disocclusionBlurStrength.gpuDescriptorWriteAccess,
        Denoiser_Args::TemporalSupersampling_MinSmoothingFactor,
        Denoiser_Args::TemporalSupersampling_ForceUseMinSmoothingFactor,
        Denoiser_Args::TemporalSupersampling_ClampCachedValues_UseClamping,
        Denoiser_Args::TemporalSupersampling_ClampCachedValues_StdDevGamma,
        Denoiser_Args::TemporalSupersampling_ClampCachedValues_MinStdDevTolerance,
        Denoiser_Args::MinTsppToUseTemporalVariance,
        Denoiser_Args::LowTsppMaxTspp,
        Denoiser_Args::LowTsppDecayConstant,
        isCheckerboardSamplingEnabled,
        checkerboardLoadEvenPixels,
        Denoiser_Args::TemporalSupersampling_ClampDifferenceToTsppScale);

    // Transition output resource to SRV state.        
    {
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::CoefficientSquaredMean], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::RayHitDistance], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_varianceResources[AOVarianceResource::Raw], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_disocclusionBlurStrength, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    // Smoothen the local variance which is prone to error due to undersampled input.
    if (Denoiser_Args::UseSmoothedVariance)
    {
        ScopedTimer _prof(L"Mean Variance Smoothing", commandList);

        resourceStateTracker->TransitionResource(&m_varianceResources[AOVarianceResource::Smoothed], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->InsertUAVBarrier(&m_varianceResources[AOVarianceResource::Raw]);

        resourceStateTracker->FlushResourceBarriers();
        m_gaussianSmoothingKernel.Run(
            commandList,
            m_denoisingWidth,
            m_denoisingHeight,
            RTAOGpuKernels::GaussianFilter::Filter3x3,
            m_cbvSrvUavHeap->GetHeap(),
            m_varianceResources[AOVarianceResource::Raw].gpuDescriptorReadAccess,
            m_varianceResources[AOVarianceResource::Smoothed].gpuDescriptorWriteAccess);

        resourceStateTracker->TransitionResource(&m_varianceResources[AOVarianceResource::Smoothed], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    resourceStateTracker->TransitionResource(TemporalOutCoefficient, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void Denoiser::BlurDisocclusions(Pathtracer& pathtracer)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();

    ScopedTimer _prof(L"Disocclusions blur", commandList);
    GpuResource* inOutResource = &m_temporalAOCoefficient[m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex];

    // Force a barrier flush to avoid two same resource transitions since prev atrous pass sets the resource to SRV.
    resourceStateTracker->FlushResourceBarriers();
    resourceStateTracker->TransitionResource(inOutResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    
    GpuResource(&GBufferResources)[GBufferResource::Count] = pathtracer.GBufferResources(RTAO_Args::QuarterResAO);

    UINT filterStep = 1;
    UINT numPasses = static_cast<UINT>(Denoiser_Args::LowTspBlurPasses);
    for (UINT i = 0; i < numPasses; i++)
    {
        wstring passName = L"Depth Aware Gaussian Blur with a pixel step " + to_wstring(filterStep);
        ScopedTimer _prof(passName.c_str(), commandList);

        resourceStateTracker->InsertUAVBarrier(inOutResource);

        resourceStateTracker->FlushResourceBarriers();
        m_bilateralFilterKernel.Run(
            commandList,
            filterStep,
            m_cbvSrvUavHeap->GetHeap(),
            GBufferResources[GBufferResource::Depth].gpuDescriptorReadAccess,
            m_disocclusionBlurStrength.gpuDescriptorReadAccess,
            inOutResource);
        filterStep *= 2;
    }

    resourceStateTracker->TransitionResource(inOutResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    resourceStateTracker->InsertUAVBarrier(inOutResource);
}

// Applies a single pass of a Atrous wavelet transform filter.
void Denoiser::ApplyAtrousWaveletTransformFilter(Pathtracer& pathtracer, RTAO& rtao)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    ScopedTimer _prof(L"AtrousWaveletTransformFilter", commandList);

    GpuResource* AOResources = rtao.AOResources();
    GpuResource* VarianceResource = Denoiser_Args::UseSmoothedVariance ? &m_varianceResources[AOVarianceResource::Smoothed] : &m_varianceResources[AOVarianceResource::Raw];
    
    // Adaptive kernel radius rotation.
    float kernelRadiusLerfCoef = 0;
    if (Denoiser_Args::KernelRadius_RotateKernel_Enabled)
    {
        static UINT frameID = 0;
        UINT i = frameID++ % Denoiser_Args::KernelRadius_RotateKernel_NumCycles;
        kernelRadiusLerfCoef = i / static_cast<float>(Denoiser_Args::KernelRadius_RotateKernel_NumCycles);
    }

    GpuResource(&GBufferResources)[GBufferResource::Count] = pathtracer.GBufferResources(RTAO_Args::QuarterResAO);
    GpuResource* InputAOCoefficientResource = &m_temporalAOCoefficient[m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex];

    // ToDoF clean this up so that its clear.
    m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex = (m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex + 1) % 2;
    GpuResource* OutputResource = &m_temporalAOCoefficient[m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex];
    resourceStateTracker->TransitionResource(OutputResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    
    // A-trous edge-preserving wavelet tranform filter
    {
        // Adjust factors that change based on max ray hit distance.
        // Values were empirically found.
        float RayHitDistanceScaleFactor = 22 / RTAO_Args::MaxRayHitTime * Denoiser_Args::AdaptiveKernelSize_RayHitDistanceScaleFactor;
        float RayHitDistanceScaleExponent = lerp(1, Denoiser_Args::AdaptiveKernelSize_RayHitDistanceScaleExponent, relativeCoef(RTAO_Args::MaxRayHitTime, 4, 22));

        resourceStateTracker->FlushResourceBarriers();
        m_atrousWaveletTransformFilter.Run(
            commandList,
            m_cbvSrvUavHeap->GetHeap(),
            static_cast<RTAOGpuKernels::AtrousWaveletTransformCrossBilateralFilter::FilterType>(static_cast<UINT>(Denoiser_Args::Mode)),
            InputAOCoefficientResource->gpuDescriptorReadAccess,
            GBufferResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorReadAccess,
            VarianceResource->gpuDescriptorReadAccess,
            m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::RayHitDistance].gpuDescriptorReadAccess,
            GBufferResources[GBufferResource::PartialDepthDerivatives].gpuDescriptorReadAccess,
            m_temporalCache[m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp].gpuDescriptorReadAccess,
            OutputResource,
            Denoiser_Args::AODenoiseValueSigma,
            Denoiser_Args::AODenoiseDepthSigma,
            Denoiser_Args::AODenoiseNormalSigma,
            Denoiser_Args::PerspectiveCorrectDepthInterpolation,
            Denoiser_Args::UseAdaptiveKernelSize,
            kernelRadiusLerfCoef,
            RayHitDistanceScaleFactor,
            RayHitDistanceScaleExponent,
            Denoiser_Args::FilterMinKernelWidth,
            static_cast<UINT>((Denoiser_Args::FilterMaxKernelWidthPercentage / 100) * m_denoisingWidth),
            RTAO_Args::QuarterResAO,
            Denoiser_Args::MinVarianceToDenoise,
            Denoiser_Args::AODenoiseDepthWeightCutoff);
    }
    resourceStateTracker->TransitionResource(OutputResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}
