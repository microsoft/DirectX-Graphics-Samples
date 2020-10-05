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
#include "RTAO/RTAO.h"
#include "Pathtracer.h"
#include "Denoiser.h"


namespace Composition_Args
{
    extern EnumVar CompositionMode;
    extern BoolVar AOEnabled;
}

class RTAO;
class Denoiser;

class Composition
{
public:
    // Public methods.
    void Setup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DX::DescriptorHeap> descriptorHeap);
    void Render(GpuResource* outputResource, Scene& scene, Pathtracer& pathtracer, RTAO& rtao, Denoiser& denoiser, UINT GBufferWidth, UINT GBufferHeight);
    void SetResolution(UINT width, UINT height);

private:
    void CreateShaderResources();
    void CreateDeviceDependentResources();
    void CreateAuxilaryDeviceResources();
    void CreateTextureResources();
    void CreateResolutionDependentResources();
    void BilateralUpsample(
        UINT hiResWidth,
        UINT hiResHeight,
        GpuKernels::UpsampleBilateralFilter::FilterType filterType,
        D3D12_GPU_DESCRIPTOR_HANDLE inputLowResValueResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputLowResNormalDepthResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputHiResNormalDepthResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputHiResPartialDepthDerivativesResourceHandle,
        GpuResource* outputHiResValueResource,
        LPCWCHAR passName);

    void UpsampleResourcesForRenderComposePass(Pathtracer& pathtracer, RTAO& rtao, Denoiser& denoiser, UINT GBufferWidth, UINT GBufferHeight);


    static UINT s_numInstances;
    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::shared_ptr<DX::DescriptorHeap> m_cbvSrvUavHeap;

    ComPtr<ID3D12PipelineState>         m_computePSOs[ComputeShader::Type::Count];
    ComPtr<ID3D12RootSignature>         m_computeRootSigs[ComputeShader::Type::Count];

    ConstantBuffer<ComposeRenderPassesConstantBuffer>   m_csComposeRenderPassesCB;

    UINT m_renderingWidth = 0;
    UINT m_renderingHeight = 0;

    GpuResource m_upsampledAOValueResource;
    GpuResource m_upsampledTsppResource;
    GpuResource m_upsampledAORayHitDistanceResource;
    GpuResource m_upsampledVarianceResource;
    GpuResource m_upsampledLocalMeanVarianceResource;

    GpuKernels::UpsampleBilateralFilter	    m_upsampleBilateralFilterKernel;
};