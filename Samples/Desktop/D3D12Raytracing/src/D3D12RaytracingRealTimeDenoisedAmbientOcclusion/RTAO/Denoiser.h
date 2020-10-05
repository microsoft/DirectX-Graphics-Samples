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
#include "Scene.h"
#include "RTAO/RTAO.h"

class RTAO;

namespace Denoiser_Args
{
    extern BoolVar UseSmoothedVariance;
    extern NumVar MinVarianceToDenoise;
    extern BoolVar LowTspp;
}

class Denoiser
{
public:
    enum class ResourceType {
        Variance = 0,
        LocalMeanVariance
    };

    enum DenoiseStage {
        Denoise_Stage1_TemporalSupersamplingReverseReproject = 0x1 << 0,
        Denoise_Stage2_Denoise = 0x1 << 1,
        Denoise_StageAll = Denoise_Stage1_TemporalSupersamplingReverseReproject | Denoise_Stage2_Denoise
    };

    // Public methods.
    void Setup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DX::DescriptorHeap> descriptorHeap);
    void Run(Pathtracer& pathtracer, RTAO& rtao, DenoiseStage stage = Denoise_StageAll);
    void SetResolution(UINT width, UINT height);
        
    // Getters/Setters.
    static DXGI_FORMAT ResourceFormat(ResourceType resourceType);
    UINT DenoisingWidth() { return m_denoisingWidth; }
    UINT DenoisingHeight() { return m_denoisingHeight; }

private:
    void TemporalSupersamplingReverseReproject(Pathtracer& pathtracer);
    void TemporalSupersamplingBlendWithCurrentFrame(RTAO& rtao);
    void BlurDisocclusions(Pathtracer& pathtracer);

    void CreateDeviceDependentResources();
    void CreateAuxilaryDeviceResources();
    void CreateTextureResources();
    void ApplyAtrousWaveletTransformFilter(Pathtracer& pathtracer, RTAO& rtao);
    void CreateResolutionDependentResources();

    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::shared_ptr<DX::DescriptorHeap> m_cbvSrvUavHeap;

    UINT m_denoisingWidth = 0;
    UINT m_denoisingHeight = 0;
 
    GpuResource m_temporalCache[2][TemporalSupersampling::Count]; // ~array[Read/Write ping pong resource][Resources].
    GpuResource m_temporalAOCoefficient[2];
    GpuResource m_cachedTsppValueSquaredValueRayHitDistance;

    UINT          m_temporalCacheCurrentFrameResourceIndex = 0;
    UINT          m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex = 0;

    GpuResource m_varianceResources[AOVarianceResource::Count];
    GpuResource m_localMeanVarianceResources[AOVarianceResource::Count];
    GpuResource m_disocclusionBlurStrength;
    GpuResource m_prevFrameGBufferNormalDepth;

    // RTAOGpuKernels
    RTAOGpuKernels::FillInCheckerboard      m_fillInCheckerboardKernel;
    RTAOGpuKernels::GaussianFilter          m_gaussianSmoothingKernel;
    RTAOGpuKernels::TemporalSupersampling_ReverseReproject m_temporalCacheReverseReprojectKernel;
    RTAOGpuKernels::TemporalSupersampling_BlendWithCurrentFrame m_temporalCacheBlendWithCurrentFrameKernel;
    RTAOGpuKernels::AtrousWaveletTransformCrossBilateralFilter m_atrousWaveletTransformFilter;
    RTAOGpuKernels::CalculateMeanVariance   m_calculateMeanVarianceKernel;
    RTAOGpuKernels::DisocclusionBilateralFilter m_disocclusionBlurKernel;

    friend class Composition;
public:
    static const UINT c_MaxNumDisocllusionBlurPasses = 6;
};