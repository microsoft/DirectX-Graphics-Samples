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
#include "Composition.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "EngineProfiling.h"
#include "GpuTimeManager.h"
#include "Composition.h"
#include "CompiledShaders\CompositionCS.hlsl.h"
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;

namespace Composition_Args
{
    const WCHAR* CompositionModes[CompositionType::Count] = {
        L"Specular PBR Pathtracer",
        L"Denoised AO",
        L"Raw one-frame AO",
        L"Disocclusion Map",
        L"AO Variance",
        L"AO Local Variance",
        L"Temporal AO Ray Hit Distance",
        L"Normal Map",
        L"Depth Buffer",
        L"Albedo/Texture",
        L"Base Material Albedo"
    };
    EnumVar CompositionMode(L"Render/Render composition/Mode", CompositionType::AmbientOcclusionOnly_Denoised, CompositionType::Count, CompositionModes);
    BoolVar Compose_VarianceVisualizeStdDeviation(L"Render/Render composition/Variance/Visualize std deviation", true);
    NumVar Compose_VarianceScale(L"Render/Render composition/Variance/Variance scale", 1.0f, 0, 10, 0.1f);

    BoolVar AOEnabled(L"Render/AO/Enabled", true);
}

void Composition::Setup(shared_ptr<DeviceResources> deviceResources, shared_ptr<DX::DescriptorHeap> descriptorHeap)
{
    m_deviceResources = deviceResources;
    m_cbvSrvUavHeap = descriptorHeap;

    CreateDeviceDependentResources();
}

// Create resources that depend on the device.
void Composition::CreateDeviceDependentResources()
{
    CreateAuxilaryDeviceResources();
    CreateShaderResources();
}

void Composition::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    m_upsampleBilateralFilterKernel.Initialize(device, FrameCount);
}

void Composition::CreateResolutionDependentResources()
{
    CreateTextureResources();
}

void Composition::SetResolution(UINT width, UINT height)
{
    m_renderingWidth = width;
    m_renderingHeight = height;

    CreateResolutionDependentResources();
}

void Composition::CreateTextureResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    CreateRenderTargetResource(device, RTAO::ResourceFormat(RTAO::ResourceType::AOCoefficient), m_renderingWidth, m_renderingHeight, m_cbvSrvUavHeap.get(), &m_upsampledAOValueResource, initialResourceState, L"Upsampled AO value");
    CreateRenderTargetResource(device, DXGI_FORMAT_R8_UINT, m_renderingWidth, m_renderingHeight, m_cbvSrvUavHeap.get(), &m_upsampledTsppResource, initialResourceState, L"Upsampled Tspp");
    CreateRenderTargetResource(device, RTAO::ResourceFormat(RTAO::ResourceType::RayHitDistance), m_renderingWidth, m_renderingHeight, m_cbvSrvUavHeap.get(), &m_upsampledAORayHitDistanceResource, initialResourceState, L"Upsampled AO Ray Hit Distance");
    CreateRenderTargetResource(device, Denoiser::ResourceFormat(Denoiser::ResourceType::Variance), m_renderingWidth, m_renderingHeight, m_cbvSrvUavHeap.get(), &m_upsampledVarianceResource, initialResourceState, L"Upsampled Variance");
    CreateRenderTargetResource(device, Denoiser::ResourceFormat(Denoiser::ResourceType::LocalMeanVariance), m_renderingWidth, m_renderingHeight, m_cbvSrvUavHeap.get(), &m_upsampledLocalMeanVarianceResource, initialResourceState, L"Upsampled Local Mean Variance");
}

void Composition::CreateShaderResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    // Create root signature.
    {
        using namespace CSRootSignature::CompositionCS;

        CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count]; 
        ranges[Slot::Output].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  
        ranges[Slot::GBufferResources].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 2); 
        ranges[Slot::AO].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);  
        ranges[Slot::AORayHitDistance].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9); 
        ranges[Slot::Tspp].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10); 
        ranges[Slot::Color].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11);
        ranges[Slot::AOSurfaceAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12);
        ranges[Slot::Variance].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13);
        ranges[Slot::LocalMeanVariance].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 14);

        CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
        rootParameters[Slot::Output].InitAsDescriptorTable(1, &ranges[Slot::Output]);
        rootParameters[Slot::GBufferResources].InitAsDescriptorTable(1, &ranges[Slot::GBufferResources]);
        rootParameters[Slot::AO].InitAsDescriptorTable(1, &ranges[Slot::AO]);
        rootParameters[Slot::AORayHitDistance].InitAsDescriptorTable(1, &ranges[Slot::AORayHitDistance]);
        rootParameters[Slot::Tspp].InitAsDescriptorTable(1, &ranges[Slot::Tspp]);
        rootParameters[Slot::Color].InitAsDescriptorTable(1, &ranges[Slot::Color]);
        rootParameters[Slot::AOSurfaceAlbedo].InitAsDescriptorTable(1, &ranges[Slot::AOSurfaceAlbedo]);
        rootParameters[Slot::Variance].InitAsDescriptorTable(1, &ranges[Slot::Variance]);
        rootParameters[Slot::LocalMeanVariance].InitAsDescriptorTable(1, &ranges[Slot::LocalMeanVariance]);
        rootParameters[Slot::MaterialBuffer].InitAsShaderResourceView(7);
        rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);


        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_computeRootSigs[CSType::CompositionCS], L"Root signature: CompositionCS");
    }

    m_csComposeRenderPassesCB.Create(device, FrameCount, L"Constant Buffer: CompositionCS");

    // Create compute pipeline state.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
        descComputePSO.pRootSignature = m_computeRootSigs[CSType::CompositionCS].Get();
        descComputePSO.CS = CD3DX12_SHADER_BYTECODE((void*)g_pCompositionCS, ARRAYSIZE(g_pCompositionCS));

        ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_computePSOs[CSType::CompositionCS])));
        m_computePSOs[CSType::CompositionCS]->SetName(L"PSO: CompositionCS");
    }
}

// Upsample downsampled resources needed for the composition pass.
void Composition::UpsampleResourcesForRenderComposePass(
    Pathtracer& pathtracer,
    RTAO& rtao,
    Denoiser& denoiser,
    UINT GBufferWidth, 
    UINT GBufferHeight)
{
    auto commandList = m_deviceResources->GetCommandList();
    GpuResource* inputLowResValueResource = nullptr;
    GpuResource* outputHiResValueResource = nullptr;
    wstring passName;
    GpuKernels::UpsampleBilateralFilter::FilterType filterType = GpuKernels::UpsampleBilateralFilter::Filter2x2FloatR;

    switch (Composition_Args::CompositionMode)
    {
    case CompositionType::PBRShading:
    case CompositionType::AmbientOcclusionOnly_Denoised:
    case CompositionType::AmbientOcclusionOnly_RawOneFrame:
    {
        passName = L"Upsample AO";
        outputHiResValueResource = &m_upsampledAOValueResource;

        if (Composition_Args::CompositionMode == CompositionType::AmbientOcclusionOnly_RawOneFrame)
        {
            inputLowResValueResource = &rtao.AOResources()[AOResource::AmbientCoefficient];
        }
        else
        {
            inputLowResValueResource = &denoiser.m_temporalAOCoefficient[denoiser.m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex];
        }
        break;
    }
    case CompositionType::AmbientOcclusionAndDisocclusionMap:
    {
        passName = L"Upsample Disocclusion Map";
        filterType = GpuKernels::UpsampleBilateralFilter::Filter2x2UintR;
        inputLowResValueResource = &denoiser.m_temporalCache[denoiser.m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::Tspp];
        outputHiResValueResource = &m_upsampledTsppResource;
        break;
    }
    case CompositionType::RTAOHitDistance:
    {
        passName = L"Upsample AO ray hit distance";
        inputLowResValueResource = &denoiser.m_temporalCache[denoiser.m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::RayHitDistance];
        outputHiResValueResource = &m_upsampledAORayHitDistanceResource;
        break;
    }
    case CompositionType::AmbientOcclusionVariance:
    {
        passName = L"Upsample AO variance";
        inputLowResValueResource = Denoiser_Args::UseSmoothedVariance ? &denoiser.m_varianceResources[AOVarianceResource::Smoothed] : &denoiser.m_varianceResources[AOVarianceResource::Raw];
        outputHiResValueResource = &m_upsampledVarianceResource;
        break;
    }
    case CompositionType::AmbientOcclusionLocalVariance:
    {
        passName = L"Upsample AO local variance";
        filterType = GpuKernels::UpsampleBilateralFilter::Filter2x2FloatRG;
        inputLowResValueResource = Denoiser_Args::UseSmoothedVariance ? &denoiser.m_localMeanVarianceResources[AOVarianceResource::Smoothed] : &denoiser.m_localMeanVarianceResources[AOVarianceResource::Raw];
        outputHiResValueResource = &m_upsampledLocalMeanVarianceResource;
        break;
    }
    default:
        break;
    }

    if (inputLowResValueResource)
    {
        auto& GBufferQuarterResResources = pathtracer.GBufferResources(true);
        auto& GBufferResources = pathtracer.GBufferResources(false);

        BilateralUpsample(
            GBufferWidth,
            GBufferHeight,
            filterType,
            inputLowResValueResource->gpuDescriptorReadAccess,
            GBufferQuarterResResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorReadAccess,
            GBufferResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorReadAccess,
            GBufferResources[GBufferResource::PartialDepthDerivatives].gpuDescriptorReadAccess,
            outputHiResValueResource,
            passName.c_str());
    }
}

void Composition::BilateralUpsample(
    UINT hiResWidth,
    UINT hiResHeight,
    GpuKernels::UpsampleBilateralFilter::FilterType filterType,
    D3D12_GPU_DESCRIPTOR_HANDLE inputLowResValueResourceHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE inputLowResNormalDepthResourceHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE inputHiResNormalDepthResourceHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE inputHiResPartialDepthDerivativesResourceHandle,
    GpuResource* outputHiResValueResource,
    LPCWCHAR passName)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();

    ScopedTimer _prof(passName, commandList);

    resourceStateTracker->TransitionResource(outputHiResValueResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    resourceStateTracker->FlushResourceBarriers();
    m_upsampleBilateralFilterKernel.Run(
        commandList,
        hiResWidth,
        hiResHeight,
        filterType,
        m_cbvSrvUavHeap->GetHeap(),
        inputLowResValueResourceHandle,
        inputLowResNormalDepthResourceHandle,
        inputHiResNormalDepthResourceHandle,
        inputHiResPartialDepthDerivativesResourceHandle,
        outputHiResValueResource->gpuDescriptorWriteAccess
    );

    resourceStateTracker->TransitionResource(outputHiResValueResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

// Composite render pass results into a final image.
void Composition::Render(
    GpuResource* outputResource,
    Scene& scene,
    Pathtracer& pathtracer, 
    RTAO& rtao,
    Denoiser& denoiser,
    UINT GBufferWidth, 
    UINT GBufferHeight)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto& GBufferResources = pathtracer.GBufferResources(false);

    ScopedTimer _prof(L"CompositionCS", commandList);

    if (RTAO_Args::QuarterResAO)
    {
        UpsampleResourcesForRenderComposePass(pathtracer, rtao, denoiser, GBufferWidth, GBufferHeight);
    }

    // Update constant buffer.
    {
        m_csComposeRenderPassesCB->isAOEnabled = Composition_Args::AOEnabled;
        m_csComposeRenderPassesCB->compositionType = static_cast<CompositionType>(static_cast<UINT>(Composition_Args::CompositionMode));
        m_csComposeRenderPassesCB->defaultAmbientIntensity = Pathtracer_Args::DefaultAmbientIntensity;

        m_csComposeRenderPassesCB->variance_visualizeStdDeviation = Composition_Args::Compose_VarianceVisualizeStdDeviation;
        m_csComposeRenderPassesCB->variance_scale = Composition_Args::Compose_VarianceScale;
        m_csComposeRenderPassesCB->RTAO_MaxRayHitDistance = rtao.MaxRayHitTime();
        m_csComposeRenderPassesCB.CopyStagingToGpu(frameIndex);
    }

    // Set pipeline state.
    {
        using namespace ComputeShader::RootSignature::CompositionCS;

        GpuResource* TemporalResources = denoiser.m_temporalCache[denoiser.m_temporalCacheCurrentFrameResourceIndex];
        GpuResource* VarianceResource = Denoiser_Args::UseSmoothedVariance ? &denoiser.m_varianceResources[AOVarianceResource::Smoothed] : &denoiser.m_varianceResources[AOVarianceResource::Raw];
        GpuResource* LocalMeanVarianceResource = &denoiser.m_localMeanVarianceResources[AOVarianceResource::Raw];
        GpuResource* AORayHitDistance = &denoiser.m_temporalCache[denoiser.m_temporalCacheCurrentFrameResourceIndex][TemporalSupersampling::RayHitDistance];

        GpuResource* AOResource;
        switch (Composition_Args::CompositionMode)
        {
        case CompositionType::PBRShading:
        case CompositionType::AmbientOcclusionOnly_Denoised:
            AOResource = &denoiser.m_temporalAOCoefficient[denoiser.m_temporalCacheCurrentFrameTemporalAOCoefficientResourceIndex];
            break;

        case CompositionType::AmbientOcclusionOnly_RawOneFrame:
        default:
            AOResource = &rtao.AOResources()[AOResource::AmbientCoefficient];
            break;
        }
        GpuResource* TsppResource = &TemporalResources[TemporalSupersampling::Tspp];

        if (RTAO_Args::QuarterResAO)
        {
            AOResource = &m_upsampledAOValueResource;
            TsppResource = &m_upsampledTsppResource;
            VarianceResource = &m_upsampledVarianceResource;
            LocalMeanVarianceResource = &m_upsampledLocalMeanVarianceResource;
            AORayHitDistance = &m_upsampledAORayHitDistanceResource;
        }

        commandList->SetDescriptorHeaps(1, m_cbvSrvUavHeap->GetAddressOf());
        commandList->SetComputeRootSignature(m_computeRootSigs[CSType::CompositionCS].Get());
        commandList->SetPipelineState(m_computePSOs[CSType::CompositionCS].Get());

        // Bind outputs.
        commandList->SetComputeRootDescriptorTable(Slot::Output, outputResource->gpuDescriptorWriteAccess);

        // Bind inputs.
        commandList->SetComputeRootDescriptorTable(Slot::GBufferResources, GBufferResources[0].gpuDescriptorReadAccess);
        commandList->SetComputeRootDescriptorTable(Slot::AO, AOResource->gpuDescriptorReadAccess);
        commandList->SetComputeRootShaderResourceView(Slot::MaterialBuffer, scene.m_materialBuffer.GpuVirtualAddress());
        commandList->SetComputeRootConstantBufferView(Slot::ConstantBuffer, m_csComposeRenderPassesCB.GpuVirtualAddress(frameIndex));
        commandList->SetComputeRootDescriptorTable(Slot::Variance, VarianceResource->gpuDescriptorReadAccess);
        commandList->SetComputeRootDescriptorTable(Slot::LocalMeanVariance, LocalMeanVarianceResource->gpuDescriptorReadAccess);

        commandList->SetComputeRootDescriptorTable(Slot::AORayHitDistance, AORayHitDistance->gpuDescriptorReadAccess);
        commandList->SetComputeRootDescriptorTable(Slot::Color, GBufferResources[GBufferResource::Color].gpuDescriptorReadAccess);
        commandList->SetComputeRootDescriptorTable(Slot::AOSurfaceAlbedo, GBufferResources[GBufferResource::AOSurfaceAlbedo].gpuDescriptorReadAccess);


        commandList->SetComputeRootDescriptorTable(Slot::Tspp, TsppResource->gpuDescriptorReadAccess);
    }

    // Dispatch.
    XMUINT2 groupSize(CeilDivide(GBufferWidth, DefaultComputeShaderParams::ThreadGroup::Width), CeilDivide(GBufferHeight, DefaultComputeShaderParams::ThreadGroup::Height));

    resourceStateTracker->FlushResourceBarriers();
    commandList->Dispatch(groupSize.x, groupSize.y, 1);
}

