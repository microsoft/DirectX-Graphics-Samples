//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"

#include "PostEffects.h"
#include "GameCore.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "GraphicsCore.h"
#include "BufferManager.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "FXAA.h"

#include "CompiledShaders/ToneMapCS.h"
#include "CompiledShaders/ToneMap2CS.h"
#include "CompiledShaders/ToneMapHDRCS.h"
#include "CompiledShaders/ToneMapHDR2CS.h"
#include "CompiledShaders/ApplyBloomCS.h"
#include "CompiledShaders/ApplyBloom2CS.h"
#include "CompiledShaders/DebugLuminanceHdrCS.h"
#include "CompiledShaders/DebugLuminanceHdr2CS.h"
#include "CompiledShaders/DebugLuminanceLdrCS.h"
#include "CompiledShaders/DebugLuminanceLdr2CS.h"
#include "CompiledShaders/GenerateHistogramCS.h"
#include "CompiledShaders/DebugDrawHistogramCS.h"
#include "CompiledShaders/AdaptExposureCS.h"
#include "CompiledShaders/DownsampleBloomCS.h"
#include "CompiledShaders/DownsampleBloomAllCS.h"
#include "CompiledShaders/UpsampleAndBlurCS.h"
#include "CompiledShaders/BlurCS.h"
#include "CompiledShaders/BloomExtractAndDownsampleHdrCS.h"
#include "CompiledShaders/BloomExtractAndDownsampleLdrCS.h"
#include "CompiledShaders/ExtractLumaCS.h"
#include "CompiledShaders/AverageLumaCS.h"
#include "CompiledShaders/CopyBackPostBufferCS.h"

using namespace Graphics;

namespace SSAO
{
    extern BoolVar DebugDraw;
}

namespace FXAA
{
    extern BoolVar DebugDraw;
}

namespace DepthOfField
{
    extern BoolVar Enable;
    extern EnumVar DebugMode;
}

namespace PostEffects
{
    const float kInitialMinLog = -12.0f;
    const float kInitialMaxLog = 4.0f;

    BoolVar EnableHDR("Graphics/HDR/Enable", true);
    BoolVar EnableAdaptation("Graphics/HDR/Adaptive Exposure", true);
    ExpVar MinExposure("Graphics/HDR/Min Exposure", 1.0f / 64.0f, -8.0f, 0.0f, 0.25f);
    ExpVar MaxExposure("Graphics/HDR/Max Exposure", 64.0f, 0.0f, 8.0f, 0.25f);
    NumVar TargetLuminance("Graphics/HDR/Key", 0.08f, 0.01f, 0.99f, 0.01f);
    NumVar AdaptationRate("Graphics/HDR/Adaptation Rate", 0.05f, 0.01f, 1.0f, 0.01f);
    ExpVar Exposure("Graphics/HDR/Exposure", 2.0f, -8.0f, 8.0f, 0.25f);
    BoolVar DrawHistogram("Graphics/HDR/Draw Histogram", false);

    BoolVar BloomEnable("Graphics/Bloom/Enable", true);
    NumVar BloomThreshold("Graphics/Bloom/Threshold", 4.0f, 0.0f, 8.0f, 0.1f);		// The threshold luminance above which a pixel will start to bloom
    NumVar BloomStrength("Graphics/Bloom/Strength", 0.1f, 0.0f, 2.0f, 0.05f);		// A modulator controlling how much bloom is added back into the image
    NumVar BloomUpsampleFactor("Graphics/Bloom/Scatter", 0.65f, 0.0f, 1.0f, 0.05f);	// Controls the "focus" of the blur.  High values spread out more causing a haze.
    BoolVar HighQualityBloom("Graphics/Bloom/High Quality", true);					// High quality blurs 5 octaves of bloom; low quality only blurs 3.

    RootSignature PostEffectsRS;
    ComputePSO ToneMapCS;
    ComputePSO ToneMapHDRCS;
    ComputePSO ApplyBloomCS;
    ComputePSO DebugLuminanceHdrCS;
    ComputePSO DebugLuminanceLdrCS;
    ComputePSO GenerateHistogramCS;
    ComputePSO DrawHistogramCS;
    ComputePSO AdaptExposureCS;
    ComputePSO DownsampleBloom2CS;
    ComputePSO DownsampleBloom4CS;
    ComputePSO UpsampleAndBlurCS;
    ComputePSO BlurCS;
    ComputePSO BloomExtractAndDownsampleHdrCS;
    ComputePSO BloomExtractAndDownsampleLdrCS;
    ComputePSO ExtractLumaCS;
    ComputePSO AverageLumaCS;
    ComputePSO CopyBackPostBufferCS;

    StructuredBuffer g_Exposure;

    void UpdateExposure(ComputeContext&);
    void BlurBuffer(ComputeContext&, ColorBuffer buffer[2], const ColorBuffer& lowerResBuf, float upsampleBlendFactor );
    void GenerateBloom(ComputeContext&);
    void ExtractLuma(ComputeContext&);
    void ProcessHDR(ComputeContext&);
    void ProcessLDR(CommandContext&);
}

void PostEffects::Initialize( void )
{
    PostEffectsRS.Reset(4, 2);
    PostEffectsRS.InitStaticSampler(0, SamplerLinearClampDesc);
    PostEffectsRS.InitStaticSampler(1, SamplerLinearBorderDesc);
    PostEffectsRS[0].InitAsConstants(0, 4);
    PostEffectsRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
    PostEffectsRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
    PostEffectsRS[3].InitAsConstantBuffer(1);
    PostEffectsRS.Finalize(L"Post Effects");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(PostEffectsRS); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        CreatePSO(ToneMapCS, g_pToneMap2CS);
        CreatePSO(ToneMapHDRCS, g_pToneMapHDR2CS);
        CreatePSO(ApplyBloomCS, g_pApplyBloom2CS);
        CreatePSO(DebugLuminanceHdrCS, g_pDebugLuminanceHdr2CS);
        CreatePSO(DebugLuminanceLdrCS, g_pDebugLuminanceLdr2CS);
    }
    else
    {
        CreatePSO(ToneMapCS, g_pToneMapCS);
        CreatePSO(ToneMapHDRCS, g_pToneMapHDRCS);
        CreatePSO(ApplyBloomCS, g_pApplyBloomCS);
        CreatePSO(DebugLuminanceHdrCS, g_pDebugLuminanceHdrCS);
        CreatePSO(DebugLuminanceLdrCS, g_pDebugLuminanceLdrCS);
    }
    CreatePSO( GenerateHistogramCS, g_pGenerateHistogramCS );
    CreatePSO( DrawHistogramCS, g_pDebugDrawHistogramCS );
    CreatePSO( AdaptExposureCS, g_pAdaptExposureCS );
    CreatePSO( DownsampleBloom2CS, g_pDownsampleBloomCS );
    CreatePSO( DownsampleBloom4CS, g_pDownsampleBloomAllCS );
    CreatePSO( UpsampleAndBlurCS, g_pUpsampleAndBlurCS );
    CreatePSO( BlurCS, g_pBlurCS );
    CreatePSO( BloomExtractAndDownsampleHdrCS, g_pBloomExtractAndDownsampleHdrCS );
    CreatePSO( BloomExtractAndDownsampleLdrCS, g_pBloomExtractAndDownsampleLdrCS );
    CreatePSO( ExtractLumaCS, g_pExtractLumaCS );
    CreatePSO( AverageLumaCS, g_pAverageLumaCS );
    CreatePSO( CopyBackPostBufferCS, g_pCopyBackPostBufferCS );


#undef CreatePSO

    __declspec(align(16)) float initExposure[] =
    {
        Exposure, 1.0f / Exposure, Exposure, 0.0f,
        kInitialMinLog, kInitialMaxLog, kInitialMaxLog - kInitialMinLog, 1.0f / (kInitialMaxLog - kInitialMinLog)
    };
    g_Exposure.Create(L"Exposure", 8, 4, initExposure);

    FXAA::Initialize();
    MotionBlur::Initialize();
    DepthOfField::Initialize();
}

void PostEffects::Shutdown( void )
{
    g_Exposure.Destroy();

    FXAA::Shutdown();
    MotionBlur::Shutdown();
    DepthOfField::Shutdown();
}

void PostEffects::BlurBuffer( ComputeContext& Context, ColorBuffer buffer[2], const ColorBuffer& lowerResBuf, float upsampleBlendFactor )
{
    // Set the shader constants
    uint32_t bufferWidth = buffer[0].GetWidth();
    uint32_t bufferHeight = buffer[0].GetHeight();
    Context.SetConstants(0, 1.0f / bufferWidth, 1.0f / bufferHeight, upsampleBlendFactor);

    // Set the input textures and output UAV
    Context.TransitionResource( buffer[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptor(1, 0, buffer[1].GetUAV());
    D3D12_CPU_DESCRIPTOR_HANDLE SRVs[2] = { buffer[0].GetSRV(), lowerResBuf.GetSRV() };
    Context.SetDynamicDescriptors(2, 0, 2, SRVs);

    // Set the shader:  upsample and blur or just blur
    Context.SetPipelineState(&buffer[0] == &lowerResBuf ? BlurCS : UpsampleAndBlurCS);

    // Dispatch the compute shader with default 8x8 thread groups
    Context.Dispatch2D(bufferWidth, bufferHeight);

    Context.TransitionResource( buffer[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

//--------------------------------------------------------------------------------------
// Bloom effect in CS path
//--------------------------------------------------------------------------------------
void PostEffects::GenerateBloom( ComputeContext& Context )
{
    ScopedTimer _prof(L"Generate Bloom", Context);

    // We can generate a bloom buffer up to 1/4 smaller in each dimension without undersampling.  If only downsizing by 1/2 or less, a faster
    // shader can be used which only does one bilinear sample.

    uint32_t kBloomWidth = g_LumaLR.GetWidth();
    uint32_t kBloomHeight = g_LumaLR.GetHeight();

    // These bloom buffer dimensions were chosen for their impressive divisibility by 128 and because they are roughly 16:9.
    // The blurring algorithm is exactly 9 pixels by 9 pixels, so if the aspect ratio of each pixel is not square, the blur
    // will be oval in appearance rather than circular.  Coincidentally, they are close to 1/2 of a 720p buffer and 1/3 of
    // 1080p.  This is a common size for a bloom buffer on consoles.
    ASSERT(kBloomWidth % 16 == 0 && kBloomHeight % 16 == 0, "Bloom buffer dimensions must be multiples of 16");


    Context.SetConstants(0, 1.0f / kBloomWidth, 1.0f / kBloomHeight, (float)BloomThreshold );
    Context.TransitionResource(g_aBloomUAV1[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_LumaLR, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    {

        Context.SetDynamicDescriptor(1, 0, g_aBloomUAV1[0].GetUAV());
        Context.SetDynamicDescriptor(1, 1, g_LumaLR.GetUAV());
        Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetSRV());
        Context.SetDynamicDescriptor(2, 1, g_Exposure.GetSRV());

        Context.SetPipelineState(EnableHDR ? BloomExtractAndDownsampleHdrCS : BloomExtractAndDownsampleLdrCS);
        Context.Dispatch2D(kBloomWidth, kBloomHeight);
    }

    Context.TransitionResource(g_aBloomUAV1[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(2, 0, g_aBloomUAV1[0].GetSRV());

    // The difference between high and low quality bloom is that high quality sums 5 octaves with a 2x frequency scale, and the low quality
    // sums 3 octaves with a 4x frequency scale.
    if (HighQualityBloom)
    {
        Context.TransitionResource(g_aBloomUAV2[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV4[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Set the UAVs
        D3D12_CPU_DESCRIPTOR_HANDLE UAVs[4] = {
            g_aBloomUAV2[0].GetUAV(), g_aBloomUAV3[0].GetUAV(), g_aBloomUAV4[0].GetUAV(), g_aBloomUAV5[0].GetUAV() };
        Context.SetDynamicDescriptors(1, 0, 4, UAVs);

        // Each dispatch group is 8x8 threads, but each thread reads in 2x2 source texels (bilinear filter).
        Context.SetPipelineState(DownsampleBloom4CS);
        Context.Dispatch2D(kBloomWidth / 2, kBloomHeight / 2);

        Context.TransitionResource(g_aBloomUAV2[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV4[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        float upsampleBlendFactor = BloomUpsampleFactor;

        // Blur then upsample and blur four times
        BlurBuffer( Context, g_aBloomUAV5, g_aBloomUAV5[0], 1.0f );
        BlurBuffer( Context, g_aBloomUAV4, g_aBloomUAV5[1], upsampleBlendFactor  );
        BlurBuffer( Context, g_aBloomUAV3, g_aBloomUAV4[1], upsampleBlendFactor  );
        BlurBuffer( Context, g_aBloomUAV2, g_aBloomUAV3[1], upsampleBlendFactor  );
        BlurBuffer( Context, g_aBloomUAV1, g_aBloomUAV2[1], upsampleBlendFactor  );
    }
    else
    {
        // Set the UAVs
        D3D12_CPU_DESCRIPTOR_HANDLE UAVs[2] = { g_aBloomUAV3[0].GetUAV(), g_aBloomUAV5[0].GetUAV() };
        Context.SetDynamicDescriptors(1, 0, 2, UAVs);

        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Each dispatch group is 8x8 threads, but each thread reads in 2x2 source texels (bilinear filter).
        Context.SetPipelineState(DownsampleBloom2CS);
        Context.Dispatch2D(kBloomWidth / 2, kBloomHeight / 2);

        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        float upsampleBlendFactor = BloomUpsampleFactor * 2.0f / 3.0f;

        // Blur then upsample and blur two times
        BlurBuffer( Context, g_aBloomUAV5, g_aBloomUAV5[0], 1.0f );
        BlurBuffer( Context, g_aBloomUAV3, g_aBloomUAV5[1], upsampleBlendFactor );
        BlurBuffer( Context, g_aBloomUAV1, g_aBloomUAV3[1], upsampleBlendFactor );
    }
}

void PostEffects::ExtractLuma( ComputeContext& Context )
{
    ScopedTimer _prof(L"Extract Luma", Context);

    Context.TransitionResource(g_LumaLR, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetConstants(0, 1.0f / g_LumaLR.GetWidth(), 1.0f / g_LumaLR.GetHeight());
    Context.SetDynamicDescriptor(1, 0, g_LumaLR.GetUAV());
    Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetSRV());
    Context.SetDynamicDescriptor(2, 1, g_Exposure.GetSRV());
    Context.SetPipelineState(ExtractLumaCS);
    Context.Dispatch2D(g_LumaLR.GetWidth(), g_LumaLR.GetHeight());
}

void PostEffects::UpdateExposure( ComputeContext& Context )
{
    ScopedTimer _prof(L"Update Exposure", Context);

    if (!EnableAdaptation)
    {
        __declspec(align(16)) float initExposure[] =
        {
            Exposure, 1.0f / Exposure, Exposure, 0.0f,
            kInitialMinLog, kInitialMaxLog, kInitialMaxLog - kInitialMinLog, 1.0f / (kInitialMaxLog - kInitialMinLog)
        };
        Context.WriteBuffer(g_Exposure, 0, initExposure, sizeof(initExposure));
        Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        return;
    }
    
    // Generate an HDR histogram
    Context.TransitionResource(g_Histogram, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
    Context.ClearUAV(g_Histogram);
    Context.TransitionResource(g_LumaLR, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(1, 0, g_Histogram.GetUAV() );
    Context.SetDynamicDescriptor(2, 0, g_LumaLR.GetSRV() );
    Context.SetPipelineState(GenerateHistogramCS);
    Context.Dispatch2D(g_LumaLR.GetWidth(), g_LumaLR.GetHeight(), 16, 384);

    __declspec(align(16)) struct
    {
        float TargetLuminance;
        float AdaptationRate;
        float MinExposure;
        float MaxExposure;
        uint32_t PixelCount; 
    } constants =
    {
        TargetLuminance, AdaptationRate, MinExposure, MaxExposure,
        g_LumaLR.GetWidth() * g_LumaLR.GetHeight()
    };
    Context.TransitionResource(g_Histogram, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptor(1, 0, g_Exposure.GetUAV());
    Context.SetDynamicDescriptor(2, 0, g_Histogram.GetSRV());
    Context.SetDynamicConstantBufferView(3, sizeof(constants), &constants);
    Context.SetPipelineState(AdaptExposureCS);
    Context.Dispatch();
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void PostEffects::ProcessHDR( ComputeContext& Context )
{
    ScopedTimer _prof(L"HDR Tone Mapping", Context);

    if (BloomEnable)
    {
        GenerateBloom(Context);
        Context.TransitionResource(g_aBloomUAV1[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    else if (EnableAdaptation)
        ExtractLuma(Context);

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    else
        Context.TransitionResource(g_PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetPipelineState(FXAA::DebugDraw ? DebugLuminanceHdrCS : (g_bEnableHDROutput ? ToneMapHDRCS : ToneMapCS));

    // Set constants
    Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(),
        (float)BloomStrength);

    // Separate out SDR result from its perceived luminance
    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
    else
    {
        Context.SetDynamicDescriptor(1, 0, g_PostEffectsBuffer.GetUAV());
        Context.SetDynamicDescriptor(2, 2, g_SceneColorBuffer.GetSRV());
    }
    Context.SetDynamicDescriptor(1, 1, g_LumaBuffer.GetUAV());

    // Read in original HDR value and blurred bloom buffer
    Context.SetDynamicDescriptor(2, 0, g_Exposure.GetSRV());
    Context.SetDynamicDescriptor(2, 1, BloomEnable ? g_aBloomUAV1[1].GetSRV() : TextureManager::GetBlackTex2D().GetSRV());
    
    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    // Do this last so that the bright pass uses the same exposure as tone mapping
    UpdateExposure(Context);
}

void PostEffects::ProcessLDR(CommandContext& BaseContext)
{
    ScopedTimer _prof(L"SDR Processing", BaseContext);

    ComputeContext& Context = BaseContext.GetComputeContext();

    bool bGenerateBloom = BloomEnable && !SSAO::DebugDraw;
    if (bGenerateBloom)
        GenerateBloom(Context);

    if (bGenerateBloom || FXAA::DebugDraw || SSAO::DebugDraw || !g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
            Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        else
            Context.TransitionResource(g_PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        Context.TransitionResource(g_aBloomUAV1[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Set constants
        Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(),
            (float)BloomStrength);

        // Separate out SDR result from its perceived luminance
        if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
            Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
        else
        {
            Context.SetDynamicDescriptor(1, 0, g_PostEffectsBuffer.GetUAV());
            Context.SetDynamicDescriptor(2, 2, g_SceneColorBuffer.GetSRV());
        }
        Context.SetDynamicDescriptor(1, 1, g_LumaBuffer.GetUAV());

        // Read in original SDR value and blurred bloom buffer
        Context.SetDynamicDescriptor(2, 0, bGenerateBloom ? g_aBloomUAV1[1].GetSRV() : TextureManager::GetBlackTex2D().GetSRV());

        Context.SetPipelineState(FXAA::DebugDraw ? DebugLuminanceLdrCS : ApplyBloomCS);
        Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

        Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
}

void PostEffects::CopyBackPostBuffer( ComputeContext& Context )
{
    ScopedTimer _prof(L"Copy Post back to Scene", Context);
    Context.SetRootSignature(PostEffectsRS);
    Context.SetPipelineState(CopyBackPostBufferCS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_PostEffectsBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
    Context.SetDynamicDescriptor(2, 0, g_PostEffectsBuffer.GetSRV());
    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
}

void PostEffects::Render( void )
{
    ComputeContext& Context = ComputeContext::Begin(L"Post Effects");

    Context.SetRootSignature(PostEffectsRS);

    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    if (EnableHDR && !SSAO::DebugDraw && !(DepthOfField::Enable && DepthOfField::DebugMode >= 3))
        ProcessHDR(Context);
    else
        ProcessLDR(Context);

    bool bGeneratedLumaBuffer = EnableHDR || FXAA::DebugDraw || BloomEnable;
    if (FXAA::Enable)
        FXAA::Render(Context, bGeneratedLumaBuffer);

    // In the case where we've been doing post processing in a separate buffer, we need to copy it
    // back to the original buffer.  It is possible to skip this step if the next shader knows to
    // do the manual format decode from UINT, but there are several code paths that need to be
    // changed, and some of them rely on texture filtering, which won't work with UINT.  Since this
    // is only to support legacy hardware and a single buffer copy isn't that big of a deal, this
    // is the most economical solution.
    if (!g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        CopyBackPostBuffer(Context);

    if (DrawHistogram)
    {
        ScopedTimer _prof(L"Draw Debug Histogram", Context);
        Context.SetRootSignature(PostEffectsRS);
        Context.SetPipelineState(DrawHistogramCS);
        Context.InsertUAVBarrier(g_SceneColorBuffer);
        Context.TransitionResource(g_Histogram, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
        D3D12_CPU_DESCRIPTOR_HANDLE SRVs[2] = { g_Histogram.GetSRV(), g_Exposure.GetSRV() };
        Context.SetDynamicDescriptors(2, 0, 2, SRVs);
        Context.Dispatch(1, 32);
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    Context.Finish();
}
